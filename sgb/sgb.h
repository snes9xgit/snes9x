/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SGB_H_
#define _SGB_H_

#include <cstdint>
#include <cstddef>

namespace SGB {

struct FrameBuffer
{
	const uint8_t *pixels;  // 160x144 color-indexed (4 shades / SGB palette idx)
	uint32_t       width;
	uint32_t       height;
	uint32_t       pitch;
};

enum class RunMode : uint8_t
{
	DMG  = 0,  // Plain Game Boy, monochrome
	SGB  = 1,  // SGB1 — 4.295 MHz GB CPU, SGB features enabled
	SGB2 = 2   // SGB2 — exact 4.194 MHz GB CPU
};

struct CpuState;
struct Memory;
struct Ppu;
struct Apu;
struct Timer;
struct Joypad;
struct Cart;
struct SgbState;

class Emulator
{
public:
	Emulator();
	~Emulator();

	Emulator(const Emulator &) = delete;
	Emulator &operator=(const Emulator &) = delete;

	bool Init();
	void Deinit();
	void Reset();
	// Like Reset() but ALSO clears the BIOS-handshake packet cache and
	// counters. Use for SNES-side reset / new ROM load — Reset() alone
	// preserves the cache, which is correct for in-game $6003 0→1 GB
	// resets but wrong when the user invokes File→Reset.
	void ColdReset();

	// User-facing soft reset: warm Reset() that skips the staged boot-ROM
	// overlay (game restarts at $0100 with run-mode handoff registers) and
	// preserves the handshake cache, handoff state and the ICD2 $6003
	// control latch, so a live SGB BIOS session keeps running without
	// replaying the splash. Returns false (no reset performed) in BIOS
	// mode while the boot handshake hasn't completed yet — the caller
	// should fall back to a full power-cycle.
	bool SoftReset();

	bool LoadROM(const uint8_t *data, size_t size, const char *path = nullptr);
	void UnloadROM();
	bool HasROM() const;

	bool HasBattery() const;
	bool SaveBatteryToPath(const char *path) const;
	bool LoadBatteryFromPath(const char *path);
	bool TakeSramDirty();

	// Raw cart battery-RAM accessor so a host (e.g. the libretro core) can
	// expose it as a savable memory region. Returns nullptr / 0 when the
	// loaded cart has no SRAM.
	uint8_t       *GetSRAMData();
	size_t         GetSRAMSize() const;

	// Raw cart ROM accessor for hashing / external introspection. Returns
	// nullptr when no ROM is loaded.
	const uint8_t *GetROMData() const;
	size_t         GetROMSize() const;

	// Per-pixel source-layer map for the current GB frame (160x144, parallel
	// to the framebuffer): 0 = BG, 1 = window, 2 = sprite/OBJ. Used by the host
	// frame-blend to blend only selected layers. nullptr when no ROM is loaded.
	const uint8_t *GBLayerMask() const;

	bool IsCgb() const;

	// Debug viewer accessors — side-effect-free reads of live PPU state.
	const uint8_t  *DebugVRAM() const;       // 0x4000: bank0 [0..0x1FFF], bank1 [0x2000..]
	const uint8_t  *DebugOAM() const;        // 0xA0
	const uint8_t  *DebugCgbBgPal() const;   // 64
	const uint8_t  *DebugCgbObjPal() const;  // 64
	const uint16_t *DebugSgbActivePalettes() const;  // 16 (4 palettes x 4 colors)
	const uint8_t  *DebugSgbAttrMap() const; // SGB_TILES (360)
	void            DebugGetPpuRegs(uint8_t out[12]) const;

	// Live-frame layer visibility (0=BG, 1=window, 2=OBJ). Display-only.
	void SetLayerEnabled(int layer, bool enabled);
	bool GetLayerEnabled(int layer) const;

	// When Settings.GBFrameBlendAuto is on, set Settings.GBFrameBlend/Layer from
	// the per-title table for the loaded cart (off for unlisted titles). Called
	// at LoadROM and re-runnable from the UI when the Auto toggle changes.
	void ApplyAutoBlend();

	// Read a byte from the GB-side address space using the rcheevos
	// GameBoy / GameBoy Color memory map (0x0000-0xFFFF native + the
	// 0x10000-0x33FFF extended bank window). Side-effect-free; does not
	// touch I/O registers or PPU bus state. Returns 0 for unmapped or
	// out-of-range addresses.
	uint8_t        PeekRAByte(uint32_t addr) const;

	// Cheat-engine memory access. PeekRAByte's address layout, except
	// 0xA000-0xBFFF is cart SRAM bank 0 fixed (ignores the MBC RAM-enable
	// gate) and 0x01000000|offset addresses the cart ROM image by
	// physical byte offset (Game Genie ROM patches). Writes outside
	// writable regions are dropped.
	uint8_t        CheatRead(uint32_t addr) const;
	void           CheatWrite(uint32_t addr, uint8_t value);

	// Live region pointers + byte counts for the cheat-search engine;
	// nullptr / *size=0 when unavailable.
	uint8_t       *CheatWRAM(uint32_t *size) const;   // 0x2000 DMG / 0x8000 CGB
	uint8_t       *CheatSRAM(uint32_t *size) const;
	uint8_t       *CheatHRAM(uint32_t *size) const;   // 0x7F
	const uint8_t *CheatROM(uint32_t *size) const;

	// Install the 256-byte DMG/SGB GB-side boot ROM. Takes effect on the
	// next Reset. Call with nullptr/size=0 to clear. Only loaded in
	// authentic BIOS mode — BIOS-less continues to start at 0x0100.
	bool LoadBootROM(const uint8_t *data, size_t size);

	// Populate the 5-packet boot-ROM handshake from the current cart's
	// header and queue the first packet as if the GB boot ROM had sent
	// it. The SGB BIOS waits on these to transition past its splash;
	// the GB's dumped boot ROM typically doesn't generate them, so we
	// synthesize them ourselves. Requires a cart already loaded.
	void PrimeBIOSHandshake();

	void SetRunMode(RunMode m);
	RunMode GetRunMode() const;

	// Advance the GB core by one whole video frame (ends at VBlank entry).
	void RunFrame();

	// Advance by N T-cycles. Used when snes9x drives the master clock directly.
	void RunCycles(int32_t tcycles);

	const FrameBuffer &GetFrameBuffer() const;

	// Consume pending audio samples. Returns the count written
	// (stereo frames — each frame stores L, R into out[2*i], out[2*i+1]).
	int32_t DrainAudio(int16_t *out, int32_t max_samples);

	// Discard all pending audio samples without playing them. Consumer-side
	// (moves tail to head), so it is as thread-safe as DrainAudio.
	void    ClearAudio();
	int32_t GetAudioSampleRate() const;

	// Diagnostics: GB clock_hz and cycles_per_sample as currently
	// configured in the APU. Used by the SGB OSD to verify audio
	// rate math.
	int32_t GetAudioClockHz() const;
	int32_t GetAudioCyclesPerSample() const;
	int32_t GetAudioCpsRemainderStep() const;

	// Count of int16 values (NOT stereo frames) currently ready in the
	// ring buffer. Matches the convention S9xGetSampleCount uses —
	// useful when driving snes9x's host audio pull path.
	int32_t GetAudioSamplesAvailable() const;

	// Configure the APU's downsample target in Hz. Takes effect on the
	// next Reset(), or immediately for the next sample emitted.
	void    SetAudioRate(int32_t rate_hz);

	// Mapped from a SNES joypad bitmask — see sgb.cpp for the mapping table.
	void SetJoypad(uint16_t snes_pad_mask);

	// Called by the SNES $4016 write path; feeds the SGB command-packet sniffer.
	void OnJoyserWrite(uint8_t value);

	// Snapshot format — versioned block appended to snes9x's .state.
	size_t StateSize() const;
	void   StateSave(uint8_t *buffer) const;
	bool   StateLoad(const uint8_t *buffer, size_t size);

	// 1.0 = real hardware speed. <1 underclocks (ARM targets), >1 overclocks.
	void  SetClockMultiplier(float m);
	float GetClockMultiplier() const;

	// Internal — routes a decoded SGB command into the palette/attribute
	// state. Exposed because the packet callback trampoline needs it.
	void  OnSgbCommandInternal(uint8_t cmd, const uint8_t *data, uint32_t len);

	// Composite the SGB border + GB screen into a 256 × 224 BGR555
	// buffer. `pitch_pixels` is the stride in uint16_t units (use 256
	// for a tightly-packed buffer, or the SNES GFX.Screen's PPL).
	void  BlitScreen(uint16_t *dest, uint32_t pitch_pixels);
	void  BlitScreenGB(uint16_t *dest, uint32_t pitch_pixels);

	// BIOS-mode overlay path — see S9xSGBOverlayBiosBorder for the
	// full description. Renders the captured custom border on top of
	// the SNES-rendered output, sparing the central 20×18 GB area.
	void  OverlayBiosBorder(uint16_t *dest, uint32_t pitch_pixels);

	// Write a one-line status snapshot — PC, SP, A, halt/stop flag,
	// total T-cycles, illegal-op count.
	void  GetStatus(char *buf, size_t cap) const;

	// ICD2 (BIOS-mode cart chip) register access — 0x6000-0x7FFF on the
	// SNES side. See S9xSGBGetICD2 / S9xSGBSetICD2 below for the C facade.
	// Non-const because reading $7000-$700F drains a single-slot FIFO
	// (clears the packet-ready flag exposed at $6002).
	uint8_t GetICD2(uint16_t addr);
	void    SetICD2(uint8_t value, uint16_t addr);

	// True once the BIOS has released the GB CPU (bit 7 of $6003). Used
	// by cpuexec to gate per-frame GB core stepping.
	bool    IsGBReleased() const;

	// True while synth handshake packets are still pending drain.
	bool    IsHandshakePending() const;

	bool    IsBootHandoffCaptured() const;
	uint32_t GetPacketCount() const;

	// GB PPU scanline event hooks for SGB row/bank counter advance.
	void    OnPpuHBlank();
	void    OnPpuVBlank();
	void    CaptureScanline(const uint8_t *pixels);

	// Opaque implementation struct — forward-declared so file-local
	// helpers in sgb.cpp can reference the full type. External code
	// has no way to obtain one.
	struct Impl;

	Impl *DebugImpl() const { return impl_; }

private:
	Impl *impl_;
};

// Global singleton — snes9x keeps one active SGB core.
Emulator &Instance();

} // namespace SGB

// C-style facade used by snes9x integration code (matches S9x* naming).
bool S9xSGBInit(void);
void S9xSGBDeinit(void);
void S9xSGBReset(void);
bool S9xSGBSoftReset(void);
bool S9xSGBLoadROM(const char *filename);

// Hand the 256-byte GB-side boot ROM to the SGB core. Only used in
// authentic BIOS mode — boot ROM scrolls the Nintendo logo and sends
// 5 SGB handshake packets that the BIOS is waiting for before it
// transitions out of its splash screen.
bool S9xSGBLoadBootROMBytes(const unsigned char *data, size_t size);

// Install the built-in SGB1 (mode=1) or SGB2 (mode=2) boot ROM. Used
// as the default in BIOS mode — most publicly-dumped sgb*.boot.rom
// files are plain DMG boot ROMs without the handshake, so we ship the
// real SGB-specific variants (from LIJI32/SameBoy under MIT).
bool S9xSGBLoadEmbeddedBootROM(unsigned char mode);

// Prime the 5-packet SGB handshake from the loaded cart. Call after
// S9xSGBLoadROM/S9xSGBLoadROMBytes in BIOS mode — boot ROM dumps in
// the wild almost never contain the SGB-specific handshake code, so
// we synthesize the packets from cart header bytes $0104-$014F.
void S9xSGBPrimeBIOSHandshake(void);

// Load a GB ROM from an in-memory buffer. Callers that already have the
// bytes (e.g. after snes9x's FileLoader has unzipped a .zip/.jma/.7z
// container) use this to skip the stdio re-read.
bool S9xSGBLoadROMBytes(const unsigned char *data, size_t size, const char *path_for_sram);
void S9xSGBRunFrame(void);
void S9xSGBRunCycles(int tcycles);

// Accumulator-based SNES→GB cycle stepping for BIOS mode. Call per
// SNES opcode (or per H event) with the number of SNES master cycles
// just consumed. Internally divides by 5 (SGB1 clock ratio) and steps
// the GB core by the resulting GB T-cycle count, carrying the fractional
// remainder to the next call. Much finer-grained than per-scanline
// batch stepping — the BIOS's $B9BE row-diff check needs continuous
// row advancement to progress.
void S9xSGBTickSnes(int snes_master_cycles);
void S9xSGBResetClockSync(void);

// Coroutine-style mid-opcode catch-up. Call with current CPU.Cycles
// before every ICD2 register read/write and at end of each SNES
// opcode iteration. Internally tracks the last-synced anchor cycle;
// compensates for SNES scanline wrap using the h_max value set via
// S9xSGBSetHMax(). The GB core is always caught up to the SNES's
// exact master-cycle position before the host side reads $6000/
// $6002/etc., so cross-domain reads reflect the most recent GB PPU
// state — matches bsnes's libco thread sync-point semantics without
// the coroutine runtime.
void S9xSGBSyncToSnesCycle(int32_t cpu_cycles);

// Tell sgb.cpp the SNES scanline period (Timings.H_Max) so it can
// recover from CPU.Cycles wrap. Call once per frame from cpuexec.
// Kept out of S9xSGBSyncToSnesCycle's signature so getset.h inlines
// don't have to see the STimings struct.
void S9xSGBSetHMax(int32_t h_max);

// Reset the sync anchor to the given SNES cycle. Call at known-good
// restart points — start of each SNES frame, just after $6003 bit-7
// release (since Reset() zeroes the GB state). Without a reset, a
// stale anchor from before a scanline wrap can make the first delta
// appear huge (or negative beyond h_max correction).
void S9xSGBResetSyncAnchor(int32_t cpu_cycles);

// GB PPU scanline-event hooks used to advance the SGB row/bank counters
// exposed at $6000. Call at HBlank end of each visible line (mode 0 →
// mode 2 transition for lines 0..142) and at VBlank entry (line 143 →
// 144 transition). Benign no-ops in BIOS-less mode.
void S9xSGBOnPpuHBlank(void);
void S9xSGBOnPpuVBlank(void);
// Monotonic count of completed GB frames (PPU VBlank entries), valid in both
// BIOS and BIOS-less modes. The frame-blend hook reads it to pair frames
// correctly across the GB↔SNES refresh-rate beat. See g_gb_vblank_count.
uint32_t S9xSGBGetGBFrameCount(void);
uint32_t S9xSGBGetPacketCount(void);
// Source-layer tag values stored per pixel in the GB layer map (see GBLayerMask).
enum GBPixelLayer
{
	GB_PIXEL_BG     = 0,
	GB_PIXEL_WINDOW = 1,
	GB_PIXEL_OBJ    = 2   // sprite
};

// Per-pixel source-layer map for the current GB frame (160x144), values per
// GBPixelLayer. nullptr when no ROM is loaded. See Emulator::GBLayerMask.
const uint8_t *S9xSGBGetGBLayerMask(void);
// Re-apply the per-title auto frame-blend selection (see Emulator::ApplyAutoBlend);
// used by the UI when the Auto toggle changes mid-session.
void S9xSGBApplyAutoBlend(void);

// True when the loaded cart runs in Game Boy Color mode (color output).
bool S9xSGBIsCgb(void);

// ---- Debug viewer accessors (GB/GBC/SGB tile/tilemap/sprite viewers) --------
struct SgbPpuRegs
{
	uint8_t lcdc, stat, scy, scx, ly, lyc, bgp, obp0, obp1, wy, wx, vbk;
};

// VRAM is 0x4000 bytes: bank 0 at [0..0x1FFF], bank 1 (CGB) at [0x2000..0x3FFF].
const uint8_t  *S9xSGBGetVRAM(void);
const uint8_t  *S9xSGBGetOAM(void);            // 0xA0 bytes
const uint8_t  *S9xSGBGetCgbBgPal(void);       // 64 bytes, CGB BG palette RAM
const uint8_t  *S9xSGBGetCgbObjPal(void);      // 64 bytes, CGB OBJ palette RAM
const uint16_t *S9xSGBGetActivePalettes(void); // 16 RGB555 (4 SGB palettes x 4)
const uint8_t  *S9xSGBGetAttrMap(void);        // 360 bytes (20x18 SGB cells)
void            S9xSGBGetPpuRegs(struct SgbPpuRegs *out);

// Live-frame layer visibility (0=BG, 1=window, 2=OBJ) — debug show/hide.
void S9xSGBSetLayerEnabled(int layer, bool enabled);
bool S9xSGBGetLayerEnabled(int layer);

// Capture a drawn GB scanline (160 palette indices, 0..3) into the SGB
// char-transfer ring. Call immediately after RenderScanline; writes to
// lcd_ring[sgb_bank] at row (sgb_row & 7). Benign no-op in BIOS-less mode.
void S9xSGBCaptureScanline(const unsigned char *pixels);
void S9xSGBSetJoypad(uint16_t snes_pad_mask);
void S9xSGBOnJoyserWrite(uint8_t value);
bool S9xSGBIsActive(void);

// Composite the current frame (border + GB screen + mask mode) into a
// 256 × 224 BGR555 buffer. `pitch_pixels` is the stride in uint16_t
// units. Call after S9xSGBRunFrame.
void S9xSGBBlitScreen(uint16_t *dest, uint32_t pitch_pixels);
void S9xSGBBlitScreenGB(uint16_t *dest, uint32_t pitch_pixels);
constexpr uint32_t SGB_GB_SCREEN_W = 160;
constexpr uint32_t SGB_GB_SCREEN_H = 144;

// BIOS-mode border overlay. In BIOS mode the SNES 65816 runs the SGB2
// BIOS, which renders its OWN device-frame border into GFX.Screen via
// normal SNES BG layers. Once the loaded GB game uploads a custom
// border via PCT_TRN/CHR_TRN (captured by our framebuffer-capture
// path even in BIOS mode), this routine overlays our captured border
// onto GFX.Screen — but ONLY the outer ring, leaving the central
// 20×18 GB area at offset (48,40) untouched so the BIOS's authentic
// GB rendering stays visible. No-op if no custom border has been
// captured yet (the BIOS's default frame stays).
//   `dest`         — must point at a 256-wide BGR555 buffer (typically
//                    GFX.Screen).
//   `pitch_pixels` — uint16 stride per row.
// Wired into S9xEndScreenRefresh AFTER FLUSH_REDRAW (which finalizes
// the PPU output into GFX.Screen) and BEFORE S9xDisplayMessages, so
// the overlay isn't clobbered by the PPU blit and the snes9x OSD
// still draws on top.
void S9xSGBOverlayBiosBorder(uint16_t *dest, uint32_t pitch_pixels);

// Audio bridge — match snes9x's S9xGetSampleCount / S9xMixSamples
// contract. `count_int16s` is the number of int16 samples (stereo frame
// × 2), returning the number actually drained.
int32_t S9xSGBGetSampleCount(void);
int32_t S9xSGBDrainSamples(int16_t *dest, int32_t count_int16s);
void    S9xSGBClearSamples(void);
// Host UI channel toggles (Sound > Channels): bits 0..3 enable GB
// CH1..CH4 (pulse A, pulse B, wave, noise). Not emulation state.
void    S9xSGBSetSoundChannelMask(uint8_t mask);
// Per-channel scope capture for the audio-waveform viewer. Enable while
// the window is open; GetChannelWaveform copies the newest mono samples
// (oldest first) for GB channel 0..3 and returns the count.
void    S9xSGBSetWaveCaptureEnabled(bool enabled);
int32_t S9xSGBGetChannelWaveform(int32_t channel, int16_t *out, int32_t max_samples);
void    S9xSGBSetAudioRate(int32_t rate_hz);
int32_t S9xSGBGetAudioRate(void);

// Diagnostics — see Emulator::GetAudioClockHz / GetAudioCyclesPerSample.
int32_t S9xSGBGetAudioClockHz(void);
int32_t S9xSGBGetAudioCyclesPerSample(void);
int32_t S9xSGBGetAudioCpsRemainderStep(void);

// Timing knobs — push Settings.GameBoyRunMode and GBClockMultiplier
// into the emulator from snes9x's per-frame dispatch. Both take effect
// from the next RunFrame forward; SetRunMode does NOT implicitly reset
// so applying it mid-game is harmless (but won't rewrite boot-state
// registers until the next Reset).
void    S9xSGBSetRunMode(uint8_t mode /* 0=DMG, 1=SGB1, 2=SGB2 */);
void    S9xSGBSetClockMultiplier(float mul);

// Save-state bridge — snapshot.cpp branches on Settings.SuperGameBoy
// and writes/reads the SGB state directly to/from the save file.
// Format: 4-byte "SGB!" magic + version + payload size + opaque blob.
size_t  S9xSGBStateSize(void);
void    S9xSGBStateSave(uint8_t *buffer);
bool    S9xSGBStateLoad(const uint8_t *buffer, size_t size);

// Fill `buf` with a one-line status snapshot:
//   "SGB PC=abcd T=xxxxxx(+nnnnn) HALT illegal=N"
// Used by cpuexec.cpp's periodic OSD hook to diagnose hangs. `cap`
// is the buffer size; truncated on overflow.
void    S9xSGBGetStatus(char *buf, size_t cap);

// Convenience file I/O wrappers — write the full blob to `filename`,
// or read it back. Return false on I/O error or format mismatch.
bool    S9xSGBSaveStateToFile(const char *filename);
bool    S9xSGBLoadStateFromFile(const char *filename);

// ---- P2 — ICD2 bridge ------------------------------------------------------
// The real SGB cart contains a custom "ICD2" chip that memory-maps a set of
// registers at 0x6000-0x7FFF on the SNES side. The BIOS reads/writes these
// to talk to the GB subsystem on the cart. Our bridge routes those accesses
// into our GB core.
//
// P2b scope: skeleton handlers that track register writes so the BIOS can
// read back consistent values. P2c+ add real semantics (packet FIFO,
// joypad routing, VRAM readback, reset gating).
unsigned char S9xSGBGetICD2 (unsigned short addr);
void          S9xSGBSetICD2 (unsigned char value, unsigned short addr);

// True when the BIOS has released the GB CPU from reset (via the ICD2
// reset register). cpuexec.cpp gates per-frame GB core stepping on this —
// before release the BIOS has sole control of the SNES, after release
// the GB runs in parallel. P2c defines the actual release bit.
bool          S9xSGBBIOSGBIsReleased (void);

// True while synth handshake packets are still queued in the single-slot
// FIFO. cpuexec uses this as an additional gate on GB core stepping —
// real GB packets must not clobber a pending synth packet before the
// BIOS reads it, so the GB is paused until the handshake drains out.
bool          S9xSGBBIOSHandshakePending (void);

bool          S9xSGBBootHandoffCaptured (void);

// ---- Integration hooks for RetroAchievements -------------------------------
// Expose the loaded GB ROM so the platform can hash it under the GameBoy /
// GameBoy Color console IDs (the SNES Memory.ROM only holds the SGB BIOS in
// authentic-BIOS mode, and is empty in BIOS-less mode). Returns false if no
// GB ROM is loaded. The buffer is owned by the SGB module and is valid until
// the next LoadROM/Deinit.
bool          S9xSGBGetROMBytes (const unsigned char **out_data, size_t *out_size);

// One-byte read from the GB address space for RetroAchievements memory
// inspection. Reads are done against raw backing storage (WRAM, HRAM,
// cart ROM bank-mapped, cart SRAM bank-mapped) — no I/O side effects,
// no interrupt or PPU bus contention. `addr` is the rcheevos GameBoy /
// GameBoy Color memory map address (0x0000-0xFFFF for native space,
// 0x10000-0x33FFF for the extended SRAM/CGB-WRAM bank window). Returns
// 0 for unmapped or out-of-range addresses.
unsigned char S9xSGBPeekRAByte (unsigned int addr);

// Cheat-engine GB memory access — see Emulator::CheatRead/CheatWrite.
unsigned char S9xSGBCheatRead (unsigned int addr);
void          S9xSGBCheatWrite (unsigned int addr, unsigned char value);
unsigned char *S9xSGBCheatWRAMPtr (unsigned int *size);
unsigned char *S9xSGBCheatSRAMPtr (unsigned int *size);
unsigned char *S9xSGBCheatHRAMPtr (unsigned int *size);
const unsigned char *S9xSGBCheatROMPtr (unsigned int *size);

bool S9xSGBHasBattery(void);
bool S9xSGBSaveBatteryToPath(const char *path);
bool S9xSGBLoadBatteryFromPath(const char *path);
bool S9xSGBTakeSramDirty(void);

// Raw cart battery-RAM pointer + size, for hosts that persist save RAM via a
// memory-region API (e.g. libretro's RETRO_MEMORY_SAVE_RAM). nullptr / 0 when
// the loaded cart has no battery-backed SRAM.
unsigned char *S9xSGBGetSRAM(void);
size_t         S9xSGBGetSRAMSize(void);

#endif
