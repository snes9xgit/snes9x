/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Game Boy PPU — per-pixel implementation (one render call per dot).
//
// Timing (T-cycles, per Pan Docs):
//   line = 456 dots, split as
//     mode 2 (OAM scan)       : 80 dots
//     mode 3 (pixel transfer) : 172 + sprite_count*6 + (SCX & 7) dots
//                               (12-dot setup + 6 dots per OAM-scan-hit
//                               sprite + 1 dot per pixel the BG fetcher
//                               has to discard for SCX-fine alignment,
//                               then 160 pixels emit, total 172..239)
//     mode 0 (HBlank)         : 204 - sprite_count*6 - (SCX & 7) dots
//   144 visible lines + 10 VBlank lines = 154 total. Frame = 70224 dots.
//
// Mode 3 timing model:
//   The first 12 dots of mode 3 are setup (BG fetcher fills its 16-pixel
//   shift register with two tiles before pixel 0 emits). On lines with
//   sprites, the fetcher additionally stalls ~6 dots per sprite covering
//   the scanline while reading sprite tile data. When SCX is not
//   tile-aligned (SCX & 7 != 0), the fetcher discards (SCX & 7) pixels
//   from the first tile before emitting pixel 0 — each discarded pixel
//   costs 1 dot. Pixels start emitting at
//     mode_clock = 12 + sprite_count*6 + (SCX & 7)
//   and the LAST pixel emits 160 dots later. Mode 0 shrinks by the same
//   total to keep the scanline at 456 dots. We model the stall as a
//   single upfront block (computed at mode 2 → 3 transition from
//   sprite_count and the SCX latched at that boundary) rather than the
//   per-sprite-position pipeline — close enough that STAT-IRQ handler
//   writes land at the same effective pixel boundary as real HW.
//
// PpuStep iterates one GB t-cycle at a time. During mode 3 the renderer
// emits ONE pixel per dot (RenderPixel), re-sampling LCDC/SCX/SCY/BGP/
// OBP0/OBP1/WY at each pixel — this catches mid-LY register writes that
// some games use for parallax (Animaniacs cloud strip), palette effects,
// and partial-line scroll. WX is the exception: it's latched at
// mode 2 → 3 and held for the line, so STAT-handler-driven mid-mode-3
// WX writes apply to the NEXT line. This matches the 8-dot tile-boundary
// granularity real DMG uses for window engage (rather than per-pixel).
// Without latching, games that toggle WX from a STAT IRQ to engage/
// disengage the window for a dialog overlay (One Piece - Maboroshi no
// Grand Line) tear the boundary scanlines.
//
// OBJ-over-BG priority: the per-pixel BG raw value (scanline_bg_raw) is
// tracked alongside the palette-mapped framebuffer so SampleSpritePixel
// can respect the "BG wins when BG color != 0" flag.
//
// STAT IRQ uses edge detection: the four sources (LYC match / mode 2 /
// mode 1 / mode 0) are ORed into a single "stat line"; we raise
// IRQ_LCDSTAT only on the 0 → 1 transition, matching real HW's single-
// wire behavior.

#include "gb_ppu.h"
#include "gb_memory.h"
#include "gb_cpu.h"
#include "sgb.h"

#include <cstring>

namespace SGB {

namespace {

// Mode 3 base lengths; per-scanline length shifts by p.mode3_sprite_stall.
constexpr int32_t MODE2_DOTS        = 80;
constexpr int32_t MODE3_DOTS        = 172;
constexpr int32_t MODE0_DOTS        = 204;
constexpr int32_t MODE3_SETUP_DOTS  = 12;
constexpr int32_t SPRITE_STALL_DOTS = 6;
constexpr int32_t LINE_DOTS  = MODE2_DOTS + MODE3_DOTS + MODE0_DOTS;  // 456
constexpr int32_t VISIBLE_LINES = 144;
constexpr int32_t TOTAL_LINES   = 154;

inline uint8_t ApplyPalette(uint8_t palette, uint8_t color_idx)
{
	// BGP/OBP0/OBP1 are 2-bit mappings packed into 8 bits:
	//   bits 1:0 = shade for color 0, 3:2 for color 1, 5:4 for color 2, 7:6 for color 3.
	return static_cast<uint8_t>((palette >> (color_idx * 2)) & 0x03);
}

void RecomputeStatLine(Ppu &p, Memory &mem)
{
	// Rebuild the low 3 bits of STAT (mode + LYC coincidence).
	uint8_t new_stat = static_cast<uint8_t>((p.stat & 0xF8) | static_cast<uint8_t>(p.mode));
	if (p.ly == p.lyc) new_stat |= 0x04;
	p.stat = new_stat;

	// Compute the combined IRQ line from the four source-enable flags.
	// When the LCD is off (LCDC.bit7 = 0), real DMG holds the STAT IRQ
	// line low regardless of the source-enable bits and the PPU state
	// (mode/LY/LYC are parked). PpuStep mirrors this by forcing mode 0 /
	// LY=0 with the LCD off, but writes to FF41/FF44/FF45 also call
	// RecomputeStatLine — without this LCDC guard those writes would see
	// "mode 0 active + bit 3 enabled" or "LY=LYC=0 + bit 6 enabled" and
	// raise a STAT IRQ that real hardware never fires while the LCD is
	// disabled. Zerd no Densetsu's bank-1 init exposed this: it writes
	// STAT.bit6 inside a DI region with the LCD off via $02CF; without
	// the guard the spurious raise leaks into the post-EI flow.
	bool line_high = false;
	if (p.lcdc & 0x80)
	{
		if ((p.stat & 0x40) && (p.ly == p.lyc))                 line_high = true;
		if ((p.stat & 0x20) && p.mode == PpuMode::OamScan)       line_high = true;
		if ((p.stat & 0x10) && p.mode == PpuMode::VBlank)        line_high = true;
		if ((p.stat & 0x08) && p.mode == PpuMode::HBlank)        line_high = true;
	}

	// Rising edge on the combined line raises LCDSTAT IRQ.
	if (line_high && !p.stat_line_high)
		mem.if_ = static_cast<uint8_t>(mem.if_ | IRQ_LCDSTAT);
	p.stat_line_high = line_high;
}

// Sample one BG pixel at (x, ly) using the CURRENT register values.
// Inlined so the per-dot path doesn't pay function-call overhead per pixel.
inline uint8_t SampleBgPixel(const Ppu &p, int x)
{
	const uint16_t map_base  = (p.lcdc & 0x08) ? 0x1C00 : 0x1800;
	const bool     tiles_un  = (p.lcdc & 0x10) != 0;
	const uint8_t  bg_y      = static_cast<uint8_t>(p.ly + p.scy);
	const uint32_t tile_row  = bg_y >> 3;
	const uint32_t fine_y    = bg_y & 7;
	const uint8_t  bg_x      = static_cast<uint8_t>(x + p.scx);
	const uint32_t tile_col  = bg_x >> 3;
	const uint32_t fine_x    = bg_x & 7;

	const uint8_t tile_num = p.vram[map_base + (tile_row * 32) + tile_col];

	uint16_t tile_addr;
	if (tiles_un)
		tile_addr = static_cast<uint16_t>(tile_num * 16);
	else
		tile_addr = static_cast<uint16_t>(0x1000 + static_cast<int8_t>(tile_num) * 16);
	tile_addr = static_cast<uint16_t>(tile_addr + fine_y * 2);

	const uint8_t lo  = p.vram[tile_addr];
	const uint8_t hi  = p.vram[tile_addr + 1];
	const uint8_t bit = static_cast<uint8_t>(7 - fine_x);
	return static_cast<uint8_t>((((hi >> bit) & 1) << 1) | ((lo >> bit) & 1));
}

// Sample one window pixel. Uses latched_wx (sampled at mode 2 → 3)
// rather than live p.wx — caller already confirmed engage with the
// same latched value.
inline uint8_t SampleWindowPixel(const Ppu &p, int x)
{
	const uint16_t map_base = (p.lcdc & 0x40) ? 0x1C00 : 0x1800;
	const bool     tiles_un = (p.lcdc & 0x10) != 0;
	const uint32_t win_y    = static_cast<uint32_t>(p.window_line);
	const uint32_t tile_row = win_y >> 3;
	const uint32_t fine_y   = win_y & 7;
	const int      wx       = static_cast<int>(p.latched_wx) - 7;
	const int      win_col  = x - wx;
	const uint32_t tile_col = static_cast<uint32_t>(win_col) >> 3;
	const uint32_t fine_x   = static_cast<uint32_t>(win_col) & 7;

	const uint8_t tile_num = p.vram[map_base + (tile_row * 32) + tile_col];

	uint16_t tile_addr;
	if (tiles_un)
		tile_addr = static_cast<uint16_t>(tile_num * 16);
	else
		tile_addr = static_cast<uint16_t>(0x1000 + static_cast<int8_t>(tile_num) * 16);
	tile_addr = static_cast<uint16_t>(tile_addr + fine_y * 2);

	const uint8_t lo  = p.vram[tile_addr];
	const uint8_t hi  = p.vram[tile_addr + 1];
	const uint8_t bit = static_cast<uint8_t>(7 - fine_x);
	return static_cast<uint8_t>((((hi >> bit) & 1) << 1) | ((lo >> bit) & 1));
}

// Pull a per-LY sprite list at mode 2 → mode 3. Up to 10 sprites in OAM
// scan order (matches DMG hardware), pre-sorted by DMG draw priority
// (lowest X first, ties by OAM index). Render-time uses this list to
// answer "is there a sprite covering pixel x?" with a linear scan.
void EvalSprites(Ppu &p)
{
	const bool large    = (p.lcdc & 0x04) != 0;
	const int  sprite_h = large ? 16 : 8;

	p.sprite_count = 0;
	for (int i = 0; i < 40 && p.sprite_count < 10; ++i)
	{
		const uint8_t oy  = p.oam[i * 4 + 0];
		const int     top = static_cast<int>(oy) - 16;
		const int     ly  = static_cast<int>(p.ly);
		if (ly < top || ly >= top + sprite_h) continue;
		const uint8_t ox = p.oam[i * 4 + 1];
		Ppu::SpriteHit &h = p.sprites[p.sprite_count++];
		h.x       = static_cast<int16_t>(static_cast<int>(ox) - 8);
		h.oam_idx = static_cast<uint8_t>(i);
	}

	// DMG priority: lower X wins, ties by lower OAM index. Sort ascending
	// (insertion sort, stable) so render-time picks the FIRST sprite that
	// produces a non-zero pixel.
	for (uint8_t i = 1; i < p.sprite_count; ++i)
	{
		Ppu::SpriteHit cur = p.sprites[i];
		uint8_t j = i;
		while (j > 0)
		{
			const Ppu::SpriteHit &prev = p.sprites[j - 1];
			const bool prev_higher_pri = (prev.x < cur.x) ||
			                             (prev.x == cur.x && prev.oam_idx < cur.oam_idx);
			if (prev_higher_pri) break;
			p.sprites[j] = p.sprites[j - 1];
			--j;
		}
		p.sprites[j] = cur;
	}
}

// Sample one sprite-covering-pixel value using CURRENT registers. Returns
// {covered, sprite_color_2bit, sprite_palette_8bit, bg_priority} via out
// args. covered=false means no sprite covers this pixel or all covering
// sprites are color-0 (transparent).
struct SpritePixel { bool covered; uint8_t color; uint8_t palette; bool bg_over; };

SpritePixel SampleSpritePixel(const Ppu &p, int x)
{
	SpritePixel out{ false, 0, 0, false };
	if (!(p.lcdc & 0x02)) return out;

	const bool large    = (p.lcdc & 0x04) != 0;
	const int  sprite_h = large ? 16 : 8;

	// First sprite in priority order whose pixel at x is non-transparent wins.
	for (uint8_t i = 0; i < p.sprite_count; ++i)
	{
		const int sx = p.sprites[i].x;
		if (x < sx || x >= sx + 8) continue;

		const uint8_t oi    = p.sprites[i].oam_idx;
		const uint8_t oy    = p.oam[oi * 4 + 0];
		uint8_t       tile  = p.oam[oi * 4 + 2];
		const uint8_t flags = p.oam[oi * 4 + 3];

		int sprite_top = static_cast<int>(oy) - 16;
		int tile_row   = static_cast<int>(p.ly) - sprite_top;
		if (flags & 0x40) tile_row = sprite_h - 1 - tile_row;

		if (large) tile = static_cast<uint8_t>(tile & 0xFE);
		const uint8_t sub_tile = static_cast<uint8_t>(tile + (tile_row / 8));
		const int     fine_y   = tile_row & 7;

		const uint16_t tile_addr = static_cast<uint16_t>(sub_tile * 16 + fine_y * 2);
		const uint8_t  lo        = p.vram[tile_addr];
		const uint8_t  hi        = p.vram[tile_addr + 1];

		const int     px        = x - sx;
		const int     bit       = (flags & 0x20) ? px : (7 - px);
		const uint8_t color_idx = static_cast<uint8_t>(
			(((hi >> bit) & 1) << 1) | ((lo >> bit) & 1));
		if (color_idx == 0) continue;   // transparent — try next sprite

		out.covered  = true;
		out.color    = color_idx;
		out.palette  = (flags & 0x10) ? p.obp1 : p.obp0;
		out.bg_over  = (flags & 0x80) != 0;
		return out;
	}
	return out;
}

// Render exactly one pixel at p.draw_x for the current LY. Re-samples
// every relevant register so mid-LY changes (SCX/SCY/BGP/LCDC/WX/etc.)
// take effect at the right pixel, matching real hardware's per-dot
// fetch behavior.
void RenderPixel(Ppu &p)
{
	const int x = static_cast<int>(p.draw_x);
	if (x < 0 || x >= GB_SCREEN_WIDTH) return;
	if (p.ly >= GB_SCREEN_HEIGHT)      return;

	uint8_t *const line = &p.framebuffer[p.ly * GB_SCREEN_WIDTH];

	// BG / window resolve.
	uint8_t bg_color = 0;   // raw 2-bit pre-palette
	if (p.lcdc & 0x01)
	{
		const int wx = static_cast<int>(p.latched_wx) - 7;
		const bool win_active_here =
			(p.lcdc & 0x20) != 0 &&
			static_cast<int>(p.ly) >= static_cast<int>(p.wy) &&
			x >= wx;

		if (win_active_here)
		{
			bg_color = SampleWindowPixel(p, x);
			// Latch the "window did draw" state for window_line bookkeeping
			// at end of LY.
			if (!p.window_active)
			{
				p.window_active  = true;
				p.window_start_x = static_cast<int16_t>(x);
			}
		}
		else
		{
			bg_color = SampleBgPixel(p, x);
		}
	}

	p.scanline_bg_raw[x] = bg_color;
	p.scanline_raw[x]    = bg_color;
	line[x]              = ApplyPalette(p.bgp, bg_color);

	// Sprite resolve — overwrite if visible.
	const SpritePixel sp = SampleSpritePixel(p, x);
	if (sp.covered && (!sp.bg_over || bg_color == 0))
	{
		p.scanline_raw[x] = sp.color;
		line[x]           = ApplyPalette(sp.palette, sp.color);
	}
}

// Called once at the mode 3 → HBlank transition (after all 160 pixels
// have been emitted by RenderPixel). Snapshots the raw indices for the
// SGB BIOS-less border-capture path and hands the post-palette LCD line
// to the SGB ICD2 char-transfer ring. window_line advances only when the
// window actually drew at least one pixel this LY (matches Pan Docs:
// Window Internal Line Counter).
void FinalizeScanline(Ppu &p)
{
	if (p.ly >= GB_SCREEN_HEIGHT) return;

	uint8_t *const line = &p.framebuffer[p.ly * GB_SCREEN_WIDTH];

	std::memcpy(&p.raw_framebuffer[p.ly * GB_SCREEN_WIDTH],
	            p.scanline_raw, GB_SCREEN_WIDTH);
	S9xSGBCaptureScanline(line);

	if (p.window_active)
		++p.window_line;
}

} // anonymous

void PpuReset(Ppu &p)
{
	std::memset(p.vram,        0, sizeof p.vram);
	std::memset(p.oam,         0, sizeof p.oam);
	std::memset(p.framebuffer,     0, sizeof p.framebuffer);
	std::memset(p.raw_framebuffer, 0, sizeof p.raw_framebuffer);
	std::memset(p.scanline_bg_raw, 0, sizeof p.scanline_bg_raw);
	std::memset(p.scanline_raw,    0, sizeof p.scanline_raw);
	p.lcdc = 0x91;   // LCD on, BG on, BG tile data at 0x8000, BG map at 0x9800.
	p.stat = 0x85;
	p.scy = p.scx = p.ly = p.lyc = 0;
	p.bgp = 0xFC; p.obp0 = 0xFF; p.obp1 = 0xFF;
	p.wy = p.wx = 0;
	p.mode = PpuMode::OamScan;
	p.mode_clock    = 0;
	p.window_line   = 0;
	p.stat_line_high = false;
	p.vblank_irq_delay = 0;
	p.frame_ready   = false;
	p.t_cycles      = 0;
	p.draw_x        = 0;
	p.sprite_count  = 0;
	p.window_active = false;
	p.window_start_x = 0;
	p.mode3_sprite_stall = 0;
	p.latched_wx    = 0;
}

// One GB t-cycle's worth of PPU work. Drives mode 2 sprite eval (latched
// at the mode 2→3 boundary), mode 3 per-pixel render (one pixel per dot,
// re-sampling registers fresh — fixes mid-LY scroll/palette/LCDC tricks),
// and mode 0/1 timing.
inline void ExecPpuDot(Ppu &p, Memory &mem)
{
	p.mode_clock += 1;
	bool transitioned = false;

	// Deferred VBlank IRQ — see Ppu::vblank_irq_delay for rationale.
	if (p.vblank_irq_delay > 0)
	{
		--p.vblank_irq_delay;
		if (p.vblank_irq_delay == 0)
			mem.if_ = static_cast<uint8_t>(mem.if_ | IRQ_VBLANK);
	}

	switch (p.mode)
	{
	case PpuMode::OamScan:
		if (p.mode_clock >= MODE2_DOTS)
		{
			p.mode_clock -= MODE2_DOTS;
			p.mode        = PpuMode::Transfer;
			// Latch the per-LY sprite list at mode 2→3 boundary, matching
			// real-hw OAM-scan completion. Per-pixel sprite resolve in
			// SampleSpritePixel uses this list.
			EvalSprites(p);
			p.draw_x        = 0;
			p.window_active = false;
			// Mode 3 extra-dots budget covers two penalties that real DMG
			// applies at the mode 2 → 3 boundary:
			//   1. Per OAM-scan-hit sprite: ~6 dots of fetcher stall
			//      (regardless of LCDC.1) while sprite tile data fetches.
			//   2. SCX-fine penalty: when SCX is not tile-aligned, the BG
			//      fetcher discards (SCX & 7) pixels from the first tile
			//      before emitting pixel 0. Each discarded pixel costs 1
			//      dot. Per-scanline raster-effect games (Initial D Gaiden
			//      racing road perspective) that ramp SCX 0..31 across
			//      consecutive scanlines need this penalty modeled or the
			//      handler ends up writing the NEXT line's SCY/BGP into
			//      mode 3 instead of HBlank, producing split scanlines.
			// Mode 0 (HBlank) shrinks by the same amount so each scanline
			// still totals 456 dots.
			p.mode3_sprite_stall = static_cast<int16_t>(
				p.sprite_count * SPRITE_STALL_DOTS +
				(p.scx & 0x07));
			// WX is snapshotted here so mid-mode-3 writes don't engage
			// the window mid-line — they take effect on the next line.
			p.latched_wx = p.wx;
			transitioned = true;
		}
		break;

	case PpuMode::Transfer:
	{
		// Pixels start emitting after the fetcher setup + sprite stalls.
		// Mid-mode-3 register writes (LCDC.1 sprite toggle, etc.) that
		// land before pixel 0 apply to the whole line; writes after that
		// apply only to subsequent pixels. STAT-IRQ handlers driving WX
		// toggles for a dialog overlay rely on the upfront setup window
		// being large enough that the WX write completes before pixel 0
		// emits — combined with the WX latch on the OamScan → Transfer
		// edge, the boundary scanlines render cleanly.
		const int32_t pixel_start_dot =
			MODE3_SETUP_DOTS + p.mode3_sprite_stall;
		if (p.mode_clock > pixel_start_dot && p.draw_x < GB_SCREEN_WIDTH)
		{
			if (p.lcdc & 0x01)
			{
				RenderPixel(p);
			}
			else
			{
				// LCDC.0 cleared on DMG = BG/window OFF, force pixel 0.
				// Sprites still composite on top.
				const int x = static_cast<int>(p.draw_x);
				p.scanline_bg_raw[x] = 0;
				p.scanline_raw[x]    = 0;
				p.framebuffer[p.ly * GB_SCREEN_WIDTH + x] =
					ApplyPalette(p.bgp, 0);
				const SpritePixel sp = SampleSpritePixel(p, x);
				if (sp.covered)
				{
					p.scanline_raw[x] = sp.color;
					p.framebuffer[p.ly * GB_SCREEN_WIDTH + x] =
						ApplyPalette(sp.palette, sp.color);
				}
			}
			++p.draw_x;
		}
		const int32_t mode3_length = MODE3_DOTS + p.mode3_sprite_stall;
		if (p.mode_clock >= mode3_length)
		{
			p.mode_clock -= mode3_length;
			p.mode        = PpuMode::HBlank;
			FinalizeScanline(p);
			transitioned = true;
		}
		break;
	}

	case PpuMode::HBlank:
	{
		// Mode 0 absorbs the sprite stall so the scanline still totals 456 dots.
		const int32_t mode0_length = MODE0_DOTS - p.mode3_sprite_stall;
		if (p.mode_clock >= mode0_length)
		{
			p.mode_clock -= mode0_length;
			++p.ly;
			S9xSGBOnPpuHBlank();
			if (p.ly == VISIBLE_LINES)
			{
				p.mode          = PpuMode::VBlank;
				p.frame_ready   = true;
				p.window_line   = 0;
				p.window_active = false;
				p.vblank_irq_delay = 24;
				S9xSGBOnPpuVBlank();
			}
			else
			{
				p.mode = PpuMode::OamScan;
			}
			transitioned = true;
		}
		break;
	}

	case PpuMode::VBlank:
		// LY=153 hardware quirk (Pan Docs §STAT.lyc-glitch):
		// On real DMG, LY only reads as 153 for the first 4 dots of
		// scanline 153, then visibly becomes 0 for the remaining 452
		// dots while the PPU stays in mode 1 (VBlank). That early LY=0
		// is the moment LYC=0 STAT IRQ rises on real hardware — not at
		// scanline 0 as a naive scanline-end counter would imply.
		// Models the LY transition; the actual end-of-scanline-153 →
		// scanline-0 mode transition happens 452 dots later.
		if (p.ly == 153 && p.mode_clock == 4)
		{
			p.ly = 0;
			transitioned = true;
		}
		if (p.mode_clock >= LINE_DOTS)
		{
			p.mode_clock -= LINE_DOTS;
			if (p.ly == 0)
			{
				// Quirk already set LY=0; the scanline-153 → scanline-0
				// boundary just transitions mode to OamScan with LY
				// unchanged.
				p.mode = PpuMode::OamScan;
			}
			else
			{
				++p.ly;
				if (p.ly >= TOTAL_LINES)
				{
					// Safety fallback (shouldn't fire if the quirk did).
					p.ly   = 0;
					p.mode = PpuMode::OamScan;
				}
			}
			transitioned = true;
		}
		break;
	}

	if (transitioned)
		RecomputeStatLine(p, mem);
}

void PpuStep(Ppu &p, Memory &mem, int32_t tcycles)
{
	if (tcycles <= 0) return;
	p.t_cycles += tcycles;

	// LCD master disable (LCDC bit 7). Real HW parks the PPU in mode 0
	// with LY=0 until the bit toggles back on. We keep the framebuffer
	// contents so the display keeps showing the last valid frame.
	//
	// Crucially: STAT IRQs do NOT fire while the LCD is off. The STAT
	// line is forced low, which means we must NOT call RecomputeStatLine
	// here — that function's "rising edge" detector would see a live
	// HBlank (mode=0) every call and raise IRQ_LCDSTAT on every CPU
	// instruction. Pokemon Yellow's Pikachu-voice routine disables the
	// LCD with STAT bit 3 (HBlank IRQ) still enabled, and we were
	// storm-firing its ISR forever at PC=15C1 (one instruction before
	// RETI, right after POP AF).
	if (!(p.lcdc & 0x80))
	{
		p.mode           = PpuMode::HBlank;
		p.ly             = 0;
		p.mode_clock     = 0;
		p.window_line    = 0;
		p.stat_line_high = false;
		p.vblank_irq_delay = 0;
		p.draw_x         = 0;
		p.window_active  = false;
		p.mode3_sprite_stall = 0;
		p.latched_wx     = p.wx;
		p.stat = static_cast<uint8_t>(p.stat & 0xF8);
		(void)mem;
		return;
	}

	while (tcycles-- > 0)
		ExecPpuDot(p, mem);
}

uint8_t PpuReadReg(const Ppu &p, uint16_t addr)
{
	switch (addr)
	{
		case 0xFF40: return p.lcdc;
		case 0xFF41: return static_cast<uint8_t>(p.stat | 0x80);  // bit 7 always 1
		case 0xFF42: return p.scy;
		case 0xFF43: return p.scx;
		case 0xFF44: return p.ly;
		case 0xFF45: return p.lyc;
		case 0xFF47: return p.bgp;
		case 0xFF48: return p.obp0;
		case 0xFF49: return p.obp1;
		case 0xFF4A: return p.wy;
		case 0xFF4B: return p.wx;
	}
	return 0xFF;
}

void PpuWriteReg(Ppu &p, Memory &mem, uint16_t addr, uint8_t value)
{
	switch (addr)
	{
		case 0xFF40:
		{
			const bool was_on = (p.lcdc & 0x80) != 0;
			p.lcdc = value;
			const bool is_on  = (p.lcdc & 0x80) != 0;
			// LCD turn-on resets to line 0 / mode 2.
			if (!was_on && is_on)
			{
				p.ly          = 0;
				p.mode        = PpuMode::OamScan;
				p.mode_clock  = 0;
				p.window_line = 0;
			}
			if (is_on) RecomputeStatLine(p, mem);
			break;
		}
		case 0xFF41:
		{
			// DMG STAT write quirk (Pan Docs §STAT.spurious-stat-interrupts):
			// On DMG, writing to STAT momentarily glitches the enable bits
			// to all-1 for one cycle. If a source condition (mode 0/1/2 or
			// LYC=LY) is currently active during that glitch AND the STAT
			// line was previously low, a spurious LCDSTAT IRQ is raised.
			//
			// Zerd no Densetsu's bank-1 init relies on this. At $62DB
			// (SET 6 / LDH ($41),A) the PPU is mid-VBlank (mode 1 active).
			// The quirk fires IF.STAT immediately — with IME=0 from the
			// surrounding DI region it sits pending; after the $62E7 EI
			// and the CALL into $021E, the IRQ dispatches with bank=1
			// still mapped, so the trampoline at $C2CC reads the real
			// handler in bank 1. Without the quirk, IF.STAT only sets on
			// the next LYC=0 edge ~3000 cycles later — by then the wait
			// loop has entered the bank-7 sound CALL ($4023) inside its
			// DI region, the trampoline reads bank 7 garbage ($7B), and
			// the CPU falls into RST 38 forever.
			//
			// Edge gate: only fire the quirk when the write ACTUALLY
			// newly-enables at least one source bit (3..6 going 0→1).
			// Idempotent rewrites (e.g. STAT=$40 → STAT=$40) don't
			// produce a hardware-visible edge — without this gate
			// Initial D Gaiden's racing-scene STAT handler over-fires
			// itself catastrophically: the handler does EI early then
			// writes STAT idempotently as part of its work, each write
			// quirk-firing a new STAT IRQ that nests on top of the
			// running handler, stack-bombing WRAM down hundreds of bytes
			// per frame and walking the per-scanline SCY/SCX/BGP table
			// way past one entry per line. The road perspective falls
			// apart into stripes.
			const uint8_t old_enables = static_cast<uint8_t>(p.stat & 0x78);
			const uint8_t new_enables = static_cast<uint8_t>(value & 0x78);
			const bool    newly_set   = (~old_enables & new_enables) != 0;
			if (newly_set && !p.stat_line_high && (p.lcdc & 0x80))
			{
				const bool any_source_active =
					p.mode == PpuMode::HBlank ||
					p.mode == PpuMode::VBlank ||
					p.mode == PpuMode::OamScan ||
					p.ly == p.lyc;
				if (any_source_active)
					mem.if_ = static_cast<uint8_t>(mem.if_ | IRQ_LCDSTAT);
			}
			p.stat = static_cast<uint8_t>((p.stat & 0x07) | (value & 0x78));
			RecomputeStatLine(p, mem);
			break;
		}
		case 0xFF42: p.scy = value; break;
		case 0xFF43: p.scx = value; break;
		case 0xFF44: p.ly  = 0; RecomputeStatLine(p, mem); break;
		case 0xFF45: p.lyc = value; RecomputeStatLine(p, mem); break;
		case 0xFF47: p.bgp  = value; break;
		case 0xFF48: p.obp0 = value; break;
		case 0xFF49: p.obp1 = value; break;
		case 0xFF4A: p.wy = value;   break;
		case 0xFF4B: p.wx = value;   break;
	}
}

} // namespace SGB
