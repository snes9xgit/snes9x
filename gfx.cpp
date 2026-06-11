/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "ppu.h"
#include "tile.h"
#include "controls.h"
#include "crosshairs.h"
#include "cheats.h"
#include "movie.h"
#include "screenshot.h"
#include "display.h"
#include "sgb/sgb.h"   // S9xSGBOverlayBiosBorder — runs after FLUSH_REDRAW
                        // so the BIOS-mode custom-border overlay isn't
                        // clobbered by the PPU's final blit.

extern struct SCheatData		Cheat;
extern struct SLineData			LineData[240];
extern struct SLineMatrixData	LineMatrixData[240];

void S9xComputeClipWindows (void);

void (*S9xCustomDisplayString) (const char *, int, int, bool, int) = NULL;

static void SetupOBJ (void);
static void DrawOBJS (int);
static void DisplayTime (void);
static void DisplayFrameRate (void);
static void DisplayFrameNumber (void);
static void DisplayPressedKeys (void);
static void DisplayWatchedAddresses (void);
static void DisplayStringFromBottom (const char *, int, int, bool);
static void DrawBackground (int, uint8, uint8);
static void DrawBackgroundMosaic (int, uint8, uint8);
static void DrawBackgroundOffset (int, uint8, uint8, int);
static void DrawBackgroundOffsetMosaic (int, uint8, uint8, int);
static inline void DrawBackgroundMode7 (int, void (*DrawMath) (uint32, uint32, int), void (*DrawNomath) (uint32, uint32, int), int);
static inline void DrawBackdrop (void);
static inline void RenderScreen (bool8);
static uint16 get_crosshair_color (uint8);
static void S9xDisplayStringType (const char *, int, int, bool, int);

#define TILE_PLUS(t, x)	(((t) & 0xfc00) | ((t + x) & 0x3ff))


bool8 S9xGraphicsInit (void)
{
	S9xInitTileRenderer();
	memset(BlackColourMap, 0, 256 * sizeof(uint16));

	IPPU.OBJChanged = TRUE;
	Settings.BG_Forced = 0;
	Settings.ForcedBackdrop = 0;
	S9xFixColourBrightness();
	S9xBuildDirectColourMaps();

	GFX.ScreenBuffer.resize(MAX_SNES_WIDTH * (MAX_SNES_HEIGHT + 64));
	GFX.Screen = &GFX.ScreenBuffer[GFX.RealPPL * 32];
	GFX.ZERO = (uint16 *) malloc(sizeof(uint16) * 0x10000);
	GFX.SubScreen  = (uint16 *) malloc(GFX.ScreenSize * sizeof(uint16));
	GFX.ZBuffer    = (uint8 *)  malloc(GFX.ScreenSize);
	GFX.SubZBuffer = (uint8 *)  malloc(GFX.ScreenSize);

	if (!GFX.ZERO || !GFX.SubScreen || !GFX.ZBuffer || !GFX.SubZBuffer)
	{
		S9xGraphicsDeinit();
		return (FALSE);
	}

	// Lookup table for 1/2 color subtraction
	memset(GFX.ZERO, 0, 0x10000 * sizeof(uint16));
	for (uint32 r = 0; r <= MAX_RED; r++)
	{
		uint32	r2 = r;
		if (r2 & 0x10)
			r2 &= ~0x10;
		else
			r2 = 0;

		for (uint32 g = 0; g <= MAX_GREEN; g++)
		{
			uint32	g2 = g;
			if (g2 & GREEN_HI_BIT)
				g2 &= ~GREEN_HI_BIT;
			else
				g2 = 0;

			for (uint32 b = 0; b <= MAX_BLUE; b++)
			{
				uint32	b2 = b;
				if (b2 & 0x10)
					b2 &= ~0x10;
				else
					b2 = 0;

				GFX.ZERO[BUILD_PIXEL2(r, g, b)] = BUILD_PIXEL2(r2, g2, b2);
				GFX.ZERO[BUILD_PIXEL2(r, g, b) & ~ALPHA_BITS_MASK] = BUILD_PIXEL2(r2, g2, b2);
			}
		}
	}

	return (TRUE);
}

void S9xGraphicsDeinit (void)
{
	if (GFX.ZERO)       { free(GFX.ZERO);       GFX.ZERO       = NULL; }
	if (GFX.SubScreen)  { free(GFX.SubScreen);  GFX.SubScreen  = NULL; }
	if (GFX.ZBuffer)    { free(GFX.ZBuffer);    GFX.ZBuffer    = NULL; }
	if (GFX.SubZBuffer) { free(GFX.SubZBuffer); GFX.SubZBuffer = NULL; }
}

void S9xGraphicsScreenResize (void)
{
	IPPU.MaxBrightness = PPU.Brightness;

	IPPU.Interlace    = Memory.FillRAM[0x2133] & 1;
	IPPU.InterlaceOBJ = Memory.FillRAM[0x2133] & 2;
	IPPU.PseudoHires = Memory.FillRAM[0x2133] & 8;

	if (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.PseudoHires)
	{
		IPPU.DoubleWidthPixels = TRUE;
		IPPU.RenderedScreenWidth = SNES_WIDTH << 1;
	}
	else
	{
		IPPU.DoubleWidthPixels = FALSE;
		IPPU.RenderedScreenWidth = SNES_WIDTH;
	}

	if (IPPU.Interlace)
	{
		GFX.PPL = GFX.RealPPL << 1;
		IPPU.DoubleHeightPixels = TRUE;
		IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
		GFX.DoInterlace++;
	}
	else
	{
		GFX.PPL = GFX.RealPPL;
		IPPU.DoubleHeightPixels = FALSE;
		IPPU.RenderedScreenHeight = PPU.ScreenHeight;
	}
}

void S9xBuildDirectColourMaps (void)
{
	IPPU.XB = mul_brightness[PPU.Brightness];

	for (uint32 p = 0; p < 8; p++)
	{
		for (uint32 c = 0; c < 256; c++)
		{
			uint8 r = IPPU.XB[((c & 7) << 2) | ((p & 1) << 1)];
			uint8 g = IPPU.XB[((c & 0x38) >> 1) | (p & 2)];
			uint8 b = IPPU.XB[((c & 0xc0) >> 3) | (p & 4)];
			S9xApplyColorAdjustments(r, g, b, 0x1f);
			DirectColourMaps[p][c] = BUILD_PIXEL(r, g, b);
		}
	}
}

void S9xStartScreenRefresh (void)
{
	if (GFX.DoInterlace)
		GFX.DoInterlace--;

	if (IPPU.RenderThisFrame)
	{
		if (!GFX.DoInterlace || !S9xInterlaceField())
		{
			if (!S9xInitUpdate())
			{
				IPPU.RenderThisFrame = FALSE;
				return;
			}

			S9xGraphicsScreenResize();

			IPPU.RenderedFramesCount++;
		}

		PPU.MosaicStart = 0;
		PPU.RecomputeClipWindows = TRUE;
		IPPU.PreviousLine = IPPU.CurrentLine = 0;

		memset(GFX.ZBuffer, 0, GFX.ScreenSize);
		memset(GFX.SubZBuffer, 0, GFX.ScreenSize);
	}

	if (++IPPU.FrameCount == (uint32)Memory.ROMFramesPerSecond)
	{
		IPPU.DisplayedRenderedFrameCount = IPPU.RenderedFramesCount;
		IPPU.RenderedFramesCount = 0;
		IPPU.FrameCount = 0;
	}

	if (GFX.InfoStringTimeout > 0 && --GFX.InfoStringTimeout == 0)
		GFX.InfoString.clear();

	IPPU.TotalEmulatedFrames++;
}

// Temporal frame-blend for the Game Boy display (Settings.GBFrameBlend):
//   1 = "Simple Blend"  — average the two most recent DISTINCT GB frames 50/50,
//                       fully cancelling flicker-based fake transparency.
//   2 = "LCD Blend"     — IIR decay (out = 3/8 cur + 5/8 accumulator) that
//                       mimics an LCD's slow pixel response: softer, with light
//                       ghost trails, robust without exact frame pairing.
//
// The hard part is pairing. The GB and the SNES present at slightly different
// rates, so in BIOS mode a GB frame is periodically duplicated (SGB2/DMG, GB a
// touch slower) or skipped (SGB1, GB a touch faster). Blending "this SNES
// frame" with "last SNES frame" then pairs a frame with itself (dup) or with a
// same-phase frame (skip) — neither cancels flicker, so it shows through.
//
// We can't drive this off the GB-VBlank counter alone: that counter ticks at
// SNES VBlank, but the displayed picture is composited earlier (during the
// visible scanlines), so there's a variable phase offset that makes "is this a
// fresh GB frame?" misfire by a frame. Instead:
//   * DUPLICATES are detected by comparing the freshly composited image to the
//     previous one — phase-immune and exact. On a dup we re-present the last
//     blended frame (the composite is unchanged, so there is nothing new to
//     pair with).
//   * SKIPS are detected from the GB-frame count elapsed BETWEEN two distinct
//     composites. Both samples are taken at the same point in the SNES frame,
//     so the phase offset cancels: a delta of 1 is a normal step, >=2 means a
//     GB frame was dropped. On a skip we hold the last blended frame and adopt
//     the current image as the next pair partner so the following frame
//     realigns to opposite phase.
// BIOS-less mode is frame-locked 1:1 by RunFrame, so every frame is a clean
// distinct step here.
// GB picture placement: centered in the frame — (48,40) inside the 256x224
// SGB composite (BIOS mode), (0,0) in the borderless 160x144 frame.
#define GB_BLEND_W 160
#define GB_BLEND_H 144

static void S9xBlendGameBoyFrames (void)
{
	static uint16 prev[SNES_WIDTH * SNES_HEIGHT_EXTENDED];     // raw composite of last DISTINCT frame
	static uint16 lastout[SNES_WIDTH * SNES_HEIGHT_EXTENDED];  // last presented blended frame
	static uint8  prevLayer[GB_BLEND_W * GB_BLEND_H];          // GB layer map of last distinct frame
	static uint32 prevW = 0, prevH = 0;
	static uint32 gbPrev = 0;
	static uint32 dupRun = 0;
	static bool   primed = false;

	// Suppress blending during fast-forward: the GB races many frames per
	// displayed frame, so pairing/skip-holding stutters. Present raw frames and
	// resume the saved mode (we only gate here, never change GBFrameBlend) once
	// fast-forward is released.
	if (!Settings.GBFrameBlend ||
	    !(Settings.SuperGameBoy || Settings.SGB_BIOSModeActive) ||
	    Settings.TurboMode)
	{
		primed = false;
		return;
	}

	const uint32 w = IPPU.RenderedScreenWidth;
	const uint32 h = IPPU.RenderedScreenHeight;
	if (w == 0 || h == 0 || w > SNES_WIDTH || h > SNES_HEIGHT_EXTENDED)
	{
		primed = false;
		return;
	}

	const uint32 pitch = GFX.RealPPL;
	const uint32 gbNow = S9xSGBGetGBFrameCount();
	const uint8  mode  = Settings.GBFrameBlend;

	// Layer-selective blend (Settings.GBFrameBlendLayer): 0=all, 1=background
	// (BG), 2=window, 3=sprites. S9xSGBGetGBLayerMask() gives the per-pixel
	// source layer (0=BG, 1=window, 2=OBJ) for the 160x144 GB picture.
	//   * BG/window flicker alternates COLOR (the pixel stays that layer), so we
	//     blend a pixel only when it is the selected layer in BOTH this and the
	//     previous frame — flicker cancels while moving sprites and the trails
	//     they vacate stay crisp.
	//   * Sprite flicker-transparency alternates PRESENCE (object drawn every
	//     other frame), so "sprites" blends when OBJ appears in EITHER frame.
	const uint8  layerSel = Settings.GBFrameBlendLayer;
	const uint8 *curLayer = (layerSel != GB_BLEND_LAYER_ALL) ? S9xSGBGetGBLayerMask() : NULL;
	const bool   useLayer = curLayer && w >= GB_BLEND_W && h >= GB_BLEND_H;
	const uint32 gbX = useLayer ? (w - GB_BLEND_W) / 2 : 0;
	const uint32 gbY = useLayer ? (h - GB_BLEND_H) / 2 : 0;

	// In BIOS mode the GB is held in reset during the boot/reset splash (control
	// bit 7 clear) and produces no frames while the BIOS animates its own logo.
	// That animation must NOT be blended (it stutters), so treat "GB not
	// released" as not running. BIOS-less mode has no such line — the GB runs as
	// soon as it is loaded, so the gate is bypassed there.
	const bool gbRunning = !Settings.SGB_BIOSModeActive || S9xSGBBIOSGBIsReleased();

	// (Re)prime — and pass the raw image straight through — on the first frame, a
	// resolution change, a counter reset (game reset / savestate load), or while
	// the GB is not running (boot/reset splash). Blending resumes from the next
	// real gameplay frame.
	if (!primed || w != prevW || h != prevH || gbNow < gbPrev || !gbRunning)
	{
		for (uint32 y = 0; y < h; y++)
		{
			const uint16 *line = GFX.Screen + y * pitch;
			uint16 *p = prev + y * w, *o = lastout + y * w;
			for (uint32 x = 0; x < w; x++)
				p[x] = o[x] = line[x];
		}
		if (curLayer) memcpy(prevLayer, curLayer, sizeof prevLayer);
		prevW = w; prevH = h; gbPrev = gbNow; dupRun = 0; primed = gbRunning;
		return;
	}

	// GB frames elapsed in THIS SNES frame (gbNow and gbPrev both sampled here,
	// so the counter's phase offset cancels): 0/1 under SGB2/DMG, occasionally 2
	// under SGB1 (the dropped frame). Measured per SNES frame — not since the
	// last distinct composite — so a long static stretch can't masquerade as a
	// skip when motion resumes.
	const uint32 gbDelta = gbNow - gbPrev;
	gbPrev = gbNow;

	// Duplicate detection: is the freshly composited image identical to the
	// previous distinct one? (Assumes the surrounding border/HUD is static
	// frame-to-frame, which holds for the flicker games this targets.)
	bool identical = true;
	for (uint32 y = 0; y < h && identical; y++)
	{
		const uint16 *line = GFX.Screen + y * pitch;
		const uint16 *p    = prev + y * w;
		for (uint32 x = 0; x < w; x++)
			if (line[x] != p[x]) { identical = false; break; }
	}

	if (identical)
	{
		// A lone duplicate is the GB<->SNES refresh beat re-showing a flicker
		// frame: re-present the last blended frame so the flicker stays
		// cancelled. But once the picture has held still for a couple of frames
		// it is genuinely static (a game title/menu, or a scene that has
		// settled), not a flicker phase — continuing to show the stale blend
		// freezes a "ghost" of whatever preceded it (e.g. doubled title text).
		// So converge to the raw still image, matching the crisp look of
		// blend-off on static screens.
		if (++dupRun >= 2)
		{
			for (uint32 y = 0; y < h; y++)
			{
				const uint16 *line = GFX.Screen + y * pitch;
				uint16 *p = prev + y * w, *o = lastout + y * w;
				for (uint32 x = 0; x < w; x++)
					p[x] = o[x] = line[x];
			}
			if (curLayer) memcpy(prevLayer, curLayer, sizeof prevLayer);
		}
		else
		{
			for (uint32 y = 0; y < h; y++)
			{
				uint16 *line = GFX.Screen + y * pitch;
				const uint16 *o = lastout + y * w;
				for (uint32 x = 0; x < w; x++)
					line[x] = o[x];
			}
		}
		return;
	}

	// First changed frame after the picture was static (e.g. game title -> play,
	// or a scene cut): don't cross-fade the new content with the stale still
	// image — present it raw and start a fresh pairing. dupRun still holds the
	// just-ended static run here; gameplay/flicker never reach 2 consecutive
	// identical frames, so this only fires leaving a genuinely static screen.
	const bool wasStatic = dupRun >= 2;
	dupRun = 0;
	if (wasStatic)
	{
		for (uint32 y = 0; y < h; y++)
		{
			const uint16 *line = GFX.Screen + y * pitch;
			uint16 *p = prev + y * w, *o = lastout + y * w;
			for (uint32 x = 0; x < w; x++)
				p[x] = o[x] = line[x];
		}
		if (curLayer) memcpy(prevLayer, curLayer, sizeof prevLayer);
		return;
	}

	// Skip: a GB frame was dropped, so the current image is the same flicker
	// phase as our pair partner. Blending would not cancel — hold the last good
	// frame and adopt the current image as the next partner to realign phase.
	if (gbDelta >= 2)
	{
		for (uint32 y = 0; y < h; y++)
		{
			uint16 *line = GFX.Screen + y * pitch;
			uint16 *p = prev + y * w;
			const uint16 *o = lastout + y * w;
			for (uint32 x = 0; x < w; x++)
			{
				p[x]    = line[x];
				line[x] = o[x];
			}
		}
		if (curLayer) memcpy(prevLayer, curLayer, sizeof prevLayer);
		return;
	}

	// Normal distinct frame: blend.
	for (uint32 y = 0; y < h; y++)
	{
		uint16 *line = GFX.Screen + y * pitch;
		uint16 *p = prev + y * w, *o = lastout + y * w;

		// Per-row layer pointers (only the GB picture rows carry a layer map;
		// the surrounding border is treated as background).
		const bool   yIn = useLayer && y >= gbY && y < gbY + GB_BLEND_H;
		const uint8 *clr = yIn ? curLayer  + (y - gbY) * GB_BLEND_W : NULL;
		const uint8 *plr = yIn ? prevLayer + (y - gbY) * GB_BLEND_W : NULL;

		for (uint32 x = 0; x < w; x++)
		{
			const uint16 cur = line[x];

			// Decide whether THIS pixel's layer is selected for blending.
			bool blendPixel = true;
			if (useLayer)
			{
				uint8 cl = 0, pl = 0;   // border defaults to background (0)
				if (yIn && x >= gbX && x < gbX + GB_BLEND_W)
				{
					const uint32 gi = x - gbX;
					cl = clr[gi];
					pl = plr[gi];
				}
				if (layerSel == GB_BLEND_LAYER_SPRITES)
					blendPixel = (cl == GB_PIXEL_OBJ || pl == GB_PIXEL_OBJ);  // OBJ in either frame
				else
				{
					const uint8 want = (layerSel == GB_BLEND_LAYER_WINDOW) ? GB_PIXEL_WINDOW : GB_PIXEL_BG;
					blendPixel = (cl == want && pl == want);                  // that layer in both frames
				}
			}

			if (blendPixel)
			{
				uint32 rc, gc, bc, ra, ga, ba;
				DECOMPOSE_PIXEL(cur,  rc, gc, bc);
				if (mode == GB_BLEND_LCD)
				{
					DECOMPOSE_PIXEL(o[x], ra, ga, ba);   // accumulator
					const uint16 out = (uint16) BUILD_PIXEL((rc * 3 + ra * 5) >> 3,
					                                        (gc * 3 + ga * 5) >> 3,
					                                        (bc * 3 + ba * 5) >> 3);
					line[x] = out;
					o[x]    = out;
				}
				else
				{
					DECOMPOSE_PIXEL(p[x], ra, ga, ba);   // previous distinct frame
					const uint16 out = (uint16) BUILD_PIXEL((rc + ra) >> 1,
					                                        (gc + ga) >> 1,
					                                        (bc + ba) >> 1);
					line[x] = out;
					o[x]    = out;
				}
			}
			else
			{
				// Excluded layer: leave the raw pixel (line[x] is already cur)
				// and resync the accumulator so it carries no stale ghost.
				o[x] = cur;
			}
			p[x] = cur;
		}
	}
	if (curLayer) memcpy(prevLayer, curLayer, sizeof prevLayer);
}

void S9xEndScreenRefresh (void)
{
	if (IPPU.RenderThisFrame)
	{
		FLUSH_REDRAW();

		// SGB BIOS-mode custom-border overlay. FLUSH_REDRAW above
		// finalized the SNES PPU output (including the SGB2 BIOS's
		// default device-frame border) into GFX.Screen. If the loaded
		// GB game uploaded a custom border via PCT_TRN/CHR_TRN — which
		// our packet decoder captures even in BIOS mode — replace the
		// outer ring with the captured border. Runs BEFORE
		// S9xDisplayMessages so the snes9x OSD still draws on top.
		// No-op until both halves of CHR_TRN + PCT_TRN have arrived.
		if (Settings.SGB_BIOSModeActive)
			S9xSGBOverlayBiosBorder(GFX.Screen, GFX.RealPPL);

		S9xBlendGameBoyFrames();

		if (GFX.DoInterlace && S9xInterlaceField() == 0)
		{
			S9xControlEOF();
			S9xContinueUpdate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
		}
		else
		{
			if (IPPU.ColorsChanged)
			{
				uint32 saved = PPU.CGDATA[0];
				IPPU.ColorsChanged = FALSE;
				PPU.CGDATA[0] = saved;
			}

			S9xControlEOF();

			if (Settings.TakeScreenshot)
				S9xDoScreenshot(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);

			if (Settings.AutoDisplayMessages)
				S9xDisplayMessages(GFX.Screen, GFX.RealPPL, IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight, 1);

			S9xDeinitUpdate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
		}
	}
	else
		S9xControlEOF();

	S9xUpdateCheatsInMemory ();

#ifdef DEBUGGER
	if (CPU.Flags & FRAME_ADVANCE_FLAG)
	{
		if (ICPU.FrameAdvanceCount)
		{
			ICPU.FrameAdvanceCount--;
			IPPU.RenderThisFrame = TRUE;
			IPPU.FrameSkip = 0;
		}
		else
		{
			CPU.Flags &= ~FRAME_ADVANCE_FLAG;
			CPU.Flags |= DEBUG_MODE_FLAG;
		}
	}
#endif

	if (S9xSGBTakeSramDirty())
		CPU.SRAMModified = TRUE;

	if (CPU.SRAMModified)
	{
		if (!CPU.AutoSaveTimer)
		{
			if (!(CPU.AutoSaveTimer = Settings.AutoSaveDelay * Memory.ROMFramesPerSecond))
				CPU.SRAMModified = FALSE;
		}
		else
		{
			if (!--CPU.AutoSaveTimer)
			{
				S9xAutoSaveSRAM();
				CPU.SRAMModified = FALSE;
			}
		}
	}
}

void RenderLine (uint8 C)
{
	if (IPPU.RenderThisFrame)
	{
		LineData[C].BG[0].VOffset = PPU.BG[0].VOffset + 1;
		LineData[C].BG[0].HOffset = PPU.BG[0].HOffset;
		LineData[C].BG[1].VOffset = PPU.BG[1].VOffset + 1;
		LineData[C].BG[1].HOffset = PPU.BG[1].HOffset;

		if (PPU.BGMode == 7)
		{
			struct SLineMatrixData *p = &LineMatrixData[C];
			p->MatrixA = PPU.MatrixA;
			p->MatrixB = PPU.MatrixB;
			p->MatrixC = PPU.MatrixC;
			p->MatrixD = PPU.MatrixD;
			p->CentreX = PPU.CentreX;
			p->CentreY = PPU.CentreY;
			p->M7HOFS  = PPU.M7HOFS;
			p->M7VOFS  = PPU.M7VOFS;
		}
		else
		{
			LineData[C].BG[2].VOffset = PPU.BG[2].VOffset + 1;
			LineData[C].BG[2].HOffset = PPU.BG[2].HOffset;
			LineData[C].BG[3].VOffset = PPU.BG[3].VOffset + 1;
			LineData[C].BG[3].HOffset = PPU.BG[3].HOffset;
		}

		IPPU.CurrentLine = C + 1;
	}
	else
	{
		// if we're not rendering this frame, we still need to update this
		// XXX: Check ForceBlank? Or anything else?
		if (IPPU.OBJChanged)
			SetupOBJ();
		PPU.RangeTimeOver |= GFX.OBJLines[C].RTOFlags;
	}
}

static inline void RenderScreen (bool8 sub)
{
	uint8	BGActive;
	int		D;

	if (!sub)
	{
		GFX.S = GFX.Screen;
		if (GFX.DoInterlace && S9xInterlaceField())
			GFX.S += GFX.RealPPL;
		GFX.DB = GFX.ZBuffer;
		GFX.Clip = IPPU.Clip[0];
		BGActive = Memory.FillRAM[0x212c] & ~Settings.BG_Forced;
		D = 32;
	}
	else
	{
		GFX.S = GFX.SubScreen;
		GFX.DB = GFX.SubZBuffer;
		GFX.Clip = IPPU.Clip[1];
		BGActive = Memory.FillRAM[0x212d] & ~Settings.BG_Forced;
		D = (Memory.FillRAM[0x2130] & 2) << 4; // 'do math' depth flag
	}

	if (BGActive & 0x10)
	{
		BG.TileAddress = PPU.OBJNameBase;
		BG.NameSelect = PPU.OBJNameSelect;
		BG.EnableMath = !sub && (Memory.FillRAM[0x2131] & 0x10);
		BG.StartPalette = 128;
		S9xSelectTileConverter(4, FALSE, sub, FALSE);
		S9xSelectTileRenderers(PPU.BGMode, sub, TRUE);
		DrawOBJS(D + 4);
	}

	BG.NameSelect = 0;
	S9xSelectTileRenderers(PPU.BGMode, sub, FALSE);

	#define DO_BG(n, pal, depth, hires, offset, Zh, Zl, voffoff) \
		if (BGActive & (1 << n)) \
		{ \
			BG.StartPalette = pal; \
			BG.EnableMath = !sub && (Memory.FillRAM[0x2131] & (1 << n)); \
			BG.TileSizeH = (!hires && PPU.BG[n].BGSize) ? 16 : 8; \
			BG.TileSizeV = (PPU.BG[n].BGSize) ? 16 : 8; \
			S9xSelectTileConverter(depth, hires, sub, PPU.BGMosaic[n]); \
			\
			if (offset) \
			{ \
				BG.OffsetSizeH = (!hires && PPU.BG[2].BGSize) ? 16 : 8; \
				BG.OffsetSizeV = (PPU.BG[2].BGSize) ? 16 : 8; \
				\
				if (PPU.BGMosaic[n] && (hires || PPU.Mosaic > 1)) \
					DrawBackgroundOffsetMosaic(n, D + Zh, D + Zl, voffoff); \
				else \
					DrawBackgroundOffset(n, D + Zh, D + Zl, voffoff); \
			} \
			else \
			{ \
				if (PPU.BGMosaic[n] && (hires || PPU.Mosaic > 1)) \
					DrawBackgroundMosaic(n, D + Zh, D + Zl); \
				else \
					DrawBackground(n, D + Zh, D + Zl); \
			} \
		}

	switch (PPU.BGMode)
	{
		case 0:
			DO_BG(0,  0, 2, FALSE, FALSE, 15, 11, 0);
			DO_BG(1, 32, 2, FALSE, FALSE, 14, 10, 0);
			DO_BG(2, 64, 2, FALSE, FALSE,  7,  3, 0);
			DO_BG(3, 96, 2, FALSE, FALSE,  6,  2, 0);
			break;

		case 1:
			DO_BG(0,  0, 4, FALSE, FALSE, 15, 11, 0);
			DO_BG(1,  0, 4, FALSE, FALSE, 14, 10, 0);
			DO_BG(2,  0, 2, FALSE, FALSE, (PPU.BG3Priority ? 17 : 7), 3, 0);
			break;

		case 2:
			DO_BG(0,  0, 4, FALSE, TRUE,  15,  7, 8);
			DO_BG(1,  0, 4, FALSE, TRUE,  11,  3, 8);
			break;

		case 3:
			DO_BG(0,  0, 8, FALSE, FALSE, 15,  7, 0);
			DO_BG(1,  0, 4, FALSE, FALSE, 11,  3, 0);
			break;

		case 4:
			DO_BG(0,  0, 8, FALSE, TRUE,  15,  7, 0);
			DO_BG(1,  0, 2, FALSE, TRUE,  11,  3, 0);
			break;

		case 5:
			DO_BG(0,  0, 4, TRUE,  FALSE, 15,  7, 0);
			DO_BG(1,  0, 2, TRUE,  FALSE, 11,  3, 0);
			break;

		case 6:
			DO_BG(0,  0, 4, TRUE,  TRUE,  15,  7, 8);
			break;

		case 7:
			if (BGActive & 0x01)
			{
				BG.EnableMath = !sub && (Memory.FillRAM[0x2131] & 1);
				DrawBackgroundMode7(0, GFX.DrawMode7BG1Math, GFX.DrawMode7BG1Nomath, D);
			}

			if ((Memory.FillRAM[0x2133] & 0x40) && (BGActive & 0x02))
			{
				BG.EnableMath = !sub && (Memory.FillRAM[0x2131] & 2);
				DrawBackgroundMode7(1, GFX.DrawMode7BG2Math, GFX.DrawMode7BG2Nomath, D);
			}

			break;
	}

	#undef DO_BG

	BG.EnableMath = !sub && (Memory.FillRAM[0x2131] & 0x20);

	DrawBackdrop();
}

void S9xUpdateScreen (void)
{
	if (IPPU.OBJChanged || IPPU.InterlaceOBJ)
		SetupOBJ();

	// XXX: Check ForceBlank? Or anything else?
	PPU.RangeTimeOver |= GFX.OBJLines[GFX.EndY].RTOFlags;

	GFX.StartY = IPPU.PreviousLine;
	if ((GFX.EndY = IPPU.CurrentLine - 1) >= PPU.ScreenHeight)
		GFX.EndY = PPU.ScreenHeight - 1;

	if (!PPU.ForcedBlanking)
	{
		// If force blank, may as well completely skip all this. We only did
		// the OBJ because (AFAWK) the RTO flags are updated even during force-blank.

		if (PPU.RecomputeClipWindows)
		{
			S9xComputeClipWindows();
			PPU.RecomputeClipWindows = FALSE;
		}

		if (!IPPU.DoubleWidthPixels && (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.PseudoHires))
		{
			// Have to back out of the regular speed hack
			for (uint32 y = 0; y < GFX.StartY; y++)
			{
				uint16	*p = GFX.Screen + y * GFX.PPL + 255;
				uint16	*q = GFX.Screen + y * GFX.PPL + 510;

				for (int x = 255; x >= 0; x--, p--, q -= 2)
					*q = *(q + 1) = *p;
			}

			IPPU.DoubleWidthPixels = TRUE;
			IPPU.RenderedScreenWidth = 512;
		}

		if (!IPPU.DoubleHeightPixels && IPPU.Interlace && (PPU.BGMode == 5 || PPU.BGMode == 6))
		{
			IPPU.DoubleHeightPixels = TRUE;
			IPPU.RenderedScreenHeight = PPU.ScreenHeight << 1;
			GFX.PPL = GFX.RealPPL << 1;
			GFX.DoInterlace = 2;

			for (int32 y = (int32) GFX.StartY - 2; y >= 0; y--)
				memmove(GFX.Screen + (y + 1) * GFX.PPL, GFX.Screen + y * GFX.RealPPL, GFX.PPL * sizeof(uint16));
		}

		if ((Memory.FillRAM[0x2130] & 0x30) != 0x30 && (Memory.FillRAM[0x2131] & 0x3f))
			GFX.FixedColour = BUILD_PIXEL(IPPU.XB[PPU.FixedColourRed], IPPU.XB[PPU.FixedColourGreen], IPPU.XB[PPU.FixedColourBlue]);

		if (PPU.BGMode == 5 || PPU.BGMode == 6 || IPPU.PseudoHires ||
			((Memory.FillRAM[0x2130] & 0x30) != 0x30 && (Memory.FillRAM[0x2130] & 2) && (Memory.FillRAM[0x2131] & 0x3f) && (Memory.FillRAM[0x212d] & 0x1f)))
			// If hires (Mode 5/6 or pseudo-hires) or math is to be done
			// involving the subscreen, then we need to render the subscreen...
			RenderScreen(TRUE);

		RenderScreen(FALSE);
	}
	else
	{
		const uint16	black = BUILD_PIXEL(0, 0, 0);

		GFX.S = GFX.Screen + GFX.StartY * GFX.PPL;
		if (GFX.DoInterlace && S9xInterlaceField())
			GFX.S += GFX.RealPPL;

		for (uint32 l = GFX.StartY; l <= GFX.EndY; l++, GFX.S += GFX.PPL)
			for (int x = 0; x < IPPU.RenderedScreenWidth; x++)
				GFX.S[x] = black;
	}

	IPPU.PreviousLine = IPPU.CurrentLine;
}

static void SetupOBJ (void)
{
	int	SmallWidth, SmallHeight, LargeWidth, LargeHeight;

	switch (PPU.OBJSizeSelect)
	{
		case 0:
			SmallWidth = SmallHeight = 8;
			LargeWidth = LargeHeight = 16;
			break;

		case 1:
			SmallWidth = SmallHeight = 8;
			LargeWidth = LargeHeight = 32;
			break;

		case 2:
			SmallWidth = SmallHeight = 8;
			LargeWidth = LargeHeight = 64;
			break;

		case 3:
			SmallWidth = SmallHeight = 16;
			LargeWidth = LargeHeight = 32;
			break;

		case 4:
			SmallWidth = SmallHeight = 16;
			LargeWidth = LargeHeight = 64;
			break;

		case 5:
		default:
			SmallWidth = SmallHeight = 32;
			LargeWidth = LargeHeight = 64;
			break;

		case 6:
			SmallWidth = 16; SmallHeight = 32;
			LargeWidth = 32; LargeHeight = 64;
			break;

		case 7:
			SmallWidth = 16; SmallHeight = 32;
			LargeWidth = LargeHeight = 32;
			break;
	}

	int	inc = IPPU.InterlaceOBJ ? 2 : 1;

	int startline = (IPPU.InterlaceOBJ && S9xInterlaceField()) ? 1 : 0;

	// OK, we have three cases here. Either there's no priority, priority is
	// normal FirstSprite, or priority is FirstSprite+Y. The first two are
	// easy, the last is somewhat more ... interesting. So we split them up.

	int		Height;
	uint8	S;
	int sprite_limit = (Settings.MaxSpriteTilesPerLine == 128) ? 128 : 32;

	if (!PPU.OAMPriorityRotation || !(PPU.OAMFlip & PPU.OAMAddr & 1)) // normal case
	{
		uint8	LineOBJ[SNES_HEIGHT_EXTENDED];
		memset(LineOBJ, 0, sizeof(LineOBJ));

		for (int i = 0; i < SNES_HEIGHT_EXTENDED; i++)
		{
			GFX.OBJLines[i].RTOFlags = 0;
			GFX.OBJLines[i].Tiles = Settings.MaxSpriteTilesPerLine;
			for (int j = 0; j < sprite_limit; j++)
				GFX.OBJLines[i].OBJ[j].Sprite = -1;
		}

		uint8	FirstSprite = PPU.FirstSprite;
		S = FirstSprite;

		do
		{
			if (PPU.OBJ[S].Size)
			{
				GFX.OBJWidths[S] = LargeWidth;
				Height = LargeHeight;
			}
			else
			{
				GFX.OBJWidths[S] = SmallWidth;
				Height = SmallHeight;
			}

			int	HPos = PPU.OBJ[S].HPos;
			if (HPos == -256)
				HPos = 0;

			if (HPos > -GFX.OBJWidths[S] && HPos <= 256)
			{
				if (HPos < 0)
					GFX.OBJVisibleTiles[S] = (GFX.OBJWidths[S] + HPos + 7) >> 3;
				else if (HPos + GFX.OBJWidths[S] > 255)
					GFX.OBJVisibleTiles[S] = (256 - HPos + 7) >> 3;
				else
					GFX.OBJVisibleTiles[S] = GFX.OBJWidths[S] >> 3;

				for (uint8 line = startline, Y = (uint8) (PPU.OBJ[S].VPos & 0xff); line < Height; Y++, line += inc)
				{
					if (Y >= SNES_HEIGHT_EXTENDED)
						continue;

					if (LineOBJ[Y] >= sprite_limit)
					{
						GFX.OBJLines[Y].RTOFlags |= 0x40;
						continue;
					}

					GFX.OBJLines[Y].Tiles -= GFX.OBJVisibleTiles[S];
					if (GFX.OBJLines[Y].Tiles < 0)
						GFX.OBJLines[Y].RTOFlags |= 0x80;

					GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Sprite = S;
					if (PPU.OBJ[S].VFlip)
						// Yes, Width not Height. It so happens that the
						// sprites with H=2*W flip as two WxW sprites.
						GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Line = line ^ (GFX.OBJWidths[S] - 1);
					else
						GFX.OBJLines[Y].OBJ[LineOBJ[Y]].Line = line;

					LineOBJ[Y]++;
				}
			}

			S = (S + 1) & 0x7f;
		} while (S != FirstSprite);

		for (int Y = 1; Y < SNES_HEIGHT_EXTENDED; Y++)
			GFX.OBJLines[Y].RTOFlags |= GFX.OBJLines[Y - 1].RTOFlags;
	}
	else // evil FirstSprite+Y case
	{
		// First, find out which sprites are on which lines
		uint8 OBJOnLine[SNES_HEIGHT_EXTENDED][128];
		// memset(OBJOnLine, 0, sizeof(OBJOnLine));
		/* Hold on here, that's a lot of bytes to initialise at once!
		 * So we only initialise them per line, as needed. [Neb]
		 * Bonus: We can quickly avoid looping if a line has no OBJs.
		 */
        bool8 AnyOBJOnLine[SNES_HEIGHT_EXTENDED];
        memset(AnyOBJOnLine, FALSE, sizeof(AnyOBJOnLine)); // better

		for (S = 0; S < 128; S++)
		{
			if (PPU.OBJ[S].Size)
			{
				GFX.OBJWidths[S] = LargeWidth;
				Height = LargeHeight;
			}
			else
			{
				GFX.OBJWidths[S] = SmallWidth;
				Height = SmallHeight;
			}

			int	HPos = PPU.OBJ[S].HPos;
			if (HPos == -256)
				HPos = 256;

			if (HPos > -GFX.OBJWidths[S] && HPos <= 256)
			{
				if (HPos < 0)
					GFX.OBJVisibleTiles[S] = (GFX.OBJWidths[S] + HPos + 7) >> 3;
				else if (HPos + GFX.OBJWidths[S] >= 257)
					GFX.OBJVisibleTiles[S] = (257 - HPos + 7) >> 3;
				else
					GFX.OBJVisibleTiles[S] = GFX.OBJWidths[S] >> 3;

				for (uint8 line = startline, Y = (uint8) (PPU.OBJ[S].VPos & 0xff); line < Height; Y++, line += inc)
				{
					if (Y >= SNES_HEIGHT_EXTENDED)
						continue;

					if (!AnyOBJOnLine[Y]) {
						memset(OBJOnLine[Y], 0, sizeof(OBJOnLine[Y]));
						AnyOBJOnLine[Y] = TRUE;
					}

					if (PPU.OBJ[S].VFlip)
						// Yes, Width not Height. It so happens that the
						// sprites with H=2*W flip as two WxW sprites.
						OBJOnLine[Y][S] = (line ^ (GFX.OBJWidths[S] - 1)) | 0x80;
					else
						OBJOnLine[Y][S] = line | 0x80;
				}
			}
		}

		// Now go through and pull out those OBJ that are actually visible.
		int	j;
		for (int Y = 0; Y < SNES_HEIGHT_EXTENDED; Y++)
		{
			GFX.OBJLines[Y].RTOFlags = Y ? GFX.OBJLines[Y - 1].RTOFlags : 0;
			GFX.OBJLines[Y].Tiles = Settings.MaxSpriteTilesPerLine;

			uint8	FirstSprite = (PPU.FirstSprite + Y) & 0x7f;
			S = FirstSprite;
			j = 0;

			if (AnyOBJOnLine[Y])
			{
				do
				{
					if (OBJOnLine[Y][S])
					{
						if (j >= sprite_limit)
						{
							GFX.OBJLines[Y].RTOFlags |= 0x40;
							break;
						}

						GFX.OBJLines[Y].Tiles -= GFX.OBJVisibleTiles[S];
						if (GFX.OBJLines[Y].Tiles < 0)
							GFX.OBJLines[Y].RTOFlags |= 0x80;
						GFX.OBJLines[Y].OBJ[j].Sprite = S;
						GFX.OBJLines[Y].OBJ[j++].Line = OBJOnLine[Y][S] & ~0x80;
					}

					S = (S + 1) & 0x7f;
				} while (S != FirstSprite);
			}

			if (j < sprite_limit)
				GFX.OBJLines[Y].OBJ[j].Sprite = -1;
		}
	}

	IPPU.OBJChanged = FALSE;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize ("no-tree-vrp")
#endif
static void DrawOBJS (int D)
{
	void (*DrawTile) (uint32, uint32, uint32, uint32) = NULL;
	void (*DrawClippedTile) (uint32, uint32, uint32, uint32, uint32, uint32) = NULL;

	int	PixWidth = IPPU.DoubleWidthPixels ? 2 : 1;
	BG.InterlaceLine = S9xInterlaceField() ? 8 : 0;
	GFX.Z1 = 2;
	int sprite_limit = (Settings.MaxSpriteTilesPerLine == 128) ? 128 : 32;

	for (uint32 Y = GFX.StartY, Offset = Y * GFX.PPL; Y <= GFX.EndY; Y++, Offset += GFX.PPL)
	{
		int	I = 0;
		int	tiles = GFX.OBJLines[Y].Tiles;

		for (int S = GFX.OBJLines[Y].OBJ[I].Sprite; S >= 0 && I < sprite_limit; S = GFX.OBJLines[Y].OBJ[++I].Sprite)
		{
			tiles += GFX.OBJVisibleTiles[S];
			if (tiles <= 0)
				continue;

			int	BaseTile = (((GFX.OBJLines[Y].OBJ[I].Line << 1) + (PPU.OBJ[S].Name & 0xf0)) & 0xf0) | (PPU.OBJ[S].Name & 0x100) | (PPU.OBJ[S].Palette << 10);
			int	TileX = PPU.OBJ[S].Name & 0x0f;
			int	TileLine = (GFX.OBJLines[Y].OBJ[I].Line & 7) * 8;
			int	TileInc = 1;

			if (PPU.OBJ[S].HFlip)
			{
				TileX = (TileX + (GFX.OBJWidths[S] >> 3) - 1) & 0x0f;
				BaseTile |= H_FLIP;
				TileInc = -1;
			}

			GFX.Z2 = D + PPU.OBJ[S].Priority * 4;

			int	DrawMode = 3;
			int	clip = 0, next_clip = -1000;
			int	X = PPU.OBJ[S].HPos;
			if (X == -256)
				X = 256;

			for (int t = tiles, O = Offset + X * PixWidth; X <= 256 && X < PPU.OBJ[S].HPos + GFX.OBJWidths[S]; TileX = (TileX + TileInc) & 0x0f, X += 8, O += 8 * PixWidth)
			{
				if (X < -7 || --t < 0 || X == 256)
					continue;

				for (int x = X; x < X + 8;)
				{
					if (x >= next_clip)
					{
						for (; clip < GFX.Clip[4].Count && GFX.Clip[4].Left[clip] <= x; clip++) ;
						if (clip == 0 || x >= GFX.Clip[4].Right[clip - 1])
						{
							DrawMode = 0;
							next_clip = ((clip < GFX.Clip[4].Count) ? GFX.Clip[4].Left[clip] : 1000);
						}
						else
						{
							DrawMode = GFX.Clip[4].DrawMode[clip - 1];
							next_clip = GFX.Clip[4].Right[clip - 1];
							GFX.ClipColors = !(DrawMode & 1);

							if (BG.EnableMath && (PPU.OBJ[S].Palette & 4) && (DrawMode & 2))
							{
								DrawTile = GFX.DrawTileMath;
								DrawClippedTile = GFX.DrawClippedTileMath;
							}
							else
							{
								DrawTile = GFX.DrawTileNomath;
								DrawClippedTile = GFX.DrawClippedTileNomath;
							}
						}
					}

					if (x == X && x + 8 < next_clip)
					{
						if (DrawMode)
							DrawTile(BaseTile | TileX, O, TileLine, 1);
						x += 8;
					}
					else
					{
						int	w = (next_clip <= X + 8) ? next_clip - x : X + 8 - x;
						if (DrawMode)
							DrawClippedTile(BaseTile | TileX, O, x - X, w, TileLine, 1);
						x += w;
					}
				}
			}
		}
	}
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

static void DrawBackground (int bg, uint8 Zh, uint8 Zl)
{
	BG.TileAddress = PPU.BG[bg].NameBase << 1;

	uint32	Tile;
	uint16	*SC0, *SC1, *SC2, *SC3;

	SC0 = (uint16 *) &Memory.VRAM[PPU.BG[bg].SCBase << 1];
	SC1 = (PPU.BG[bg].SCSize & 1) ? SC0 + 1024 : SC0;
	if (SC1 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC1 -= 0x8000;
	SC2 = (PPU.BG[bg].SCSize & 2) ? SC1 + 1024 : SC0;
	if (SC2 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC2 -= 0x8000;
	SC3 = (PPU.BG[bg].SCSize & 1) ? SC2 + 1024 : SC2;
	if (SC3 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC3 -= 0x8000;

	uint32	Lines;
	int		OffsetMask  = (BG.TileSizeH == 16) ? 0x3ff : 0x1ff;
	int		OffsetShift = (BG.TileSizeV == 16) ? 4 : 3;
	int		PixWidth = IPPU.DoubleWidthPixels ? 2 : 1;
	bool8	HiresInterlace = IPPU.Interlace && IPPU.DoubleWidthPixels;

	void (*DrawTile) (uint32, uint32, uint32, uint32);
	void (*DrawClippedTile) (uint32, uint32, uint32, uint32, uint32, uint32);

	for (int clip = 0; clip < GFX.Clip[bg].Count; clip++)
	{
		GFX.ClipColors = !(GFX.Clip[bg].DrawMode[clip] & 1);

		if (BG.EnableMath && (GFX.Clip[bg].DrawMode[clip] & 2))
		{
			DrawTile = GFX.DrawTileMath;
			DrawClippedTile = GFX.DrawClippedTileMath;
		}
		else
		{
			DrawTile = GFX.DrawTileNomath;
			DrawClippedTile = GFX.DrawClippedTileNomath;
		}

		for (uint32 Y = GFX.StartY; Y <= GFX.EndY; Y += Lines)
		{
			uint32	Y2 = HiresInterlace ? Y * 2 + S9xInterlaceField() : Y;
			uint32	VOffset = LineData[Y].BG[bg].VOffset + (HiresInterlace ? 1 : 0);
			uint32	HOffset = LineData[Y].BG[bg].HOffset;
			int		VirtAlign = ((Y2 + VOffset) & 7) >> (HiresInterlace ? 1 : 0);

			for (Lines = 1; Lines < GFX.LinesPerTile - VirtAlign; Lines++)
			{
				if ((VOffset != LineData[Y + Lines].BG[bg].VOffset) || (HOffset != LineData[Y + Lines].BG[bg].HOffset))
					break;
			}

			if (Y + Lines > GFX.EndY)
				Lines = GFX.EndY - Y + 1;

			VirtAlign <<= 3;

			uint32	t1, t2;
			uint32	TilemapRow = (VOffset + Y2) >> OffsetShift;
			BG.InterlaceLine = ((VOffset + Y2) & 1) << 3;

			if ((VOffset + Y2) & 8)
			{
				t1 = 16;
				t2 = 0;
			}
			else
			{
				t1 = 0;
				t2 = 16;
			}

			uint16	*b1, *b2;

			if (TilemapRow & 0x20)
			{
				b1 = SC2;
				b2 = SC3;
			}
			else
			{
				b1 = SC0;
				b2 = SC1;
			}

			b1 += (TilemapRow & 0x1f) << 5;
			b2 += (TilemapRow & 0x1f) << 5;

			uint32	Left   = GFX.Clip[bg].Left[clip];
			uint32	Right  = GFX.Clip[bg].Right[clip];
			uint32	Offset = Left * PixWidth + Y * GFX.PPL;
			uint32	HPos   = (HOffset + Left) & OffsetMask;
			uint32	HTile  = HPos >> 3;
			uint16	*t;

			if (BG.TileSizeH == 8)
			{
				if (HTile > 31)
					t = b2 + (HTile & 0x1f);
				else
					t = b1 + HTile;
			}
			else
			{
				if (HTile > 63)
					t = b2 + ((HTile >> 1) & 0x1f);
				else
					t = b1 + (HTile >> 1);
			}

			uint32	Width = Right - Left;

			if (HPos & 7)
			{
				uint32	l = HPos & 7;
				uint32	w = 8 - l;
				if (w > Width)
					w = Width;

				Offset -= l * PixWidth;
				Tile = READ_WORD(t);
				GFX.Z1 = GFX.Z2 = (Tile & 0x2000) ? Zh : Zl;

				if (BG.TileSizeV == 16)
					Tile = TILE_PLUS(Tile, ((Tile & V_FLIP) ? t2 : t1));

				if (BG.TileSizeH == 8)
				{
					DrawClippedTile(Tile, Offset, l, w, VirtAlign, Lines);
					t++;
					if (HTile == 31)
						t = b2;
					else
					if (HTile == 63)
						t = b1;
				}
				else
				{
					if (!(Tile & H_FLIP))
						DrawClippedTile(TILE_PLUS(Tile, (HTile & 1)), Offset, l, w, VirtAlign, Lines);
					else
						DrawClippedTile(TILE_PLUS(Tile, 1 - (HTile & 1)), Offset, l, w, VirtAlign, Lines);
					t += HTile & 1;
					if (HTile == 63)
						t = b2;
					else
					if (HTile == 127)
						t = b1;
				}

				HTile++;
				Offset += 8 * PixWidth;
				Width -= w;
			}

			while (Width >= 8)
			{
				Tile = READ_WORD(t);
				GFX.Z1 = GFX.Z2 = (Tile & 0x2000) ? Zh : Zl;

				if (BG.TileSizeV == 16)
					Tile = TILE_PLUS(Tile, ((Tile & V_FLIP) ? t2 : t1));

				if (BG.TileSizeH == 8)
				{
					DrawTile(Tile, Offset, VirtAlign, Lines);
					t++;
					if (HTile == 31)
						t = b2;
					else
					if (HTile == 63)
						t = b1;
				}
				else
				{
					if (!(Tile & H_FLIP))
						DrawTile(TILE_PLUS(Tile, (HTile & 1)), Offset, VirtAlign, Lines);
					else
						DrawTile(TILE_PLUS(Tile, 1 - (HTile & 1)), Offset, VirtAlign, Lines);
					t += HTile & 1;
					if (HTile == 63)
						t = b2;
					else
					if (HTile == 127)
						t = b1;
				}

				HTile++;
				Offset += 8 * PixWidth;
				Width -= 8;
			}

			if (Width)
			{
				Tile = READ_WORD(t);
				GFX.Z1 = GFX.Z2 = (Tile & 0x2000) ? Zh : Zl;

				if (BG.TileSizeV == 16)
					Tile = TILE_PLUS(Tile, ((Tile & V_FLIP) ? t2 : t1));

				if (BG.TileSizeH == 8)
					DrawClippedTile(Tile, Offset, 0, Width, VirtAlign, Lines);
				else
				{
					if (!(Tile & H_FLIP))
						DrawClippedTile(TILE_PLUS(Tile, (HTile & 1)), Offset, 0, Width, VirtAlign, Lines);
					else
						DrawClippedTile(TILE_PLUS(Tile, 1 - (HTile & 1)), Offset, 0, Width, VirtAlign, Lines);
				}
			}
		}
	}
}

static void DrawBackgroundMosaic (int bg, uint8 Zh, uint8 Zl)
{
	BG.TileAddress = PPU.BG[bg].NameBase << 1;

	uint32	Tile;
	uint16	*SC0, *SC1, *SC2, *SC3;

	SC0 = (uint16 *) &Memory.VRAM[PPU.BG[bg].SCBase << 1];
	SC1 = (PPU.BG[bg].SCSize & 1) ? SC0 + 1024 : SC0;
	if (SC1 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC1 -= 0x8000;
	SC2 = (PPU.BG[bg].SCSize & 2) ? SC1 + 1024 : SC0;
	if (SC2 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC2 -= 0x8000;
	SC3 = (PPU.BG[bg].SCSize & 1) ? SC2 + 1024 : SC2;
	if (SC3 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC3 -= 0x8000;

	int	Lines;
	int	OffsetMask  = (BG.TileSizeH == 16) ? 0x3ff : 0x1ff;
	int	OffsetShift = (BG.TileSizeV == 16) ? 4 : 3;
	int	PixWidth = IPPU.DoubleWidthPixels ? 2 : 1;
	bool8	HiresInterlace = IPPU.Interlace && IPPU.DoubleWidthPixels;

	void (*DrawPix) (uint32, uint32, uint32, uint32, uint32, uint32);

	int	MosaicStart = ((uint32) GFX.StartY - PPU.MosaicStart) % PPU.Mosaic;

	for (int clip = 0; clip < GFX.Clip[bg].Count; clip++)
	{
		GFX.ClipColors = !(GFX.Clip[bg].DrawMode[clip] & 1);

		if (BG.EnableMath && (GFX.Clip[bg].DrawMode[clip] & 2))
			DrawPix = GFX.DrawMosaicPixelMath;
		else
			DrawPix = GFX.DrawMosaicPixelNomath;

		for (uint32 Y = GFX.StartY - MosaicStart; Y <= GFX.EndY; Y += PPU.Mosaic)
		{
			uint32	Y2 = HiresInterlace ? Y * 2 : Y;
			uint32	VOffset = LineData[Y + MosaicStart].BG[bg].VOffset + (HiresInterlace ? 1 : 0);
			uint32	HOffset = LineData[Y + MosaicStart].BG[bg].HOffset;

			Lines = PPU.Mosaic - MosaicStart;
			if (Y + MosaicStart + Lines > GFX.EndY)
				Lines = GFX.EndY - Y - MosaicStart + 1;

			int	VirtAlign = (((Y2 + VOffset) & 7) >> (HiresInterlace ? 1 : 0)) << 3;

			uint32	t1, t2;
			uint32	TilemapRow = (VOffset + Y2) >> OffsetShift;
			BG.InterlaceLine = ((VOffset + Y2) & 1) << 3;

			if ((VOffset + Y2) & 8)
			{
				t1 = 16;
				t2 = 0;
			}
			else
			{
				t1 = 0;
				t2 = 16;
			}

			uint16	*b1, *b2;

			if (TilemapRow & 0x20)
			{
				b1 = SC2;
				b2 = SC3;
			}
			else
			{
				b1 = SC0;
				b2 = SC1;
			}

			b1 += (TilemapRow & 0x1f) << 5;
			b2 += (TilemapRow & 0x1f) << 5;

			uint32	Left   = GFX.Clip[bg].Left[clip];
			uint32	Right  = GFX.Clip[bg].Right[clip];
			uint32	Offset = Left * PixWidth + (Y + MosaicStart) * GFX.PPL;
			uint32	HPos   = (HOffset + Left - (Left % PPU.Mosaic)) & OffsetMask;
			uint32	HTile  = HPos >> 3;
			uint16	*t;

			if (BG.TileSizeH == 8)
			{
				if (HTile > 31)
					t = b2 + (HTile & 0x1f);
				else
					t = b1 + HTile;
			}
			else
			{
				if (HTile > 63)
					t = b2 + ((HTile >> 1) & 0x1f);
				else
					t = b1 + (HTile >> 1);
			}

			uint32	Width = Right - Left;

			HPos &= 7;

			while (Left < Right)
			{
				uint32	w = PPU.Mosaic - (Left % PPU.Mosaic);
				if (w > Width)
					w = Width;

				Tile = READ_WORD(t);
				GFX.Z1 = GFX.Z2 = (Tile & 0x2000) ? Zh : Zl;

				if (BG.TileSizeV == 16)
					Tile = TILE_PLUS(Tile, ((Tile & V_FLIP) ? t2 : t1));

				if (BG.TileSizeH == 8)
					DrawPix(Tile, Offset, VirtAlign, HPos & 7, w, Lines);
				else
				{
					if (!(Tile & H_FLIP))
						DrawPix(TILE_PLUS(Tile, (HTile & 1)), Offset, VirtAlign, HPos & 7, w, Lines);
					else
						DrawPix(TILE_PLUS(Tile, 1 - (HTile & 1)), Offset, VirtAlign, HPos & 7, w, Lines);
				}

				HPos += PPU.Mosaic;

				while (HPos >= 8)
				{
					HPos -= 8;

					if (BG.TileSizeH == 8)
					{
						t++;
						if (HTile == 31)
							t = b2;
						else
						if (HTile == 63)
							t = b1;
					}
					else
					{
						t += HTile & 1;
						if (HTile == 63)
							t = b2;
						else
						if (HTile == 127)
							t = b1;
					}

					HTile++;
				}

				Offset += w * PixWidth;
				Width -= w;
				Left += w;
			}

			MosaicStart = 0;
		}
	}
}

static void DrawBackgroundOffset (int bg, uint8 Zh, uint8 Zl, int VOffOff)
{
	BG.TileAddress = PPU.BG[bg].NameBase << 1;

	uint32	Tile;
	uint16	*SC0, *SC1, *SC2, *SC3;
	uint16	*BPS0, *BPS1, *BPS2, *BPS3;

	BPS0 = (uint16 *) &Memory.VRAM[PPU.BG[2].SCBase << 1];
	BPS1 = (PPU.BG[2].SCSize & 1) ? BPS0 + 1024 : BPS0;
	if (BPS1 >= (uint16 *) (Memory.VRAM + 0x10000))
		BPS1 -= 0x8000;
	BPS2 = (PPU.BG[2].SCSize & 2) ? BPS1 + 1024 : BPS0;
	if (BPS2 >= (uint16 *) (Memory.VRAM + 0x10000))
		BPS2 -= 0x8000;
	BPS3 = (PPU.BG[2].SCSize & 1) ? BPS2 + 1024 : BPS2;
	if (BPS3 >= (uint16 *) (Memory.VRAM + 0x10000))
		BPS3 -= 0x8000;

	SC0 = (uint16 *) &Memory.VRAM[PPU.BG[bg].SCBase << 1];
	SC1 = (PPU.BG[bg].SCSize & 1) ? SC0 + 1024 : SC0;
	if (SC1 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC1 -= 0x8000;
	SC2 = (PPU.BG[bg].SCSize & 2) ? SC1 + 1024 : SC0;
	if (SC2 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC2 -= 0x8000;
	SC3 = (PPU.BG[bg].SCSize & 1) ? SC2 + 1024 : SC2;
	if (SC3 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC3 -= 0x8000;

	int	OffsetMask   = (BG.TileSizeH   == 16) ? 0x3ff : 0x1ff;
	int	OffsetShift  = (BG.TileSizeV   == 16) ? 4 : 3;
	int	Offset2Mask  = (BG.OffsetSizeH == 16) ? 0x3ff : 0x1ff;
	int	Offset2Shift = (BG.OffsetSizeV == 16) ? 4 : 3;
	int	OffsetEnableMask = 0x2000 << bg;
	int	PixWidth = IPPU.DoubleWidthPixels ? 2 : 1;
	bool8	HiresInterlace = IPPU.Interlace && IPPU.DoubleWidthPixels;

	void (*DrawClippedTile) (uint32, uint32, uint32, uint32, uint32, uint32);

	for (int clip = 0; clip < GFX.Clip[bg].Count; clip++)
	{
		GFX.ClipColors = !(GFX.Clip[bg].DrawMode[clip] & 1);

		if (BG.EnableMath && (GFX.Clip[bg].DrawMode[clip] & 2))
		{
			DrawClippedTile = GFX.DrawClippedTileMath;
		}
		else
		{
			DrawClippedTile = GFX.DrawClippedTileNomath;
		}

		for (uint32 Y = GFX.StartY; Y <= GFX.EndY; Y++)
		{
			uint32	Y2 = HiresInterlace ? Y * 2 + S9xInterlaceField() : Y;
			uint32	VOff = LineData[Y].BG[2].VOffset - 1;
			uint32	HOff = LineData[Y].BG[2].HOffset;
			uint32	HOffsetRow = VOff >> Offset2Shift;
			uint32	VOffsetRow = (VOff + VOffOff) >> Offset2Shift;
			uint16	*s, *s1, *s2;

			if (HOffsetRow & 0x20)
			{
				s1 = BPS2;
				s2 = BPS3;
			}
			else
			{
				s1 = BPS0;
				s2 = BPS1;
			}

			s1 += (HOffsetRow & 0x1f) << 5;
			s2 += (HOffsetRow & 0x1f) << 5;
			s = ((VOffsetRow & 0x20) ? BPS2 : BPS0) + ((VOffsetRow & 0x1f) << 5);
			int32	VOffsetOffset = s - s1;

			uint32	Left  = GFX.Clip[bg].Left[clip];
			uint32	Right = GFX.Clip[bg].Right[clip];
			uint32	Offset = Left * PixWidth + Y * GFX.PPL;
			uint32	HScroll = LineData[Y].BG[bg].HOffset;
			bool8	left_edge = (Left < (8 - (HScroll & 7)));
			uint32	Width = Right - Left;

			while (Left < Right)
			{
				uint32	VOffset, HOffset;

				if (left_edge)
				{
					// SNES cannot do OPT for leftmost tile column
					VOffset = LineData[Y].BG[bg].VOffset;
					HOffset = HScroll;
					left_edge = FALSE;
				}
				else
				{
					int HOffTile = ((HOff + Left - 1) & Offset2Mask) >> 3;

					if (BG.OffsetSizeH == 8)
					{
						if (HOffTile > 31)
							s = s2 + (HOffTile & 0x1f);
						else
							s = s1 + HOffTile;
					}
					else
					{
						if (HOffTile > 63)
							s = s2 + ((HOffTile >> 1) & 0x1f);
						else
							s = s1 + (HOffTile >> 1);
					}

					uint16	HCellOffset = READ_WORD(s);
					uint16	VCellOffset;

					if (VOffOff)
						VCellOffset = READ_WORD(s + VOffsetOffset);
					else
					{
						if (HCellOffset & 0x8000)
						{
							VCellOffset = HCellOffset;
							HCellOffset = 0;
						}
						else
							VCellOffset = 0;
					}

					if (VCellOffset & OffsetEnableMask)
						VOffset = VCellOffset + 1;
					else
						VOffset = LineData[Y].BG[bg].VOffset;

					if (HCellOffset & OffsetEnableMask)
						HOffset = (HCellOffset & ~7) | (HScroll & 7);
					else
						HOffset = HScroll;
				}

				if (HiresInterlace)
					VOffset++;

				uint32	t1, t2;
				int		VirtAlign = (((Y2 + VOffset) & 7) >> (HiresInterlace ? 1 : 0)) << 3;
				int		TilemapRow = (VOffset + Y2) >> OffsetShift;
				BG.InterlaceLine = ((VOffset + Y2) & 1) << 3;

				if ((VOffset + Y2) & 8)
				{
					t1 = 16;
					t2 = 0;
				}
				else
				{
					t1 = 0;
					t2 = 16;
				}

				uint16	*b1, *b2;

				if (TilemapRow & 0x20)
				{
					b1 = SC2;
					b2 = SC3;
				}
				else
				{
					b1 = SC0;
					b2 = SC1;
				}

				b1 += (TilemapRow & 0x1f) << 5;
				b2 += (TilemapRow & 0x1f) << 5;

				uint32	HPos = (HOffset + Left) & OffsetMask;
				uint32	HTile = HPos >> 3;
				uint16	*t;

				if (BG.TileSizeH == 8)
				{
					if (HTile > 31)
						t = b2 + (HTile & 0x1f);
					else
						t = b1 + HTile;
				}
				else
				{
					if (HTile > 63)
						t = b2 + ((HTile >> 1) & 0x1f);
					else
						t = b1 + (HTile >> 1);
				}

				uint32	l = HPos & 7;
				uint32	w = 8 - l;
				if (w > Width)
					w = Width;

				Offset -= l * PixWidth;
				Tile = READ_WORD(t);
				GFX.Z1 = GFX.Z2 = (Tile & 0x2000) ? Zh : Zl;

				if (BG.TileSizeV == 16)
					Tile = TILE_PLUS(Tile, ((Tile & V_FLIP) ? t2 : t1));

				if (BG.TileSizeH == 8)
				{
					DrawClippedTile(Tile, Offset, l, w, VirtAlign, 1);
				}
				else
				{
					if (!(Tile & H_FLIP))
						DrawClippedTile(TILE_PLUS(Tile, (HTile & 1)), Offset, l, w, VirtAlign, 1);
					else
						DrawClippedTile(TILE_PLUS(Tile, 1 - (HTile & 1)), Offset, l, w, VirtAlign, 1);
				}

				Left += w;
				Offset += 8 * PixWidth;
				Width -= w;
			}
		}
	}
}

static void DrawBackgroundOffsetMosaic (int bg, uint8 Zh, uint8 Zl, int VOffOff)
{
	BG.TileAddress = PPU.BG[bg].NameBase << 1;

	uint32	Tile;
	uint16	*SC0, *SC1, *SC2, *SC3;
	uint16	*BPS0, *BPS1, *BPS2, *BPS3;

	BPS0 = (uint16 *) &Memory.VRAM[PPU.BG[2].SCBase << 1];
	BPS1 = (PPU.BG[2].SCSize & 1) ? BPS0 + 1024 : BPS0;
	if (BPS1 >= (uint16 *) (Memory.VRAM + 0x10000))
		BPS1 -= 0x8000;
	BPS2 = (PPU.BG[2].SCSize & 2) ? BPS1 + 1024 : BPS0;
	if (BPS2 >= (uint16 *) (Memory.VRAM + 0x10000))
		BPS2 -= 0x8000;
	BPS3 = (PPU.BG[2].SCSize & 1) ? BPS2 + 1024 : BPS2;
	if (BPS3 >= (uint16 *) (Memory.VRAM + 0x10000))
		BPS3 -= 0x8000;

	SC0 = (uint16 *) &Memory.VRAM[PPU.BG[bg].SCBase << 1];
	SC1 = (PPU.BG[bg].SCSize & 1) ? SC0 + 1024 : SC0;
	if (SC1 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC1 -= 0x8000;
	SC2 = (PPU.BG[bg].SCSize & 2) ? SC1 + 1024 : SC0;
	if (SC2 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC2 -= 0x8000;
	SC3 = (PPU.BG[bg].SCSize & 1) ? SC2 + 1024 : SC2;
	if (SC3 >= (uint16 *) (Memory.VRAM + 0x10000))
		SC3 -= 0x8000;

	int	Lines;
	int	OffsetMask   = (BG.TileSizeH   == 16) ? 0x3ff : 0x1ff;
	int	OffsetShift  = (BG.TileSizeV   == 16) ? 4 : 3;
	int	Offset2Shift = (BG.OffsetSizeV == 16) ? 4 : 3;
	int	OffsetEnableMask = 0x2000 << bg;
	int	PixWidth = IPPU.DoubleWidthPixels ? 2 : 1;
	bool8	HiresInterlace = IPPU.Interlace && IPPU.DoubleWidthPixels;

	void (*DrawPix) (uint32, uint32, uint32, uint32, uint32, uint32);

	int	MosaicStart = ((uint32) GFX.StartY - PPU.MosaicStart) % PPU.Mosaic;

	for (int clip = 0; clip < GFX.Clip[bg].Count; clip++)
	{
		GFX.ClipColors = !(GFX.Clip[bg].DrawMode[clip] & 1);

		if (BG.EnableMath && (GFX.Clip[bg].DrawMode[clip] & 2))
			DrawPix = GFX.DrawMosaicPixelMath;
		else
			DrawPix = GFX.DrawMosaicPixelNomath;

		for (uint32 Y = GFX.StartY - MosaicStart; Y <= GFX.EndY; Y += PPU.Mosaic)
		{
			uint32	Y2 = HiresInterlace ? Y * 2 : Y;
			uint32	VOff = LineData[Y + MosaicStart].BG[2].VOffset - 1;
			uint32	HOff = LineData[Y + MosaicStart].BG[2].HOffset;

			Lines = PPU.Mosaic - MosaicStart;
			if (Y + MosaicStart + Lines > GFX.EndY)
				Lines = GFX.EndY - Y - MosaicStart + 1;

			uint32	HOffsetRow = VOff >> Offset2Shift;
			uint32	VOffsetRow = (VOff + VOffOff) >> Offset2Shift;
			uint16	*s, *s1, *s2;

			if (HOffsetRow & 0x20)
			{
				s1 = BPS2;
				s2 = BPS3;
			}
			else
			{
				s1 = BPS0;
				s2 = BPS1;
			}

			s1 += (HOffsetRow & 0x1f) << 5;
			s2 += (HOffsetRow & 0x1f) << 5;
			s = ((VOffsetRow & 0x20) ? BPS2 : BPS0) + ((VOffsetRow & 0x1f) << 5);
			int32	VOffsetOffset = s - s1;

			uint32	Left =  GFX.Clip[bg].Left[clip];
			uint32	Right = GFX.Clip[bg].Right[clip];
			uint32	Offset = Left * PixWidth + (Y + MosaicStart) * GFX.PPL;
			uint32	HScroll = LineData[Y + MosaicStart].BG[bg].HOffset;
			uint32	Width = Right - Left;

			while (Left < Right)
			{
				uint32	VOffset, HOffset;

				if (Left < (8 - (HScroll & 7)))
				{
					// SNES cannot do OPT for leftmost tile column
					VOffset = LineData[Y + MosaicStart].BG[bg].VOffset;
					HOffset = HScroll;
				}
				else
				{
					int HOffTile = (((Left + (HScroll & 7)) - 8) + (HOff & ~7)) >> 3;

					if (BG.OffsetSizeH == 8)
					{
						if (HOffTile > 31)
							s = s2 + (HOffTile & 0x1f);
						else
							s = s1 + HOffTile;
					}
					else
					{
						if (HOffTile > 63)
							s = s2 + ((HOffTile >> 1) & 0x1f);
						else
							s = s1 + (HOffTile >> 1);
					}

					uint16	HCellOffset = READ_WORD(s);
					uint16	VCellOffset;

					if (VOffOff)
						VCellOffset = READ_WORD(s + VOffsetOffset);
					else
					{
						if (HCellOffset & 0x8000)
						{
							VCellOffset = HCellOffset;
							HCellOffset = 0;
						}
						else
							VCellOffset = 0;
					}

					if (VCellOffset & OffsetEnableMask)
						VOffset = VCellOffset + 1;
					else
						VOffset = LineData[Y + MosaicStart].BG[bg].VOffset;

					if (HCellOffset & OffsetEnableMask)
						HOffset = (HCellOffset & ~7) | (HScroll & 7);
					else
						HOffset = HScroll;
				}

				if (HiresInterlace)
					VOffset++;

				uint32	t1, t2;
				int		VirtAlign = (((Y2 + VOffset) & 7) >> (HiresInterlace ? 1 : 0)) << 3;
				int		TilemapRow = (VOffset + Y2) >> OffsetShift;
				BG.InterlaceLine = ((VOffset + Y2) & 1) << 3;

				if ((VOffset + Y2) & 8)
				{
					t1 = 16;
					t2 = 0;
				}
				else
				{
					t1 = 0;
					t2 = 16;
				}

				uint16	*b1, *b2;

				if (TilemapRow & 0x20)
				{
					b1 = SC2;
					b2 = SC3;
				}
				else
				{
					b1 = SC0;
					b2 = SC1;
				}

				b1 += (TilemapRow & 0x1f) << 5;
				b2 += (TilemapRow & 0x1f) << 5;

				uint32	HPos = (HOffset + Left - (Left % PPU.Mosaic)) & OffsetMask;
				uint32	HTile = HPos >> 3;
				uint16	*t;

				if (BG.TileSizeH == 8)
				{
					if (HTile > 31)
						t = b2 + (HTile & 0x1f);
					else
						t = b1 + HTile;
				}
				else
				{
					if (HTile > 63)
						t = b2 + ((HTile >> 1) & 0x1f);
					else
						t = b1 + (HTile >> 1);
				}

				uint32	w = PPU.Mosaic - (Left % PPU.Mosaic);
				if (w > Width)
					w = Width;

				Tile = READ_WORD(t);
				GFX.Z1 = GFX.Z2 = (Tile & 0x2000) ? Zh : Zl;

				if (BG.TileSizeV == 16)
					Tile = TILE_PLUS(Tile, ((Tile & V_FLIP) ? t2 : t1));

				if (BG.TileSizeH == 8)
					DrawPix(Tile, Offset, VirtAlign, HPos & 7, w, Lines);
				else
				{
					if (!(Tile & H_FLIP))
						DrawPix(TILE_PLUS(Tile, (HTile & 1)), Offset, VirtAlign, HPos & 7, w, Lines);
					else
					if (!(Tile & V_FLIP))
						DrawPix(TILE_PLUS(Tile, 1 - (HTile & 1)), Offset, VirtAlign, HPos & 7, w, Lines);
				}

				Left += w;
				Offset += w * PixWidth;
				Width -= w;
			}

			MosaicStart = 0;
		}
	}
}

static inline void DrawBackgroundMode7 (int bg, void (*DrawMath) (uint32, uint32, int), void (*DrawNomath) (uint32, uint32, int), int D)
{
	for (int clip = 0; clip < GFX.Clip[bg].Count; clip++)
	{
		GFX.ClipColors = !(GFX.Clip[bg].DrawMode[clip] & 1);

		if (BG.EnableMath && (GFX.Clip[bg].DrawMode[clip] & 2))
			DrawMath(GFX.Clip[bg].Left[clip], GFX.Clip[bg].Right[clip], D);
		else
			DrawNomath(GFX.Clip[bg].Left[clip], GFX.Clip[bg].Right[clip], D);
	}
}

static inline void DrawBackdrop (void)
{
	uint32	Offset = GFX.StartY * GFX.PPL;

	for (int clip = 0; clip < GFX.Clip[5].Count; clip++)
	{
		GFX.ClipColors = !(GFX.Clip[5].DrawMode[clip] & 1);

		if (BG.EnableMath && (GFX.Clip[5].DrawMode[clip] & 2))
			GFX.DrawBackdropMath(Offset, GFX.Clip[5].Left[clip], GFX.Clip[5].Right[clip]);
		else
			GFX.DrawBackdropNomath(Offset, GFX.Clip[5].Left[clip], GFX.Clip[5].Right[clip]);
	}
}

void S9xReRefresh (void)
{
	// Be careful when calling this function from the thread other than the emulation one...
	// Here it's assumed no drawing occurs from the emulation thread when Settings.Paused is TRUE.
	if (Settings.Paused)
		S9xDeinitUpdate(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);
}

void S9xSetInfoString (const char *string)
{
	if (Settings.InitialInfoStringTimeout > 0)
	{
		GFX.InfoString = string;
		GFX.InfoStringTimeout = Settings.InitialInfoStringTimeout;
		S9xReRefresh();
	}
}

#include "var8x10font.h"
static const int font_width = 8;
static const int font_height = 10;

static inline int CharWidth(uint8 c)
{
	return font_width - var8x10font_kern[c - 32][0] - var8x10font_kern[c - 32][1];
}

static int StringWidth(const char* str)
{
	int length = strlen(str);
	int pixcount = 0;

	if (length > 0)
		pixcount++;

	for (int i = 0; i < length; i++)
	{
		pixcount += (CharWidth(str[i]) - 1);
	}

	return pixcount;
}

static void VariableDisplayChar(int x, int y, uint8 c, bool monospace = false, int overlap = 0)
{
	int cindex = c - 32;
	int crow = cindex >> 4;
	int ccol = cindex & 15;
	int cwidth = font_width - (monospace ? 0 : (var8x10font_kern[cindex][0] + var8x10font_kern[cindex][1]));

	int	line = crow * font_height;
	int	offset = ccol * font_width + (monospace ? 0 : var8x10font_kern[cindex][0]);
	int scale = IPPU.RenderedScreenWidth / SNES_WIDTH;

	uint16* s = GFX.Screen + y * GFX.RealPPL + x * scale;

	for (int h = 0; h < font_height; h++, line++, s += GFX.RealPPL - cwidth * scale)
	{
		for (int w = 0; w < cwidth; w++, s++)
		{
			if (var8x10font[line][offset + w] == '#')
				*s = Settings.DisplayColor;
			else if (var8x10font[line][offset + w] == '.')
				*s = 0x0000;
			//            else if (!monospace && w >= overlap)
			//                *s = (*s & 0xf7de) >> 1;
			//                *s = (*s & 0xe79c) >> 2;

			if (scale > 1)
			{
				s[1] = s[0];
				s++;
			}
		}
	}
}

void S9xVariableDisplayString(const char* string, int linesFromBottom,	int pixelsFromLeft, bool allowWrap, int type)
{
	if (GFX.ScreenBuffer.empty() || IPPU.RenderedScreenWidth == 0)
		return;

	bool monospace = true;
	if (type == S9X_NO_INFO)
	{
		if (linesFromBottom <= 0)
			linesFromBottom = 1;

		if (linesFromBottom >= 5 && !Settings.DisplayPressedKeys)
		{
			if (!Settings.DisplayPressedKeys)
				linesFromBottom -= 3;
			else
				linesFromBottom -= 1;
		}

		if (pixelsFromLeft > 128)
			pixelsFromLeft = SNES_WIDTH - StringWidth(string);

		monospace = false;
	}

	int min_lines = 1;
	std::string msg(string);
	for (auto& c : msg)
		if (c == '\n')
			min_lines++;
	if (min_lines > linesFromBottom)
		linesFromBottom = min_lines;

	int dst_x = pixelsFromLeft;
	int dst_y = IPPU.RenderedScreenHeight - (font_height)*linesFromBottom;
	int len = strlen(string);

	if (IPPU.RenderedScreenHeight % 224 && !Settings.ShowOverscan)
		dst_y -= 8;
	else if (Settings.ShowOverscan)
		dst_y += 8;

	int overlap = 0;

	for (int i = 0; i < len; i++)
	{
		int cindex = (uint8)string[i] - 32;
		int char_width = font_width - (monospace ? 1 : (var8x10font_kern[cindex][0] + var8x10font_kern[cindex][1]));

		if (dst_x + char_width > SNES_WIDTH || string[i] == '\n')
		{
			if (!allowWrap)
				break;

			linesFromBottom--;
			dst_y = IPPU.RenderedScreenHeight - font_height * linesFromBottom;
			dst_x = pixelsFromLeft;

			if (dst_y >= IPPU.RenderedScreenHeight)
				break;
		}

		if (string[i] == '\n')
			continue;

		VariableDisplayChar(dst_x, dst_y, string[i], monospace, overlap);

		dst_x += char_width - 1;
		overlap = 1;
	}
}

static void DisplayStringFromBottom(const char* string, int linesFromBottom, int pixelsFromLeft, bool allowWrap)
{
	if (S9xCustomDisplayString)
	{
		S9xCustomDisplayString(string, linesFromBottom, pixelsFromLeft, allowWrap, S9X_NO_INFO);
		return;
	}

	S9xVariableDisplayString(string, linesFromBottom, pixelsFromLeft, allowWrap, S9X_NO_INFO);
}

static void S9xDisplayStringType(const char* string, int linesFromBottom, int pixelsFromLeft, bool allowWrap, int type)
{
	if (S9xCustomDisplayString)
	{
		S9xCustomDisplayString(string, linesFromBottom, pixelsFromLeft, allowWrap, type);
		return;
	}

	S9xVariableDisplayString(string, linesFromBottom, pixelsFromLeft, allowWrap, type);
}

static void DisplayTime (void)
{
	char string[10];

	time_t rawtime;
	struct tm *timeinfo;

	time (&rawtime);
	timeinfo = localtime(&rawtime);

	sprintf(string, "%02u:%02u", timeinfo->tm_hour, timeinfo->tm_min);
	S9xDisplayString(string, 0, 0, false);
}

static void DisplayFrameRate (void)
{
	char	string[10];
	static uint32 lastFrameCount = 0, calcFps = 0;
	static time_t lastTime = time(NULL);

	// With run-ahead enabled, each iteration emulates (RunAhead + 1) SNES
	// frames but only one of them is displayed. Divide both FPS metrics by
	// that multiplier so the counter reflects the logical game frame rate
	// (target ~60) instead of raw emulation throughput.
	uint32 runAheadMul = (Settings.RunAhead > 0) ? (uint32)Settings.RunAhead + 1 : 1;

	time_t currTime = time(NULL);
	if (lastTime != currTime) {
		if (lastFrameCount < IPPU.TotalEmulatedFrames) {
			calcFps = (IPPU.TotalEmulatedFrames - lastFrameCount) / (uint32)(currTime - lastTime);
		}
		lastTime = currTime;
		lastFrameCount = IPPU.TotalEmulatedFrames;
	}
	sprintf(string, "%u fps", calcFps / runAheadMul);
	S9xDisplayString(string, 2, IPPU.RenderedScreenWidth - (font_width - 1) * strlen(string) - 1, false);

	uint32 displayedRendered = IPPU.DisplayedRenderedFrameCount * runAheadMul;

#ifdef DEBUGGER
	const int	len = 8;
	sprintf(string, "%02d/%02d %02d", (int) displayedRendered, (int) Memory.ROMFramesPerSecond, (int) IPPU.FrameCount);
#else
	const int	len = 5;
	sprintf(string, "%02d/%02d",      (int) displayedRendered, (int) Memory.ROMFramesPerSecond);
#endif

	S9xDisplayString(string, 1, IPPU.RenderedScreenWidth - (font_width - 1) * len - 1, false);
}

static void DisplayFrameNumber (void)
{
	char string[32];
	sprintf(string, "#%u", (unsigned)IPPU.TotalEmulatedFrames);
	const int line = Settings.DisplayFrameRate ? 3 : 2;
	S9xDisplayString(string, line, IPPU.RenderedScreenWidth - (font_width - 1) * (int)strlen(string) - 1, false);
}

static void DisplayPressedKeys (void)
{
	static unsigned char	KeyMap[]   = { '0', '1', '2', 'R', 'L', 'X', 'A', 225, 224, 227, 226, 'S', 's', 'Y', 'B' };
	static int		KeyOrder[] = { 8, 10, 7, 9, 0, 6, 14, 13, 5, 1, 4, 3, 2, 11, 12 }; // < ^ > v   A B Y X  L R  S s

	enum controllers	controller;
    int					line = Settings.DisplayMovieFrame && S9xMovieActive() ? 2 : 1;
	int8				ids[4];
	char				string[255];

	for (int port = 0; port < 2; port++)
	{
		S9xGetController(port, &controller, &ids[0], &ids[1], &ids[2], &ids[3]);

		switch (controller)
		{
			case CTL_MOUSE:
			{
				uint8 buf[5];
				if (!MovieGetMouse(port, buf))
					break;
				int16 x = READ_WORD(buf);
				int16 y = READ_WORD(buf + 2);
				uint8 buttons = buf[4];
				sprintf(string, "#%d %d: (%03d,%03d) %c%c", port + 1, ids[0] + 1, x, y,
						(buttons & 0x40) ? 'L' : ' ', (buttons & 0x80) ? 'R' : ' ');
				S9xDisplayStringType(string, line++, 1, false, S9X_PRESSED_KEYS_INFO);
				break;
			}

			case CTL_SUPERSCOPE:
			{
				uint8 buf[6];
				if (!MovieGetScope(port, buf))
					break;
				int16 x = READ_WORD(buf);
				int16 y = READ_WORD(buf + 2);
				uint8 buttons = buf[4];
				sprintf(string, "#%d %d: (%03d,%03d) %c%c%c%c", port + 1, ids[0] + 1, x, y,
						(buttons & 0x80) ? 'F' : ' ', (buttons & 0x40) ? 'C' : ' ',
						(buttons & 0x20) ? 'T' : ' ', (buttons & 0x10) ? 'P' : ' ');
				S9xDisplayStringType(string, line++, 1, false, S9X_PRESSED_KEYS_INFO);
				break;
			}

			case CTL_JUSTIFIER:
			{
				uint8 buf[11];
				if (!MovieGetJustifier(port, buf))
					break;
				int16 x1 = READ_WORD(buf);
				int16 x2 = READ_WORD(buf + 2);
				int16 y1 = READ_WORD(buf + 4);
				int16 y2 = READ_WORD(buf + 6);
				uint8 buttons = buf[8];
				bool8 offscreen1 = buf[9];
				bool8 offscreen2 = buf[10];
				sprintf(string, "#%d %d: (%03d,%03d) %c%c%c / (%03d,%03d) %c%c%c", port + 1, ids[0] + 1,
						x1, y1, (buttons & 0x80) ? 'T' : ' ', (buttons & 0x20) ? 'S' : ' ', offscreen1 ? 'O' : ' ',
						x2, y2, (buttons & 0x40) ? 'T' : ' ', (buttons & 0x10) ? 'S' : ' ', offscreen2 ? 'O' : ' ');
				S9xDisplayStringType(string, line++, 1, false, S9X_PRESSED_KEYS_INFO);
				break;
			}

			case CTL_JOYPAD:
			{
				sprintf(string, "#%d %d:                  ", port + 1, ids[0] + 1);
				uint16 pad = MovieGetJoypad(ids[0]);
				for (int i = 0; i < 15; i++)
				{
					int j = KeyOrder[i];
					int mask = (1 << (j + 1));
					string[6 + i]= (pad & mask) ? KeyMap[j] : ' ';
				}

				S9xDisplayStringType(string, line++, 1, false, S9X_PRESSED_KEYS_INFO);
				break;
			}

			case CTL_MP5:
			{
				for (int n = 0; n < 4; n++)
				{
					if (ids[n] != -1)
					{
						sprintf(string, "#%d %d:                  ", port + 1, ids[n] + 1);
						uint16 pad = MovieGetJoypad(ids[n]);
						for (int i = 0; i < 15; i++)
						{
							int j = KeyOrder[i];
							int mask = (1 << (j + 1));
							string[6 + i]= (pad & mask) ? KeyMap[j] : ' ';
						}

						S9xDisplayStringType(string, line++, 1, false, S9X_PRESSED_KEYS_INFO);
					}
				}

				break;
			}

			case CTL_MACSRIFLE:
			{
				/*
				uint8 buf[6], *p = buf;
				MovieGetScope(port, buf);
				int16 x = READ_WORD(p);
				int16 y = READ_WORD(p + 2);
				uint8 buttons = buf[4];
				sprintf(string, "#%d %d: (%03d,%03d) %c%c%c%c", port, ids[0], x, y,
						(buttons & 0x80) ? 'F' : ' ', (buttons & 0x40) ? 'C' : ' ',
						(buttons & 0x20) ? 'T' : ' ', (buttons & 0x10) ? 'P' : ' ');
				S9xDisplayString(string, line++, 1, false);
				*/
				break;
			}

			case CTL_NONE:
			{
				// Display Nothing
				break;
			}
		}
	}
}

static void DisplayWatchedAddresses (void)
{
	for (unsigned int i = 0; i < sizeof(watches) / sizeof(watches[0]); i++)
	{
		if (!watches[i].on)
			break;

		int32	displayNumber = 0;
		char	buf[64];

		for (int r = 0; r < watches[i].size; r++)
			displayNumber += (Cheat.CWatchRAM[(watches[i].address - 0x7E0000) + r]) << (8 * r);

		if (watches[i].format == 1)
			sprintf(buf, "%s,%du = %u", watches[i].desc, watches[i].size, (unsigned int) displayNumber);
		else
		if (watches[i].format == 3)
			sprintf(buf, "%s,%dx = %X", watches[i].desc, watches[i].size, (unsigned int) displayNumber);
		else // signed
		{
			if (watches[i].size == 1)
				displayNumber = (int32) ((int8)  displayNumber);
			else if (watches[i].size == 2)
				displayNumber = (int32) ((int16) displayNumber);
			else if (watches[i].size == 3)
				if (displayNumber >= 8388608)
					displayNumber -= 16777216;

			sprintf(buf, "%s,%ds = %d", watches[i].desc, watches[i].size, (int) displayNumber);
		}

		S9xDisplayString(buf, 6 + i, 1, false);
	}
}

void S9xDisplayMessages (uint16 *screen, int ppl, int width, int height, int scale)
{
	if (Settings.DisplayTime)
		DisplayTime();

	if (Settings.DisplayFrameRate)
		DisplayFrameRate();

	if (Settings.DisplayFrameNumber)
		DisplayFrameNumber();

	if (Settings.DisplayWatchedAddresses)
		DisplayWatchedAddresses();

	if (Settings.DisplayPressedKeys)
		DisplayPressedKeys();

	if (Settings.DisplayMovieFrame && S9xMovieActive())
		S9xDisplayString(GFX.FrameDisplayString, 1, 1, false);

	if (!GFX.InfoString.empty())
		S9xDisplayString(GFX.InfoString.c_str(), 5, 1, true);
}

static uint16 get_crosshair_color (uint8 color)
{
	switch (color & 15)
	{
		case  0: return (BUILD_PIXEL( 0,  0,  0)); // transparent, shouldn't be used
		case  1: return (BUILD_PIXEL( 0,  0,  0)); // Black
		case  2: return (BUILD_PIXEL( 8,  8,  8)); // 25Grey
		case  3: return (BUILD_PIXEL(16, 16, 16)); // 50Grey
		case  4: return (BUILD_PIXEL(23, 23, 23)); // 75Grey
		case  5: return (BUILD_PIXEL(31, 31, 31)); // White
		case  6: return (BUILD_PIXEL(31,  0,  0)); // Red
		case  7: return (BUILD_PIXEL(31, 16,  0)); // Orange
		case  8: return (BUILD_PIXEL(31, 31,  0)); // Yellow
		case  9: return (BUILD_PIXEL( 0, 31,  0)); // Green
		case 10: return (BUILD_PIXEL( 0, 31, 31)); // Cyan
		case 11: return (BUILD_PIXEL( 0, 23, 31)); // Sky
		case 12: return (BUILD_PIXEL( 0,  0, 31)); // Blue
		case 13: return (BUILD_PIXEL(23,  0, 31)); // Violet
		case 14: return (BUILD_PIXEL(31,  0, 31)); // Magenta
		case 15: return (BUILD_PIXEL(31,  0, 16)); // Purple
	}

	return (0);
}

void S9xDrawCrosshair (const char *crosshair, uint8 fgcolor, uint8 bgcolor, int16 x, int16 y)
{
	if (!crosshair)
		return;

	int16	r, rx = 1, c, cx = 1, W = SNES_WIDTH, H = PPU.ScreenHeight;
	uint16	fg, bg;

	x -= 7;
	y -= 7;

	if (IPPU.DoubleWidthPixels)  { cx = 2; x *= 2; W *= 2; }
	if (IPPU.DoubleHeightPixels) { rx = 2; y *= 2; H *= 2; }

	fg = get_crosshair_color(fgcolor);
	bg = get_crosshair_color(bgcolor);

	uint16	*s = GFX.Screen + y * (int32)GFX.RealPPL + x;

	for (r = 0; r < 15 * rx; r++, s += GFX.RealPPL - 15 * cx)
	{
		if (y + r < 0)
		{
			s += 15 * cx;
			continue;
		}

		if (y + r >= H)
			break;

		for (c = 0; c < 15 * cx; c++, s++)
		{
			if (x + c < 0 || s < GFX.Screen)
				continue;

			if (x + c >= W)
			{
				s += 15 * cx - c;
				break;
			}

			uint8	p = crosshair[(r / rx) * 15 + (c / cx)];

			if (p == '#' && fgcolor)
				*s = (fgcolor & 0x10) ? COLOR_ADD::fn1_2(fg, *s) : fg;
			else
			if (p == '.' && bgcolor)
				*s = (bgcolor & 0x10) ? COLOR_ADD::fn1_2(*s, bg) : bg;
		}
	}
}

