/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#define _TILEIMPL_CPP_
#include "tileimpl.h"

namespace TileImpl {

	template<class MATH, class BPSTART>
	void HiresBase<MATH, BPSTART>::Draw(int N, int M, uint32 Offset, uint32 OffsetInLine, uint8 Pix, uint8 Z1, uint8 Z2)
	{
		if (Z1 > GFX.DB[Offset + 2 * N] && (M))
		{
			GFX.S[Offset + 2 * N + 1] = MATH::Calc(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + 2 * N], GFX.SubZBuffer[Offset + 2 * N]);
			if ((OffsetInLine + 2 * N ) != (SNES_WIDTH - 1) << 1)
				GFX.S[Offset + 2 * N + 2] = MATH::Calc((GFX.ClipColors ? 0 : GFX.SubScreen[Offset + 2 * N + 2]), GFX.RealScreenColors[Pix], GFX.SubZBuffer[Offset + 2 * N]);
			if ((OffsetInLine + 2 * N) == 0 || (OffsetInLine + 2 * N) == GFX.RealPPL)
				GFX.S[Offset + 2 * N] = MATH::Calc((GFX.ClipColors ? 0 : GFX.SubScreen[Offset + 2 * N]), GFX.RealScreenColors[Pix], GFX.SubZBuffer[Offset + 2 * N]);
			GFX.DB[Offset + 2 * N] = GFX.DB[Offset + 2 * N + 1] = Z2;
		}
	}


	// hires double width
	template struct Renderers<DrawTile16, Hires>;
	template struct Renderers<DrawClippedTile16, Hires>;
	template struct Renderers<DrawMosaicPixel16, Hires>;
	template struct Renderers<DrawBackdrop16, Hires>;
	template struct Renderers<DrawMode7MosaicBG1, Hires>;
	template struct Renderers<DrawMode7BG1, Hires>;
	template struct Renderers<DrawMode7MosaicBG2, Hires>;
	template struct Renderers<DrawMode7BG2, Hires>;

	// hires double width interlace
	template struct Renderers<DrawTile16, HiresInterlace>;
	template struct Renderers<DrawClippedTile16, HiresInterlace>;
	template struct Renderers<DrawMosaicPixel16, HiresInterlace>;
	//template struct Renderers<DrawBackdrop16, Hires>;
	//template struct Renderers<DrawMode7MosaicBG1, Hires>;
	//template struct Renderers<DrawMode7BG1, Hires>;
	//template struct Renderers<DrawMode7MosaicBG2, Hires>;
	//template struct Renderers<DrawMode7BG2, Hires>;

} // namespace TileImpl
