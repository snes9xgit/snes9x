# snes9x → Mesen2 save state converter

`tools/s9x_to_mss.py` converts a Snes9x save state (`.000`–`.010`, `.oops`) into
a Mesen2 save state (`.mss`) for the same ROM. It's a one-shot Python script,
no dependencies beyond the standard library.

## Usage

```
python3 tools/s9x_to_mss.py <input.NNN> <template.mss> <output.mss>
```

- **`<input.NNN>`** — your Snes9x save state. Plain or gzipped (snes9x writes
  both at different times; the tool detects).
- **`<template.mss>`** — *any* existing Mesen2 `.mss` for the **same ROM**.
  Required as scaffolding because Mesen2 stores ~1300 keys per state and the
  tool only overlays the ones it knows how to translate; everything else
  inherits from the template. Easiest way to get one: in Mesen2, load the ROM
  and save state once.
- **`<output.mss>`** — destination path. The tool always inserts
  `_from_snes9x` before the `.mss` extension, so e.g. passing
  `Firemen_11.mss` writes `Firemen_11_from_snes9x.mss`. If the name already
  contains `_from_snes9x`, it's left alone (idempotent).

To load the converted state in Mesen2, either:

1. Rename it to match a Mesen2 slot name (`<ROM>_1.mss` through `_11.mss`) and
   place it in `<Documents>/Mesen2/SaveStates/`, then pick the slot, or
2. Use **File → Load State From File** and point at the `_from_snes9x.mss`.

## What it transfers

| Snes9x source            | Mesen2 destination                          | Notes                                                |
| ------------------------ | ------------------------------------------- | ---------------------------------------------------- |
| `RAM` (128 KB)           | `memoryManager.workRam`                     | Byte-identical                                       |
| `VRA` (64 KB)            | `ppu.vram`                                  | Byte-identical                                       |
| `PPU.OAMData` (544 B)    | `ppu.oamRam`                                | Byte-identical                                       |
| `PPU.CGDATA` (512 B)     | `ppu.cgram`                                 | uint16 byte-swap (snes9x writes BE, Mesen2 stores LE)|
| `SRA` (SRAM, if present) | `cart.saveRam`                              | Byte-identical                                       |
| `SND[:64K]` (APU RAM)    | `spc.ram`                                   | Byte-identical                                       |
| `SND[$F4..$F7]` (apuram) | `spc.outputReg[0..3]`                       | SPC→CPU port direction (game reads at $2140–$2143)   |
| `SND` tail 4 bytes       | `spc.cpuRegs[0..3]`                         | CPU→SPC port direction (SPC reads at $F4–$F7)        |
| 65C816 registers (`REG`) | `cpu.a/x/y/sp/d/pc/dbr/k/ps/emulationMode`  | BE→LE for 16-bit; emulation bit extracted from P     |
| `CPU.V_Counter`          | `ppu.scanline`, `internalRegisters.vCounter`| Sets the rendering position                          |
| `CPU.Cycles`             | `memoryManager.hClock`                      | Per-scanline cycle counter                           |
| `CPU.WhichEvent`         | `memoryManager.nextEvent`                   | snes9x HC\_\* enum mapped to Mesen2's SnesEventType  |
| `CPU.NextEvent`          | `memoryManager.nextEventClock`              |                                                      |
| `CPU.NMIPending`         | `cpu.needNmi`                               |                                                      |
| `CPU.WaitingForInterrupt`| `cpu.waiOver` (inverted)                    | `WAI` completion flag                                |
| `CPU.IRQPending`         | `cpu.irqSource`                             |                                                      |
| `FillRAM[$2100]` INIDISP | `ppu.forcedBlank`, `ppu.screenBrightness`   |                                                      |
| `FillRAM[$2101]` OBSEL   | `ppu.oamMode`, `oamBaseAddress`, `oamAddressOffset` |                                              |
| `FillRAM[$2105]` BGMODE  | `ppu.bgMode`, `mode1Bg3Priority`, `layers[N].largeTiles` |                                          |
| `FillRAM[$2106]` MOSAIC  | `ppu.mosaicSize`, `mosaicEnabled`           |                                                      |
| `FillRAM[$210B/$210C]`   | `ppu.layers[N].chrAddress`                  | unused                                               |
| `FillRAM[$2115]` VMAIN   | `ppu.vramIncrementValue/Remapping/AddrIncrementOnSecondReg` |                                      |
| `FillRAM[$2121]` CGADD   | `ppu.cgramAddress`                          |                                                      |
| `FillRAM[$212C/$212D]`   | `ppu.mainScreenLayers`, `subScreenLayers`   |                                                      |
| `FillRAM[$2130/$2131]`   | `ppu.colorMath*`                            |                                                      |
| `FillRAM[$2133]` SETINI  | `ppu.hiResMode`, `screenInterlace`, `objInterlace`, `overscanMode` |                               |
| `PPU.BG[0..3]` fields    | `ppu.layers[N].tilemapAddress/hscroll/vscroll/doubleWidth/doubleHeight` |                          |
| `PPU` Mode7 matrix       | `ppu.mode7.matrix[0..3]`, `centerX/Y`, `hscroll/vscroll` |                                         |
| `DMA[N]` (snes9x SDMA)   | `dmaController.channel[N].*` (16 fields)    | HDMA active channels only; inactive marked finished  |
| `FillRAM[$420C]` HDMAEN  | `dmaController.hdmaChannels`                |                                                      |
| `FillRAM[$4200]` NMITIMEN| `internalRegisters.enableNmi/enableVerticalIrq/enableHorizontalIrq/enableAutoJoypadRead` |             |
| `FillRAM[$4207..$420A]`  | `internalRegisters.horizontalTimer`, `verticalTimer` |                                             |
| `FillRAM[$420D]` MEMSEL  | `internalRegisters.enableFastRom`           |                                                      |
| SMP registers from `SND` | `spc.pc/a/x/y/sp/ps/dspReg`                 | SPC700 CPU state                                     |
| SMP timer state          | `spc.timer{N}.target/stage0/stage2/output/enabled/timersEnabled` |                                  |
| DSP regs (128 B from SND)| `spc.dsp.regs`                              | Byte-identical                                       |
| DSP external_regs (128 B)| `spc.dsp.externalRegs`                      | Byte-identical, located by SND-tail offset           |
| DSP voice state (active) | `spc.dsp.voices[N].*` (11 fields per voice) | Skipped if <4 voices active — see Limitations        |

## What it can't transfer

These are emulator-internal pieces of state that **aren't preserved in
snes9x's snapshot**, so the conversion leaves the template's values intact:

- Mesen2's `memoryManager.masterClock` (rounded up to a scanline boundary)
- SPC700 `$F0` TEST register state (`spc.timersEnabled/timersDisabled/writeEnabled`)
- Mesen2's DSP "voice transition" sub-state — key-on delay state machine,
  sample buffer wrap position, mid-fade envelope interpolation
- Cycle-precise mid-instruction state (snes9x stores `opcode_number` /
  `opcode_cycle` but Mesen2 doesn't expose equivalent fields)
- Echo history (the audio echo buffer)
- ALU multiply/divide state mid-operation
- Joypad auto-read state machine mid-cycle

In addition: **WRAM init pattern** differs between the two emulators
(snes9x uses one pattern, Mesen2 uses another), so the ~75 KB of WRAM the
game never wrote to during this playthrough will differ between the
converted state and a native Mesen2 save. This is harmless — by definition
the game doesn't read those bytes.

## Limitations

Some snes9x saves don't translate cleanly. The big ones:

- **Mid-music-transition saves.** If only one or two DSP voices are
  active (fading out, ramping in, or between phrases) the SPC's music
  driver is in a delicate handshake with the CPU that depends on
  voice-internal latches that snes9x doesn't save. The converter detects
  this (≥ 4 active voices = stable, < 4 = transitional) and **skips
  voice-state transfer** for transitional saves to avoid corrupting the
  voice further. The resulting state may have brief audio artifacts on
  load but the game advances.

- **Special-chip ROMs** (SuperFX, SA-1, DSP-1, SPC7110, C4, OBC1,
  ST010, S-RTC, BS-X, MSU-1). Their coprocessor state is left at the
  template's values. The tool prints no warning; it simply won't sync the
  coprocessor.

- **Mid-DMA / mid-HDMA cycle.** If snes9x's `CPU.InDMA` or `CPU.InHDMA` is
  set at save time, the CPU is paused mid-transfer. We don't propagate
  this; first frame after load may show one-frame DMA artifacts.

- **Joypad reads.** If the game reads `$4218–$421F` (auto-joypad) in the
  first frame after load, the values may be stale until the next
  auto-read cycle completes (Mesen2 starts a fresh auto-read on
  scanline 225).

## Tips for clean conversions

1. **Save during stable gameplay**, not during pause menus, fades,
   transitions, or boss intros. The fewer state-machine handshakes in
   flight, the better.
2. **Music actively playing** with multiple voices is ideal — the voice
   state transfer kicks in and audio resumes cleanly.
3. **Same ROM, same version.** The Mesen2 template must come from the
   same ROM file (header CRC matches). A template from a different ROM
   region or revision won't load.
4. **If a converted state hangs**, the most useful diagnostic in Mesen2
   is **Debug → SNES Debugger** to see the CPU's stuck PC. If it's at a
   `LDA $XX / BEQ -2` pattern, the loop is waiting on a flag that the
   NMI handler or the SPC was supposed to write — usually points at
   missing audio state.

## Implementation notes

- The Mesen2 `.mss` format starts with a `MSS` magic, then a small header
  (versions, console type), an embedded PNG-like compressed frame buffer
  for the slot preview, the ROM filename, then a zlib-compressed
  serialized state. The serialized state is a sequence of
  `NUL-terminated key`,`uint32 LE size`,`size bytes value` records. See
  `Core/Utilities/Serializer.cpp` in the Mesen2 source for details.
- Snes9x's `.NNN` format is documented in `docs/snapshots.txt`. All
  multi-byte fields are written big-endian; the converter byte-swaps
  where Mesen2 expects little-endian.
- Source references in the script's comments cite the relevant
  snes9x file:line and Mesen2 file:line for every non-obvious mapping
  (port direction, HDMA `Repeat` inversion, SPC timer field cascade,
  external_regs offset).
