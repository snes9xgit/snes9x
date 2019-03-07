/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#ifdef USE_XVIDEO
#include <X11/extensions/Xvlib.h>

#define FOURCC_YUY2 0x32595559
#endif

#ifdef USE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
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

// Wrapper struct to make generic XvImage vs XImage
struct Image
{
#ifdef USE_XVIDEO
	union
	{
		XvImage*	xvimage;
#endif
		XImage*	ximage;
#ifdef USE_XVIDEO
	};
#endif

	char *data;

	uint32 height;
	uint32 data_size;
	uint32 bits_per_pixel;
	uint32 bytes_per_line;
};

struct GUIData
{
	Display			*display;
	Screen			*screen;
	Visual			*visual;
	GC				gc;
	int				screen_num;
	int				depth;
	int				pixel_format;
	int				bytes_per_pixel;
	uint32			red_shift;
	uint32			blue_shift;
	uint32			green_shift;
	uint32			red_size;
	uint32			green_size;
	uint32			blue_size;
	Window			window;
	Image			*image;
	uint8			*snes_buffer;
	uint8			*filter_buffer;
	uint8			*blit_screen;
	uint32			blit_screen_pitch;
	bool8			need_convert;
	Cursor			point_cursor;
	Cursor			cross_hair_cursor;
	int				video_mode;
	int				mouse_x;
	int				mouse_y;
	bool8			mod1_pressed;
	bool8			no_repeat;
	bool8			fullscreen;
	int				x_offset;
	int				y_offset;
#ifdef USE_XVIDEO
	bool8			use_xvideo;
	int				xv_port;
	int				scale_w;
	int				scale_h;

	bool8			maxaspect;
	int			imageHeight;

	int				xv_format;
	int				xv_bpp;
	unsigned char		y_table[1 << 15];
	unsigned char		u_table[1 << 15];
	unsigned char		v_table[1 << 15];
#endif
#ifdef USE_XINERAMA
    uint32 xinerama_head;
#endif
#ifdef MITSHM
	XShmSegmentInfo	sm_info;
	bool8			use_shared_memory;
#endif
};

static struct GUIData	GUI;

typedef	std::pair<std::string, std::string>	strpair_t;
extern	std::vector<strpair_t>				keymaps;

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

static int ErrorHandler (Display *, XErrorEvent *);
static bool8 CheckForPendingXEvents (Display *);
static void SetXRepeat (bool8);
static void SetupImage (void);
static void TakedownImage (void);
static void SetupXImage (void);
static void TakedownXImage (void);
#ifdef USE_XVIDEO
static void SetupXvImage (void);
static void TakedownXvImage (void);
#endif
static void Repaint (bool8);
static void Convert16To24 (int, int);
static void Convert16To24Packed (int, int);


void S9xExtraDisplayUsage (void)
{
	/*                               12345678901234567890123456789012345678901234567890123456789012345678901234567890 */

	S9xMessage(S9X_INFO, S9X_USAGE, "-setrepeat                      Allow altering keyboard auto-repeat");
	S9xMessage(S9X_INFO, S9X_USAGE, "");
	S9xMessage(S9X_INFO, S9X_USAGE, "-fullscreen                     Switch to full-screen on start");
#ifdef USE_XVIDEO
	S9xMessage(S9X_INFO, S9X_USAGE, "-xvideo                         Hardware accelerated scaling");
	S9xMessage(S9X_INFO, S9X_USAGE, "-maxaspect                      Try to fill the display, in fullscreen");
#endif
#ifdef USE_XINERAMA
	S9xMessage(S9X_INFO, S9X_USAGE, "-xineramahead                   Xinerama head number for multi-monitor setups");
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
	if (!strcasecmp(argv[i], "-setrepeat"))
		GUI.no_repeat = FALSE;
	else
	if (!strcasecmp(argv[i], "-fullscreen"))
		GUI.fullscreen = TRUE;
	else
#ifdef USE_XVIDEO
	if (!strcasecmp(argv[i], "-xvideo"))
		GUI.use_xvideo = TRUE;
	else
	if (!strcasecmp(argv[i], "-maxaspect"))
		GUI.maxaspect = TRUE;
	else
#endif
#ifdef USE_XINERAMA
	if (!strcasecmp(argv[i], "-xineramahead"))
	{
		if (i + 1 < argc)
			GUI.xinerama_head = atoi(argv[++i]);
		else
			S9xUsage();
	}
	else
#endif
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
		return ("Unix/X11");

	if (!conf.GetBool("Unix::ClearAllControls", false))
	{
		keymaps.push_back(strpair_t("K00:k",            "Joypad1 Right"));
		keymaps.push_back(strpair_t("K00:Right",        "Joypad1 Right"));
		keymaps.push_back(strpair_t("K00:h",            "Joypad1 Left"));
		keymaps.push_back(strpair_t("K00:Left",         "Joypad1 Left"));
		keymaps.push_back(strpair_t("K00:j",            "Joypad1 Down"));
		keymaps.push_back(strpair_t("K00:n",            "Joypad1 Down"));
		keymaps.push_back(strpair_t("K00:Down",         "Joypad1 Down"));
		keymaps.push_back(strpair_t("K00:u",            "Joypad1 Up"));
		keymaps.push_back(strpair_t("K00:Up",           "Joypad1 Up"));
		keymaps.push_back(strpair_t("K00:Return",       "Joypad1 Start"));
		keymaps.push_back(strpair_t("K00:space",        "Joypad1 Select"));
		keymaps.push_back(strpair_t("K00:S+d",          "Joypad1 ToggleTurbo A"));
		keymaps.push_back(strpair_t("K00:C+d",          "Joypad1 ToggleSticky A"));
		keymaps.push_back(strpair_t("K00:d",            "Joypad1 A"));
		keymaps.push_back(strpair_t("K00:S+c",          "Joypad1 ToggleTurbo B"));
		keymaps.push_back(strpair_t("K00:C+c",          "Joypad1 ToggleSticky B"));
		keymaps.push_back(strpair_t("K00:c",            "Joypad1 B"));
		keymaps.push_back(strpair_t("K00:S+s",          "Joypad1 ToggleTurbo X"));
		keymaps.push_back(strpair_t("K00:C+s",          "Joypad1 ToggleSticky X"));
		keymaps.push_back(strpair_t("K00:s",            "Joypad1 X"));
		keymaps.push_back(strpair_t("K00:S+x",          "Joypad1 ToggleTurbo Y"));
		keymaps.push_back(strpair_t("K00:C+x",          "Joypad1 ToggleSticky Y"));
		keymaps.push_back(strpair_t("K00:x",            "Joypad1 Y"));
		keymaps.push_back(strpair_t("K00:S+a",          "Joypad1 ToggleTurbo L"));
		keymaps.push_back(strpair_t("K00:S+v",          "Joypad1 ToggleTurbo L"));
		keymaps.push_back(strpair_t("K00:C+a",          "Joypad1 ToggleSticky L"));
		keymaps.push_back(strpair_t("K00:C+v",          "Joypad1 ToggleSticky L"));
		keymaps.push_back(strpair_t("K00:a",            "Joypad1 L"));
		keymaps.push_back(strpair_t("K00:v",            "Joypad1 L"));
		keymaps.push_back(strpair_t("K00:S+z",          "Joypad1 ToggleTurbo R"));
		keymaps.push_back(strpair_t("K00:C+z",          "Joypad1 ToggleSticky R"));
		keymaps.push_back(strpair_t("K00:z",            "Joypad1 R"));

		keymaps.push_back(strpair_t("K00:KP_Left",      "Joypad2 Left"));
		keymaps.push_back(strpair_t("K00:KP_Right",     "Joypad2 Right"));
		keymaps.push_back(strpair_t("K00:KP_Up",        "Joypad2 Up"));
		keymaps.push_back(strpair_t("K00:KP_Down",      "Joypad2 Down"));
		keymaps.push_back(strpair_t("K00:KP_Enter",     "Joypad2 Start"));
		keymaps.push_back(strpair_t("K00:KP_Add",       "Joypad2 Select"));
		keymaps.push_back(strpair_t("K00:Prior",        "Joypad2 A"));
		keymaps.push_back(strpair_t("K00:Next",         "Joypad2 B"));
		keymaps.push_back(strpair_t("K00:Home",         "Joypad2 X"));
		keymaps.push_back(strpair_t("K00:End",          "Joypad2 Y"));
		keymaps.push_back(strpair_t("K00:Insert",       "Joypad2 L"));
		keymaps.push_back(strpair_t("K00:Delete",       "Joypad2 R"));

		keymaps.push_back(strpair_t("K00:A+F4",         "SoundChannel0"));
		keymaps.push_back(strpair_t("K00:C+F4",         "SoundChannel0"));
		keymaps.push_back(strpair_t("K00:A+F5",         "SoundChannel1"));
		keymaps.push_back(strpair_t("K00:C+F5",         "SoundChannel1"));
		keymaps.push_back(strpair_t("K00:A+F6",         "SoundChannel2"));
		keymaps.push_back(strpair_t("K00:C+F6",         "SoundChannel2"));
		keymaps.push_back(strpair_t("K00:A+F7",         "SoundChannel3"));
		keymaps.push_back(strpair_t("K00:C+F7",         "SoundChannel3"));
		keymaps.push_back(strpair_t("K00:A+F8",         "SoundChannel4"));
		keymaps.push_back(strpair_t("K00:C+F8",         "SoundChannel4"));
		keymaps.push_back(strpair_t("K00:A+F9",         "SoundChannel5"));
		keymaps.push_back(strpair_t("K00:C+F9",         "SoundChannel5"));
		keymaps.push_back(strpair_t("K00:A+F10",        "SoundChannel6"));
		keymaps.push_back(strpair_t("K00:C+F10",        "SoundChannel6"));
		keymaps.push_back(strpair_t("K00:A+F11",        "SoundChannel7"));
		keymaps.push_back(strpair_t("K00:C+F11",        "SoundChannel7"));
		keymaps.push_back(strpair_t("K00:A+F12",        "SoundChannelsOn"));
		keymaps.push_back(strpair_t("K00:C+F12",        "SoundChannelsOn"));

		keymaps.push_back(strpair_t("K00:S+1",          "BeginRecordingMovie"));
		keymaps.push_back(strpair_t("K00:S+2",          "EndRecordingMovie"));
		keymaps.push_back(strpair_t("K00:S+3",          "LoadMovie"));
		keymaps.push_back(strpair_t("K00:A+F1",         "SaveSPC"));
		keymaps.push_back(strpair_t("K00:C+F1",         "SaveSPC"));
		keymaps.push_back(strpair_t("K00:F10",          "LoadOopsFile"));
		keymaps.push_back(strpair_t("K00:A+F2",         "LoadFreezeFile"));
		keymaps.push_back(strpair_t("K00:C+F2",         "LoadFreezeFile"));
		keymaps.push_back(strpair_t("K00:F11",          "LoadFreezeFile"));
		keymaps.push_back(strpair_t("K00:A+F3",         "SaveFreezeFile"));
		keymaps.push_back(strpair_t("K00:C+F3",         "SaveFreezeFile"));
		keymaps.push_back(strpair_t("K00:F12",          "SaveFreezeFile"));
		keymaps.push_back(strpair_t("K00:F1",           "QuickLoad000"));
		keymaps.push_back(strpair_t("K00:F2",           "QuickLoad001"));
		keymaps.push_back(strpair_t("K00:F3",           "QuickLoad002"));
		keymaps.push_back(strpair_t("K00:F4",           "QuickLoad003"));
		keymaps.push_back(strpair_t("K00:F5",           "QuickLoad004"));
		keymaps.push_back(strpair_t("K00:F6",           "QuickLoad005"));
		keymaps.push_back(strpair_t("K00:F7",           "QuickLoad006"));
		keymaps.push_back(strpair_t("K00:F8",           "QuickLoad007"));
		keymaps.push_back(strpair_t("K00:F9",           "QuickLoad008"));
		keymaps.push_back(strpair_t("K00:S+F1",         "QuickSave000"));
		keymaps.push_back(strpair_t("K00:S+F2",         "QuickSave001"));
		keymaps.push_back(strpair_t("K00:S+F3",         "QuickSave002"));
		keymaps.push_back(strpair_t("K00:S+F4",         "QuickSave003"));
		keymaps.push_back(strpair_t("K00:S+F5",         "QuickSave004"));
		keymaps.push_back(strpair_t("K00:S+F6",         "QuickSave005"));
		keymaps.push_back(strpair_t("K00:S+F7",         "QuickSave006"));
		keymaps.push_back(strpair_t("K00:S+F8",         "QuickSave007"));
		keymaps.push_back(strpair_t("K00:S+F9",         "QuickSave008"));

		keymaps.push_back(strpair_t("K00:Scroll_Lock",  "Pause"));
		keymaps.push_back(strpair_t("K00:CS+Escape",    "Reset"));
		keymaps.push_back(strpair_t("K00:S+Escape",     "SoftReset"));
		keymaps.push_back(strpair_t("K00:Escape",       "ExitEmu"));
		keymaps.push_back(strpair_t("K00:Tab",          "EmuTurbo"));
		keymaps.push_back(strpair_t("K00:S+Tab",        "ToggleEmuTurbo"));
		keymaps.push_back(strpair_t("K00:A+equal",      "IncEmuTurbo"));
		keymaps.push_back(strpair_t("K00:A+minus",      "DecEmuTurbo"));
		keymaps.push_back(strpair_t("K00:C+equal",      "IncTurboSpeed"));
		keymaps.push_back(strpair_t("K00:C+minus",      "DecTurboSpeed"));
		keymaps.push_back(strpair_t("K00:equal",        "IncFrameRate"));
		keymaps.push_back(strpair_t("K00:minus",        "DecFrameRate"));
		keymaps.push_back(strpair_t("K00:S+equal",      "IncFrameTime"));
		keymaps.push_back(strpair_t("K00:S+minus",      "DecFrameTime"));
		keymaps.push_back(strpair_t("K00:6",            "SwapJoypads"));
		keymaps.push_back(strpair_t("K00:Print",        "Screenshot"));

		keymaps.push_back(strpair_t("K00:1",            "ToggleBG0"));
		keymaps.push_back(strpair_t("K00:2",            "ToggleBG1"));
		keymaps.push_back(strpair_t("K00:3",            "ToggleBG2"));
		keymaps.push_back(strpair_t("K00:4",            "ToggleBG3"));
		keymaps.push_back(strpair_t("K00:5",            "ToggleSprites"));
		keymaps.push_back(strpair_t("K00:9",            "ToggleTransparency"));
		keymaps.push_back(strpair_t("K00:BackSpace",    "ClipWindows"));
		keymaps.push_back(strpair_t("K00:A+Escape",     "Debugger"));

		keymaps.push_back(strpair_t("M00:B0",           "{Mouse1 L,Superscope Fire,Justifier1 Trigger}"));
		keymaps.push_back(strpair_t("M00:B1",           "{Justifier1 AimOffscreen Trigger,Superscope AimOffscreen}"));
		keymaps.push_back(strpair_t("M00:B2",           "{Mouse1 R,Superscope Cursor,Justifier1 Start}"));
		keymaps.push_back(strpair_t("M00:Pointer",      "Pointer Mouse1+Superscope+Justifier1"));
		keymaps.push_back(strpair_t("K00:grave",        "Superscope ToggleTurbo"));
		keymaps.push_back(strpair_t("K00:slash",        "Superscope Pause"));

		keymaps.push_back(strpair_t("K00:r",            "Rewind"));
                keymaps.push_back(strpair_t("K00:l",            "Advance"));
	}

	GUI.no_repeat = !conf.GetBool("Unix/X11::SetKeyRepeat", TRUE);
	GUI.fullscreen = conf.GetBool("Unix/X11::Fullscreen", FALSE);
#ifdef USE_XVIDEO
	GUI.use_xvideo = conf.GetBool("Unix/X11::Xvideo", FALSE);
	GUI.maxaspect = conf.GetBool("Unix/X11::MaxAspect", FALSE);
#endif
#ifdef USE_XINERAMA
    GUI.xinerama_head = conf.GetUInt("Unix/X11::XineramaHead", 0);
#endif

	if (conf.Exists("Unix/X11::VideoMode"))
	{
		GUI.video_mode = conf.GetUInt("Unix/X11::VideoMode", VIDEOMODE_BLOCKY);
		if (GUI.video_mode < 1 || GUI.video_mode > 8)
			GUI.video_mode = VIDEOMODE_BLOCKY;
	}
	else
		GUI.video_mode = VIDEOMODE_BLOCKY;

	return ("Unix/X11");
}

static void FatalError (const char *str)
{
	fprintf(stderr, "%s\n", str);
	S9xExit();
}

static int ErrorHandler (Display *display, XErrorEvent *event)
{
#ifdef MITSHM
	GUI.use_shared_memory = FALSE;
#endif
	return (0);
}

#ifdef USE_XVIDEO
static int get_inv_shift (uint32 mask, int bpp)
{
    int i;

    // Find mask
    for (i = 0; (i < bpp) && !(mask & (1 << i)); i++) {};

    // Find start of mask
    for (; (i < bpp) && (mask & (1 << i)); i++) {};

    return (bpp - i);
}

static unsigned char CLAMP (int v, int min, int max)
{
	if (v < min) return min;
	if (v > max) return max;
	return v;
}

static bool8 SetupXvideo()
{
	int ret;

	// Init xv_port
	GUI.xv_port = -1;

	/////////////////////
	// Check that Xvideo extension seems OK
	unsigned int p_version, p_release, p_request_base, p_event_base, p_error_base;
	ret = XvQueryExtension(GUI.display,
		&p_version, &p_release, &p_request_base,
		&p_event_base, &p_error_base);
	if (ret != Success) { fprintf(stderr,"XvQueryExtension error\n"); return FALSE; }
	printf("XvExtension version %i.%i\n",p_version,p_release);

	/////////////////////
	// Get info about the Adaptors available for this window
	unsigned int	p_num_adaptors;
	XvAdaptorInfo*	ai;
	ret = XvQueryAdaptors(GUI.display, GUI.window, &p_num_adaptors, &ai);

	if (ret != Success || p_num_adaptors == 0) {
		fprintf(stderr,"XvQueryAdaptors error.");
		return FALSE;
	}
	printf("XvQueryAdaptors: %d adaptor(s) found.\n",p_num_adaptors);

	unsigned int minAdaptor = 0, maxAdaptor = p_num_adaptors;
	// Allow user to force adaptor choice
	/* if (adaptor >= 0 && adaptor < p_num_adaptors)
	{
		if (verbose) std::cout << "Forcing adaptor " << adaptor << ", '" << ai[adaptor].name << "'" << std::endl;
		minAdaptor = adaptor;
		maxAdaptor = adaptor + 1;
	} */

	/////////////////////
	// Iterate through list of available adaptors.
	//  Grab a port if we can.
	for (unsigned int i = minAdaptor; i < maxAdaptor && GUI.xv_port < 0; i++)
	{
		// We need to find one supporting XvInputMask and XvImageMask.
		if (! (ai[i].type & XvImageMask)) continue;
		if (! (ai[i].type & XvInputMask)) continue;

		printf("\tAdaptor #%d: [%s]: %ld port(s) available.\n", i, ai[i].name, ai[i].num_ports);

		// Get encodings available here
		//  AFAIK all ports on an adapter share the same encodings info.
		unsigned int encodings;
		XvEncodingInfo	*ei;
		ret = XvQueryEncodings(GUI.display, ai[i].base_id, &encodings, &ei);
		if (ret != Success || encodings == 0) {
	                fprintf(stderr,"XvQueryEncodings error.");
			continue;
		}

		// Ensure the XV_IMAGE encoding available has sufficient width/height for us.
		bool8 can_fit = FALSE;
		for (unsigned int j = 0; j < encodings; j++)
		{
			if (strcmp(ei[j].name,"XV_IMAGE")) continue;
			if (ei[j].width >= SNES_WIDTH * 2 &&
				ei[j].height >= SNES_HEIGHT_EXTENDED * 2)
			{
				can_fit = TRUE;
				break;
			}
		}
		XvFreeEncodingInfo(ei);

		if (can_fit == FALSE)
		{
			fprintf(stderr,"\tDid not find XV_IMAGE encoding with enough max size\n");
			continue;
		}

		// Phew. If we've made it this far, we can try to choose it for our output port.
		for (unsigned int p = ai[i].base_id; p < ai[i].base_id+ai[i].num_ports; p++)
		{
			ret = XvGrabPort(GUI.display, p, CurrentTime);
			if (ret == Success)
			{
				printf("\tSuccessfully bound to Xv port %d\n",p);
				GUI.xv_port = p;
				break;
			} else {
				fprintf(stderr,"\tXvGrabPort port %d fail.\n",p);
			}
		}
	}
	XvFreeAdaptorInfo(ai);

	/////////////////////
	// Bail out here if we haven't managed to bind to any port.
	if (GUI.xv_port < 0)
	{
		fprintf(stderr,"No suitable xv_port found in any Adaptors.\n");
		return FALSE;
	}

	// Xv ports can have Attributes (hue, saturation, etc)
	/* Set XV_AUTOPAINT_COLORKEY _only_ if available */
	int num_attrs;
	XvAttribute*	port_attr;
	port_attr = XvQueryPortAttributes (GUI.display, GUI.xv_port, &num_attrs);

	for (int i = 0; i < num_attrs; i++)
	{
		if (!strcmp (port_attr[i].name, "XV_AUTOPAINT_COLORKEY"))
		{
			Atom colorkey;

			colorkey = XInternAtom (GUI.display, "XV_AUTOPAINT_COLORKEY", True);
			if (colorkey != None)
			{
				XvSetPortAttribute (GUI.display, GUI.xv_port, colorkey, 1);
				printf("\tSet XV_AUTOPAINT_COLORKEY.\n");
			}
		}
	}
	XFree(port_attr);

	// Now we need to find to find the image format to use for output.
	//  There are two steps to this:
	//    Prefer an XvRGB version of lowest bitdepth.
	//  If that's not available use YUY2
	int	formats;
	XvImageFormatValues*	fo;
	fo = XvListImageFormats(GUI.display, GUI.xv_port, &formats);
	if (formats == 0)
	{
		fprintf(stderr,"No valid image formats for Xv port!");
		return FALSE;
	}

	/* Ok time to search for a good Format */
	GUI.xv_format = FOURCC_YUY2;
	GUI.xv_bpp = 0x7FFFFFFF;

	for (int i = 0; i < formats; i++)
	{
		if (fo[i].id == 0x3 || fo[i].type == XvRGB)
		{
			if (fo[i].bits_per_pixel < GUI.xv_bpp)
			{
				GUI.xv_format = fo[i].id;
				GUI.xv_bpp = fo[i].bits_per_pixel;
				GUI.bytes_per_pixel = (GUI.xv_bpp == 15) ? 2 : GUI.xv_bpp >> 3;
				GUI.depth = fo[i].depth;

				GUI.red_shift = get_inv_shift (fo[i].red_mask, GUI.xv_bpp);
				GUI.green_shift = get_inv_shift (fo[i].green_mask, GUI.xv_bpp);
				GUI.blue_shift = get_inv_shift (fo[i].blue_mask, GUI.xv_bpp);

				/* Check for red-blue inversion on SiliconMotion drivers */
				if (fo[i].red_mask  == 0x001f &&
					fo[i].blue_mask == 0x7c00)
				{
					int copy = GUI.red_shift;
					GUI.red_shift = GUI.blue_shift;
					GUI.blue_shift = copy;
				}

				/* on big-endian Xv still seems to like LSB order */
				/*if (config->force_inverted_byte_order)
					S9xSetEndianess (ENDIAN_MSB);
				else
					S9xSetEndianess (ENDIAN_LSB); */
			}
		}
	}
	free (fo);

	if (GUI.xv_format != FOURCC_YUY2)
	{
		printf("Selected XvRGB format: %d bpp\n",GUI.xv_bpp);
	} else {
		// use YUY2
		printf("Fallback to YUY2 format.\n");
		GUI.depth = 15;

		/* Build a table for yuv conversion */
		for (unsigned int color = 0; color < (1 << 15); color++)
		{
			int r, g, b;
			int y, u, v;

			r = (color & 0x7c00) >> 7;
			g = (color & 0x03e0) >> 2;
			b = (color & 0x001F) << 3;

			y = (int) ((0.257  * ((double) r)) + (0.504  * ((double) g)) + (0.098  * ((double) b)) + 16.0);
			u = (int) ((-0.148 * ((double) r)) + (-0.291 * ((double) g)) + (0.439  * ((double) b)) + 128.0);
			v = (int) ((0.439  * ((double) r)) + (-0.368 * ((double) g)) + (-0.071 * ((double) b)) + 128.0);

			GUI.y_table[color] = CLAMP (y, 0, 255);
			GUI.u_table[color] = CLAMP (u, 0, 255);
			GUI.v_table[color] = CLAMP (v, 0, 255);
		}
	}

	return TRUE;
}
#endif

void S9xInitDisplay (int argc, char **argv)
{
	GUI.display = XOpenDisplay(NULL);
	if (GUI.display == NULL)
		FatalError("Failed to connect to X server.");

	GUI.screen     = DefaultScreenOfDisplay(GUI.display);
	GUI.screen_num = XScreenNumberOfScreen(GUI.screen);
	GUI.visual     = DefaultVisualOfScreen(GUI.screen);

	XVisualInfo	plate, *matches;
	int			count;

	plate.visualid = XVisualIDFromVisual(GUI.visual);
	matches = XGetVisualInfo(GUI.display, VisualIDMask, &plate, &count);
	if (!count)
		FatalError("Your X Window System server is unwell!");

	GUI.depth = matches[0].depth;
	if ((GUI.depth != 15 && GUI.depth != 16 && GUI.depth != 24) || (matches[0].c_class != TrueColor))
		FatalError("Requiers 15, 16, 24 or 32-bit color depth supporting TrueColor.");

	GUI.red_shift   = ffs(matches[0].red_mask)   - 1;
	GUI.green_shift = ffs(matches[0].green_mask) - 1;
	GUI.blue_shift  = ffs(matches[0].blue_mask)  - 1;
	GUI.red_size    = matches[0].red_mask   >> GUI.red_shift;
	GUI.green_size  = matches[0].green_mask >> GUI.green_shift;
	GUI.blue_size   = matches[0].blue_mask  >> GUI.blue_shift;
	if (GUI.depth == 16 && GUI.green_size == 63)
		GUI.green_shift++;

	XFree(matches);

	// Init various scale-filters
	S9xBlitFilterInit();
	S9xBlit2xSaIFilterInit();
	S9xBlitHQ2xFilterInit();

	/* Set up parameters for creating the window */
	XSetWindowAttributes	attrib;

	memset(&attrib, 0, sizeof(attrib));
	attrib.background_pixel = BlackPixelOfScreen(GUI.screen);
	attrib.colormap = XCreateColormap(GUI.display, RootWindowOfScreen(GUI.screen), GUI.visual, AllocNone);

	int screen_left = 0, screen_top = 0;
	int screen_w = WidthOfScreen(GUI.screen), screen_h = HeightOfScreen(GUI.screen);

#ifdef USE_XINERAMA
	int heads = 0;
	XineramaScreenInfo* si = 0;

	int useless1, useless2;
	if (!XineramaQueryExtension(GUI.display, &useless1, &useless2)) {
		puts("Xinerama is not available");
		goto xinerama_end;
	}

	if (!XineramaIsActive(GUI.display)) {
		puts("Xinerama is not active");
		goto xinerama_end;
	}

	si = XineramaQueryScreens(GUI.display, &heads);
	if (!si) {
		puts("XineramaQueryScreens failed");
		goto xinerama_end;
	}

	if (GUI.xinerama_head >= heads) {
		printf("Invalid xinerama head id (expected 0-%d, got %u)\n", heads - 1, GUI.xinerama_head);
		goto xinerama_end;
	}

	si = &si[GUI.xinerama_head];
	screen_left = si->x_org;
	screen_top = si->y_org;
	screen_w = si->width;
	screen_h = si->height;

	printf("Selected xinerama head %u (%d,%d %dx%d)\n", GUI.xinerama_head, screen_left, screen_top, screen_w, screen_h);

xinerama_end:
#endif

	XSizeHints      Hints;
	memset((void *) &Hints, 0, sizeof(XSizeHints));

	/* Try to switch to Fullscreen. */
	if (GUI.fullscreen == TRUE)
	{
		Hints.flags = PPosition;
		Hints.x     = screen_left;
		Hints.y     = screen_top;

		/* Create the window with maximum screen width,height positioned at 0,0. */
		GUI.window = XCreateWindow(GUI.display, RootWindowOfScreen(GUI.screen),
							Hints.x, Hints.y,
							screen_w, screen_h, 0,
							GUI.depth, InputOutput, GUI.visual, CWBackPixel | CWColormap, &attrib);

		/* Try to tell the Window Manager not to decorate this window. */
		Atom wm_state   = XInternAtom (GUI.display, "_NET_WM_STATE", true );
		Atom wm_fullscreen = XInternAtom (GUI.display, "_NET_WM_STATE_FULLSCREEN", true );

		XChangeProperty(GUI.display, GUI.window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&wm_fullscreen, 1);

#ifdef USE_XVIDEO
		if (GUI.use_xvideo)
		{
			// Set some defaults
			GUI.scale_w = screen_w;
			GUI.scale_h = screen_h;

			GUI.imageHeight = SNES_HEIGHT_EXTENDED * 2;

			if (! GUI.maxaspect)
			{
				// Compute the maximum screen size for scaling xvideo window.
				double screenAspect = (double)screen_w / screen_h;
				double snesAspect = (double)SNES_WIDTH / SNES_HEIGHT_EXTENDED;
				double ratio = screenAspect / snesAspect;

				printf("\tScreen (%dx%d) aspect %f vs SNES (%dx%d) aspect %f (ratio: %f)\n",
					screen_w,screen_h,screenAspect,
					SNES_WIDTH,SNES_HEIGHT_EXTENDED,snesAspect,
					ratio);

				// Correct aspect ratio
				if (screenAspect > snesAspect)
				{
					// widescreen monitor, 4:3 snes
					//  match height, scale width
					GUI.scale_w /= ratio;
					GUI.x_offset = (screen_w - GUI.scale_w) / 2;
				} else {
					// narrow monitor, 4:3 snes
					//  match width, scale height
					GUI.scale_h *= ratio;
					GUI.y_offset = (screen_h - GUI.scale_h) / 2;
				}
			}

			printf("\tUsing size %dx%d with offset (%d,%d)\n",GUI.scale_w,GUI.scale_h,GUI.x_offset,GUI.y_offset);
		}
		else
#endif
		{
			/* Last: position the output window in the center of the screen. */
			GUI.x_offset = (screen_w - SNES_WIDTH * 2) / 2;
			GUI.y_offset = (screen_h - SNES_HEIGHT_EXTENDED * 2) / 2;
		}
	} else {
		/* Tell the Window Manager that we do not wish to be resizable */
		Hints.flags      = PSize | PMinSize | PMaxSize | PPosition;
		Hints.x          = screen_left + (screen_w - SNES_WIDTH * 2) / 2;
		Hints.y          = screen_top + (screen_h - SNES_HEIGHT_EXTENDED * 2) / 2;
		Hints.min_width  = Hints.max_width  = Hints.base_width  = SNES_WIDTH * 2;
		Hints.min_height = Hints.max_height = Hints.base_height = SNES_HEIGHT_EXTENDED * 2;

		/* Create the window. */
		GUI.window = XCreateWindow(GUI.display, RootWindowOfScreen(GUI.screen),
								   Hints.x, Hints.y,
								   SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2, 0, GUI.depth, InputOutput, GUI.visual, CWBackPixel | CWColormap, &attrib);

		/* Last: Windowed SNES is not drawn with any offsets. */
		GUI.x_offset = GUI.y_offset = 0;
#ifdef USE_XVIDEO
		GUI.scale_w = SNES_WIDTH * 2;
		GUI.scale_h = SNES_HEIGHT_EXTENDED * 2;
#endif
	}

	XSetWMNormalHints(GUI.display, GUI.window, &Hints);

	/* Load UI cursors */
	static XColor	bg, fg;
	static char		data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	Pixmap			bitmap;

	bitmap = XCreateBitmapFromData(GUI.display, GUI.window, data, 8, 8);
	GUI.point_cursor = XCreatePixmapCursor(GUI.display, bitmap, bitmap, &fg, &bg, 0, 0);
	XDefineCursor(GUI.display, GUI.window, GUI.point_cursor);
	GUI.cross_hair_cursor = XCreateFontCursor(GUI.display, XC_crosshair);

	GUI.gc = DefaultGCOfScreen(GUI.screen);

	/* Other window-manager hints */
	XWMHints	WMHints;

	memset((void *) &WMHints, 0, sizeof(XWMHints));

	/* Rely on the Window Manager to provide us with keyboard input */
	WMHints.input    = True;
	WMHints.flags    = InputHint;

	XSetWMHints(GUI.display, GUI.window, &WMHints);
	XSelectInput(GUI.display, GUI.window, FocusChangeMask | ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | ButtonPressMask | ButtonReleaseMask);

	/* Bring up our window (and put it in foreground) */
	XMapRaised(GUI.display, GUI.window);
	XClearWindow(GUI.display, GUI.window);

	// Wait for map
	XEvent event;
	do {
		XNextEvent(GUI.display, &event);
	} while (event.type != MapNotify || event.xmap.event != GUI.window);

#ifdef USE_XVIDEO
	if (GUI.use_xvideo)
	{
		GUI.use_xvideo = SetupXvideo();
	}
#endif

	GUI.pixel_format = 565;

	SetupImage();

	switch (GUI.depth)
	{
		default:
		case 32:
			GUI.bytes_per_pixel = 4;
			break;

		case 24:
			if (GUI.image->bits_per_pixel == 24)
				GUI.bytes_per_pixel = 3;
			else
				GUI.bytes_per_pixel = 4;
			break;

		case 15:
		case 16:
			GUI.bytes_per_pixel = 2;
			break;
	}

	printf("Using internal pixel format %d\n",GUI.pixel_format);
}

void S9xDeinitDisplay (void)
{
	TakedownImage();
	if (GUI.display != NULL)
	{
#ifdef USE_XVIDEO
		if (GUI.use_xvideo)
		{
			XvUngrabPort(GUI.display,GUI.xv_port,CurrentTime);
		}
#endif
		S9xTextMode();
		XSync(GUI.display, False);
		XCloseDisplay(GUI.display);
	}
	S9xBlitFilterDeinit();
	S9xBlit2xSaIFilterDeinit();
	S9xBlitHQ2xFilterDeinit();
}

static void SetupImage (void)
{
	TakedownImage();

	// Create new image struct
	GUI.image = (Image *) calloc(sizeof(Image), 1);

#ifdef USE_XVIDEO
	if (GUI.use_xvideo)
		SetupXvImage();
	if (!GUI.use_xvideo)
#endif
		SetupXImage();

	// Setup SNES buffers
	GFX.Pitch = SNES_WIDTH * 2 * 2;
	GUI.snes_buffer = (uint8 *) calloc(GFX.Pitch * ((SNES_HEIGHT_EXTENDED + 4) * 2), 1);
	if (!GUI.snes_buffer)
		FatalError("Failed to allocate GUI.snes_buffer.");

	GFX.Screen = (uint16 *) (GUI.snes_buffer + (GFX.Pitch * 2 * 2));

	GUI.filter_buffer = (uint8 *) calloc((SNES_WIDTH * 2) * 2 * (SNES_HEIGHT_EXTENDED * 2), 1);
	if (!GUI.filter_buffer)
		FatalError("Failed to allocate GUI.filter_buffer.");

#ifdef USE_XVIDEO
	if ((GUI.depth == 15 || GUI.depth == 16) && GUI.xv_format != FOURCC_YUY2)
#else
	if (GUI.depth == 15 || GUI.depth == 16)
#endif
	{
		GUI.blit_screen_pitch = GUI.image->bytes_per_line;
		GUI.blit_screen       = (uint8 *) GUI.image->data;
		GUI.need_convert      = FALSE;
	}
	else
	{
		GUI.blit_screen_pitch = (SNES_WIDTH * 2) * 2;
		GUI.blit_screen       = GUI.filter_buffer;
		GUI.need_convert      = TRUE;
	}
	if (GUI.need_convert) { printf("\tImage conversion needed before blit.\n"); }

	S9xGraphicsInit();
}

static void TakedownImage (void)
{
	if (GUI.snes_buffer)
	{
		free(GUI.snes_buffer);
		GUI.snes_buffer = NULL;
	}

	if (GUI.filter_buffer)
	{
		free(GUI.filter_buffer);
		GUI.filter_buffer = NULL;
	}

	if (GUI.image)
	{
#ifdef USE_XVIDEO
		if (GUI.use_xvideo)
			TakedownXvImage();
		else
#endif
			TakedownXImage();

		free(GUI.image);
		GUI.image = NULL;
	}

	S9xGraphicsDeinit();
}

static void SetupXImage (void)
{
#ifdef MITSHM
	GUI.use_shared_memory = TRUE;

	int		major, minor;
	Bool	shared;

	if (!XShmQueryVersion(GUI.display, &major, &minor, &shared) || !shared)
		GUI.image->ximage = NULL;
	else
		GUI.image->ximage = XShmCreateImage(GUI.display, GUI.visual, GUI.depth, ZPixmap, NULL, &GUI.sm_info, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2);

	if (!GUI.image->ximage)
		GUI.use_shared_memory = FALSE;
	else
	{
		// set main Image struct vars
		GUI.image->height = GUI.image->ximage->height;
		GUI.image->bytes_per_line = GUI.image->ximage->bytes_per_line;
		GUI.image->data_size = GUI.image->bytes_per_line * GUI.image->height;

		GUI.sm_info.shmid = shmget(IPC_PRIVATE, GUI.image->data_size, IPC_CREAT | 0777);
		if (GUI.sm_info.shmid < 0)
		{
			XDestroyImage(GUI.image->ximage);
			GUI.use_shared_memory = FALSE;
		}
		else
		{
			GUI.image->ximage->data = GUI.sm_info.shmaddr = (char *) shmat(GUI.sm_info.shmid, 0, 0);
			if (!GUI.image->ximage->data)
			{
				XDestroyImage(GUI.image->ximage);
				shmctl(GUI.sm_info.shmid, IPC_RMID, 0);
				GUI.use_shared_memory = FALSE;
			}
			else
			{
				GUI.sm_info.readOnly = False;

				XSetErrorHandler(ErrorHandler);
				XShmAttach(GUI.display, &GUI.sm_info);
				XSync(GUI.display, False);

				// X Error handler might clear GUI.use_shared_memory if XShmAttach failed.
				if (!GUI.use_shared_memory)
				{
					XDestroyImage(GUI.image->ximage);
					shmdt(GUI.sm_info.shmaddr);
					shmctl(GUI.sm_info.shmid, IPC_RMID, 0);
				} else
					printf("Created XShmImage, size %d\n",GUI.image->data_size);
			}
		}
	}

	if (!GUI.use_shared_memory)
	{
		fprintf(stderr, "use_shared_memory failed, switching to XPutImage.\n");
#endif
		GUI.image->ximage = XCreateImage(GUI.display, GUI.visual, GUI.depth, ZPixmap, 0, NULL, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2, BitmapUnit(GUI.display), 0);
		// set main Image struct vars
		GUI.image->height = GUI.image->ximage->height;
		GUI.image->bytes_per_line = GUI.image->ximage->bytes_per_line;
		GUI.image->data_size = GUI.image->bytes_per_line * GUI.image->height;

		GUI.image->ximage->data = (char *) malloc(GUI.image->data_size);
		if (!GUI.image->ximage || !GUI.image->ximage->data)
			FatalError("XCreateImage failed.");
		printf("Created XImage, size %d\n",GUI.image->data_size);
#ifdef MITSHM
	}
#endif

	// Set final values
	GUI.image->bits_per_pixel = GUI.image->ximage->bits_per_pixel;
	GUI.image->data = GUI.image->ximage->data;

#ifdef LSB_FIRST
	GUI.image->ximage->byte_order = LSBFirst;
#else
	GUI.image->ximage->byte_order = MSBFirst;
#endif
}

static void TakedownXImage (void)
{
	if (GUI.image->ximage)
	{
	#ifdef MITSHM
		if (GUI.use_shared_memory)
		{
			XShmDetach(GUI.display, &GUI.sm_info);
			GUI.image->ximage->data = NULL;
			XDestroyImage(GUI.image->ximage);
			if (GUI.sm_info.shmaddr)
				shmdt(GUI.sm_info.shmaddr);
			if (GUI.sm_info.shmid >= 0)
				shmctl(GUI.sm_info.shmid, IPC_RMID, 0);
			GUI.image->ximage = NULL;
		}
		else
	#endif
		{
			XDestroyImage(GUI.image->ximage);
			GUI.image->ximage = NULL;
		}
	}
}

#ifdef USE_XVIDEO
static void SetupXvImage (void)
{
#ifdef MITSHM
	GUI.use_shared_memory = TRUE;

	int		major, minor;
	Bool	shared;

	if (!XShmQueryVersion(GUI.display, &major, &minor, &shared) || !shared)
		GUI.image->xvimage = NULL;
	else
		GUI.image->xvimage = XvShmCreateImage(GUI.display, GUI.xv_port, GUI.xv_format, NULL, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2, &GUI.sm_info);

	if (!GUI.image->xvimage)
		GUI.use_shared_memory = FALSE;
	else
	{
		GUI.image->height = SNES_HEIGHT_EXTENDED * 2;
		GUI.image->data_size = GUI.image->xvimage->data_size;
		GUI.image->bytes_per_line = GUI.image->data_size / GUI.image->height;
		GUI.sm_info.shmid = shmget(IPC_PRIVATE, GUI.image->data_size, IPC_CREAT | 0777);
		if (GUI.sm_info.shmid < 0)
		{
			XFree(GUI.image->xvimage);
			GUI.use_shared_memory = FALSE;
		}
		else
		{
			GUI.image->xvimage->data = GUI.sm_info.shmaddr = (char *) shmat(GUI.sm_info.shmid, 0, 0);
			if (!GUI.image->xvimage->data)
			{
				XFree(GUI.image->xvimage);
				shmctl(GUI.sm_info.shmid, IPC_RMID, 0);
				GUI.use_shared_memory = FALSE;
			}
			else
			{
				GUI.sm_info.readOnly = False;

				XSetErrorHandler(ErrorHandler);
				XShmAttach(GUI.display, &GUI.sm_info);
				XSync(GUI.display, False);

				// X Error handler might clear GUI.use_shared_memory if XShmAttach failed.
				if (!GUI.use_shared_memory)
				{
					XFree(GUI.image->xvimage);
					shmdt(GUI.sm_info.shmaddr);
					shmctl(GUI.sm_info.shmid, IPC_RMID, 0);
				} else
					printf("Created XvShmImage, size %d\n",GUI.image->data_size);
			}
		}
	}

	if (!GUI.use_shared_memory)
	{
		fprintf(stderr, "use_shared_memory failed, switching to XvPutImage.\n");
#endif
		GUI.image->xvimage = XvCreateImage(GUI.display, GUI.xv_port, GUI.xv_format, NULL, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2);
		GUI.image->height = SNES_HEIGHT_EXTENDED * 2;
		GUI.image->data_size = GUI.image->xvimage->data_size;
		GUI.image->bytes_per_line = GUI.image->data_size / GUI.image->height;

		GUI.image->xvimage->data = (char *) malloc(GUI.image->data_size);
		if (!GUI.image->xvimage || !GUI.image->xvimage->data)
		{
			fprintf(stderr, "XvCreateImage failed, falling back to software blit.\n");
			GUI.use_xvideo = FALSE;
			return;
		}
		printf("Created XvImage, size %d\n",GUI.image->data_size);
#ifdef MITSHM
	}
#endif
	// Set final values
	GUI.image->bits_per_pixel = GUI.xv_bpp;
	GUI.image->data = GUI.image->xvimage->data;
}

static void TakedownXvImage (void)
{
	if (GUI.image->xvimage)
	{
	#ifdef MITSHM
		if (GUI.use_shared_memory)
		{
			XShmDetach(GUI.display, &GUI.sm_info);
			GUI.image->xvimage->data = NULL;
			XFree(GUI.image->xvimage);
			if (GUI.sm_info.shmaddr)
				shmdt(GUI.sm_info.shmaddr);
			if (GUI.sm_info.shmid >= 0)
				shmctl(GUI.sm_info.shmid, IPC_RMID, 0);
			GUI.image->xvimage = NULL;
		}
		else
	#endif
		{
			free(GUI.image->xvimage->data);
			//GUI.image->xvimage->data = NULL;
			XFree(GUI.image->xvimage);
			GUI.image->xvimage = NULL;
		}
	}
}
#endif

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
	blitFn((uint8 *) GFX.Screen, GFX.Pitch, GUI.blit_screen, GUI.blit_screen_pitch, width, height);

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

#ifdef USE_XVIDEO
	// Adjust source blit region if SNES would only fill half the screen.
	if (height <= SNES_HEIGHT_EXTENDED)
		GUI.imageHeight = height * 2;

	if (GUI.use_xvideo && (GUI.xv_format == FOURCC_YUY2))
	{
		uint16 *s = (uint16 *)GUI.blit_screen;
		uint8 *d = (uint8 *)GUI.image->data;

		// convert GUI.blit_screen and copy to XV image
		for (int y = 0; y < copyHeight; y++)
		{
			for (int x = 0; x < SNES_WIDTH * 2; x += 2)
			{
				// Read two RGB pxls
				//  TODO: there is an assumption of endianness here...
				// ALSO todo:  The 0x7FFF works around some issue with S9xPutChar, where
				//  despite asking for RGB555 in InitImage, it insists on drawing with RGB565 instead.
				//  This may discolor messages but at least it doesn't overflow yuv-tables and crash.
				unsigned short rgb1 = (*s & 0x7FFF); s++;
				unsigned short rgb2 = (*s & 0x7FFF); s++;

				// put two YUYV pxls
				// lum1
				*d = GUI.y_table[rgb1]; d++;
				// U
				*d = (GUI.u_table[rgb1] + GUI.u_table[rgb2]) / 2; d++;
				// lum2
				*d = GUI.y_table[rgb2]; d++;
				// V
				*d = (GUI.v_table[rgb1] + GUI.v_table[rgb2]) / 2; d++;
			}
		}
	}
	else
#endif
	if (GUI.need_convert)
	{
		if (GUI.bytes_per_pixel == 3)
			Convert16To24Packed(copyWidth, copyHeight);
		else
			Convert16To24(copyWidth, copyHeight);
	}

	Repaint(TRUE);

	prevWidth  = width;
	prevHeight = height;
}

static void Convert16To24 (int width, int height)
{
	if (GUI.pixel_format == 565)
	{
		for (int y = 0; y < height; y++)
		{
			uint16	*s = (uint16 *) (GUI.blit_screen + y * GUI.blit_screen_pitch);
			uint32	*d = (uint32 *) (GUI.image->data + y * GUI.image->bytes_per_line);

			for (int x = 0; x < width; x++)
			{
				uint32	pixel = *s++;
				*d++ = (((pixel >> 11) & 0x1f) << (GUI.red_shift + 3)) | (((pixel >> 6) & 0x1f) << (GUI.green_shift + 3)) | ((pixel & 0x1f) << (GUI.blue_shift + 3));
			}
		}
	}
	else
	{
		for (int y = 0; y < height; y++)
		{
			uint16	*s = (uint16 *) (GUI.blit_screen + y * GUI.blit_screen_pitch);
			uint32	*d = (uint32 *) (GUI.image->data + y * GUI.image->bytes_per_line);

			for (int x = 0; x < width; x++)
			{
				uint32	pixel = *s++;
				*d++ = (((pixel >> 10) & 0x1f) << (GUI.red_shift + 3)) | (((pixel >> 5) & 0x1f) << (GUI.green_shift + 3)) | ((pixel & 0x1f) << (GUI.blue_shift + 3));
			}
		}
	}
}

static void Convert16To24Packed (int width, int height)
{
	if (GUI.pixel_format == 565)
	{
		for (int y = 0; y < height; y++)
		{
			uint16	*s = (uint16 *) (GUI.blit_screen + y * GUI.blit_screen_pitch);
			uint8	*d = (uint8 *)  (GUI.image->data + y * GUI.image->bytes_per_line);

		#ifdef LSB_FIRST
			if (GUI.red_shift < GUI.blue_shift)
		#else
			if (GUI.red_shift > GUI.blue_shift)
		#endif
			{
				for (int x = 0; x < width; x++)
				{
					uint32	pixel = *s++;
					*d++ = (pixel >> (11 - 3)) & 0xf8;
					*d++ = (pixel >>  (6 - 3)) & 0xf8;
					*d++ = (pixel & 0x1f) << 3;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					uint32	pixel = *s++;
					*d++ = (pixel & 0x1f) << 3;
					*d++ = (pixel >>  (6 - 3)) & 0xf8;
					*d++ = (pixel >> (11 - 3)) & 0xf8;
				}
			}
		}
	}
	else
	{
		for (int y = 0; y < height; y++)
		{
			uint16	*s = (uint16 *) (GUI.blit_screen + y * GUI.blit_screen_pitch);
			uint8	*d = (uint8 *)  (GUI.image->data + y * GUI.image->bytes_per_line);

		#ifdef LSB_FIRST
			if (GUI.red_shift < GUI.blue_shift)
		#else
			if (GUI.red_shift > GUI.blue_shift)
		#endif
			{
				for (int x = 0; x < width; x++)
				{
					uint32	pixel = *s++;
					*d++ = (pixel >> (10 - 3)) & 0xf8;
					*d++ = (pixel >>  (5 - 3)) & 0xf8;
					*d++ = (pixel & 0x1f) << 3;
				}
			}
			else
			{
				for (int x = 0; x < width; x++)
				{
					uint32	pixel = *s++;
					*d++ = (pixel & 0x1f) << 3;
					*d++ = (pixel >>  (5 - 3)) & 0xf8;
					*d++ = (pixel >> (10 - 3)) & 0xf8;
				}
			}
		}
	}
}

static void Repaint (bool8 isFrameBoundry)
{
#ifdef USE_XVIDEO
	if (GUI.use_xvideo)
	{
#ifdef MITSHM
		if (GUI.use_shared_memory)
		{
			XvShmPutImage(GUI.display, GUI.xv_port, GUI.window, GUI.gc, GUI.image->xvimage,
				0, 0, SNES_WIDTH * 2, GUI.imageHeight,
				GUI.x_offset, GUI.y_offset, GUI.scale_w, GUI.scale_h, False);
		}
		else
#endif
		XvPutImage(GUI.display, GUI.xv_port, GUI.window, GUI.gc, GUI.image->xvimage,
			0, 0, SNES_WIDTH * 2, GUI.imageHeight,
			GUI.x_offset, GUI.y_offset, GUI.scale_w, GUI.scale_h);
	}
	else
#endif
#ifdef MITSHM
	if (GUI.use_shared_memory)
	{
		XShmPutImage(GUI.display, GUI.window, GUI.gc, GUI.image->ximage, 0, 0, GUI.x_offset, GUI.y_offset, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2, False);
		XSync(GUI.display, False);
	}
	else
#endif
		XPutImage(GUI.display, GUI.window, GUI.gc, GUI.image->ximage, 0, 0, GUI.x_offset, GUI.y_offset, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2);

	Window			root, child;
	int				root_x, root_y, x, y;
	unsigned int	mask;

	// Use QueryPointer to sync X server and as a side effect also gets current pointer position for SNES mouse emulation.
	XQueryPointer(GUI.display, GUI.window, &root, &child, &root_x, &root_y, &x, &y, &mask);

	if (x >= 0 && y >= 0 && x < SNES_WIDTH * 2 && y < SNES_HEIGHT_EXTENDED * 2)
	{
		GUI.mouse_x = x >> 1;
		GUI.mouse_y = y >> 1;

		if (mask & Mod1Mask)
		{
			if (!GUI.mod1_pressed)
			{
				GUI.mod1_pressed = TRUE;
				XDefineCursor(GUI.display, GUI.window, GUI.cross_hair_cursor);
			}
		}
		else
		if (GUI.mod1_pressed)
		{
			GUI.mod1_pressed = FALSE;
			XDefineCursor(GUI.display, GUI.window, GUI.point_cursor);
		}
	}

	if (Settings.DumpStreams && isFrameBoundry)
		S9xVideoLogger(GUI.image->data, SNES_WIDTH * 2, SNES_HEIGHT_EXTENDED * 2, GUI.bytes_per_pixel, GUI.image->bytes_per_line);
}

void S9xTextMode (void)
{
	SetXRepeat(TRUE);
}

void S9xGraphicsMode (void)
{
	SetXRepeat(FALSE);
}

static bool8 CheckForPendingXEvents (Display *display)
{
#ifdef SELECT_BROKEN_FOR_SIGNALS
	int	arg = 0;

	return (XEventsQueued(display, QueuedAlready) || (ioctl(ConnectionNumber(display), FIONREAD, &arg) == 0 && arg));
#else
	return (XPending(display));
#endif
}

void S9xProcessEvents (bool8 block)
{
	while (block || CheckForPendingXEvents(GUI.display))
	{
		XEvent	event;

		XNextEvent(GUI.display, &event);
		block = FALSE;

		switch (event.type)
		{
			case KeyPress:
			case KeyRelease:
				S9xReportButton(((event.xkey.state & (ShiftMask | Mod1Mask | ControlMask | Mod4Mask)) << 8) | event.xkey.keycode, event.type == KeyPress);
			#if 1
				{
					KeyCode	kc = XKeysymToKeycode(GUI.display, XKeycodeToKeysym(GUI.display, event.xkey.keycode, 0));
					if (event.xkey.keycode != kc)
						S9xReportButton(((event.xkey.state & (ShiftMask | Mod1Mask | ControlMask | Mod4Mask)) << 8) | kc, event.type == KeyPress);
				}
			#endif
				break;

			case FocusIn:
				SetXRepeat(FALSE);
				XFlush(GUI.display);
				break;

			case FocusOut:
				SetXRepeat(TRUE);
				XFlush(GUI.display);
				break;

			case ConfigureNotify:
				break;

			case ButtonPress:
			case ButtonRelease:
				S9xReportButton(0x40000000 | (event.xbutton.button - 1), event.type == ButtonPress);
				break;

			case Expose:
				Repaint(FALSE);
				break;
		}
	}
}

const char * S9xSelectFilename (const char *def, const char *dir1, const char *ext1, const char *title)
{
	static char	s[PATH_MAX + 1];
	char		buffer[PATH_MAX + 1];

	SetXRepeat(TRUE);

	printf("\n%s (default: %s): ", title, def);
	fflush(stdout);

	SetXRepeat(FALSE);

	if (fgets(buffer, PATH_MAX + 1, stdin))
	{
		char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

		char	*p = buffer;
		while (isspace(*p))
			p++;
		if (!*p)
		{
			strncpy(buffer, def, PATH_MAX + 1);
			buffer[PATH_MAX] = 0;
			p = buffer;
		}

		char	*q = strrchr(p, '\n');
		if (q)
			*q = 0;

		_splitpath(p, drive, dir, fname, ext);
		_makepath(s, drive, *dir ? dir : dir1, fname, *ext ? ext : ext1);

		return (s);
	}

	return (NULL);
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
	XStoreName(GUI.display, GUI.window, string);
	XFlush(GUI.display);
}

static void SetXRepeat (bool8 state)
{
	if (state)
		XAutoRepeatOn(GUI.display);
	else
		XAutoRepeatOff(GUI.display);
}

s9xcommand_t S9xGetDisplayCommandT (const char *n)
{
	s9xcommand_t	cmd;

	cmd.type         = S9xBadMapping;
	cmd.multi_press  = 0;
	cmd.button_norpt = 0;
	cmd.port[0]      = 0xff;
	cmd.port[1]      = 0;
	cmd.port[2]      = 0;
	cmd.port[3]      = 0;

	return (cmd);
}

char * S9xGetDisplayCommandName (s9xcommand_t cmd)
{
	return (strdup("None"));
}

void S9xHandleDisplayCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

bool8 S9xMapDisplayInput (const char *n, s9xcommand_t *cmd)
{
	int	i, d;

	if (!isdigit(n[1]) || !isdigit(n[2]) || n[3] != ':')
		goto unrecog;

	d = ((n[1] - '0') * 10 + (n[2] - '0')) << 24;

	switch (n[0])
	{
		case 'K':
		{
			KeyCode	kc;
			KeySym	ks;

			d |= 0x00000000;

			for (i = 4; n[i] != '\0' && n[i] != '+'; i++) ;

			if (n[i] == '\0' || i == 4)
				i = 4;
			else
			{
				for (i = 4; n[i] != '+'; i++)
				{
					switch (n[i])
					{
						case 'S': d |= ShiftMask   << 8; break;
						case 'C': d |= ControlMask << 8; break;
						case 'A': d |= Mod1Mask    << 8; break;
						case 'M': d |= Mod4Mask    << 8; break;
						default:  goto unrecog;
					}
				}

				i++;
			}

			if ((ks = XStringToKeysym(n + i)) == NoSymbol)
				goto unrecog;
			if ((kc = XKeysymToKeycode(GUI.display, ks)) == 0)
				goto unrecog;

			d |= kc & 0xff;

			return (S9xMapButton(d, *cmd, false));
		}

		case 'M':
		{
			char	*c;
			int		j;

			d |= 0x40000000;

			if (!strncmp(n + 4, "Pointer", 7))
			{
				d |= 0x8000;

				if (n[11] == '\0')
					return (S9xMapPointer(d, *cmd, true));

				i = 11;
			}
			else
			if (n[4] == 'B')
				i = 5;
			else
				goto unrecog;

			d |= j = strtol(n + i, &c, 10);

			if ((c != NULL && *c != '\0') || j > 0x7fff)
				goto unrecog;

			if (d & 0x8000)
				return (S9xMapPointer(d, *cmd, true));

			return (S9xMapButton(d, *cmd, false));
		}

		default:
			break;
	}

unrecog:
	char	*err = new char[strlen(n) + 34];

	sprintf(err, "Unrecognized input device name '%s'", n);
	perror(err);
	delete [] err;

	return (false);
}

bool S9xDisplayPollButton (uint32 id, bool *pressed)
{
	return (false);
}

bool S9xDisplayPollAxis (uint32 id, int16 *value)
{
	return (false);
}

bool S9xDisplayPollPointer (uint32 id, int16 *x, int16 *y)
{
	if ((id & 0xc0008000) != 0x40008000)
		return (false);

	int	d = (id >> 24) & 0x3f,
		n = id & 0x7fff;

	if (d != 0 || n != 0)
		return (false);

	*x = GUI.mouse_x;
	*y = GUI.mouse_y;

	return (true);
}

void S9xSetPalette (void)
{
	return;
}
