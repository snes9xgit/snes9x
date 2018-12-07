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


#ifndef _mac_musicbox_h_
#define _mac_musicbox_h_

#define	kMBXSoundEmulation	1
#define	kMBXWholeEmulation	2

@interface MusicBoxController : NSObject
{
	IBOutlet id	window;
	IBOutlet id	gametitle;
	IBOutlet id	disclosure;
	IBOutlet id	rewind;
	IBOutlet id	indicator;
	IBOutlet id	led;

	float		mbxClosedHeight,
				mbxOpenedHeight;

	NSTimer		*timer;
	pthread_t	mbxThread;
}

- (id) init;
- (void) dealloc;
- (void) windowWillClose: (NSNotification *) aNotification;
- (NSWindow *) window;
- (IBAction) handlePauseButton: (id) sender;
- (IBAction) handleRewindButton: (id) sender;
- (IBAction) handleEffectButton: (id) sender;
- (IBAction) handleChannelButton: (id) sender;
- (IBAction) handleDisclosureButton: (id) sender;
- (void) updateIndicator: (NSTimer *) aTimer;

@end

@interface MusicBoxIndicatorView : NSView
{
	float		mbxOffsetX,
				mbxOffsetY,
				mbxBarWidth,
				mbxBarHeight,
				mbxBarSpace,
				mbxLRSpace,
				mbxRightBarX,
				yyscale;

	float		mbxViewWidth,
				mbxViewHeight,
				mbxMarginX,
				mbxMarginY;

	short		prevLMax[8],
				prevRMax[8],
				prevLVol[8],
				prevRVol[8];

	long long	barTimeL[8],
				barTimeR[8];
}

- (id) initWithFrame: (NSRect) frame;
- (void) drawRect: (NSRect) rect;

@end

extern volatile Boolean	mboxPause;

void MusicBoxDialog (void);

#endif
