/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// CDirectDraw.h: interface for the CDirectDraw class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CDIRECTDRAW_H_INCLUDED)
#define CDIRECTDRAW_H_INCLUDED

#include "IS9xDisplayOutput.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#if DIRECTDRAW_SUPPORT

class CDirectDraw: public IS9xDisplayOutput
{
public:	
	HRESULT dErr;
	LPDIRECTDRAW lpDD;
	LPDIRECTDRAWCLIPPER lpDDClipper;
	LPDIRECTDRAWPALETTE lpDDPalette;

	LPDIRECTDRAWSURFACE2 lpDDSPrimary2;
	LPDIRECTDRAWSURFACE2 lpDDSOffScreen2;

	RECT SizeHistory [10];

	int width, height;
	char depth;
	bool doubleBuffered;
	bool clipped;
	bool dDinitialized;

	unsigned char *convertBuffer;
	unsigned int filterScale;

	DDPIXELFORMAT DDPixelFormat;
public:
	bool SetDisplayMode(
		int pWidth, int pHeight, int pScale,
		char pDepth, int pRefreshRate, bool pWindowed,
		bool pDoubleBuffered);
    void GetPixelFormat ();
	void DeInitialize();
	bool Initialize (HWND hWnd);

	void Render(SSurface Src);
	bool ApplyDisplayChanges(void);
	bool ChangeRenderSize(unsigned int newWidth, unsigned int newHeight);
	bool SetFullscreen(bool fullscreen);
	void SetSnes9xColorFormat();

	void EnumModes(std::vector<dMode> *modeVector);

	CDirectDraw();
	virtual ~CDirectDraw();
};

#endif

#endif // !defined(CDIRECTDRAW_H_INCLUDED)
