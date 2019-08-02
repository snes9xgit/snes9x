/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "tileimpl.h"

using namespace TileImpl;

namespace {

	uint32	pixbit[8][16];
	uint8	hrbit_odd[256];
	uint8	hrbit_even[256];

	// Here are the tile converters, selected by S9xSelectTileConverter().
	// Really, except for the definition of DOBIT and the number of times it is called, they're all the same.

	#define DOBIT(n, i) \
		if ((pix = *(tp + (n)))) \
		{ \
			p1 |= pixbit[(i)][pix >> 4]; \
			p2 |= pixbit[(i)][pix & 0xf]; \
		}

	uint8 ConvertTile2 (uint8 *pCache, uint32 TileAddr, uint32)
	{
		uint8	*tp      = &Memory.VRAM[TileAddr];
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		for (line = 8; line != 0; line--, tp += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	uint8 ConvertTile4 (uint8 *pCache, uint32 TileAddr, uint32)
	{
		uint8	*tp      = &Memory.VRAM[TileAddr];
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		for (line = 8; line != 0; line--, tp += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			DOBIT(16, 2);
			DOBIT(17, 3);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	uint8 ConvertTile8 (uint8 *pCache, uint32 TileAddr, uint32)
	{
		uint8	*tp      = &Memory.VRAM[TileAddr];
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		for (line = 8; line != 0; line--, tp += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			DOBIT(16, 2);
			DOBIT(17, 3);
			DOBIT(32, 4);
			DOBIT(33, 5);
			DOBIT(48, 6);
			DOBIT(49, 7);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	#undef DOBIT

	#define DOBIT(n, i) \
		if ((pix = hrbit_odd[*(tp1 + (n))])) \
			p1 |= pixbit[(i)][pix]; \
		if ((pix = hrbit_odd[*(tp2 + (n))])) \
			p2 |= pixbit[(i)][pix];

	uint8 ConvertTile2h_odd (uint8 *pCache, uint32 TileAddr, uint32 Tile)
	{
		uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		if (Tile == 0x3ff)
			tp2 = tp1 - (0x3ff << 4);
		else
			tp2 = tp1 + (1 << 4);

		for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	uint8 ConvertTile4h_odd (uint8 *pCache, uint32 TileAddr, uint32 Tile)
	{
		uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		if (Tile == 0x3ff)
			tp2 = tp1 - (0x3ff << 5);
		else
			tp2 = tp1 + (1 << 5);

		for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			DOBIT(16, 2);
			DOBIT(17, 3);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	#undef DOBIT

	#define DOBIT(n, i) \
		if ((pix = hrbit_even[*(tp1 + (n))])) \
			p1 |= pixbit[(i)][pix]; \
		if ((pix = hrbit_even[*(tp2 + (n))])) \
			p2 |= pixbit[(i)][pix];

	uint8 ConvertTile2h_even (uint8 *pCache, uint32 TileAddr, uint32 Tile)
	{
		uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		if (Tile == 0x3ff)
			tp2 = tp1 - (0x3ff << 4);
		else
			tp2 = tp1 + (1 << 4);

		for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	uint8 ConvertTile4h_even (uint8 *pCache, uint32 TileAddr, uint32 Tile)
	{
		uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
		uint32			*p       = (uint32 *) pCache;
		uint32			non_zero = 0;
		uint8			line;

		if (Tile == 0x3ff)
			tp2 = tp1 - (0x3ff << 5);
		else
			tp2 = tp1 + (1 << 5);

		for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
		{
			uint32			p1 = 0;
			uint32			p2 = 0;
			uint8	pix;

			DOBIT( 0, 0);
			DOBIT( 1, 1);
			DOBIT(16, 2);
			DOBIT(17, 3);
			*p++ = p1;
			*p++ = p2;
			non_zero |= p1 | p2;
		}

		return (non_zero ? TRUE : BLANK_TILE);
	}

	#undef DOBIT

} // anonymous namespace

void S9xInitTileRenderer (void)
{
	int	i;

	for (i = 0; i < 16; i++)
	{
		uint32	b = 0;

	#ifdef LSB_FIRST
		if (i & 8)
			b |= 1;
		if (i & 4)
			b |= 1 << 8;
		if (i & 2)
			b |= 1 << 16;
		if (i & 1)
			b |= 1 << 24;
	#else
		if (i & 8)
			b |= 1 << 24;
		if (i & 4)
			b |= 1 << 16;
		if (i & 2)
			b |= 1 << 8;
		if (i & 1)
			b |= 1;
	#endif

		for (uint8 bitshift = 0; bitshift < 8; bitshift++)
			pixbit[bitshift][i] = b << bitshift;
	}

	for (i = 0; i < 256; i++)
	{
		uint8	m = 0;
		uint8	s = 0;

		if (i & 0x80)
			s |= 8;
		if (i & 0x40)
			m |= 8;
		if (i & 0x20)
			s |= 4;
		if (i & 0x10)
			m |= 4;
		if (i & 0x08)
			s |= 2;
		if (i & 0x04)
			m |= 2;
		if (i & 0x02)
			s |= 1;
		if (i & 0x01)
			m |= 1;

		hrbit_odd[i]  = m;
		hrbit_even[i] = s;
	}
}

// Functions to select which converter and renderer to use.

void S9xSelectTileRenderers (int BGMode, bool8 sub, bool8 obj)
{
	void	(**DT)		(uint32, uint32, uint32, uint32);
	void	(**DCT)		(uint32, uint32, uint32, uint32, uint32, uint32);
	void	(**DMP)		(uint32, uint32, uint32, uint32, uint32, uint32);
	void	(**DB)		(uint32, uint32, uint32);
	void	(**DM7BG1)	(uint32, uint32, int);
	void	(**DM7BG2)	(uint32, uint32, int);
	bool8	M7M1, M7M2;

	M7M1 = PPU.BGMosaic[0] && PPU.Mosaic > 1;
	M7M2 = PPU.BGMosaic[1] && PPU.Mosaic > 1;

	bool8 interlace = obj ? FALSE : IPPU.Interlace;
	bool8 hires = !sub && (BGMode == 5 || BGMode == 6 || IPPU.PseudoHires);

	if (!IPPU.DoubleWidthPixels)	// normal width
	{
		DT     = Renderers<DrawTile16, Normal1x1>::Functions;
		DCT    = Renderers<DrawClippedTile16, Normal1x1>::Functions;
		DMP    = Renderers<DrawMosaicPixel16, Normal1x1>::Functions;
		DB     = Renderers<DrawBackdrop16, Normal1x1>::Functions;
		DM7BG1 = M7M1 ? Renderers<DrawMode7MosaicBG1, Normal1x1>::Functions : Renderers<DrawMode7BG1, Normal1x1>::Functions;
		DM7BG2 = M7M2 ? Renderers<DrawMode7MosaicBG2, Normal1x1>::Functions : Renderers<DrawMode7BG2, Normal1x1>::Functions;
		GFX.LinesPerTile = 8;
	}
	else if(hires)					// hires double width
	{
		if (interlace)
		{
			DT     = Renderers<DrawTile16, HiresInterlace>::Functions;
			DCT    = Renderers<DrawClippedTile16, HiresInterlace>::Functions;
			DMP    = Renderers<DrawMosaicPixel16, HiresInterlace>::Functions;
			DB     = Renderers<DrawBackdrop16, Hires>::Functions;
			DM7BG1 = M7M1 ? Renderers<DrawMode7MosaicBG1, Hires>::Functions : Renderers<DrawMode7BG1, Hires>::Functions;
			DM7BG2 = M7M2 ? Renderers<DrawMode7MosaicBG2, Hires>::Functions : Renderers<DrawMode7BG2, Hires>::Functions;
			GFX.LinesPerTile = 4;
		}
		else
		{
			DT     = Renderers<DrawTile16, Hires>::Functions;
			DCT    = Renderers<DrawClippedTile16, Hires>::Functions;
			DMP    = Renderers<DrawMosaicPixel16, Hires>::Functions;
			DB     = Renderers<DrawBackdrop16, Hires>::Functions;
			DM7BG1 = M7M1 ? Renderers<DrawMode7MosaicBG1, Hires>::Functions : Renderers<DrawMode7BG1, Hires>::Functions;
			DM7BG2 = M7M2 ? Renderers<DrawMode7MosaicBG2, Hires>::Functions : Renderers<DrawMode7BG2, Hires>::Functions;
			GFX.LinesPerTile = 8;
		}
	}
	else							// normal double width
	{
		if (interlace)
		{
			DT     = Renderers<DrawTile16, Interlace>::Functions;
			DCT    = Renderers<DrawClippedTile16, Interlace>::Functions;
			DMP    = Renderers<DrawMosaicPixel16, Interlace>::Functions;
			DB     = Renderers<DrawBackdrop16, Normal2x1>::Functions;
			DM7BG1 = M7M1 ? Renderers<DrawMode7MosaicBG1, Normal2x1>::Functions : Renderers<DrawMode7BG1, Normal2x1>::Functions;
			DM7BG2 = M7M2 ? Renderers<DrawMode7MosaicBG2, Normal2x1>::Functions : Renderers<DrawMode7BG2, Normal2x1>::Functions;
			GFX.LinesPerTile = 4;
		}
		else
		{
			DT     = Renderers<DrawTile16, Normal2x1>::Functions;
			DCT    = Renderers<DrawClippedTile16, Normal2x1>::Functions;
			DMP    = Renderers<DrawMosaicPixel16, Normal2x1>::Functions;
			DB     = Renderers<DrawBackdrop16, Normal2x1>::Functions;
			DM7BG1 = M7M1 ? Renderers<DrawMode7MosaicBG1, Normal2x1>::Functions : Renderers<DrawMode7BG1, Normal2x1>::Functions;
			DM7BG2 = M7M2 ? Renderers<DrawMode7MosaicBG2, Normal2x1>::Functions : Renderers<DrawMode7BG2, Normal2x1>::Functions;
			GFX.LinesPerTile = 8;
		}
	}

	GFX.DrawTileNomath        = DT[0];
	GFX.DrawClippedTileNomath = DCT[0];
	GFX.DrawMosaicPixelNomath = DMP[0];
	GFX.DrawBackdropNomath    = DB[0];
	GFX.DrawMode7BG1Nomath    = DM7BG1[0];
	GFX.DrawMode7BG2Nomath    = DM7BG2[0];

	int	i;

	if (!Settings.Transparency)
		i = 0;
	else
	{
		i = (Memory.FillRAM[0x2131] & 0x80) ? 4 : 1;
		if (Memory.FillRAM[0x2131] & 0x40)
		{
			i++;
			if (Memory.FillRAM[0x2130] & 2)
				i++;
		}
		if (IPPU.MaxBrightness != 0xf)
		{
			if (i == 1)
				i = 7;
			else if (i == 3)
				i = 8;
		}

	}

	GFX.DrawTileMath        = DT[i];
	GFX.DrawClippedTileMath = DCT[i];
	GFX.DrawMosaicPixelMath = DMP[i];
	GFX.DrawBackdropMath    = DB[i];
	GFX.DrawMode7BG1Math    = DM7BG1[i];
	GFX.DrawMode7BG2Math    = DM7BG2[i];
}

void S9xSelectTileConverter (int depth, bool8 hires, bool8 sub, bool8 mosaic)
{
	switch (depth)
	{
		case 8:
			BG.ConvertTile      = BG.ConvertTileFlip = ConvertTile8;
			BG.Buffer           = BG.BufferFlip      = IPPU.TileCache[TILE_8BIT];
			BG.Buffered         = BG.BufferedFlip    = IPPU.TileCached[TILE_8BIT];
			BG.TileShift        = 6;
			BG.PaletteShift     = 0;
			BG.PaletteMask      = 0;
			BG.DirectColourMode = Memory.FillRAM[0x2130] & 1;

			break;

		case 4:
			if (hires)
			{
				if (sub || mosaic)
				{
					BG.ConvertTile     = ConvertTile4h_even;
					BG.Buffer          = IPPU.TileCache[TILE_4BIT_EVEN];
					BG.Buffered        = IPPU.TileCached[TILE_4BIT_EVEN];
					BG.ConvertTileFlip = ConvertTile4h_odd;
					BG.BufferFlip      = IPPU.TileCache[TILE_4BIT_ODD];
					BG.BufferedFlip    = IPPU.TileCached[TILE_4BIT_ODD];
				}
				else
				{
					BG.ConvertTile     = ConvertTile4h_odd;
					BG.Buffer          = IPPU.TileCache[TILE_4BIT_ODD];
					BG.Buffered        = IPPU.TileCached[TILE_4BIT_ODD];
					BG.ConvertTileFlip = ConvertTile4h_even;
					BG.BufferFlip      = IPPU.TileCache[TILE_4BIT_EVEN];
					BG.BufferedFlip    = IPPU.TileCached[TILE_4BIT_EVEN];
				}
			}
			else
			{
				BG.ConvertTile = BG.ConvertTileFlip = ConvertTile4;
				BG.Buffer      = BG.BufferFlip      = IPPU.TileCache[TILE_4BIT];
				BG.Buffered    = BG.BufferedFlip    = IPPU.TileCached[TILE_4BIT];
			}

			BG.TileShift        = 5;
			BG.PaletteShift     = 10 - 4;
			BG.PaletteMask      = 7 << 4;
			BG.DirectColourMode = FALSE;

			break;

		case 2:
			if (hires)
			{
				if (sub || mosaic)
				{
					BG.ConvertTile     = ConvertTile2h_even;
					BG.Buffer          = IPPU.TileCache[TILE_2BIT_EVEN];
					BG.Buffered        = IPPU.TileCached[TILE_2BIT_EVEN];
					BG.ConvertTileFlip = ConvertTile2h_odd;
					BG.BufferFlip      = IPPU.TileCache[TILE_2BIT_ODD];
					BG.BufferedFlip    = IPPU.TileCached[TILE_2BIT_ODD];
				}
				else
				{
					BG.ConvertTile     = ConvertTile2h_odd;
					BG.Buffer          = IPPU.TileCache[TILE_2BIT_ODD];
					BG.Buffered        = IPPU.TileCached[TILE_2BIT_ODD];
					BG.ConvertTileFlip = ConvertTile2h_even;
					BG.BufferFlip      = IPPU.TileCache[TILE_2BIT_EVEN];
					BG.BufferedFlip    = IPPU.TileCached[TILE_2BIT_EVEN];
				}
			}
			else
			{
				BG.ConvertTile = BG.ConvertTileFlip = ConvertTile2;
				BG.Buffer      = BG.BufferFlip      = IPPU.TileCache[TILE_2BIT];
				BG.Buffered    = BG.BufferedFlip    = IPPU.TileCached[TILE_2BIT];
			}

			BG.TileShift        = 4;
			BG.PaletteShift     = 10 - 2;
			BG.PaletteMask      = 7 << 2;
			BG.DirectColourMode = FALSE;

			break;
	}
}
