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

#ifndef W9XDIRECT3D_H
#define W9XDIRECT3D_H

#define MAX_SHADER_TEXTURES 8

#include <d3d9.h>
#include <d3dx9.h>
#include <windows.h>

#include "cgFunctions.h"
#include "CD3DCG.h"

#include "render.h"
#include "wsnes9x.h"
#include "IS9xDisplayOutput.h"

#define FVF_COORDS_TEX D3DFVF_XYZ | D3DFVF_TEX1

typedef struct _VERTEX {
		float x, y, z;
		float tx, ty;
		float lutx, luty;
		_VERTEX() {}
		_VERTEX(float x,float y,float z,float tx,float ty,float lutx, float luty) {
			this->x=x;this->y=y;this->z=z;this->tx=tx;this->ty=ty;this->lutx=lutx;this->luty=luty;
		}
} VERTEX; //our custom vertex with a constuctor for easier assignment

enum current_d3d_shader_type { D3D_SHADER_NONE, D3D_SHADER_HLSL, D3D_SHADER_CG };

class CDirect3D: public IS9xDisplayOutput
{
private:
	bool                  init_done;					//has initialize been called?
	LPDIRECT3D9           pD3D;
	LPDIRECT3DDEVICE9     pDevice;
	LPDIRECT3DTEXTURE9    drawSurface;					//the texture used for all drawing operations

	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	D3DPRESENT_PARAMETERS dPresentParams;
	unsigned int filterScale;							//the current maximum filter scale (at least 2)
	unsigned int afterRenderWidth, afterRenderHeight;	//dimensions after filter has been applied
	unsigned int quadTextureSize;						//size of the texture (only multiples of 2)
	bool fullscreen;									//are we currently displaying in fullscreen mode
	
	VERTEX vertexStream[4];								//the 4 vertices that make up our display rectangle

	static const D3DVERTEXELEMENT9 vertexElems[4];
	LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;

	LPD3DXEFFECT            effect;
	LPDIRECT3DTEXTURE9      rubyLUT[MAX_SHADER_TEXTURES];
	CGcontext cgContext;
	current_d3d_shader_type shader_type;
	bool cgAvailable;

	CD3DCG *cgShader;

	float shaderTimer;
	int shaderTimeStart;
	int shaderTimeElapsed;
	int frameCount;

	bool BlankTexture(LPDIRECT3DTEXTURE9 texture);
	void CreateDrawSurface();
	void DestroyDrawSurface();
	bool ChangeDrawSurfaceSize(unsigned int scale);
	void SetViewport();
	void SetupVertices();
	bool ResetDevice();
	void SetFiltering();
	void SetShaderVars(bool setMatrix = false);
	bool SetShader(const TCHAR *file);
	bool SetShaderHLSL(const TCHAR *file);
	void checkForCgError(const char *situation);
	bool SetShaderCG(const TCHAR *file);

public:
	CDirect3D();
	~CDirect3D();
	bool Initialize(HWND hWnd);
	void DeInitialize();
	void Render(SSurface Src);
	bool ChangeRenderSize(unsigned int newWidth, unsigned int newHeight);
	bool ApplyDisplayChanges(void);
	bool SetFullscreen(bool fullscreen);
	void SetSnes9xColorFormat();
	void EnumModes(std::vector<dMode> *modeVector);
};

#endif
