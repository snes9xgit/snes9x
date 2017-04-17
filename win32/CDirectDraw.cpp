/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2016  BearOso,
                             OV2

  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  S-SMP emulator code used in 1.54+
  (c) Copyright 2016         byuu

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2016  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2016  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones

  Libretro port
  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

#if DIRECTDRAW_SUPPORT
#pragma comment( lib, "ddraw" )

// CDirectDraw.cpp: implementation of the CDirectDraw class.
//
//////////////////////////////////////////////////////////////////////

#include "wsnes9x.h"
#include "../snes9x.h"
#include "../gfx.h"
#include "CDirectDraw.h"
#include "win32_display.h"

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDirectDraw::CDirectDraw()
{
    lpDD = NULL;
    lpDDClipper = NULL;
    lpDDPalette = NULL;

    lpDDSPrimary2 = NULL;
    lpDDSOffScreen2 = NULL;

    width = height = -1;
    depth = -1;
    doubleBuffered = false;
	dDinitialized = false;
	convertBuffer = NULL;
	filterScale = 0;
}

CDirectDraw::~CDirectDraw()
{
    DeInitialize();
}

bool CDirectDraw::Initialize (HWND hWnd)
{
	if(dDinitialized)
		return true;

    dErr = DirectDrawCreate (NULL, &lpDD, NULL);
    if(FAILED(dErr))
        return false;

    dErr = lpDD -> CreateClipper (0, &lpDDClipper, NULL);
    if(FAILED(dErr))
        return false;

    dErr = lpDDClipper->SetHWnd (0, hWnd);
    if(FAILED(dErr))
        return false;

	if (!SetDisplayMode (GUI.FullscreenMode.width, GUI.FullscreenMode.height, max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes)), GUI.FullscreenMode.depth, GUI.FullscreenMode.rate,
		TRUE, GUI.DoubleBuffered))
    {
        MessageBox( GUI.hWnd, Languages[ GUI.Language].errModeDD, TEXT("Snes9X - DirectDraw(7)"), MB_OK | MB_ICONSTOP);
        return (false);
    }

	dDinitialized = true;

    return (true);
}

void CDirectDraw::DeInitialize()
{
    if (lpDD != NULL)
    {
        if (lpDDSPrimary2 != NULL)
        {
            lpDDSPrimary2->Release();
            lpDDSPrimary2 = NULL;
        }
        if (lpDDSOffScreen2 != NULL)
        {
            lpDDSOffScreen2->PageUnlock(0);
            lpDDSOffScreen2->Release();
            lpDDSOffScreen2 = NULL;
        }
        if (lpDDClipper != NULL)
        {
            lpDDClipper->Release();
            lpDDClipper = NULL;
        }
        if (lpDDPalette != NULL)
        {
            lpDDPalette->Release();
            lpDDPalette = NULL;
        }
        lpDD->Release();
        lpDD = NULL;
    }
	if(convertBuffer) {
		delete [] convertBuffer;
		convertBuffer = NULL;
	}
	filterScale = 0;
	dDinitialized = false;
}

bool CDirectDraw::SetDisplayMode(
		int pWidth, int pHeight, int pScale,
		char pDepth, int pRefreshRate, bool pWindowed, bool pDoubleBuffered)
{
	if(pScale < 2) pScale = 2;

    static bool BLOCK = false;
    DDSURFACEDESC ddsd;
    PALETTEENTRY PaletteEntries [256];

    if (BLOCK)
        return (false);

    BLOCK = true;

    if (pWindowed)
        pDoubleBuffered = false;

    if (pDepth == 0)
        pDepth = depth;

    if (lpDDSPrimary2 != NULL)
    {
        lpDDSPrimary2->Release();
        lpDDSPrimary2 = NULL;
    }
    if (lpDDSOffScreen2 != NULL)
    {
        lpDDSOffScreen2->PageUnlock(0);
        lpDDSOffScreen2->Release();
        lpDDSOffScreen2 = NULL;
    }
    if( lpDDPalette != NULL)
    {
        lpDDPalette->Release();
        lpDDPalette = NULL;
    }

    lpDD->FlipToGDISurface();

    if (pWindowed)
    {
        lpDD->RestoreDisplayMode();

        ZeroMemory (&ddsd, sizeof (ddsd));

        ddsd.dwSize = sizeof (ddsd);
        ddsd.dwFlags = DDSD_PIXELFORMAT;
        dErr = lpDD->GetDisplayMode (&ddsd);
	    if (FAILED(dErr))
            pDepth = 8;
        else
        {
            if (ddsd.ddpfPixelFormat.dwFlags&DDPF_RGB)
                pDepth = (char) ddsd.ddpfPixelFormat.dwRGBBitCount;
            else
                pDepth = 8;
        }
        if (pDepth == 8)
            dErr = lpDD->SetCooperativeLevel (GUI.hWnd, DDSCL_FULLSCREEN|
                                              DDSCL_EXCLUSIVE|DDSCL_ALLOWREBOOT);
        else
            dErr = lpDD->SetCooperativeLevel (GUI.hWnd, DDSCL_NORMAL|DDSCL_ALLOWREBOOT);
    }
    else
    {
        dErr = lpDD->SetCooperativeLevel (GUI.hWnd, DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWREBOOT);
		// XXX: TODO: use pRefreshRate!
        dErr = lpDD->SetDisplayMode (pWidth, pHeight, pDepth);
    }

	if (FAILED(dErr))
    {
        BLOCK = false;
        return false;
    }

    ZeroMemory (&ddsd, sizeof (ddsd));
    ddsd.dwSize = sizeof (ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	if(GUI.BilinearFilter)
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | (GUI.LocalVidMem ? DDSCAPS_LOCALVIDMEM : DDSCAPS_NONLOCALVIDMEM);
	}
	else
	{
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	}
    ddsd.dwWidth = SNES_WIDTH * pScale;
    ddsd.dwHeight = SNES_HEIGHT_EXTENDED * pScale;

    LPDIRECTDRAWSURFACE lpDDSOffScreen;
    if (FAILED(lpDD->CreateSurface (&ddsd, &lpDDSOffScreen, NULL)))
    {
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | (GUI.LocalVidMem ? DDSCAPS_NONLOCALVIDMEM : DDSCAPS_LOCALVIDMEM);
		if(!GUI.BilinearFilter || FAILED(lpDD->CreateSurface (&ddsd, &lpDDSOffScreen, NULL)))
		{
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
			if(!GUI.BilinearFilter || FAILED(lpDD->CreateSurface (&ddsd, &lpDDSOffScreen, NULL)))
			{
				BLOCK = false;
				return (false);
			}
		}
    }

    if (FAILED (lpDDSOffScreen->QueryInterface (IID_IDirectDrawSurface2,
                                                (void **)&lpDDSOffScreen2)))
    {
        lpDDSOffScreen->Release();
        BLOCK = false;
        return (false);
    }
    lpDDSOffScreen2->PageLock(0);
    lpDDSOffScreen->Release();

    ZeroMemory (&ddsd, sizeof (ddsd));
    if (pDoubleBuffered)
    {
        ddsd.dwSize = sizeof( ddsd);
        ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;
        GUI.NumFlipFrames = 3;
        ddsd.dwBackBufferCount = 2;
    }
    else
    {
        GUI.NumFlipFrames = 1;
        ddsd.dwSize = sizeof (ddsd);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    }

    LPDIRECTDRAWSURFACE lpDDSPrimary;

    dErr = lpDD->CreateSurface (&ddsd, &lpDDSPrimary, NULL);
    if( FAILED(dErr) )
    {
        if (pDoubleBuffered)
        {
            ddsd.dwBackBufferCount = 1;
            GUI.NumFlipFrames = 2;
            if (FAILED(dErr = lpDD->CreateSurface (&ddsd, &lpDDSPrimary, NULL)))
            {
                ddsd.dwFlags = DDSD_CAPS;
                ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

                pDoubleBuffered = false;
                GUI.NumFlipFrames = 1;
                dErr = lpDD->CreateSurface (&ddsd, &lpDDSPrimary, NULL);
            }
    	}

        if (FAILED(dErr))
        {
            BLOCK = false;
            lpDDSOffScreen2->PageUnlock(0);
            lpDDSOffScreen2->Release();
            lpDDSOffScreen2 = NULL;

            return (false);
        }
    }

    ZeroMemory (&DDPixelFormat, sizeof (DDPixelFormat));
    DDPixelFormat.dwSize = sizeof (DDPixelFormat);
    lpDDSPrimary->GetPixelFormat (&DDPixelFormat);

	clipped = true;
    if((!pWindowed && pDoubleBuffered) || FAILED(lpDDSPrimary->SetClipper( lpDDClipper)))
		clipped = false;

    if (FAILED (lpDDSPrimary->QueryInterface (IID_IDirectDrawSurface2, (void **)&lpDDSPrimary2)))
    {
        BLOCK = false;
        lpDDSPrimary->Release();
        lpDDSPrimary = NULL;

        return (FALSE);
    }

    lpDDSPrimary->Release();
    lpDDSPrimary = NULL;

    if((!pWindowed && pDoubleBuffered) || FAILED(lpDDSPrimary2->SetClipper( lpDDClipper)))
		clipped = false;

    if (pDepth == 8)
    {
        dErr = lpDD->CreatePalette (DDPCAPS_8BIT | DDPCAPS_ALLOW256,
                                    PaletteEntries, &lpDDPalette, NULL);
        if( FAILED(dErr))
        {
            lpDDPalette = NULL;
            BLOCK = false;
            return false;
        }
    }

    depth = pDepth;
    height = pHeight;
    width = pWidth;
    doubleBuffered = pDoubleBuffered;
    BLOCK = false;

    return (true);
}

void CDirectDraw::GetPixelFormat ()
{
    if (lpDDSPrimary2)
    {
        ZeroMemory (&DDPixelFormat, sizeof (DDPixelFormat));
        DDPixelFormat.dwSize = sizeof (DDPixelFormat);
        lpDDSPrimary2->GetPixelFormat (&DDPixelFormat);
    }
}

static bool LockSurface2 (LPDIRECTDRAWSURFACE2 lpDDSurface, SSurface *lpSurface)
{
    DDSURFACEDESC ddsd;
    HRESULT hResult;
    int retry;

    ddsd.dwSize = sizeof (ddsd);

    retry = 0;
    while (true)
    {
        hResult = lpDDSurface->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL);

        if( hResult == DD_OK)
        {
            lpSurface->Width = ddsd.dwWidth;
            lpSurface->Height = ddsd.dwHeight;
            lpSurface->Pitch = ddsd.lPitch;
            lpSurface->Surface = (unsigned char *)ddsd.lpSurface;
            return (true);
        }

        if (hResult == DDERR_SURFACELOST)
        {
            retry++;
            if (retry > 5)
                return (false);

            hResult = lpDDSurface->Restore();
            if (hResult != DD_OK)
                return (false);

            continue;
        }

        if (hResult != DDERR_WASSTILLDRAWING)
            return (false);
    }
}

void CDirectDraw::Render(SSurface Src)
{
    LPDIRECTDRAWSURFACE2 lpDDSurface2 = NULL;
    LPDIRECTDRAWSURFACE2 pDDSurface = NULL;
	SSurface Dst;
	RECT srcRect;

	RECT dstRect = { 0, 512, 0, 448 };

	DDSCAPS caps;

	unsigned int newFilterScale;

	if(!dDinitialized) return;

	memset(&caps, 0,sizeof(DDSCAPS));
	caps.dwCaps = DDSCAPS_BACKBUFFER;

	if (lpDDSPrimary2->GetAttachedSurface (&caps, &pDDSurface) != DD_OK ||
		pDDSurface == NULL)
	{
		lpDDSurface2 = lpDDSPrimary2;
	}
	else
		lpDDSurface2 = pDDSurface;

	lpDDSurface2 = lpDDSOffScreen2;
	if (!LockSurface2 (lpDDSurface2, &Dst))
		return;

	if (!GUI.DepthConverted)
	{
		SSurface tmp;
		
		newFilterScale = max(2,max(GetFilterScale(GUI.ScaleHiRes),GetFilterScale(GUI.Scale)));

		if(newFilterScale!=filterScale) {
			if(convertBuffer)
				delete [] convertBuffer;
			filterScale = newFilterScale;
			convertBuffer = new unsigned char[SNES_WIDTH * sizeof(uint16) * SNES_HEIGHT_EXTENDED * sizeof(uint16) *filterScale*filterScale]();
		}

		tmp.Surface = convertBuffer;

		if(CurrentScale == FILTER_NONE) {
			tmp.Pitch = Src.Pitch;
			tmp.Width = Src.Width;
			tmp.Height = Src.Height;
		} else {
			tmp.Pitch = Dst.Pitch;
			tmp.Width = Dst.Width;
			tmp.Height = Dst.Height;
		}
		RenderMethod (Src, tmp, &srcRect);
		ConvertDepth (&tmp, &Dst, &srcRect);
	}
	else
	{
		RenderMethod (Src, Dst, &srcRect);
	}

	if(!Settings.AutoDisplayMessages) {
		WinSetCustomDisplaySurface((void *)Dst.Surface, (Dst.Pitch*8/GUI.ScreenDepth), srcRect.right-srcRect.left, srcRect.bottom-srcRect.top, GetFilterScale(CurrentScale));
		S9xDisplayMessages ((uint16*)Dst.Surface, Dst.Pitch/2, srcRect.right-srcRect.left, srcRect.bottom-srcRect.top, GetFilterScale(CurrentScale));
	}

	RECT lastRect = SizeHistory [GUI.FlipCounter % GUI.NumFlipFrames];
	POINT p;

	if (GUI.Stretch)
	{
		p.x = p.y = 0;
		ClientToScreen (GUI.hWnd, &p);
		GetClientRect (GUI.hWnd, &dstRect);
//				dstRect.bottom = int(double(dstRect.bottom) * double(239.0 / 240.0));

		if(GUI.AspectRatio)
		{
			int width = dstRect.right - dstRect.left;
			int height = dstRect.bottom - dstRect.top;

			int oldWidth = GUI.AspectWidth;
			int oldHeight = GUI.HeightExtend ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT;
			int newWidth, newHeight;

			if(oldWidth * height > oldHeight * width)
			{
				newWidth = width;
				newHeight = oldHeight*width/oldWidth;
			}
			else
			{
				newWidth = oldWidth*height/oldHeight;
				newHeight = height;
			}
			int xOffset = (width - newWidth)/2;
			int yOffset = (height - newHeight)/2;

			dstRect.right = dstRect.left + newWidth;
			dstRect.bottom = dstRect.top + newHeight;

			OffsetRect(&dstRect, p.x + xOffset, p.y + yOffset);
		}
		else
		{
			OffsetRect(&dstRect, p.x, p.y);
		}
	}
	else
	{
		GetClientRect (GUI.hWnd, &dstRect);
		int width = srcRect.right - srcRect.left;
		int height = srcRect.bottom - srcRect.top;

		//if (GUI.Scale == 1)
		//{
		//    width = MAX_SNES_WIDTH;
		//    if (height < 240)
		//        height *= 2;
		//}
		p.x = ((dstRect.right - dstRect.left) - width) >> 1;
		p.y = ((dstRect.bottom - dstRect.top) - height) >> 1;

		if(p.y < 0) p.y = 0;
		if(p.x < 0) p.x = 0;

		ClientToScreen (GUI.hWnd, &p);

		dstRect.top = p.y;
		dstRect.left = p.x;
		dstRect.bottom = dstRect.top + height;
		dstRect.right  = dstRect.left + width;
	}

	lpDDSurface2->Unlock (Dst.Surface);

	memset(&caps, 0,sizeof(DDSCAPS));
	caps.dwCaps = DDSCAPS_BACKBUFFER;

	if (lpDDSPrimary2->GetAttachedSurface (&caps, &pDDSurface) != DD_OK ||
		pDDSurface == NULL)
	{
		lpDDSurface2 = lpDDSPrimary2;
	}
	else
		lpDDSurface2 = pDDSurface;

	// actually draw it onto the screen (unless in fullscreen mode; see UpdateBackBuffer() for that)
	while (lpDDSurface2->Blt (&dstRect, lpDDSOffScreen2, &srcRect, DDBLT_WAIT, NULL) == DDERR_SURFACELOST)
		lpDDSurface2->Restore ();


	RECT rect;
	DDBLTFX fx;

	memset (&fx, 0, sizeof (fx));
	fx.dwSize = sizeof (fx);

	if (GUI.FlipCounter >= GUI.NumFlipFrames)
	{
		if (lastRect.top < dstRect.top)
		{
			rect.top = lastRect.top;
			rect.bottom = dstRect.top;
			rect.left = min(lastRect.left, dstRect.left);
			rect.right = max(lastRect.right, dstRect.right);
			lpDDSurface2->Blt (&rect, NULL, &rect,
							   DDBLT_WAIT | DDBLT_COLORFILL, &fx);
		}
		if (lastRect.bottom > dstRect.bottom)
		{
			rect.left = min(lastRect.left, dstRect.left);
			rect.right = max(lastRect.right, dstRect.right);
			rect.top = dstRect.bottom;
			rect.bottom = lastRect.bottom;
			lpDDSurface2->Blt (&rect, NULL, &rect,
							   DDBLT_WAIT | DDBLT_COLORFILL, &fx);
		}
		if (lastRect.left < dstRect.left)
		{
			rect.left = lastRect.left;
			rect.right = dstRect.left;
			rect.top = dstRect.top;
			rect.bottom = dstRect.bottom;
			lpDDSurface2->Blt (&rect, NULL, &rect,
							   DDBLT_WAIT | DDBLT_COLORFILL, &fx);
		}
		if (lastRect.right > dstRect.right)
		{
			rect.left = dstRect.right;
			rect.right = lastRect.right;
			rect.top = dstRect.top;
			rect.bottom = dstRect.bottom;
			lpDDSurface2->Blt (&rect, NULL, &rect,
							   DDBLT_WAIT | DDBLT_COLORFILL, &fx);
		}
	}

	if(GUI.Vsync) {
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,NULL);
	}

	lpDDSPrimary2->Flip (NULL, GUI.Vsync?DDFLIP_WAIT:DDFLIP_NOVSYNC);

	SizeHistory [GUI.FlipCounter % GUI.NumFlipFrames] = dstRect;
}

bool CDirectDraw::ApplyDisplayChanges(void)
{
	return SetDisplayMode (GUI.FullscreenMode.width, GUI.FullscreenMode.height, max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes)), GUI.FullscreenMode.depth, GUI.FullscreenMode.rate,
		!GUI.FullScreen, GUI.DoubleBuffered);
}

bool CDirectDraw::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
	return true;
}

bool CDirectDraw::SetFullscreen(bool fullscreen)
{
	if (!SetDisplayMode (GUI.FullscreenMode.width, GUI.FullscreenMode.height, max(GetFilterScale(GUI.Scale), GetFilterScale(GUI.ScaleHiRes)), GUI.FullscreenMode.depth, GUI.FullscreenMode.rate, !fullscreen, GUI.DoubleBuffered))
	{
		MessageBox( GUI.hWnd, Languages[ GUI.Language].errModeDD, TEXT("Snes9X - DirectDraw(2)"), MB_OK | MB_ICONSTOP);
		return false;
	}
	return true;
}

int ffs (uint32 mask)
{
    int m = 0;
    if (mask)
    {
        while (!(mask & (1 << m)))
            m++;

        return (m);
    }

    return (0);
}

void CDirectDraw::SetSnes9xColorFormat()
{
	GUI.ScreenDepth = DDPixelFormat.dwRGBBitCount;
        if (GUI.ScreenDepth == 15)
            GUI.ScreenDepth = 16;

        GUI.RedShift = ffs (DDPixelFormat.dwRBitMask);
        GUI.GreenShift = ffs (DDPixelFormat.dwGBitMask);
        GUI.BlueShift = ffs (DDPixelFormat.dwBBitMask);

        if((DDPixelFormat.dwFlags&DDPF_RGB) != 0 &&
           GUI.ScreenDepth == 16 &&
           DDPixelFormat.dwRBitMask == 0xF800 &&
           DDPixelFormat.dwGBitMask == 0x07E0 &&
           DDPixelFormat.dwBBitMask == 0x001F)
        {
            S9xSetRenderPixelFormat (RGB565);
        }
        else
            if( (DDPixelFormat.dwFlags&DDPF_RGB) != 0 &&
                GUI.ScreenDepth == 16 &&
                DDPixelFormat.dwRBitMask == 0x7C00 &&
                DDPixelFormat.dwGBitMask == 0x03E0 &&
                DDPixelFormat.dwBBitMask == 0x001F)
            {
                S9xSetRenderPixelFormat (RGB555);
            }
            else
                if((DDPixelFormat.dwFlags&DDPF_RGB) != 0 &&
                   GUI.ScreenDepth == 16 &&
                   DDPixelFormat.dwRBitMask == 0x001F &&
                   DDPixelFormat.dwGBitMask == 0x07E0 &&
                   DDPixelFormat.dwBBitMask == 0xF800)
                {
                    S9xSetRenderPixelFormat (BGR565);
                }
                else
                    if( (DDPixelFormat.dwFlags&DDPF_RGB) != 0 &&
                        GUI.ScreenDepth == 16 &&
                        DDPixelFormat.dwRBitMask == 0x001F &&
                        DDPixelFormat.dwGBitMask == 0x03E0 &&
                        DDPixelFormat.dwBBitMask == 0x7C00)
                    {
                        S9xSetRenderPixelFormat (BGR555);
                    }
                    else
                        if (DDPixelFormat.dwRGBBitCount == 8 ||
                            DDPixelFormat.dwRGBBitCount == 24 ||
                            DDPixelFormat.dwRGBBitCount == 32)
                        {
                            S9xSetRenderPixelFormat (RGB565);
                        }

        if ((GUI.ScreenDepth == 24 || GUI.ScreenDepth == 32))
            GUI.NeedDepthConvert = TRUE;

		GUI.DepthConverted = !GUI.NeedDepthConvert;

        if ((GUI.ScreenDepth == 24 || GUI.ScreenDepth == 32))
        {
            GUI.RedShift += 3;
            GUI.GreenShift += 3;
            GUI.BlueShift += 3;
        }

	S9xBlit2xSaIFilterInit();
	S9xBlitHQ2xFilterInit();
}

HRESULT CALLBACK EnumModesCallback( LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
	dMode curmode;
	std::vector<dMode> *modeVector=(std::vector<dMode> *)lpContext;
	HWND hDlg = (HWND)lpContext;

	if((lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 15 &&
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 16 &&
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount != 32) ||
        (lpDDSurfaceDesc->dwWidth < SNES_WIDTH ||
		lpDDSurfaceDesc->dwHeight < SNES_HEIGHT_EXTENDED))
	{
		// let them muck with the .cfg file if they really want to set such a poor display mode
		return DDENUMRET_OK; // keep going without adding mode to list
	}

	curmode.width=lpDDSurfaceDesc->dwWidth;
	curmode.height=lpDDSurfaceDesc->dwHeight;
	curmode.depth=lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
	curmode.rate=lpDDSurfaceDesc->dwRefreshRate;

	modeVector->push_back(curmode);

	return DDENUMRET_OK;
}


void CDirectDraw::EnumModes(std::vector<dMode> *modeVector)
{
	if(!dDinitialized)
		return;
	lpDD->EnumDisplayModes(DDEDM_REFRESHRATES,NULL,(void *)modeVector,(LPDDENUMMODESCALLBACK)EnumModesCallback);
}

#endif
