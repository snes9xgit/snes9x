/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SGB_GB_PPU_H_
#define _SGB_GB_PPU_H_

#include <cstdint>

namespace SGB {

struct Memory;

// GB screen is 160x144. Pixel values are 0-3 (2-bit shade index).
// SGB attribute layer adds a 4-bit palette index per 8x8 tile cell later.
constexpr int GB_SCREEN_WIDTH  = 160;
constexpr int GB_SCREEN_HEIGHT = 144;

enum class PpuMode : uint8_t
{
	HBlank     = 0,  // 204-cycle mode 0
	VBlank     = 1,  // 4560 cycles covering lines 144-153
	OamScan    = 2,  // 80 cycles (mode 2)
	Transfer   = 3   // 172-289 cycles (mode 3)
};

struct Ppu
{
	uint8_t  vram[0x4000];
	uint8_t  oam[0xA0];

	uint8_t  lcdc = 0;    // 0xFF40
	uint8_t  stat = 0;    // 0xFF41
	uint8_t  scy  = 0, scx = 0;
	uint8_t  ly   = 0;
	uint8_t  lyc  = 0;
	uint8_t  bgp  = 0, obp0 = 0, obp1 = 0;
	uint8_t  wy   = 0, wx  = 0;

	bool     cgb = false;

	uint8_t  vbk  = 0;            // 0xFF4F
	uint8_t  bcps = 0, ocps = 0;  // 0xFF68 / 0xFF6A
	uint8_t  bg_pal[64]  = {0};
	uint8_t  obj_pal[64] = {0};

	PpuMode  mode = PpuMode::OamScan;
	int32_t  mode_clock = 0;

	// Per-pixel rendering state (mode-3 active draw).
	// draw_x advances 0..160 across mode 3, one pixel per GB t-cycle dot.
	// Each pixel re-samples SCX/SCY/BGP/OBP0/OBP1/LCDC/WX/WY so games that
	// change those registers mid-scanline (Animaniacs cloud strip, Balloon
	// Kid title parallax, others) render with the right per-pixel state
	// instead of a single sample latched at mode-3 entry.
	int16_t  draw_x = 0;

	// Sprite list for the current LY, latched at mode 2 → mode 3 transition.
	// Up to 10 sprites overlap a scanline; we pre-sort by render priority
	// (lowest X first, ties by OAM index) so per-pixel coverage checks can
	// be a linear scan against sprite_x[i] / sprite_x_end[i].
	struct SpriteHit {
		int16_t  x;        // OAM X-8 (screen-space leftmost pixel)
		uint8_t  oam_idx;  // 0..39
	};
	SpriteHit sprites[10];
	uint8_t   sprite_count    = 0;
	bool      window_active   = false;   // window engaged on this LY
	int16_t   window_start_x  = 0;       // x at which window engaged

	// Set at mode 2 → 3 transition. Real DMG extends mode 3 by ~6 dots
	// per OAM-scan-hit sprite and delays pixel emission by the same.
	int16_t   mode3_sprite_stall = 0;

	// WX snapshot taken at mode 2 → 3. The renderer uses this instead
	// of live p.wx so STAT-handler-driven mid-mode-3 WX writes don't tear
	// the current line (they apply to the next line instead). $FF4B reads
	// still return the live p.wx.
	uint8_t   latched_wx = 0;

	// BGP snapshot taken at mode 2 → 3 — same rationale as latched_wx.
	// Initial D Gaiden's per-scanline raster effect handler can extend
	// into the next line's mode 2 / early mode 3 (handler is ~55 m-cycles,
	// HBlank with sprite/SCX-fine stalls can be as short as ~30 m-cycles).
	// Those late BGP writes were splitting the next scanline under our
	// per-pixel BGP sampling: pixels emitted before the write used the
	// previous line's BGP, pixels after used the new one. Latching at
	// the mode 2 → 3 boundary gives each line a single BGP for its
	// entire mode 3, matching how real DMG sources palette for the
	// majority of games. $FF47 reads still return live p.bgp. TRADE-OFF:
	// games that intentionally change BGP mid-mode-3 for a horizontal
	// gradient (Animaniacs cloud strip, Balloon Kid title) will see one
	// BGP per line instead of the per-pixel sweep — revisit if those
	// regress.
	uint8_t   latched_bgp = 0;

	// Monotonic GB t-cycle counter — advanced by PpuStep. Used by
	// AdvanceMasterCycles (sgb.cpp) to drive PPU exactly to the SNES
	// master-cycle target on each sync, decoupled from CPU instruction
	// boundaries. Required for cycle-exact $6000 reads.
	int64_t  t_cycles = 0;

	// Internal window line counter — increments only on lines where the
	// window was actually drawn, independent of LY (Pan Docs: Window
	// Internal Line Counter).
	int32_t  window_line = 0;

	// Per-scanline raw BG/window 2-bit color (pre-palette). Used by the
	// sprite renderer so the BG-priority flag can check "BG color was 0"
	// after the palette mapping has already been applied to framebuffer.
	uint8_t  scanline_bg_raw[GB_SCREEN_WIDTH];

	// Per-scanline FULL composited raw 2-bit color indices (BG + window
	// + sprites, all pre-palette). bsnes/Mesen2 feed these raw indices
	// to the SGB ICD2 $7800 readback — NOT BGP/OBP-mapped shades. If a
	// game sets BGP to anything other than identity ($E4), the captured
	// shades no longer match raw indices and CHR_TRN / live char-transfer
	// produces mangled tile output on the SGB side.
	uint8_t  scanline_raw[GB_SCREEN_WIDTH];

	// Latched state of the STAT IRQ line — edge-triggered, so we fire an
	// LCDSTAT interrupt only when this transitions 0 → 1.
	bool     stat_line_high = false;

	// VBlank IRQ latency. Real DMG samples IF on the LAST M-cycle of the
	// current instruction, so when LY transitions to 144 mid-instruction the
	// current LDH/CP/JR can still observe LY=144 before IRQ dispatch. Our
	// CPU runs instructions atomically and checks IF before the next step —
	// firing IF.VBLANK on the dot LY hits 144 pre-empts the polling LDH that
	// would have caught LY=144. Defer the IF.VBLANK set by N dots: when LY
	// transitions, arm this counter; ExecPpuDot decrements it; when it hits
	// 0, set IF.VBLANK. 24 dots ≈ one LDH+CP — enough for Altered Space's
	// "wait until LY=$90" busy loop at $078A to capture LY=144 into A.
	uint8_t  vblank_irq_delay = 0;

	uint8_t  framebuffer[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];

	// Per-frame raw 2-bit BG/window/sprite indices (pre-BGP/OBP). The
	// SGB BIOS-less border-capture path reads this when reconstructing
	// CHR_TRN / PCT_TRN data from the LCD, so non-identity BGP doesn't
	// mangle the byte stream the game intended to send.
	uint8_t  raw_framebuffer[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];

	// Per-pixel source layer, parallel to framebuffer: 0 = BG, 1 = window,
	// 2 = sprite/OBJ. Lets the host frame-blend selectively blend only the
	// background (BG+window, where flicker-transparency lives) and leave
	// moving sprites crisp, or vice versa. Display-only, NOT serialized.
	uint8_t  layer[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];

	uint16_t color_fb[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];

	bool     frame_ready = false;

	uint64_t vram_writes = 0;
};

void PpuReset(Ppu &p);
void PpuStep(Ppu &p, Memory &mem, int32_t tcycles);

// Register I/O — 0xFF40 .. 0xFF4B (LCDC/STAT/SCY/SCX/LY/LYC/DMA/BGP/OBP0/OBP1/WY/WX).
uint8_t PpuReadReg(const Ppu &p, uint16_t addr);
void    PpuWriteReg(Ppu &p, Memory &mem, uint16_t addr, uint8_t value);

} // namespace SGB

#endif
