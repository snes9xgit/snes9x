/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#import "snes9x.h"
#import "memmap.h"
#import "apu.h"
#import "snapshot.h"
#import "snes.hpp"

#import <Cocoa/Cocoa.h>
#import <sys/time.h>
#import <pthread.h>

#import "mac-prefix.h"
#import "mac-audio.h"
#import "mac-file.h"
#import "mac-os.h"
#import "mac-musicbox.h"

volatile bool8			mboxPause = false;

static volatile bool8	stopNow, showIndicator, headPressed;
static int32			oldCPUCycles;
static uint16			stereo_switch;
static uint8			storedSoundSnapshot[SPC_SAVE_STATE_BLOCK_SIZE];

static void SPCPlayExec (void);
static void SPCPlayFreeze (void);
static void SPCPlayDefrost (void);
static void MusicBoxForceFreeze (void);
static void MusicBoxForceDefrost (void);
static void * SoundTask (void *);


@implementation MusicBoxController

- (id) init
{
	NSUserDefaults	*defaults;
	NSString		*s;
	NSRect			rect;
	NSSize			size;
	BOOL			apuonly, r;
	char			drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	self = [super init];
	if (!self)
		return (self);

	r = [NSBundle loadNibNamed: @"musicbox" owner: self];
	if (!r)
		return (self);

	apuonly = (musicboxmode == kMBXSoundEmulation);

	if (apuonly)
		SPCPlayFreeze();
	else
		MusicBoxForceFreeze();

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	[gametitle setStringValue: [NSString stringWithUTF8String: fname]];

	[led setImage: [NSImage imageNamed: (apuonly ? @"musicbox_ledoff.icns" : @"musicbox_ledon.icns")]];

	if (!apuonly)
	{
		[rewind setState: NSOffState];
		[rewind setEnabled: NO];
	}

	defaults = [NSUserDefaults standardUserDefaults];
	s = [defaults stringForKey: @"frame_musicbox"];
	if (s)
	{
		rect = NSRectFromString(s);
		[window setFrame: rect display: NO];
	}

	if (!savewindowpos)
		[window center];

	size = [window minSize];
	mbxClosedHeight = size.height;
	size = [window maxSize];
	mbxOpenedHeight = size.height;

	rect = [window frame];
	showIndicator = (rect.size.height > mbxClosedHeight) ? true : false;

	[disclosure setState: (showIndicator ? NSOnState : NSOffState)];

	[window makeKeyAndOrderFront: self];

	mboxPause   = false;
	headPressed = false;

	stereo_switch = ~0;
	SNES::dsp.spc_dsp.set_stereo_switch(stereo_switch);

	for (int i = 0; i < MAC_MAX_PLAYERS; i++)
		controlPad[i] = 0;

	stopNow = false;
	MacStartSound();
	pthread_create(&mbxThread, NULL, SoundTask, NULL);

	timer = [[NSTimer scheduledTimerWithTimeInterval: (2.0 / (double) Memory.ROMFramesPerSecond) target: self selector: @selector(updateIndicator:) userInfo: nil repeats: YES] retain];

	return (self);
}

- (void) windowWillClose: (NSNotification *) aNotification
{
	NSUserDefaults	*defaults;
	NSString		*s;
	BOOL			r;

	[timer invalidate];
	[timer release];

	showIndicator = false;

	stopNow = true;
	pthread_join(mbxThread, NULL);
	MacStopSound();

	defaults = [NSUserDefaults standardUserDefaults];
	s = NSStringFromRect([window frame]);
	[defaults setObject: s forKey: @"frame_musicbox"];
	r = [defaults synchronize];

	[NSApp stopModal];
}

- (void) dealloc
{
	stereo_switch = ~0;
	SNES::dsp.spc_dsp.set_stereo_switch(stereo_switch);

	if (musicboxmode == kMBXSoundEmulation)
		SPCPlayDefrost();
	else
		MusicBoxForceDefrost();

	[window release];

	[super dealloc];
}

- (NSWindow *) window
{
	return (window);
}

- (IBAction) handlePauseButton: (id) sender
{
	mboxPause = !mboxPause;
	S9xSetSoundMute(mboxPause);
}

- (IBAction) handleRewindButton: (id) sender
{
	headPressed = true;
}

- (IBAction) handleEffectButton: (id) sender
{
	[window orderOut: self];
	showIndicator = false;
	ConfigureSoundEffects();
	showIndicator = true;
	[window makeKeyAndOrderFront: self];
}

- (IBAction) handleChannelButton: (id) sender
{
	stereo_switch ^= (1 << [sender tag]);
	SNES::dsp.spc_dsp.set_stereo_switch(stereo_switch);
}

- (IBAction) handleDisclosureButton: (id) sender
{
	NSRect	rect;
	float	h;

	showIndicator = !showIndicator;
	rect = [window frame];
	h = rect.size.height;
	rect.size.height = showIndicator ? mbxOpenedHeight : mbxClosedHeight;
	rect.origin.y += (h - rect.size.height);
	[window setFrame: rect display: YES animate: YES];
}

- (void) updateIndicator: (NSTimer *) aTimer
{
	if (showIndicator)
		[indicator setNeedsDisplay: YES];
}

@end

@implementation MusicBoxIndicatorView

- (id) initWithFrame: (NSRect) frame
{
	self = [super initWithFrame: frame];
	if (self)
	{
		NSRect			rect;
		long long		currentTime;
		struct timeval	tv;

		mbxOffsetX   =   0.0f;
		mbxOffsetY   =   0.0f;
		mbxBarWidth  =  12.0f;
		mbxBarHeight = 128.0f;
		mbxBarSpace  =   2.0f;
		mbxLRSpace   =  20.0f;
		mbxRightBarX = (mbxLRSpace + (mbxBarWidth * 8.0f + mbxBarSpace * 7.0f));
		yyscale      = (float) (128.0 / sqrt(64.0));

		rect = [self bounds];
		mbxViewWidth  = rect.size.width;
		mbxViewHeight = rect.size.height;
		mbxMarginX = (mbxViewWidth - ((mbxBarWidth * 8.0f + mbxBarSpace * 7.0f) * 2.0f + mbxLRSpace)) / 2.0f;
		mbxMarginY = (mbxViewHeight - mbxBarHeight) / 2.0f;

		gettimeofday(&tv, NULL);
		currentTime = tv.tv_sec * 1000000 + tv.tv_usec;

		for (int i = 0; i < 8; i++)
		{
			prevLMax[i] = prevRMax[i] = 0;
			prevLVol[i] = prevRVol[i] = 0;
			barTimeL[i] = barTimeR[i] = currentTime;
		}
	}

	return (self);
}

- (void) drawRect: (NSRect) rect
{
	CGContextRef	mboxctx;

	mboxctx = (CGContextRef) [[NSGraphicsContext currentContext] graphicsPort];

	// Bar

	const float	length[] = { 1.0f, 1.0f };

	CGContextSetLineWidth(mboxctx, mbxBarWidth);
	CGContextSetLineDash(mboxctx, 0, length, 2);
	CGContextSetLineJoin(mboxctx, kCGLineJoinMiter);

	CGContextBeginPath(mboxctx);

	float   x = mbxOffsetX + mbxMarginX + mbxBarWidth / 2.0f;

	for (int h = 0; h < 8; h++)
	{
		// Inactive

		CGContextSetRGBStrokeColor(mboxctx, (196.0f / 256.0f), (200.0f / 256.0f), (176.0f / 256.0f), 1.0f);

		CGContextMoveToPoint   (mboxctx, x,                mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x,                mbxOffsetY + mbxMarginY + mbxBarHeight);

		CGContextMoveToPoint   (mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + mbxBarHeight);

		CGContextStrokePath(mboxctx);

		// Max

		short			vl = (SNES::dsp.spc_dsp.reg_value(h, 0x00) * SNES::dsp.spc_dsp.envx_value(h)) >> 11;
		short			vr = (SNES::dsp.spc_dsp.reg_value(h, 0x01) * SNES::dsp.spc_dsp.envx_value(h)) >> 11;
		long long		currentTime;
		struct timeval	tv;

		if (vl <= 0) vl = 0; else if (vl > 64) vl = 64; else vl = (short) (yyscale * sqrt((double) vl)) & (~0 << 1);
		if (vr <= 0) vr = 0; else if (vr > 64) vr = 64; else vr = (short) (yyscale * sqrt((double) vr)) & (~0 << 1);

		if (vl < prevLVol[h]) vl = ((prevLVol[h] + vl) >> 1);
		if (vr < prevRVol[h]) vr = ((prevRVol[h] + vr) >> 1);

		gettimeofday(&tv, NULL);
		currentTime = tv.tv_sec * 1000000 + tv.tv_usec;

		// left

		if ((vl >= prevLMax[h]) && (vl > prevLVol[h]))
		{
			barTimeL[h] = currentTime;
			prevLMax[h] = vl;
		}
		else
		if ((prevLMax[h] > 0) && (barTimeL[h] + 1000000 > currentTime))
		{
			CGContextSetRGBStrokeColor(mboxctx, (22.0f / 256.0f), (156.0f / 256.0f), (20.0f / 256.0f), (float) (barTimeL[h] + 1000000 - currentTime) / 1000000.0f);

			CGContextMoveToPoint   (mboxctx, x, mbxOffsetY + mbxMarginY + (float) (prevLMax[h] - 2));
			CGContextAddLineToPoint(mboxctx, x, mbxOffsetY + mbxMarginY + (float) (prevLMax[h]    ));

			CGContextStrokePath(mboxctx);
		}
		else
			prevLMax[h] = 0;

		prevLVol[h] = vl;

		// right

		if ((vr >= prevRMax[h]) && (vr > prevRVol[h]))
		{
			barTimeR[h] = currentTime;
			prevRMax[h] = vr;
		}
		else
		if ((prevRMax[h] > 0) && (barTimeR[h] + 1000000 > currentTime))
		{
			CGContextSetRGBStrokeColor(mboxctx, (22.0f / 256.0f), (156.0f / 256.0f), (20.0f / 256.0f), (float) (barTimeR[h] + 1000000 - currentTime) / 1000000.0f);

			CGContextMoveToPoint   (mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + (float) (prevRMax[h] - 2));
			CGContextAddLineToPoint(mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + (float) (prevRMax[h]    ));

			CGContextStrokePath(mboxctx);
		}
		else
			prevRMax[h] = 0;

		prevRVol[h] = vr;

		// Active

		CGContextSetRGBStrokeColor(mboxctx, (22.0f / 256.0f), (22.0f / 256.0f), (20.0f / 256.0f), 1.0f);

		CGContextMoveToPoint   (mboxctx, x,                mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x,                mbxOffsetY + mbxMarginY + (float) vl);
		CGContextStrokePath(mboxctx);
		CGContextMoveToPoint   (mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY);
		CGContextAddLineToPoint(mboxctx, x + mbxRightBarX, mbxOffsetY + mbxMarginY + (float) vr);
		CGContextStrokePath(mboxctx);

		x += (mbxBarWidth + mbxBarSpace);
	}
}

@end

static void * SoundTask (void *)
{
	long long		last, curr;
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	last = tv.tv_sec * 1000000 + tv.tv_usec;

	while (!stopNow)
	{
		if (!mboxPause)
		{
			if (musicboxmode == kMBXSoundEmulation)
				SPCPlayExec();
			else
				S9xMainLoop();
		}

		if (headPressed)
		{
			showIndicator = false;
			SPCPlayDefrost();
			showIndicator = true;

			headPressed = false;
		}

		last += (1000000 / Memory.ROMFramesPerSecond);
		gettimeofday(&tv, NULL);
		curr = tv.tv_sec * 1000000 + tv.tv_usec;

		if (last > curr)
			usleep((useconds_t) (last - curr));
	}

	return (NULL);
}

static void SPCPlayExec (void)
{
	for (int v = 0; v < Timings.V_Max; v++)
	{
		CPU.Cycles = Timings.H_Max;
		S9xAPUEndScanline();
		CPU.Cycles = 0;
		S9xAPUSetReferenceTime(0);
	}
}

static void MusicBoxForceFreeze (void)
{
	char	filename[PATH_MAX + 1];

	strcpy(filename, S9xGetFreezeFilename(999));
	strcat(filename, ".tmp");

	S9xFreezeGame(filename);
}

static void MusicBoxForceDefrost (void)
{
	char	filename[PATH_MAX + 1];

	strcpy(filename, S9xGetFreezeFilename(999));
	strcat(filename, ".tmp");

	S9xUnfreezeGame(filename);
	remove(filename);
}

static void SPCPlayFreeze (void)
{
	oldCPUCycles = CPU.Cycles;

	S9xSetSoundMute(true);
	S9xAPUSaveState(storedSoundSnapshot);
	S9xSetSoundMute(false);
}

static void SPCPlayDefrost (void)
{
	CPU.Cycles = oldCPUCycles;

	S9xSetSoundMute(true);
	S9xAPULoadState(storedSoundSnapshot);
	S9xSetSoundMute(false);
}

void MusicBoxDialog (void)
{
	MusicBoxController	*controller;
	NSAutoreleasePool	*pool;

	if (!cartOpen)
		return;

	pool = [[NSAutoreleasePool alloc] init];
	controller = [[MusicBoxController alloc] init];
	[pool release];

	if (!controller)
		return;

	[NSApp runModalForWindow: [controller window]];

	pool = [[NSAutoreleasePool alloc] init];
	[controller release];
	[pool release];
}
