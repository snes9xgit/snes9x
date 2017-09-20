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



#include "COpenGL.h"
#include "win32_display.h"
#include "../snes9x.h"
#include "../gfx.h"
#include "../display.h"
#include "wsnes9x.h"
#include <msxml2.h>

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"


COpenGL::COpenGL(void)
{
	hDC = NULL;
	hRC = NULL;
	hWnd = NULL;
	drawTexture = 0;
	initDone = false;
	quadTextureSize = 0;
	filterScale = 0;
	afterRenderWidth = 0;
	afterRenderHeight = 0;
	fullscreen = false;
	shaderFunctionsLoaded = false;
	shader_type = OGL_SHADER_NONE;
	pboFunctionsLoaded = false;
	shaderProgram = 0;
    vertexShader = 0;
    fragmentShader = 0;
	cgContext = NULL;
	cgVertexProgram = cgFragmentProgram = NULL;
	cgAvailable = false;
	frameCount = 0;
	cgShader = NULL;
}

COpenGL::~COpenGL(void)
{
	DeInitialize();
}

bool COpenGL::Initialize(HWND hWnd)
{
	int pfdIndex;
	RECT windowRect;

	this->hWnd = hWnd;
	this->hDC = GetDC(hWnd);

	PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		16,												// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		16,												// 16Bit Z-Buffer (Depth Buffer)
		0,												// No Stencil Buffer
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};
	PIXELFORMATDESCRIPTOR pfdSel;

	if(!(pfdIndex=ChoosePixelFormat(hDC,&pfd))) {
		DeInitialize();
		return false;
	}
	if(!SetPixelFormat(hDC,pfdIndex,&pfd)) {
		DeInitialize();
		return false;
	}
	if(!(hRC=wglCreateContext(hDC))) {
		DeInitialize();
		return false;
	}
	if(!wglMakeCurrent(hDC,hRC)) {
		DeInitialize();
		return false;
	}

	LoadPBOFunctions();

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1, 1);
	
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	cgAvailable = loadCgFunctions();
	if(cgAvailable) {
		cgContext = cgCreateContext();
		cgShader = new CGLCG(cgContext);
	}

	ApplyDisplayChanges();

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(hDC);

	initDone = true;
	return true;
}

void COpenGL::DeInitialize()
{
	initDone = false;
	SetShaders(NULL);
	DestroyDrawSurface();
	wglMakeCurrent(NULL,NULL);
	if(hRC) {
		wglDeleteContext(hRC);
		hRC = NULL;
	}
	if(hDC) {
		ReleaseDC(hWnd,hDC);
		hDC = NULL;
	}
	hWnd = NULL;
	initDone = false;
	quadTextureSize = 0;
	filterScale = 0;
	afterRenderWidth = 0;
	afterRenderHeight = 0;
	shaderFunctionsLoaded = false;
	shader_type = OGL_SHADER_NONE;
	if(cgShader) {
		delete cgShader;
		cgShader = NULL;
	}
	if(cgAvailable)
		unloadCgLibrary();
	cgAvailable = false;
}

void COpenGL::CreateDrawSurface()
{
	unsigned int neededSize;
	HRESULT hr;

	//we need at least 512 pixels (SNES_WIDTH * 2) so we can start with that value
	quadTextureSize = 512;
	neededSize = SNES_WIDTH * filterScale;
	while(quadTextureSize < neededSize)
		quadTextureSize *=2;

	if(!drawTexture) {
		glGenTextures(1,&drawTexture);
		glBindTexture(GL_TEXTURE_2D,drawTexture);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,quadTextureSize,quadTextureSize,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,NULL);
		if(pboFunctionsLoaded) {
			glGenBuffers(1,&drawBuffer);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER,drawBuffer);
			glBufferData(GL_PIXEL_UNPACK_BUFFER,quadTextureSize*quadTextureSize*2,NULL,GL_STREAM_DRAW);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
		} else {
			noPboBuffer = new BYTE[quadTextureSize*quadTextureSize*2];
		}

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
}

void COpenGL::DestroyDrawSurface()
{
	if(drawTexture) {
		glDeleteTextures(1,&drawTexture);
		drawTexture = NULL;
	}
	if(drawBuffer) {
		glDeleteBuffers(1,&drawBuffer);
		drawBuffer = NULL;
	}
	if(noPboBuffer) {
		delete [] noPboBuffer;
		noPboBuffer = NULL;
	}
}

bool COpenGL::ChangeDrawSurfaceSize(unsigned int scale)
{
	filterScale = scale;

	DestroyDrawSurface();
	CreateDrawSurface();
	SetupVertices();
	return true;
}

void COpenGL::SetupVertices()
{
	vertices[0] = 0.0f;
    vertices[1] = 0.0f;
	vertices[2] = 1.0f;
    vertices[3] = 0.0f;
	vertices[4] = 1.0f;
	vertices[5] = 1.0f;
    vertices[6] = 0.0f;
	vertices[7] = 1.0f;

	float tX = (float)afterRenderWidth / (float)quadTextureSize;
	float tY = (float)afterRenderHeight / (float)quadTextureSize;

	texcoords[0] = 0.0f;
    texcoords[1] = tY;
    texcoords[2] = tX;
    texcoords[3] = tY;
    texcoords[4] = tX;
    texcoords[5] = 0.0f;
    texcoords[6] = 0.0f;
    texcoords[7] = 0.0f;
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
}

void COpenGL::Render(SSurface Src)
{
	SSurface Dst;
	RECT dstRect;
	unsigned int newFilterScale;
	GLenum error;

	if(!initDone) return;

	//create a new draw surface if the filter scale changes
	//at least factor 2 so we can display unscaled hi-res images
	newFilterScale = max(2,max(GetFilterScale(GUI.ScaleHiRes),GetFilterScale(GUI.Scale)));
	if(newFilterScale!=filterScale) {
		ChangeDrawSurfaceSize(newFilterScale);
	}

	if(pboFunctionsLoaded) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, drawBuffer);
		Dst.Surface = (unsigned char *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER,GL_READ_WRITE);
	} else {
		Dst.Surface = noPboBuffer;
	}
	Dst.Height = quadTextureSize;
	Dst.Width = quadTextureSize;
	Dst.Pitch = quadTextureSize * 2;

	RenderMethod (Src, Dst, &dstRect);
	if(!Settings.AutoDisplayMessages) {
		WinSetCustomDisplaySurface((void *)Dst.Surface, Dst.Pitch/2, dstRect.right-dstRect.left, dstRect.bottom-dstRect.top, GetFilterScale(CurrentScale));
		S9xDisplayMessages ((uint16*)Dst.Surface, Dst.Pitch/2, dstRect.right-dstRect.left, dstRect.bottom-dstRect.top, GetFilterScale(CurrentScale));
	}

	if(pboFunctionsLoaded)
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	if(afterRenderHeight != dstRect.bottom || afterRenderWidth != dstRect.right) {
		afterRenderHeight = dstRect.bottom;
		afterRenderWidth = dstRect.right;

		ChangeRenderSize(0,0);
	}

	glBindTexture(GL_TEXTURE_2D,drawTexture);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, quadTextureSize);
	glTexSubImage2D (GL_TEXTURE_2D,0,0,0,dstRect.right-dstRect.left,dstRect.bottom-dstRect.top,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,pboFunctionsLoaded?0:noPboBuffer);

	if(pboFunctionsLoaded)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	if (shader_type != OGL_SHADER_NONE) {

		if(shader_type == OGL_SHADER_GLSL) {
			GLint location;

			float inputSize[2] = { (float)afterRenderWidth, (float)afterRenderHeight };
			RECT windowSize;
			GetClientRect(hWnd,&windowSize);
			float outputSize[2] = {(float)(GUI.Stretch?windowSize.right:afterRenderWidth),
								(float)(GUI.Stretch?windowSize.bottom:afterRenderHeight) };
			float textureSize[2] = { (float)quadTextureSize, (float)quadTextureSize };
			float frameCnt = (float)++frameCount;
			location = glGetUniformLocation (shaderProgram, "rubyInputSize");
			glUniform2fv (location, 1, inputSize);

			location = glGetUniformLocation (shaderProgram, "rubyOutputSize");
			glUniform2fv (location, 1, outputSize);

			location = glGetUniformLocation (shaderProgram, "rubyTextureSize");
			glUniform2fv (location, 1, textureSize);
		} else if(shader_type == OGL_SHADER_CG) {
			xySize inputSize = { (float)afterRenderWidth, (float)afterRenderHeight };
			RECT windowSize, displayRect;
			GetClientRect(hWnd,&windowSize);
			xySize xywindowSize = { (double)windowSize.right, (double)windowSize.bottom };
			//Get maximum rect respecting AR setting
			displayRect=CalculateDisplayRect(windowSize.right,windowSize.bottom,windowSize.right,windowSize.bottom);
			xySize viewportSize = { (double)(displayRect.right - displayRect.left),
				                    (double)(displayRect.bottom - displayRect.top) };
			xySize textureSize = { (double)quadTextureSize, (double)quadTextureSize };
			cgShader->Render(drawTexture, textureSize, inputSize, viewportSize, xywindowSize);
		}
    }

	if(GUI.BilinearFilter) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays (GL_QUADS, 0, 4);

	glFlush();
	SwapBuffers(hDC);
}

bool COpenGL::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
	RECT displayRect, windowSize;
	if(newWidth==0||newHeight==0) {
		GetClientRect(hWnd,&windowSize);
		newWidth = windowSize.right;
		newHeight = windowSize.bottom;
	}
	displayRect=CalculateDisplayRect(afterRenderWidth,afterRenderHeight,newWidth,newHeight);
	glViewport(displayRect.left,newHeight-displayRect.bottom,displayRect.right-displayRect.left,displayRect.bottom-displayRect.top);
	SetupVertices();
	return true;
}

bool COpenGL::ApplyDisplayChanges(void)
{
	if(wglSwapIntervalEXT) {
		wglSwapIntervalEXT(GUI.Vsync?1:0);
	}
	if(GUI.shaderEnabled && GUI.OGLshaderFileName)
		SetShaders(GUI.OGLshaderFileName);
	else
		SetShaders(NULL);

	ChangeRenderSize(0,0);
	return true;
}

bool COpenGL::SetFullscreen(bool fullscreen)
{
	if(!initDone)
		return false;

	if(this->fullscreen==fullscreen)
		return true;

	this->fullscreen = fullscreen;

	if(fullscreen) {
		DEVMODE dmScreenSettings={0};
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= GUI.FullscreenMode.width;
		dmScreenSettings.dmPelsHeight	= GUI.FullscreenMode.height;
		dmScreenSettings.dmBitsPerPel	= GUI.FullscreenMode.depth;
		dmScreenSettings.dmDisplayFrequency = GUI.FullscreenMode.rate;
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
		if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
			this->fullscreen = false;
			return false;
		}
		ChangeRenderSize(GUI.FullscreenMode.width,GUI.FullscreenMode.height);
	} else {
		ChangeDisplaySettings(NULL,0);
	}

	

	return true;
}

void COpenGL::SetSnes9xColorFormat()
{
	GUI.ScreenDepth = 16;
	GUI.BlueShift = 0;
	GUI.GreenShift = 6;
	GUI.RedShift = 11;
	S9xSetRenderPixelFormat (RGB565);
	S9xBlit2xSaIFilterInit();
	S9xBlitHQ2xFilterInit();
	GUI.NeedDepthConvert = FALSE;
	GUI.DepthConverted = TRUE;
	return;
}

void COpenGL::EnumModes(std::vector<dMode> *modeVector)
{
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	DWORD dev = 0;
	int iMode = 0;
	dMode mode;

	while (EnumDisplayDevices(0, dev, &dd, 0))
	{
		if (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
		{
			DEVMODE dm;
			memset(&dm, 0, sizeof(dm));
			dm.dmSize = sizeof(dm);
			iMode = 0;
			while(EnumDisplaySettings(dd.DeviceName,iMode,&dm)) {
				if(dm.dmBitsPerPel>=16) {
					mode.width = dm.dmPelsWidth;
					mode.height = dm.dmPelsHeight;
					mode.rate = dm.dmDisplayFrequency;
					mode.depth = dm.dmBitsPerPel;
					modeVector->push_back(mode);
				}
				iMode++;
			}
		}
		dev++;
	}
}

bool COpenGL::LoadPBOFunctions()
{
	if(GUI.OGLdisablePBOs)
		return false;

	if(pboFunctionsLoaded)
		return true;

	const char *extensions = (const char *) glGetString(GL_EXTENSIONS);

	if(extensions && strstr(extensions, "pixel_buffer_object")) {
		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
		glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
		glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");

		if(glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers && glMapBuffer) {
			pboFunctionsLoaded = true;
		}
		 
	}
	return pboFunctionsLoaded;
}

bool COpenGL::LoadShaderFunctions()
{
	if(shaderFunctionsLoaded)
		return true;

	const char *extensions = (const char *) glGetString(GL_EXTENSIONS);

    if(extensions && strstr(extensions, "fragment_program")) {
		glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress ("glCreateProgram");
		glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress ("glCreateShader");
		glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress ("glCompileShader");
		glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress ("glDeleteShader");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress ("glDeleteProgram");
		glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress ("glAttachShader");
		glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress ("glDetachShader");
		glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress ("glLinkProgram");
		glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress ("glUseProgram");
		glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress ("glShaderSource");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress ("glGetUniformLocation");
		glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress ("glUniform2fv");

		if(glCreateProgram      &&
		   glCreateShader       &&
		   glCompileShader      &&
		   glDeleteShader       &&
		   glDeleteProgram      &&
		   glAttachShader       &&
		   glDetachShader       &&
		   glLinkProgram        &&
		   glUseProgram         &&
		   glShaderSource       &&
		   glGetUniformLocation &&
		   glUniform2fv) {
			   shaderFunctionsLoaded = true;
		}
	}
	return shaderFunctionsLoaded;
}

bool COpenGL::SetShaders(const TCHAR *file)
{
	SetShadersCG(NULL);
	SetShadersGLSL(NULL);
	shader_type = OGL_SHADER_NONE;
	if(file!=NULL && (
		(lstrlen(file)>3 && _tcsncicmp(&file[lstrlen(file)-3],TEXT(".cg"),3)==0) ||
		(lstrlen(file)>4 && _tcsncicmp(&file[lstrlen(file)-4],TEXT(".cgp"),4)==0))) {
		return SetShadersCG(file);
	} else {
		return SetShadersGLSL(file);
	}
}

void COpenGL::checkForCgError(const char *situation)
{
	char buffer[4096];
	CGerror error = cgGetError();
	const char *string = cgGetErrorString(error);

	if (error != CG_NO_ERROR) {
		sprintf(buffer,
			  "Situation: %s\n"
			  "Error: %s\n\n"
			  "Cg compiler output...\n", situation, string);
		MessageBoxA(0, buffer,
				  "Cg error", MB_OK|MB_ICONEXCLAMATION);
		if (error == CG_COMPILER_ERROR) {
			MessageBoxA(0, cgGetLastListing(cgContext),
					  "Cg compilation error", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

bool COpenGL::SetShadersCG(const TCHAR *file)
{
	if(!cgAvailable) {
		if(file)
			MessageBox(NULL, TEXT("The CG runtime is unavailable, CG shaders will not run.\nConsult the snes9x readme for information on how to obtain the runtime."), TEXT("CG Error"),
				MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

	if(!cgShader->LoadShader(file))
		return false;

	shader_type = OGL_SHADER_CG;

	return true;
}

bool COpenGL::SetShadersGLSL(const TCHAR *glslFileName)
{
	char *fragment=NULL, *vertex=NULL;
	IXMLDOMDocument * pXMLDoc = NULL;
	IXMLDOMElement * pXDE = NULL;
	IXMLDOMNode * pXDN = NULL;
	HRESULT hr;
	BSTR queryString, nodeContent;

	TCHAR errorMsg[MAX_PATH + 50];

	if(fragmentShader) {
		glDetachShader(shaderProgram,fragmentShader);
		glDeleteShader(fragmentShader);
		fragmentShader = 0;
	}
	if(vertexShader) {
		glDetachShader(shaderProgram,vertexShader);
		glDeleteShader(vertexShader);
		vertexShader = 0;
	}
	if(shaderProgram) {
		glUseProgram(0);
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
	}

	if(glslFileName==NULL || *glslFileName==TEXT('\0'))
		return true;

	if(!LoadShaderFunctions()) {
        MessageBox(NULL, TEXT("Unable to load OpenGL shader functions"), TEXT("Shader Loading Error"),
			MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

	hr = CoCreateInstance(CLSID_DOMDocument,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pXMLDoc));

	if(FAILED(hr)) {
		MessageBox(NULL, TEXT("Error creating XML Parser"), TEXT("Shader Loading Error"),
			MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	VARIANT fileName;
	VARIANT_BOOL ret;
	fileName.vt = VT_BSTR;
#ifdef UNICODE
	fileName.bstrVal = SysAllocString(glslFileName);
#else
	wchar_t tempfilename[MAX_PATH];
	MultiByteToWideChar(CP_UTF8,0,glslFileName,-1,tempfilename,MAX_PATH);
	fileName.bstrVal = SysAllocString(tempfilename);
#endif
	hr = pXMLDoc->load(fileName,&ret);
	SysFreeString(fileName.bstrVal);

	if(FAILED(hr) || hr==S_FALSE) {
		_stprintf(errorMsg,TEXT("Error loading GLSL shader file:\n%s"),glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		pXMLDoc->Release();
		return false;
	}

	VARIANT attributeValue;
	BSTR attributeName;

	hr = pXMLDoc->get_documentElement(&pXDE);
	if(FAILED(hr) || hr==S_FALSE) {
		_stprintf(errorMsg,TEXT("Error loading root element from file:\n%s"),glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		pXMLDoc->Release();
		return false;
	}

	attributeName=SysAllocString(L"language");
	pXDE->getAttribute(attributeName,&attributeValue);
	SysFreeString(attributeName);
	pXDE->Release();

	if(attributeValue.vt!=VT_BSTR || lstrcmpiW(attributeValue.bstrVal,L"glsl")) {
		_stprintf(errorMsg,TEXT("Shader language is <%s>, expected <GLSL> in file:\n%s"),attributeValue.bstrVal,glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		if(attributeValue.vt==VT_BSTR) SysFreeString(attributeValue.bstrVal);
		pXMLDoc->Release();
		return false;
	}
	if(attributeValue.vt==VT_BSTR) SysFreeString(attributeValue.bstrVal);

	queryString=SysAllocString(L"/shader/fragment");
	hr = pXMLDoc->selectSingleNode(queryString,&pXDN);
	SysFreeString(queryString);

	if(hr == S_OK) {
		hr = pXDN->get_text(&nodeContent);
		if(hr == S_OK) {
			int requiredChars = WideCharToMultiByte(CP_ACP,0,nodeContent,-1,fragment,0,NULL,NULL);
			fragment = new char[requiredChars];
			WideCharToMultiByte(CP_UTF8,0,nodeContent,-1,fragment,requiredChars,NULL,NULL);
		}
		SysFreeString(nodeContent);
		pXDN->Release();
		pXDN = NULL;
	}

	queryString=SysAllocString(L"/shader/vertex");
	hr = pXMLDoc->selectSingleNode(queryString,&pXDN);
	SysFreeString(queryString);

	if(hr == S_OK) {
		hr = pXDN->get_text(&nodeContent);
		if(hr == S_OK) {
			int requiredChars = WideCharToMultiByte(CP_ACP,0,nodeContent,-1,vertex,0,NULL,NULL);
			vertex = new char[requiredChars];
			WideCharToMultiByte(CP_UTF8,0,nodeContent,-1,vertex,requiredChars,NULL,NULL);
		}
		SysFreeString(nodeContent);
		pXDN->Release();
		pXDN = NULL;
	}

	pXMLDoc->Release();

	if(!fragment && !vertex) {
		_stprintf(errorMsg,TEXT("No vertex or fragment program in file:\n%s"),glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

    shaderProgram = glCreateProgram ();
	if(vertex) {
		vertexShader = glCreateShader (GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, (const GLchar **)&vertex, NULL);
		glCompileShader(vertexShader);
		glAttachShader(shaderProgram, vertexShader);
		delete[] vertex;
	}
	if(fragment) {
		fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, (const GLchar **)&fragment, NULL);    
		glCompileShader(fragmentShader);    
		glAttachShader(shaderProgram, fragmentShader);
		delete[] fragment;
	}

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

	shader_type = OGL_SHADER_GLSL;

    return true;
}
