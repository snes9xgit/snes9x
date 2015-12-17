/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  See CREDITS file to find the copyright owners of this file.

  SDL Input/Audio/Video code (many lines of code come from snes9x & drnoksnes)
  (c) Copyright 2011         Makoto Sugano (makoto.sugano@gmail.com)

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

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "ppu.h"
#include "controls.h"
#include "movie.h"
#include "logger.h"
#include "conffile.h"
#include "blit.h"
#include "display.h"

#include "sdl_snes9x.h"

// grkenn - opengl backend
#ifdef USE_OPENGL
	#include <SDL/SDL_opengl.h>
#endif

struct GUIData
{
	SDL_Surface             *sdl_screen;
	uint8			*snes_buffer;
	uint8			*blit_screen;
	uint32			blit_screen_pitch;
	int			video_mode;

        bool8                   fullscreen;
        bool8                   double_buffer;
#ifdef USE_OPENGL
        bool8                   opengl;
	GLint			gl_filter;
	GLuint			texture;
	GLuint			displaylist;
#endif
};
static struct GUIData	GUI;

typedef	void (* Blitter) (uint8 *, int, uint8 *, int, int, int);

#ifdef __linux
// Select seems to be broken in 2.x.x kernels - if a signal interrupts a
// select system call with a zero timeout, the select call is restarted but
// with an infinite timeout! The call will block until data arrives on the
// selected fd(s).
//
// The workaround is to stop the X library calling select in the first
// place! Replace XPending - which polls for data from the X server using
// select - with an ioctl call to poll for data and then only call the blocking
// XNextEvent if data is waiting.
#define SELECT_BROKEN_FOR_SIGNALS
#endif

enum
{
	VIDEOMODE_BLOCKY = 1,
	VIDEOMODE_TV,
	VIDEOMODE_SMOOTH,
	VIDEOMODE_SUPEREAGLE,
	VIDEOMODE_2XSAI,
	VIDEOMODE_SUPER2XSAI,
	VIDEOMODE_EPX,
	VIDEOMODE_HQ2X
};

static void SetupImage (void);
static void TakedownImage (void);
static void Repaint (bool8);

void S9xExtraDisplayUsage (void)
{
	S9xMessage(S9X_INFO, S9X_USAGE, "-fullscreen                     fullscreen mode");
	S9xMessage(S9X_INFO, S9X_USAGE, "-double_buffer                  SDL double-buffering");
#ifdef USE_OPENGL
	S9xMessage(S9X_INFO, S9X_USAGE, "-opengl                         opengl scaling (no filter)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-opengl_linear                  opengl scaling (bilinear)");
#endif
	S9xMessage(S9X_INFO, S9X_USAGE, "");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v1                             Video mode: Blocky (default)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v2                             Video mode: TV");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v3                             Video mode: Smooth");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v4                             Video mode: SuperEagle");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v5                             Video mode: 2xSaI");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v6                             Video mode: Super2xSaI");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v7                             Video mode: EPX");
	S9xMessage(S9X_INFO, S9X_USAGE, "-v8                             Video mode: hq2x");
	S9xMessage(S9X_INFO, S9X_USAGE, "");
}

void S9xParseDisplayArg (char **argv, int &i, int argc)
{
#ifdef USE_OPENGL
	if (!strncasecmp(argv[i], "-opengl", 7))
	{
		GUI.opengl = TRUE;
                printf ("Using OPENGL backend.\n");
		GUI.gl_filter = GL_NEAREST;
		if (!strncasecmp(argv[i], "-opengl_linear", 14))
		{
                	printf ("\tGL_LINEAR filter selected.\n");
			GUI.gl_filter = GL_LINEAR;
		}
	}
	else
#endif
	if (!strncasecmp(argv[i], "-double_buffer", 14))
	{
		GUI.double_buffer = TRUE;
	}
	else
	if (!strncasecmp(argv[i], "-fullscreen", 11))
        {
                GUI.fullscreen = TRUE;
                printf ("Entering fullscreen mode.\n");
        }
        else
	if (!strncasecmp(argv[i], "-v", 2))
	{
		switch (argv[i][2])
		{
			case '1':	GUI.video_mode = VIDEOMODE_BLOCKY;		break;
			case '2':	GUI.video_mode = VIDEOMODE_TV;			break;
			case '3':	GUI.video_mode = VIDEOMODE_SMOOTH;		break;
			case '4':	GUI.video_mode = VIDEOMODE_SUPEREAGLE;	break;
			case '5':	GUI.video_mode = VIDEOMODE_2XSAI;		break;
			case '6':	GUI.video_mode = VIDEOMODE_SUPER2XSAI;	break;
			case '7':	GUI.video_mode = VIDEOMODE_EPX;			break;
			case '8':	GUI.video_mode = VIDEOMODE_HQ2X;		break;
		}
	}
	else
		S9xUsage();
}

const char * S9xParseDisplayConfig (ConfigFile &conf, int pass)
{
	if (pass != 1)
		return ("Unix/SDL");

	if (conf.Exists("Unix/SDL::VideoMode"))
	{
		GUI.video_mode = conf.GetUInt("Unix/SDL::VideoMode", VIDEOMODE_BLOCKY);
		if (GUI.video_mode < 1 || GUI.video_mode > 8)
			GUI.video_mode = VIDEOMODE_BLOCKY;
	}
	else
		GUI.video_mode = VIDEOMODE_BLOCKY;

	return ("Unix/SDL");
}

static void FatalError (const char *str)
{
	fprintf(stderr, "%s\n", str);
	S9xExit();
}

void S9xInitDisplay (int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
	}
  
	atexit(SDL_Quit);
	
	/*
	 * domaemon
	 *
	 * we just go along with RGB565 for now, nothing else..
	 */
	
	S9xSetRenderPixelFormat(RGB565);
	
	S9xBlitFilterInit();
	S9xBlit2xSaIFilterInit();
	S9xBlitHQ2xFilterInit();


	/*
	 * domaemon
	 *
	 * FIXME: The secreen size should be flexible
	 * FIXME: Check if the SDL screen is really in RGB565 mode. screen->fmt	
	 */	
	// grkenn - mode flags should be either OPENGL or (hwsurface with (double-buffer or none))
#ifdef USE_OPENGL
	int MODE_FLAGS = (GUI.opengl == TRUE ? SDL_OPENGL : SDL_HWSURFACE | (GUI.double_buffer == TRUE ? SDL_DOUBLEBUF : 0));
#else
	int MODE_FLAGS = SDL_HWSURFACE | (GUI.double_buffer == TRUE ? SDL_DOUBLEBUF : 0);
#endif
        if (GUI.fullscreen == TRUE)
        {
                GUI.sdl_screen = SDL_SetVideoMode(0, 0, 16, SDL_FULLSCREEN | MODE_FLAGS);
        } else {
                GUI.sdl_screen = SDL_SetVideoMode(SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2, 16, MODE_FLAGS);
        }

        if (GUI.sdl_screen == NULL)
	{
		printf("Unable to set video mode: %s\n", SDL_GetError());
		exit(1);
        }

	// grkenn - set up opengl params
#ifdef USE_OPENGL
	if (GUI.opengl == TRUE)
	{
    		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 6 );
    		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 0 );
    		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, (GUI.double_buffer == TRUE) );
	}
#endif

	/*
	 * domaemon
	 *
	 * buffer allocation, quite important
	 */
	SetupImage();
}

void S9xDeinitDisplay (void)
{
	TakedownImage();

	SDL_Quit();

	S9xBlitFilterDeinit();
	S9xBlit2xSaIFilterDeinit();
	S9xBlitHQ2xFilterDeinit();
}

static void TakedownImage (void)
{
	if (GUI.snes_buffer)
	{
		free(GUI.snes_buffer);
		GUI.snes_buffer = NULL;
	}

#ifdef USE_OPENGL
	if (GUI.opengl == TRUE)
	{
		if (GUI.texture) {
			glDeleteTextures(1,&GUI.texture);
			GUI.texture=0;
		}
		if (GUI.displaylist)
		{
			glDeleteLists(GUI.displaylist,1);
			GUI.displaylist = 0;
		}
	}
#endif

	S9xGraphicsDeinit();
}

static void SetupImage (void)
{
	TakedownImage();

	// FIXME: grkenn - SDL offers nice methods for creating (possibly hw-accel)
	//  drawing surfaces, we should use those instead of a raw buffer o' bytes

	// domaemon: The whole unix code basically assumes output=(original * 2);
	// This way the code can handle the SNES filters, which does the 2X.
	int SNES_FULL_WIDTH = SNES_WIDTH * 2;
	int SNES_FULL_HEIGHT = (SNES_HEIGHT_EXTENDED + 4) * 2;
	GFX.Pitch = SNES_FULL_WIDTH * 2;
	GUI.snes_buffer = (uint8 *) calloc(GFX.Pitch * SNES_FULL_HEIGHT, 1);
	if (!GUI.snes_buffer)
		FatalError("Failed to allocate GUI.snes_buffer.");

	// domaemon: Add 2 lines before drawing.
	GFX.Screen = (uint16 *) (GUI.snes_buffer + (GFX.Pitch * 2 * 2));

#ifdef USE_OPENGL
	if (GUI.opengl == TRUE)
	{
		// enable GL texturing
		glEnable(GL_TEXTURE_2D);
		// create opengl texture
		glGenTextures(1, &GUI.texture);
		// attach to texture
		glBindTexture(GL_TEXTURE_2D, GUI.texture);
		// set texture parameters
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GUI.gl_filter); // Filter modes
    		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GUI.gl_filter);
    		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

		// Compute next-highest power-of-2 for holding SNES buffer output
		int tex_width = 1;
		while(tex_width < SNES_FULL_WIDTH) tex_width *= 2;
		int tex_height = 1;
		while(tex_height < SNES_FULL_HEIGHT) tex_width *= 2;

		// Fill the texture with empty data first
		uint16 *empty = (uint16 *)calloc(tex_width*tex_height,sizeof(uint16));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, empty);
		free(empty);

		// Setup screen view based on display size.
		// Set the clear color to all black (0, 0, 0)
		glClearColor(0.0f,0.0f,0.0f,1.0f);

		SDL_Surface *screen = SDL_GetVideoSurface();
		glViewport(0,0,screen->w,screen->h);
		glMatrixMode(GL_PROJECTION);
		//glLoadIdentity();
		// Compute the actual screen aspect ratio, and letter/pillarbox glOrtho to fit.
		double screenAspect = (double)screen->w/screen->h;
		double snesAspect = (double)SNES_FULL_WIDTH/SNES_FULL_HEIGHT;

		GLuint x_off=0,y_off=0;

		if (screenAspect > snesAspect)
		{
			// widescreen monitor, 4:3 snes
			//  match height, scale width
			glOrtho(0,SNES_FULL_HEIGHT*screenAspect,SNES_FULL_HEIGHT,0,-1.0f,1.0f);
			x_off=((SNES_FULL_HEIGHT*screenAspect)-SNES_FULL_WIDTH) / 2;
		} else {
			// narrow monitor, 4:3 snes
			//  match width, scale height
	// grkenn FIXME: TODO: this calculation is wrong wrong wrong
			glOrtho(0,SNES_FULL_WIDTH,SNES_FULL_WIDTH*screenAspect,0,-1.0f,1.0f);
			y_off = ((SNES_FULL_WIDTH*screenAspect)-SNES_FULL_HEIGHT) / 2;
		}
		
		glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();

		// Create the display list to draw to the screen.
		GUI.displaylist = glGenLists(1);
		glNewList(GUI.displaylist,GL_COMPILE);
			// clear screen
			glClear(GL_COLOR_BUFFER_BIT);
			glBegin(GL_QUADS);
				glTexCoord2f(0,0);
				glVertex2i(x_off,y_off);
				glTexCoord2f(1,0);
				glVertex2i(x_off+SNES_FULL_WIDTH,y_off);
				glTexCoord2f(1,1);
				glVertex2i(x_off+SNES_FULL_WIDTH,y_off+SNES_FULL_HEIGHT);
				glTexCoord2f(0,1);
				glVertex2i(x_off,y_off+SNES_FULL_HEIGHT);
			glEnd();
		glEndList();

		// All set!
	}
	else
#endif
	if (GUI.fullscreen == TRUE)
	{
		int offset_height_pix;
		int offset_width_pix;
		int offset_byte;


		offset_height_pix = (GUI.sdl_screen->h - (SNES_HEIGHT * 2)) / 2;
		offset_width_pix = (GUI.sdl_screen->w - (SNES_WIDTH * 2)) / 2;
		
		offset_byte = (GUI.sdl_screen->w * offset_height_pix + offset_width_pix) * 2;

		GUI.blit_screen       = (uint8 *) GUI.sdl_screen->pixels + offset_byte;
		GUI.blit_screen_pitch = GUI.sdl_screen->w * 2;
	}
	else 
	{
		GUI.blit_screen       = (uint8 *) GUI.sdl_screen->pixels;
		GUI.blit_screen_pitch = SNES_WIDTH * 2 * 2; // window size =(*2); 2 byte pir pixel =(*2)
	}

	S9xGraphicsInit();
}

void S9xPutImage (int width, int height)
{
	static int	prevWidth = 0, prevHeight = 0;
	int			copyWidth, copyHeight;
	Blitter		blitFn = NULL;

	if (GUI.video_mode == VIDEOMODE_BLOCKY || GUI.video_mode == VIDEOMODE_TV || GUI.video_mode == VIDEOMODE_SMOOTH)
		if ((width <= SNES_WIDTH) && ((prevWidth != width) || (prevHeight != height)))
			S9xBlitClearDelta();

	if (width <= SNES_WIDTH)
	{
		if (height > SNES_HEIGHT_EXTENDED)
		{
			copyWidth  = width * 2;
			copyHeight = height;
			blitFn = S9xBlitPixSimple2x1;
		}
		else
		{
			copyWidth  = width  * 2;
			copyHeight = height * 2;

			switch (GUI.video_mode)
			{
				case VIDEOMODE_BLOCKY:		blitFn = S9xBlitPixSimple2x2;		break;
				case VIDEOMODE_TV:			blitFn = S9xBlitPixTV2x2;			break;
				case VIDEOMODE_SMOOTH:		blitFn = S9xBlitPixSmooth2x2;		break;
				case VIDEOMODE_SUPEREAGLE:	blitFn = S9xBlitPixSuperEagle16;	break;
				case VIDEOMODE_2XSAI:		blitFn = S9xBlitPix2xSaI16;			break;
				case VIDEOMODE_SUPER2XSAI:	blitFn = S9xBlitPixSuper2xSaI16;	break;
				case VIDEOMODE_EPX:			blitFn = S9xBlitPixEPX16;			break;
				case VIDEOMODE_HQ2X:		blitFn = S9xBlitPixHQ2x16;			break;
			}
		}
	}
	else
	if (height <= SNES_HEIGHT_EXTENDED)
	{
		copyWidth  = width;
		copyHeight = height * 2;

		switch (GUI.video_mode)
		{
			default:					blitFn = S9xBlitPixSimple1x2;	break;
			case VIDEOMODE_TV:			blitFn = S9xBlitPixTV1x2;		break;
		}
	}
	else
	{
		copyWidth  = width;
		copyHeight = height;
		blitFn = S9xBlitPixSimple1x1;
	}


	// domaemon: this is place where the rendering buffer size should be changed?
	blitFn((uint8 *) GFX.Screen, GFX.Pitch, GUI.blit_screen, GUI.blit_screen_pitch, width, height);

	//  grkenn: TODO: no idea.  But I did put opengl code here so that's something : )
#ifdef USE_OPENGL
	if (GUI.opengl)
	{
		// update GL texture
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, (uint8 *) GFX.Screen);
	}
#endif

	// domaemon: does the height change on the fly?
	if (height < prevHeight)
	{
		int	p = GUI.blit_screen_pitch >> 2;
		for (int y = SNES_HEIGHT * 2; y < SNES_HEIGHT_EXTENDED * 2; y++)
		{
			uint32	*d = (uint32 *) (GUI.blit_screen + y * GUI.blit_screen_pitch);
			for (int x = 0; x < p; x++)
				*d++ = 0;
		}
	}

	Repaint(TRUE);

	prevWidth  = width;
	prevHeight = height;
}

static void Repaint (bool8 isFrameBoundry)
{
#ifdef USE_OPENGL
	if (GUI.opengl)	{ glCallList(GUI.displaylist); SDL_GL_SwapBuffers(); }
	else
#endif
        SDL_Flip(GUI.sdl_screen);
}

void S9xMessage (int type, int number, const char *message)
{
	const int	max = 36 * 3;
	static char	buffer[max + 1];

	fprintf(stdout, "%s\n", message);
	strncpy(buffer, message, max + 1);
	buffer[max] = 0;
	S9xSetInfoString(buffer);
}

const char * S9xStringInput (const char *message)
{
	static char	buffer[256];

	printf("%s: ", message);
	fflush(stdout);

	if (fgets(buffer, sizeof(buffer) - 2, stdin))
		return (buffer);

	return (NULL);
}

void S9xSetTitle (const char *string)
{
	SDL_WM_SetCaption(string, string);
}

void S9xSetPalette (void)
{
	return;
}
