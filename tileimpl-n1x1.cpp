/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#define _TILEIMPL_CPP_
#include "tileimpl.h"

namespace TileImpl {

	template<class MATH, class BPSTART>
	void Normal1x1Base<MATH, BPSTART>::Draw(int N, int M, uint32 Offset, uint32 OffsetInLine, uint8 Pix, uint8 Z1, uint8 Z2)
	{
		(void) OffsetInLine;
		if (Z1 > GFX.DB[Offset + N] && (M))
		{
			GFX.S[Offset + N] = MATH::Calc(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + N], GFX.SubZBuffer[Offset + N]);
			GFX.DB[Offset + N] = Z2;
		}
	}


	// normal width
	template struct Renderers<DrawTile16, Normal1x1>;
	template struct Renderers<DrawClippedTile16, Normal1x1>;
	template struct Renderers<DrawMosaicPixel16, Normal1x1>;
	template struct Renderers<DrawBackdrop16, Normal1x1>;
	template struct Renderers<DrawMode7MosaicBG1, Normal1x1>;
	template struct Renderers<DrawMode7BG1, Normal1x1>;
	template struct Renderers<DrawMode7MosaicBG2, Normal1x1>;
	template struct Renderers<DrawMode7BG2, Normal1x1>;

} // namespace TileImpl
