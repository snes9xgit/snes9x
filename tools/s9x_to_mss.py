#!/usr/bin/env python3
"""
Convert a Snes9x save state (.### / .oops) into a Mesen2 save state (.mss).

Usage:
    s9x_to_mss.py <snes9x.009> <mesen2_template.mss> <output.mss>

The output filename automatically gets "_from_snes9x" inserted before the
.mss extension (so "Firemen_11.mss" -> "Firemen_11_from_snes9x.mss"), making
converted states easy to tell apart from native Mesen2 states. Pass a name
that already contains "_from_snes9x" to skip the rewrite.

The Mesen2 template must be an existing .mss for the same ROM. It supplies
all the emulator-internal state (DMA, HDMA, cycle counters, SPC DSP voices,
PPU latches) that has no clean counterpart in the Snes9x snapshot. The script
overlays the bulk memory regions and CPU registers from the .009 onto that
template, then repacks the .mss.

Limitations:
  - Audio may glitch for a fraction of a second after load (DSP voices keep
    template envelopes; APU RAM is correct but voice state isn't).
  - Mid-frame HDMA / DMA in the source state won't transfer; the first frame
    after load may show one-frame artifacts.
  - Only the standard SNES memory model is converted. Special chips
    (SuperFX, SA-1, DSP-1, etc.) keep template state and won't sync.
"""
from __future__ import annotations
import os, struct, sys, zlib, gzip
from typing import Optional


# ---- Snes9x .009 parsing -----------------------------------------------------

def _upgrade_section_to_v12(version: int, tag: str, raw: bytes) -> bytes:
    """Normalize a snes9x snapshot section from any version (v6..v12) to a v12-
    equivalent byte layout. The rest of the converter assumes v12 offsets.

    For each upgrade step we add zero bytes (or sensible defaults) in the
    positions where v12 has fields that the older format lacked, and drop the
    bytes that older formats had but v12 doesn't.
    """
    if version >= 12:
        return raw

    if tag == "CPU":
        # v6 → v7: drop CPU_IRQActive(1 OBSOLETE), WaitAddress(4),
        # WaitCounter(4), PBPCAtOpcodeStart(4 DELETED) and append 5 byte-flags
        # NMIPending..IRQExternal. Net size: 56 → 48.
        # Also remap WhichEvent values (v6 used 1..12; v7+ uses 1..6; the
        # mapping below mirrors the conversion in snapshot.cpp at the
        # "Converting old snapshot version" branch).
        if version < 7:
            if len(raw) != 56:
                raise ValueError(f"v6 CPU section: expected 56 bytes, got {len(raw)}")
            we_remap = {0: 0, 1: 1, 2: 2, 3: 2, 4: 3, 5: 3, 6: 4, 7: 4,
                        8: 5, 9: 5, 10: 6, 11: 6, 12: 1}
            tmp = bytearray()
            tmp += raw[0:16]      # Cycles, PrevCycles, V_Counter, Flags
            # SKIP raw[16] (CPU_IRQActive)
            tmp += raw[17:44]     # IRQPending..WaitingForInterrupt
            # SKIP raw[44:56] (WaitAddress, WaitCounter, PBPCAtOpcodeStart)
            tmp += b"\x00" * 5    # NMIPending, IRQLine, IRQTransition,
                                  # IRQLastState, IRQExternal (defaults to 0)
            # remap WhichEvent at tmp offset 37
            tmp[37] = we_remap.get(tmp[37], 1)
            raw = bytes(tmp)
        return raw

    if tag == "PPU":
        # pre-v11 → v11: insert CGSavedByte (1 byte default 0) at offset 63
        # (right before CGDATA[256]), and append VRAMReadBuffer (2 bytes
        # default 0) at end of section. Net size: 2649 → 2652.
        if version < 11:
            if len(raw) != 2649:
                raise ValueError(f"pre-v11 PPU section: expected 2649 bytes, got {len(raw)}")
            tmp = bytearray()
            tmp += raw[0:63]      # VMA..CGADD
            tmp += b"\x00"        # CGSavedByte default 0
            tmp += raw[63:2649]   # CGDATA..end
            tmp += b"\x00\x00"    # VRAMReadBuffer default 0
            raw = bytes(tmp)
        return raw

    if tag == "TIM":
        # pre-v7 → v7: append IRQTriggerCycles (int32 = 14 — snes9x default
        # from memmap.cpp:Timings.IRQTriggerCycles=14) and APUAllowTimeOverflow
        # (uint8 = 0). Net size: 61 → 66.
        if version < 7:
            if len(raw) != 61:
                raise ValueError(f"v6 TIM section: expected 61 bytes, got {len(raw)}")
            tmp = bytearray(raw)
            tmp += b"\x00\x00\x00\x0e"   # IRQTriggerCycles = 14 (big-endian)
            tmp += b"\x00"                # APUAllowTimeOverflow = 0
            raw = bytes(tmp)
        # pre-v11 → v11: append NextIRQTimer (int32 = 0x0FFFFFFF — "no IRQ
        # scheduled" sentinel). Net size: 66 → 70.
        if version < 11:
            if len(raw) != 66:
                raise ValueError(f"pre-v11 TIM section: expected 66 bytes, got {len(raw)}")
            tmp = bytearray(raw)
            tmp += b"\x0f\xff\xff\xff"   # NextIRQTimer = 0x0FFFFFFF
            raw = bytes(tmp)
        return raw

    return raw


class S9xState:
    def __init__(self, path: str) -> None:
        with open(path, "rb") as f:
            raw = f.read()
        if raw[:2] == b"\x1f\x8b":
            raw = gzip.decompress(raw)
        if raw[:9] != b"#!s9xsnp:":
            raise ValueError("not a snes9x save state (missing magic)")
        eol = raw.index(b"\n")
        self.version = int(raw[9:eol])
        # Original snapshot version preserved across the v12 normalization
        # step below — used by callers that need to know whether the source
        # used Blargg-era APU (pre-v8) vs the modern bAPU layout (v8+).
        self.original_version = self.version
        self.sections: dict[str, bytes] = {}
        i = eol + 1
        while i < len(raw):
            # Look for a section header: 3-char tag + ':' + 6 digit size + ':'
            if i + 11 > len(raw):
                break
            tag = raw[i : i + 3]
            if raw[i + 3] != ord(":") or raw[i + 10] != ord(":"):
                # Should not happen with well-formed snes9x snapshots
                break
            size = int(raw[i + 4 : i + 10])
            data_off = i + 11
            self.sections[tag.decode("ascii")] = raw[data_off : data_off + size]
            i = data_off + size

        # Normalize all version-dependent sections to v12-equivalent layout so
        # the rest of the converter can use fixed offsets. snes9x SNAPSHOT_VERSION
        # has gone through v6→v7 (CPU: drop OBSOLETE/DELETED, add 5 IRQ flags;
        # TIM: add IRQTriggerCycles+APUAllowTimeOverflow), and v10→v11 (PPU: add
        # CGSavedByte+VRAMReadBuffer; TIM: add NextIRQTimer). Earlier versions
        # crash Mesen when loaded without this normalization because PPU offset
        # math shifts and CPU fields are read from the wrong bytes.
        for tag in ("CPU", "PPU", "TIM"):
            if tag in self.sections:
                self.sections[tag] = _upgrade_section_to_v12(
                    self.version, tag, self.sections[tag])
        self.version = max(self.version, 12)

    # --- PPU struct field accessors, by offset within the PPU section ---
    def ppu_u8(self, off: int) -> int:
        return self.sections["PPU"][off]

    def ppu_u16_be(self, off: int) -> int:
        return struct.unpack(">H", self.sections["PPU"][off : off + 2])[0]

    def ppu_s16_be(self, off: int) -> int:
        return struct.unpack(">h", self.sections["PPU"][off : off + 2])[0]

    def ppu_fields(self) -> dict[str, int]:
        """Decode the snes9x PPU struct fields that map to Mesen2 keys."""
        f: dict[str, int] = {}
        # VMA group at offset 0
        f["VMA.High"]             = self.ppu_u8(0)
        f["VMA.Increment"]        = self.ppu_u8(1)
        f["VMA.Address"]          = self.ppu_u16_be(2)
        # BG group starts at 14, 11 bytes each
        for n in range(4):
            base = 14 + n * 11
            f[f"BG{n}.SCBase"]    = self.ppu_u16_be(base + 0)
            f[f"BG{n}.HOffset"]   = self.ppu_u16_be(base + 2)
            f[f"BG{n}.VOffset"]   = self.ppu_u16_be(base + 4)
            f[f"BG{n}.BGSize"]    = self.ppu_u8     (base + 6)
            f[f"BG{n}.NameBase"]  = self.ppu_u16_be(base + 7)
            f[f"BG{n}.SCSize"]    = self.ppu_u16_be(base + 9)
        # BGMode group at offset 58
        f["BGMode"]               = self.ppu_u8(58)
        f["BG3Priority"]          = self.ppu_u8(59)
        f["CGADD"]                = self.ppu_u8(62)
        # OBJ post-fields at offset 1976 (after CGDATA + OBJ[128])
        post = _PPU_CGDATA_OFF + 512 + _PPU_OBJ  # = 1984
        f["OBJThroughMain"]       = self.ppu_u8     (post + 0)
        f["OBJThroughSub"]        = self.ppu_u8     (post + 1)
        f["OBJNameBase"]          = self.ppu_u16_be(post + 3)
        f["OBJNameSelect"]        = self.ppu_u16_be(post + 5)
        f["OBJSizeSelect"]        = self.ppu_u8     (post + 7)
        f["OAMAddr"]              = self.ppu_u16_be(post + 8)
        # End-of-section fields (offsets relative to PPU section start = 2625..)
        f["ForcedBlanking"]       = self.ppu_u8(2625)
        f["FixedColourRed"]       = self.ppu_u8(2626)
        f["FixedColourGreen"]     = self.ppu_u8(2627)
        f["FixedColourBlue"]      = self.ppu_u8(2628)
        f["Brightness"]           = self.ppu_u8(2629)
        f["ScreenHeight"]         = self.ppu_u16_be(2630)
        f["HDMA"]                 = self.ppu_u8(2634)
        f["VRAMReadBuffer"]       = self.ppu_u16_be(2638)
        # Mode7 matrix at offset 2562, then CentreX/Y, M7HOFS/VOFS
        f["MatrixA"]              = self.ppu_s16_be(2562)
        f["MatrixB"]              = self.ppu_s16_be(2564)
        f["MatrixC"]              = self.ppu_s16_be(2566)
        f["MatrixD"]              = self.ppu_s16_be(2568)
        f["CentreX"]              = self.ppu_s16_be(2570)
        f["CentreY"]              = self.ppu_s16_be(2572)
        f["M7HOFS"]               = self.ppu_s16_be(2574)
        f["M7VOFS"]               = self.ppu_s16_be(2576)
        return f

    # --- DMA section: 8 channels × 19 bytes each, big-endian ints ---
    # Per-channel layout from SnapDMA[] in snapshot.cpp:
    #   ReverseTransfer(1) HDMAIndirectAddressing(1) UnusedBit43x0(1)
    #   AAddressFixed(1) AAddressDecrement(1) TransferMode(1) BAddress(1)
    #   AAddress(2) ABank(1) DMACount_Or_HDMAIndirectAddress(2)
    #   IndirectBank(1) Address(2) Repeat(1) LineCount(1) UnknownByte(1)
    #   DoTransfer(1)
    def dma_channels(self) -> list[dict[str, int]]:
        dma = self.sections["DMA"]
        out: list[dict[str, int]] = []
        for ch in range(8):
            base = ch * 19
            out.append({
                "ReverseTransfer":          dma[base + 0],
                "HDMAIndirectAddressing":   dma[base + 1],
                "UnusedBit43x0":            dma[base + 2],
                "AAddressFixed":            dma[base + 3],
                "AAddressDecrement":        dma[base + 4],
                "TransferMode":             dma[base + 5],
                "BAddress":                 dma[base + 6],
                "AAddress":  struct.unpack(">H", dma[base + 7 : base + 9])[0],
                "ABank":                    dma[base + 9],
                "DMACount":  struct.unpack(">H", dma[base + 10 : base + 12])[0],
                "IndirectBank":             dma[base + 12],
                "Address":   struct.unpack(">H", dma[base + 13 : base + 15])[0],
                "Repeat":                   dma[base + 15],
                "LineCount":                dma[base + 16],
                "UnknownByte":              dma[base + 17],
                "DoTransfer":               dma[base + 18],
            })
        return out

    # --- FillRAM I/O register shadow (indexed by CPU address) ---
    def fil(self, addr: int) -> int:
        return self.sections["FIL"][addr]

    # --- SMP (SPC700 CPU) state, from snes9x SND section after apuram ---
    # Layout from apu/bapu/smp/smp_state.cpp save_state(): all fields written
    # as little-endian int32 (set_le32). Starts at SND offset 65536 (right
    # after the 64KB apuram block).
    def smp_state(self) -> dict[str, int]:
        snd = self.sections["SND"]
        base = 65536
        le32 = lambda off: struct.unpack("<i", snd[base + off : base + off + 4])[0]
        # Field order matches the INT32() calls in SMP::save_state.
        names = [
            "clock", "opcode_number", "opcode_cycle",
            "regs.pc", "regs.sp", "regs.a", "regs.x", "regs.y",
            "p.n", "p.v", "p.p", "p.b", "p.h", "p.i", "p.z", "p.c",
            "status.iplrom_enable", "status.dsp_addr",
            "status.ram00f8", "status.ram00f9",
            "timer0.enable", "timer0.target",
            "timer0.stage1", "timer0.stage2", "timer0.stage3",
            "timer1.enable", "timer1.target",
            "timer1.stage1", "timer1.stage2", "timer1.stage3",
            "timer2.enable", "timer2.target",
            "timer2.stage1", "timer2.stage2", "timer2.stage3",
            "rd", "wr", "dp", "sp", "ya", "bit",
        ]
        return {n: le32(i * 4) for i, n in enumerate(names)}

    # --- CPU section: SCPUState fields (48 bytes, big-endian ints) ---
    # Layout from SnapCPU[] for SNAPSHOT_VERSION >= 7 (we saw v12 in our file).
    def cpu_state(self) -> dict[str, int]:
        c = self.sections["CPU"]
        if len(c) < 48:
            raise ValueError("CPU section too small")
        u32 = lambda off: struct.unpack(">I", c[off : off + 4])[0]
        i32 = lambda off: struct.unpack(">i", c[off : off + 4])[0]
        return {
            "Cycles":              i32(0),
            "PrevCycles":          i32(4),
            "V_Counter":           i32(8),
            "Flags":               u32(12),
            "IRQPending":          i32(16),
            "MemSpeed":            i32(20),
            "MemSpeedx2":          i32(24),
            "FastROMSpeed":        i32(28),
            "InDMA":               c[32],
            "InHDMA":              c[33],
            "InDMAorHDMA":         c[34],
            "InWRAMDMAorHDMA":     c[35],
            "HDMARanInDMA":        c[36],
            "WhichEvent":          c[37],
            "NextEvent":           i32(38),
            "WaitingForInterrupt": c[42],
            "NMIPending":          c[43],
            "IRQLine":             c[44],
            "IRQTransition":       c[45],
            "IRQLastState":        c[46],
            "IRQExternal":         c[47],
        }

    # --- REG section: 65C816 registers (16 bytes, big-endian ints) ---
    def cpu_registers(self) -> dict[str, int]:
        reg = self.sections["REG"]
        if len(reg) < 16:
            raise ValueError("REG section too small")
        # FreezeStruct writes ints big-endian, in declared order:
        # PB(1) DB(1) P.W(2) A.W(2) D.W(2) S.W(2) X.W(2) Y.W(2) PCw(2)
        PB = reg[0]
        DB = reg[1]
        P  = struct.unpack(">H", reg[2:4])[0]
        A  = struct.unpack(">H", reg[4:6])[0]
        D  = struct.unpack(">H", reg[6:8])[0]
        S  = struct.unpack(">H", reg[8:10])[0]
        X  = struct.unpack(">H", reg[10:12])[0]
        Y  = struct.unpack(">H", reg[12:14])[0]
        PC = struct.unpack(">H", reg[14:16])[0]
        return dict(PB=PB, DB=DB, P=P, A=A, D=D, S=S, X=X, Y=Y, PC=PC)

    # --- PPU section: CGDATA + OAMData ---
    # We don't decode the whole 2652-byte PPU struct — only the two arrays we
    # need. Their offsets depend on the saved field order, so we locate them
    # by scanning to a recognizable bookend rather than hard-coding offsets.
    # However, since the layout is fixed by snapshot.cpp, we can compute the
    # offsets from the table we know.

    def ppu_cgdata_be(self) -> bytes:
        # CGDATA is an array of 256 uint16 stored big-endian → 512 bytes.
        # OAMData (544 bytes uint8) follows after a bunch of OBJ[N] fields.
        # We find CGDATA by computing the byte offset from the freeze layout.
        return self._ppu_slice(_PPU_CGDATA_OFF, 512)

    def ppu_oam(self) -> bytes:
        return self._ppu_slice(_PPU_OAMDATA_OFF, 544)

    def _ppu_slice(self, off: int, length: int) -> bytes:
        ppu = self.sections["PPU"]
        if off + length > len(ppu):
            raise ValueError(f"PPU section slice {off}+{length} > {len(ppu)}")
        return ppu[off : off + length]


# Byte offsets within the PPU section, computed from SnapPPU[] in
# snes9x/snapshot.cpp using actual field sizes from the SPPU struct in ppu.h.
# All multi-byte fields are written big-endian by FreezeStruct().
# Sizes verified to sum to 2652 (matches a v12 PPU section).
#
# VMA: High(1) Increment(1) Address(2) Mask1(2) FullGraphicCount(2) Shift(2) =10
# WRAM(4)                                                                    = 4
# BG[0..3] × (SCBase(2) HOffset(2) VOffset(2) BGSize(1) NameBase(2) SCSize(2))= 4×11=44
# BGMode(1) BG3Priority(1) CGFLIP(1) CGFLIPRead(1) CGADD(1) CGSavedByte(1)   = 6
# CGDATA[256] uint16                                                          = 512
_PPU_VMA = 10
_PPU_WRAM = 4
_PPU_BG = 4 * (2 + 2 + 2 + 1 + 2 + 2)   # 44  (NameBase and SCSize are uint16)
_PPU_PRE_CG = 1 + 1 + 1 + 1 + 1 + 1      # 6
_PPU_CGDATA_OFF = _PPU_VMA + _PPU_WRAM + _PPU_BG + _PPU_PRE_CG  # = 64

# After CGDATA(512), comes OBJ[0..127]:
#   HPos(2) VPos(2) HFlip(1) VFlip(1) Name(2) Priority(1) Palette(1) Size(1) = 11
# = 128 × 11 = 1408
_PPU_OBJ = 128 * (2 + 2 + 1 + 1 + 2 + 1 + 1 + 1)  # 1408

# Then these fields before OAMData (OBJNameSelect is uint16 in SPPU, not uint8):
#   OBJThroughMain(1) OBJThroughSub(1) OBJAddition(1) OBJNameBase(2)
#   OBJNameSelect(2) OBJSizeSelect(1) OAMAddr(2) SavedOAMAddr(2)
#   OAMPriorityRotation(1) OAMFlip(1) OAMReadFlip(1) OAMTileAddress(2)
#   OAMWriteRegister(2)
_PPU_PRE_OAM = 1 + 1 + 1 + 2 + 2 + 1 + 2 + 2 + 1 + 1 + 1 + 2 + 2  # 19
_PPU_OAMDATA_OFF = _PPU_CGDATA_OFF + 512 + _PPU_OBJ + _PPU_PRE_OAM  # = 2003


# ---- Mesen2 .mss parsing / writing ------------------------------------------

class MssFile:
    def __init__(self, path: str) -> None:
        with open(path, "rb") as f:
            data = f.read()
        if data[:3] != b"MSS":
            raise ValueError("not a Mesen2 .mss (missing magic)")
        self.emu_version  = struct.unpack("<I", data[3:7])[0]
        self.fmt_version  = struct.unpack("<I", data[7:11])[0]
        self.console_type = struct.unpack("<I", data[11:15])[0]
        fb_size, w, h, scale, comp_size = struct.unpack("<IIIII", data[15:35])
        self.fb_size, self.w, self.h, self.scale, self.fb_comp_size = (
            fb_size, w, h, scale, comp_size
        )
        self.fb_compressed = data[35 : 35 + comp_size]
        off = 35 + comp_size
        rom_name_len = struct.unpack("<I", data[off : off + 4])[0]
        off += 4
        self.rom_name = data[off : off + rom_name_len].decode("utf-8")
        off += rom_name_len

        # State envelope: u8 isCompressed, u32 decompSize, u32 compSize, zlib.
        is_compressed = data[off]
        off += 1
        if is_compressed != 1:
            raise ValueError("state envelope is uncompressed; not handled here")
        decomp_size = struct.unpack("<I", data[off : off + 4])[0]
        off += 4
        comp_size2 = struct.unpack("<I", data[off : off + 4])[0]
        off += 4
        compressed = data[off : off + comp_size2]
        state = zlib.decompress(compressed)
        if len(state) != decomp_size:
            raise ValueError("state size mismatch")
        self.state = bytearray(state)

        # Parse out the (key, value) entries.
        self.entries: list[tuple[str, int, int]] = []  # (key, offset, size)
        i = 0
        while i < len(self.state):
            end = self.state.find(b"\x00", i)
            if end < 0:
                break
            key = self.state[i:end].decode("ascii")
            i = end + 1
            if i + 4 > len(self.state):
                break
            sz = struct.unpack("<I", self.state[i : i + 4])[0]
            i += 4
            self.entries.append((key, i, sz))
            i += sz
        self._index = {k: (off, sz) for k, off, sz in self.entries}

    def patch(self, key: str, value: bytes) -> None:
        if key not in self._index:
            raise KeyError(f"key not present in template: {key!r}")
        off, sz = self._index[key]
        if len(value) != sz:
            raise ValueError(
                f"size mismatch for {key!r}: template has {sz}, got {len(value)}"
            )
        self.state[off : off + sz] = value

    def write(self, path: str) -> None:
        # Re-compress the state with the same envelope (zlib level 1 matches
        # Mesen2's default — see Serializer::SaveTo).
        comp = zlib.compress(bytes(self.state), level=1)
        with open(path, "wb") as f:
            f.write(b"MSS")
            f.write(struct.pack("<I", self.emu_version))
            f.write(struct.pack("<I", self.fmt_version))
            f.write(struct.pack("<I", self.console_type))
            f.write(struct.pack("<IIIII",
                self.fb_size, self.w, self.h, self.scale, self.fb_comp_size))
            f.write(self.fb_compressed)
            rom_name_bytes = self.rom_name.encode("utf-8")
            f.write(struct.pack("<I", len(rom_name_bytes)))
            f.write(rom_name_bytes)
            f.write(b"\x01")  # isCompressed
            f.write(struct.pack("<I", len(self.state)))
            f.write(struct.pack("<I", len(comp)))
            f.write(comp)


# ---- Conversion --------------------------------------------------------------

def byteswap16(buf: bytes) -> bytes:
    if len(buf) & 1:
        raise ValueError("byteswap16 requires even-length buffer")
    arr = bytearray(buf)
    for i in range(0, len(arr), 2):
        arr[i], arr[i + 1] = arr[i + 1], arr[i]
    return bytes(arr)


def convert(s9x_path: str, template_path: str, out_path: str) -> None:
    s9x = S9xState(s9x_path)
    mss = MssFile(template_path)

    # Bulk memory regions (byte-identical in both emulators)
    mss.patch("memoryManager.workRam", s9x.sections["RAM"])
    mss.patch("ppu.vram",              s9x.sections["VRA"])
    mss.patch("ppu.oamRam",            s9x.ppu_oam())

    # CGRAM: snes9x serializes 256 uint16 big-endian; Mesen2 stores little-endian
    mss.patch("ppu.cgram", byteswap16(s9x.ppu_cgdata_be()))

    # SPC700 RAM (first 64 KB of the SND block — see apu.cpp / smp_state.cpp)
    if "SND" in s9x.sections:
        snd = s9x.sections["SND"]
        mss.patch("spc.ram", snd[:65536])

        # snes9x's bAPU stores the two SPC<->CPU directions in different places
        # (see apu/bapu/snes/snes.hpp and apu/bapu/smp/memory.cpp):
        #   SPC -> CPU  =  apuram[$F4..$F7]  (SPC writes to its own RAM ports)
        #   CPU -> SPC  =  SNES::CPU::registers[4]  (written at end of SND)
        #
        # Mesen2 keeps these as:
        #   spc.outputReg[0..3]  =  what CPU reads at $2140-$2143  =  SPC->CPU
        #   spc.cpuRegs[0..3]    =  what SPC reads at $F4-$F7       =  CPU->SPC
        #
        # Locate cpu.registers as the four bytes immediately before the zero
        # padding at the tail of SND.
        for i in range(4):
            _try_patch(mss, f"spc.outputReg[{i}]", _u8(snd[0xF4 + i]))

        # The tail-scan for CPU→SPC port registers only matches the modern
        # bAPU SND layout (v8+). Pre-v8 (Blargg) snapshots store these
        # registers somewhere else entirely, so we leave Mesen's template
        # values in place — wrong sound for a frame is much better than the
        # crash the alternative produces.
        if s9x.original_version >= 8:
            tail = snd[65536:]
            last_nz = max((i for i, b in enumerate(tail) if b != 0), default=0)
            if last_nz >= 200:                       # safely past SMP state
                cpu_regs = tail[last_nz - 3 : last_nz + 1]
                for i in range(4):
                    _try_patch(mss, f"spc.cpuRegs[{i}]",    _u8(cpu_regs[i]))
                    _try_patch(mss, f"spc.newCpuRegs[{i}]", _u8(cpu_regs[i]))

    # Cartridge SRAM (present only if the ROM has battery-backed RAM)
    if "SRA" in s9x.sections:
        sra = s9x.sections["SRA"]
        tmpl_off, tmpl_sz = mss._index["cart.saveRam"]
        if tmpl_sz > 0 and len(sra) >= tmpl_sz:
            mss.patch("cart.saveRam", sra[:tmpl_sz])

    # 65C816 registers
    r = s9x.cpu_registers()
    mss.patch("cpu.a",   struct.pack("<H", r["A"]))
    mss.patch("cpu.x",   struct.pack("<H", r["X"]))
    mss.patch("cpu.y",   struct.pack("<H", r["Y"]))
    mss.patch("cpu.sp",  struct.pack("<H", r["S"]))
    mss.patch("cpu.d",   struct.pack("<H", r["D"]))
    mss.patch("cpu.pc",  struct.pack("<H", r["PC"]))
    mss.patch("cpu.dbr", bytes([r["DB"]]))
    mss.patch("cpu.k",   bytes([r["PB"]]))
    mss.patch("cpu.ps",  bytes([r["P"] & 0xFF]))
    mss.patch("cpu.emulationMode", bytes([1 if (r["P"] & 0x100) else 0]))

    _patch_ppu(s9x, mss)
    _patch_dma(s9x, mss)
    _patch_internal_registers(s9x, mss)
    _patch_cpu_timing(s9x, mss)
    _patch_smp(s9x, mss)

    mss.write(out_path)


def _u8(v: int) -> bytes:  return bytes([v & 0xFF])
def _u16(v: int) -> bytes: return struct.pack("<H", v & 0xFFFF)
def _u32(v: int) -> bytes: return struct.pack("<I", v & 0xFFFFFFFF)
def _s16(v: int) -> bytes: return struct.pack("<h", v)


def _try_patch(mss: "MssFile", key: str, value: bytes) -> None:
    """Patch only if key exists and size matches — Mesen2 versions vary slightly."""
    entry = mss._index.get(key)
    if entry is None:
        return
    _, sz = entry
    if len(value) != sz:
        # Some fields are stored as larger ints (e.g. uint32) in Mesen2.
        # Pad with zeros if the source is smaller.
        if len(value) < sz:
            value = value + b"\x00" * (sz - len(value))
        else:
            value = value[:sz]
    mss.patch(key, value)


def _patch_ppu(s9x: "S9xState", mss: "MssFile") -> None:
    """Translate snes9x PPU struct + FillRAM register shadow into Mesen2 keys."""
    p = s9x.ppu_fields()

    inidisp = s9x.fil(0x2100)        # $2100: bit7 forced blank, bits0-3 brightness
    obsel   = s9x.fil(0x2101)        # $2101: OAM addressing / sprite size
    bgmode  = s9x.fil(0x2105)        # $2105: BG mode / tile size
    mosaic  = s9x.fil(0x2106)        # $2106: mosaic
    vmain   = s9x.fil(0x2115)        # $2115: VRAM increment mode
    tm      = s9x.fil(0x212C)        # $212C: main screen layers
    ts      = s9x.fil(0x212D)        # $212D: sub screen layers
    tmw     = s9x.fil(0x212E)
    tsw     = s9x.fil(0x212F)
    cgwsel  = s9x.fil(0x2130)
    cgadsub = s9x.fil(0x2131)
    setini  = s9x.fil(0x2133)

    _try_patch(mss, "ppu.forcedBlank",     _u8((inidisp >> 7) & 1))
    _try_patch(mss, "ppu.screenBrightness", _u8(inidisp & 0x0F))
    _try_patch(mss, "ppu.bgMode",           _u8(p["BGMode"] & 7))
    _try_patch(mss, "ppu.mode1Bg3Priority", _u8(p["BG3Priority"] & 1))
    _try_patch(mss, "ppu.mainScreenLayers", _u8(tm))
    _try_patch(mss, "ppu.subScreenLayers",  _u8(ts))
    _try_patch(mss, "ppu.cgramAddress",     _u8(p["CGADD"]))
    _try_patch(mss, "ppu.vramAddress",      _u16(p["VMA.Address"]))
    _try_patch(mss, "ppu.vramIncrementValue", _u8([1,32,128,128][vmain & 3]))
    _try_patch(mss, "ppu.vramAddressRemapping", _u8((vmain >> 2) & 3))
    _try_patch(mss, "ppu.vramAddrIncrementOnSecondReg", _u8((vmain >> 7) & 1))
    _try_patch(mss, "ppu.mosaicSize",       _u8(((mosaic >> 4) & 0x0F) + 1))
    _try_patch(mss, "ppu.mosaicEnabled",    _u8(mosaic & 0x0F))
    _try_patch(mss, "ppu.oamMode",          _u8((obsel >> 5) & 7))
    _try_patch(mss, "ppu.oamBaseAddress",   _u16((obsel & 7) << 13))
    _try_patch(mss, "ppu.oamAddressOffset", _u16((((obsel >> 3) & 3) + 1) << 12))
    _try_patch(mss, "ppu.oamRamAddress",    _u16(p["OAMAddr"] & 0x3FF))
    _try_patch(mss, "ppu.enableOamPriority", _u8((p["OAMAddr"] >> 15) & 1))
    _try_patch(mss, "ppu.hiResMode",        _u8((setini >> 3) & 1))
    _try_patch(mss, "ppu.screenInterlace",  _u8(setini & 1))
    _try_patch(mss, "ppu.objInterlace",     _u8((setini >> 1) & 1))
    _try_patch(mss, "ppu.overscanMode",     _u8((setini >> 2) & 1))
    _try_patch(mss, "ppu.directColorMode",  _u8(cgwsel & 1))
    _try_patch(mss, "ppu.colorMathAddSubscreen", _u8((cgwsel >> 1) & 1))
    _try_patch(mss, "ppu.colorMathClipMode",    _u32((cgwsel >> 6) & 3))
    _try_patch(mss, "ppu.colorMathPreventMode", _u32((cgwsel >> 4) & 3))
    _try_patch(mss, "ppu.colorMathEnabled",     _u8(cgadsub & 0x3F))
    _try_patch(mss, "ppu.colorMathSubtractMode", _u8((cgadsub >> 7) & 1))
    _try_patch(mss, "ppu.colorMathHalveResult", _u8((cgadsub >> 6) & 1))
    # Fixed color: snes9x tracks per-channel; reassemble as 15-bit BGR
    fixed = (p["FixedColourBlue"] << 10) | (p["FixedColourGreen"] << 5) | p["FixedColourRed"]
    _try_patch(mss, "ppu.fixedColor", _u16(fixed))

    # BG layer addressing
    for n in range(4):
        # SCBase is stored as the byte address (snes9x shifts it from the register
        # value by <<9). Mesen2 stores it in the same byte-address form.
        _try_patch(mss, f"ppu.layers[{n}].tilemapAddress", _u16(p[f"BG{n}.SCBase"]))
        _try_patch(mss, f"ppu.layers[{n}].chrAddress",     _u16(p[f"BG{n}.NameBase"]))
        _try_patch(mss, f"ppu.layers[{n}].hscroll",        _u16(p[f"BG{n}.HOffset"]))
        _try_patch(mss, f"ppu.layers[{n}].vscroll",        _u16(p[f"BG{n}.VOffset"]))
        scsize = p[f"BG{n}.SCSize"]
        _try_patch(mss, f"ppu.layers[{n}].doubleWidth",  _u8(scsize & 1))
        _try_patch(mss, f"ppu.layers[{n}].doubleHeight", _u8((scsize >> 1) & 1))
        # $2105 high nibble: bit (4+n) selects 16x16 tiles for BG[n]
        _try_patch(mss, f"ppu.layers[{n}].largeTiles", _u8((bgmode >> (4 + n)) & 1))

    # Mode7
    _try_patch(mss, "ppu.mode7.matrix[0]", _s16(p["MatrixA"]))
    _try_patch(mss, "ppu.mode7.matrix[1]", _s16(p["MatrixB"]))
    _try_patch(mss, "ppu.mode7.matrix[2]", _s16(p["MatrixC"]))
    _try_patch(mss, "ppu.mode7.matrix[3]", _s16(p["MatrixD"]))
    _try_patch(mss, "ppu.mode7.centerX",   _s16(p["CentreX"]))
    _try_patch(mss, "ppu.mode7.centerY",   _s16(p["CentreY"]))
    _try_patch(mss, "ppu.mode7.hscroll",   _s16(p["M7HOFS"]))
    _try_patch(mss, "ppu.mode7.vscroll",   _s16(p["M7VOFS"]))


def _patch_dma(s9x: "S9xState", mss: "MssFile") -> None:
    """Transfer all 8 DMA channel configs + HDMAEN."""
    hdma_enable = s9x.fil(0x420C)
    for ch, c in enumerate(s9x.dma_channels()):
        # The 8-bit DMAP control byte = bits packed:
        #   b0-2 TransferMode, b3 FixedTransfer, b4 Decrement, b6 HdmaIndirect,
        #   b7 InvertDirection (= ReverseTransfer in snes9x)
        _try_patch(mss, f"dmaController.channel[{ch}].invertDirection",
                   _u8(1 if c["ReverseTransfer"] else 0))
        _try_patch(mss, f"dmaController.channel[{ch}].hdmaIndirectAddressing",
                   _u8(1 if c["HDMAIndirectAddressing"] else 0))
        _try_patch(mss, f"dmaController.channel[{ch}].unusedControlFlag",
                   _u8(1 if c["UnusedBit43x0"] else 0))
        _try_patch(mss, f"dmaController.channel[{ch}].fixedTransfer",
                   _u8(1 if c["AAddressFixed"] else 0))
        _try_patch(mss, f"dmaController.channel[{ch}].decrement",
                   _u8(1 if c["AAddressDecrement"] else 0))
        _try_patch(mss, f"dmaController.channel[{ch}].transferMode", _u8(c["TransferMode"]))
        _try_patch(mss, f"dmaController.channel[{ch}].destAddress",  _u8(c["BAddress"]))
        _try_patch(mss, f"dmaController.channel[{ch}].srcAddress",   _u16(c["AAddress"]))
        _try_patch(mss, f"dmaController.channel[{ch}].srcBank",      _u8(c["ABank"]))
        _try_patch(mss, f"dmaController.channel[{ch}].transferSize", _u16(c["DMACount"]))
        _try_patch(mss, f"dmaController.channel[{ch}].hdmaBank",     _u8(c["IndirectBank"]))
        _try_patch(mss, f"dmaController.channel[{ch}].hdmaTableAddress", _u16(c["Address"]))
        is_active = bool(hdma_enable & (1 << ch))
        if is_active:
            # Mesen2 stores the raw HDMA-table line counter byte (bit 7 = repeat
            # mode). snes9x splits it: LineCount holds the count, Repeat is the
            # INVERTED high bit (dma.cpp:1172  Repeat = !(line & 0x80)). So to
            # reconstruct the original byte, set bit 7 when Repeat == 0.
            line_counter = (c["LineCount"] & 0x7F) | (0 if c["Repeat"] else 0x80)
            _try_patch(mss, f"dmaController.channel[{ch}].hdmaLineCounterAndRepeat",
                       _u8(line_counter))
            _try_patch(mss, f"dmaController.channel[{ch}].doTransfer",
                       _u8(0 if c["Repeat"] else 1))
            _try_patch(mss, f"dmaController.channel[{ch}].hdmaFinished", _u8(0))
        else:
            # Inactive HDMA channels: Mesen2 marks them finished and zeroes the
            # line counter at boot. Mirroring that avoids spurious "phantom"
            # transfers from stale snes9x state for channels the game isn't
            # using this frame.
            _try_patch(mss, f"dmaController.channel[{ch}].hdmaLineCounterAndRepeat", _u8(0))
            _try_patch(mss, f"dmaController.channel[{ch}].doTransfer", _u8(0))
            _try_patch(mss, f"dmaController.channel[{ch}].hdmaFinished", _u8(1))

        _try_patch(mss, f"dmaController.channel[{ch}].unusedRegister", _u8(c["UnknownByte"]))

    # Global HDMA-enable shadow ($420C)
    _try_patch(mss, "dmaController.hdmaChannels", _u8(s9x.fil(0x420C)))


def _patch_cpu_timing(s9x: "S9xState", mss: "MssFile") -> None:
    """Align Mesen2's scanline/NMI/WAI/IRQ-pending state with the .009.

    The snes9x snapshot stores V_Counter (the current scanline) and a handful
    of CPU interrupt flags. We can't reconstruct Mesen2's full masterClock
    (snes9x doesn't track total master cycles), but we can at least put the
    scanline-driven NMI/IRQ event in the right place so the game's main loop
    sees an interrupt at the right time.
    """
    c = s9x.cpu_state()

    # Scanline alignment. Mesen2's ppu.scanline tells the PPU which line it's
    # currently on; internalRegisters.vCounter mirrors $213D's latched value.
    v = c["V_Counter"] & 0xFFFF
    _try_patch(mss, "ppu.scanline",              _u16(v))
    _try_patch(mss, "internalRegisters.vCounter", _u16(v))

    # CPU interrupt-pending bits
    _try_patch(mss, "cpu.needNmi", _u8(1 if c["NMIPending"] else 0))
    # waiOver=1 means a previously-executing WAI has completed (normal state).
    # If snes9x flags WaitingForInterrupt, the CPU is stuck in WAI → waiOver=0.
    _try_patch(mss, "cpu.waiOver", _u8(0 if c["WaitingForInterrupt"] else 1))
    # IRQ pending — snes9x's IRQPending is an int counter; non-zero means
    # an IRQ is queued. Map to Mesen2's irqSource bitfield (PpuIrq=1).
    _try_patch(mss, "cpu.irqSource",     _u8(1 if c["IRQPending"] else 0))
    _try_patch(mss, "cpu.prevIrqSource", _u8(1 if c["IRQPending"] else 0))

    # $4210 RDNMI bit 7 mirrors the NMI pending flag — snes9x writes it back
    # into FillRAM during VBlank entry.
    rdnmi = s9x.fil(0x4210)
    _try_patch(mss, "internalRegisters.nmiFlag", _u8((rdnmi >> 7) & 1))
    rdirq = s9x.fil(0x4211)
    _try_patch(mss, "internalRegisters.irqFlag", _u8((rdirq >> 7) & 1))

    # Per-scanline timing position. Mesen2's hClock is master cycles within
    # the current scanline; it advances together with masterClock until the
    # next scheduled event fires at hClock == nextEventClock.
    _try_patch(mss, "memoryManager.hClock", _u16(c["Cycles"] & 0xFFFF))

    # Event scheduler. snes9x's HC_* event enum (snes9x.h) doesn't map 1:1 to
    # Mesen2's SnesEventType, but the only events that meaningfully reschedule
    # the loader's first few cycles are HDMA init/start, DRAM refresh, and
    # end-of-scanline. Anything else we don't recognize we coerce to the
    # nearest equivalent (DramRefresh = 538, a neutral mid-scanline event).
    HC_HBLANK_START   = 1
    HC_HDMA_START     = 2
    HC_HCOUNTER_MAX   = 3
    HC_HDMA_INIT      = 4
    HC_RENDER         = 5
    HC_WRAM_REFRESH   = 6
    mesen_event = {
        HC_HDMA_INIT:    0,  # HdmaInit
        HC_WRAM_REFRESH: 1,  # DramRefresh
        HC_HDMA_START:   2,  # HdmaStart
        HC_HCOUNTER_MAX: 3,  # EndOfScanline
        HC_HBLANK_START: 1,  # no exact match; closest is DramRefresh
        HC_RENDER:       1,
    }.get(c["WhichEvent"], 1)
    _try_patch(mss, "memoryManager.nextEvent",      _u8(mesen_event))
    _try_patch(mss, "memoryManager.nextEventClock", _u16(c["NextEvent"] & 0xFFFF))

    # masterClock must remain monotonically increasing across this state load,
    # but its absolute value isn't directly observable by game code — what
    # matters is that masterClock & 7 aligns with hClock & 7 (Mesen2 uses
    # masterClock & 0x07 to compute DRAM refresh position). Round the template
    # masterClock up to a (1364 master-cycle = one-scanline) boundary so the
    # low-order alignment matches our new hClock value.
    off, sz = mss._index.get("memoryManager.masterClock", (None, None))
    if off is not None and sz == 8:
        old_mc = int.from_bytes(bytes(mss.state[off : off + sz]), "little")
        # Snap to scanline boundary then add hClock.
        new_mc = ((old_mc // 1364) + 1) * 1364 + (c["Cycles"] & 0x07)
        mss.state[off : off + sz] = new_mc.to_bytes(8, "little")


def _patch_smp_blargg(s9x: "S9xState", mss: "MssFile") -> None:
    """Extract SPC700 register state from a pre-v8 (Blargg-APU) SND section.

    Layout (matches apu/apu.cpp::S9xAPULoadBlarggState):
        [0..65535]    apuram (already patched by the caller)
        [65536..65551] REGS  ($00F0-$00FF SPC hardware regs)
        [65552..65567] REGS_IN  ($2140-$2143 port inputs as seen by SPC)
        [65568..65569] PC (uint16, big-endian per the file framing — but the
                          Blargg state_copier writes ints native-endian, so
                          we try the conservative little-endian interpretation
                          that matches the SMP::regs.pc field layout on x86
                          builds where these saves were almost always made)
        [65570]       A
        [65571]       X
        [65572]       Y
        [65573]       PSW
        [65574]       SP

    Skipped: DSP register block, timer internals, spc_time/dsp_time clocks.
    Mesen keeps its template DSP/timer state; the SPC will run from the
    saved PC with the saved registers, which is enough to advance the
    APU-handshake state machine games use to gate scene transitions.
    """
    snd = s9x.sections["SND"]
    if len(snd) < 65575:
        return

    regs    = snd[65536:65552]
    regs_in = snd[65552:65568]
    # The Blargg copier wrote ints as raw little-endian on every host where
    # snes9x produced .009 files we've encountered.
    pc  = struct.unpack("<H", snd[65568:65570])[0]
    a   = snd[65570]
    x   = snd[65571]
    y   = snd[65572]
    psw = snd[65573]
    sp  = snd[65574]

    _try_patch(mss, "spc.pc", _u16(pc))
    _try_patch(mss, "spc.a",  _u8(a))
    _try_patch(mss, "spc.x",  _u8(x))
    _try_patch(mss, "spc.y",  _u8(y))
    _try_patch(mss, "spc.sp", _u8(sp))
    _try_patch(mss, "spc.ps", _u8(psw))

    # REGS[2] = dsp_addr (the $00F2 latch)
    _try_patch(mss, "spc.dspReg", _u8(regs[2]))

    # REGS[1] = $00F1 control byte: bits 0-2 = timer enables, bit 7 = IPL ROM
    f1 = regs[1]
    _try_patch(mss, "spc.timersEnabled",  _u8(0 if (f1 & 0x07) == 0 else 1))
    _try_patch(mss, "spc.romEnabled",     _u8(1 if (f1 & 0x80) else 0))
    _try_patch(mss, "spc.timersDisabled", _u8(0))

    # CPU→SPC port-register values that the SPC reads at $00F4-$00F7.
    # Blargg stores these in REGS_IN[4..7] (see the memcpy at the tail of
    # S9xAPULoadBlarggState).
    for i in range(4):
        _try_patch(mss, f"spc.cpuRegs[{i}]",    _u8(regs_in[4 + i]))
        _try_patch(mss, f"spc.newCpuRegs[{i}]", _u8(regs_in[4 + i]))


def _patch_smp(s9x: "S9xState", mss: "MssFile") -> None:
    """Transfer SPC700 CPU registers and timer state.

    Without this, the SPC700 runs the template's code (different PC, A, X,
    Y, SP, PSW) against the .009's apuram. The music driver never advances,
    so $07c7 (the music tick counter) stays at its saved value and the game's
    main loop spins forever waiting for it to decrement.

    Pre-v8 (Blargg-APU) snapshots get a narrower extraction path: SMP regs,
    REGS ($F0-$FF), and port inputs are at fixed Blargg offsets right after
    apuram; DSP state and timer internals stay from the template. Enough to
    keep the SPC running its actual code so the APU-handshake spinloop
    (e.g. Firemen's `$80:89FB` waiting for `$7E:$07C7` to drain) advances.
    """
    if "SND" not in s9x.sections:
        return
    if s9x.original_version < 8:
        _patch_smp_blargg(s9x, mss)
        return
    smp = s9x.smp_state()

    # SPC700 registers
    _try_patch(mss, "spc.pc", _u16(smp["regs.pc"] & 0xFFFF))
    _try_patch(mss, "spc.a",  _u8(smp["regs.a"] & 0xFF))
    _try_patch(mss, "spc.x",  _u8(smp["regs.x"] & 0xFF))
    _try_patch(mss, "spc.y",  _u8(smp["regs.y"] & 0xFF))
    _try_patch(mss, "spc.sp", _u8(smp["regs.sp"] & 0xFF))

    # PSW byte: reassemble from the 8 separate flag fields snes9x stores.
    psw = ((smp["p.n"] & 1) << 7) | ((smp["p.v"] & 1) << 6) \
        | ((smp["p.p"] & 1) << 5) | ((smp["p.b"] & 1) << 4) \
        | ((smp["p.h"] & 1) << 3) | ((smp["p.i"] & 1) << 2) \
        | ((smp["p.z"] & 1) << 1) | (smp["p.c"] & 1)
    _try_patch(mss, "spc.ps", _u8(psw))

    # DSP register-address latch (the high byte of $00F2)
    _try_patch(mss, "spc.dspReg", _u8(smp["status.dsp_addr"] & 0xFF))

    # SPC timers. Mesen2 stores three counters per timer:
    #   stage0 = pre-divider state (4-bit counter that wraps to ~zero at target)
    #   stage1 = mid-stage tick accumulator
    #   stage2 = output latch
    # snes9x names them stage1/stage2/stage3 — same idea, off-by-one.
    for n in range(3):
        _try_patch(mss, f"spc.timer{n}.enabled", _u8(1 if smp[f"timer{n}.enable"] else 0))
        # Mesen2's per-timer timersEnabled mirrors the global "SPC is running"
        # state, not the per-timer enable bit. While the SPC is alive (which it
        # always is for a save state captured during a running game), all three
        # timers carry timersEnabled = 1.
        _try_patch(mss, f"spc.timer{n}.timersEnabled", _u8(1))
        _try_patch(mss, f"spc.timer{n}.target", _u8(smp[f"timer{n}.target"] & 0xFF))
        # Field-name mapping from snes9x (apu/bapu/smp/timing.cpp) to Mesen2
        # (SpcTimer.h). Both emulators have a three-stage cascade but with
        # different names — verified by tracing Timer::tick() vs Run/ClockTimer.
        #   snes9x.stage1_ticks  pre-divider counter (0 ≤ x < cycle_frequency)
        #   snes9x.stage2_ticks  count-to-target accumulator
        #   snes9x.stage3_ticks  the 4-bit output latch read at $FD/$FE/$FF
        #   Mesen2._stage0       same role as snes9x.stage1_ticks
        #   Mesen2._stage1       internal 1-bit toggle, no snes9x equivalent
        #   Mesen2._stage2       same role as snes9x.stage2_ticks
        #   Mesen2._output       same role as snes9x.stage3_ticks
        _try_patch(mss, f"spc.timer{n}.stage0", _u8(smp[f"timer{n}.stage1"] & 0xFF))
        _try_patch(mss, f"spc.timer{n}.stage2", _u8(smp[f"timer{n}.stage2"] & 0xFF))
        _try_patch(mss, f"spc.timer{n}.output", _u8(smp[f"timer{n}.stage3"] & 0x0F))

    # SPC bus configuration (from snes9x apuram[$F1] CONTROL register).
    f1 = s9x.sections["SND"][0xF1]
    _try_patch(mss, "spc.timersEnabled",   _u8(0 if (f1 & 0x07) == 0 else 1))
    _try_patch(mss, "spc.romEnabled",      _u8(1 if (f1 & 0x80) else 0))
    _try_patch(mss, "spc.timersDisabled",  _u8(0))

    # DSP register block (128 bytes at SND offset 65700, right after the SMP
    # state). These are the audio chip registers - $00..$7F. Critical ones:
    #   $4C KON   - which voices are keyed on
    #   $5C KOF   - which voices are keyed off
    #   $4D EON   - echo enable
    #   $6C FLG   - reset/mute/echo-disable
    # If KON/KOF are template values, the wrong voices play and the music
    # driver waits forever for "voices silent" while voices keep playing.
    snd = s9x.sections["SND"]
    dsp_regs = snd[65700 : 65700 + 128]
    if len(dsp_regs) == 128:
        # Mesen2 stores all 128 DSP regs in spc.dsp.regs (byte-identical).
        _try_patch(mss, "spc.dsp.regs", dsp_regs)

    # DSP voice state. Each of the 8 voices stores its currently-decoded BRR
    # sample window, envelope phase, and BRR-table position. If we transfer
    # apuram + DSP regs but leave voice state at the template's, the DSP keeps
    # decoding from the wrong BRR block - manifests as audio looping on a
    # short fragment of the template's music.
    #
    # snes9x voice layout (SPC_DSP::copy_state, 38 bytes per voice):
    #   brr_buf[12 int16] (24) interp_pos(2) brr_addr(2) env(2)
    #   hidden_env(2) buf_pos(1) brr_offset(1) kon_delay(1)
    #   env_mode(1) t_envx_out(1) extra(1)
    voice_base = 65700 + 128                  # right after DSP regs
    # First, count active voices (env > 0). If most voices are silent, the
    # SPC is between music phrases / mid-transition — transferring its
    # voice state can wedge the music driver. Only transfer when at least
    # half the voices are actively producing sound, which means the SPC is
    # mid-song at a stable moment.
    active = 0
    for v in range(8):
        vo = voice_base + v * 38
        if vo + 38 > len(snd):
            break
        env = int.from_bytes(snd[vo + 28 : vo + 30], "little")
        if env > 0:
            active += 1

    if active >= 4:
        for v in range(8):
            vo = voice_base + v * 38
            if vo + 38 > len(snd):
                break
            env = int.from_bytes(snd[vo + 28 : vo + 30], "little")
            env_mode = snd[vo + 35]
            envx_out = snd[vo + 36]
            if env == 0 and env_mode == 0 and envx_out == 0:
                continue

            sample_buffer = bytes(snd[vo : vo + 24])
            interp_pos = int.from_bytes(snd[vo + 24 : vo + 26], "little")
            brr_addr   = int.from_bytes(snd[vo + 26 : vo + 28], "little")
            hidden_env = int.from_bytes(snd[vo + 30 : vo + 32], "little", signed=True)
            buf_pos    = snd[vo + 32]
            brr_offset = snd[vo + 33]
            kon_delay  = snd[vo + 34]

            _try_patch(mss, f"spc.dsp.voices[{v}].sampleBuffer",      sample_buffer)
            _try_patch(mss, f"spc.dsp.voices[{v}].interpolationPos",  _u32(interp_pos))
            _try_patch(mss, f"spc.dsp.voices[{v}].brrAddress",        _u16(brr_addr))
            _try_patch(mss, f"spc.dsp.voices[{v}].envVolume",         _u32(env))
            _try_patch(mss, f"spc.dsp.voices[{v}].prevCalculatedEnv",
                       _u32(hidden_env & 0xFFFFFFFF))
            _try_patch(mss, f"spc.dsp.voices[{v}].bufferPos",         _u8(buf_pos))
            _try_patch(mss, f"spc.dsp.voices[{v}].brrOffset",         _u16(brr_offset))
            _try_patch(mss, f"spc.dsp.voices[{v}].keyOnDelay",        _u8(kon_delay))
            _try_patch(mss, f"spc.dsp.voices[{v}].envMode",           _u32(env_mode))
            _try_patch(mss, f"spc.dsp.voices[{v}].envOut",            _u8(envx_out))
            _try_patch(mss, f"spc.dsp.voices[{v}].voiceBit",          _u8(1 << v))

    # external_regs: 128 bytes near the end of DSP state, just before the
    # final 16-byte block (reference_time, remainder, dsp.clock, cpu.regs).
    # Locate it relative to apuram[$F4..$F7] = SPC output (which we know
    # always equals the last 4 non-zero bytes before the zero padding).
    tail = snd[65536:]
    last_nz = max((i for i, b in enumerate(tail) if b != 0), default=0)
    if last_nz > 128 + 17:
        # Tail layout (from apu/bapu/dsp/SPC_DSP.cpp::copy_state):
        #   ...
        #   external_regs          (128 bytes)
        #   final copier.extra()   (  1 byte)
        #   reference_time         (  4 bytes)
        #   remainder              (  4 bytes)
        #   dsp.clock              (  4 bytes)
        #   cpu.registers          (  4 bytes, ends at last_nz)
        #   zero padding           ...
        ext_end = last_nz - 16                # exclusive: one past last external_regs byte
        ext_start = ext_end - 128
        if ext_start >= 164:                  # safely past SMP state
            _try_patch(mss, "spc.dsp.externalRegs",
                       bytes(tail[ext_start:ext_end]))


def _patch_internal_registers(s9x: "S9xState", mss: "MssFile") -> None:
    """Patch NMITIMEN/IRQ-timer/auto-joypad/MEMSEL from FillRAM shadows."""
    nmitimen = s9x.fil(0x4200)
    htime = s9x.fil(0x4207) | ((s9x.fil(0x4208) & 1) << 8)
    vtime = s9x.fil(0x4209) | ((s9x.fil(0x420A) & 1) << 8)

    _try_patch(mss, "internalRegisters.enableNmi",           _u8((nmitimen >> 7) & 1))
    _try_patch(mss, "internalRegisters.enableVerticalIrq",   _u8((nmitimen >> 5) & 1))
    _try_patch(mss, "internalRegisters.enableHorizontalIrq", _u8((nmitimen >> 4) & 1))
    _try_patch(mss, "internalRegisters.enableAutoJoypadRead", _u8(nmitimen & 1))
    _try_patch(mss, "internalRegisters.horizontalTimer", _u16(htime))
    _try_patch(mss, "internalRegisters.verticalTimer",   _u16(vtime))
    _try_patch(mss, "internalRegisters.enableFastRom",   _u8(s9x.fil(0x420D) & 1))
    _try_patch(mss, "internalRegisters.ioPortOutput",    _u8(s9x.fil(0x4201)))


def tag_output_path(path: str) -> str:
    root, ext = os.path.splitext(path)
    if "_from_snes9x" in os.path.basename(root):
        return path
    return root + "_from_snes9x" + (ext or ".mss")


def main(argv: list[str]) -> int:
    if len(argv) != 4:
        print(__doc__, file=sys.stderr)
        return 2
    out = tag_output_path(argv[3])
    convert(argv[1], argv[2], out)
    print(f"wrote {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
