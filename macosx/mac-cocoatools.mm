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


#import <Cocoa/Cocoa.h>

#import "mac-cocoatools.h"


void CocoaPlayFreezeDefrostSound (void)
{
	NSAutoreleasePool	*pool;
	NSBundle			*bundle;
	NSString			*path;
	NSSound				*sound;
	BOOL				r;

	pool = [[NSAutoreleasePool alloc] init];

	bundle = [NSBundle mainBundle];
	path = [bundle pathForSoundResource: @"freeze_defrost"];
	if (path)
	{
		sound = [[NSSound alloc] initWithContentsOfFile: path byReference: YES];
		if (sound)
		{
			r = [sound play];
			[sound release];
		}
	}

	[pool release];
}

void CocoaAddStatTextToView (NSView *view, NSString *label, float x, float y, float w, float h, NSTextField **out)
{
	NSTextField	*control;

	control = [[[NSTextField alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setStringValue: NSLocalizedString(label, @"")];
	[control setBezeled: NO];
	[control setDrawsBackground: NO];
	[control setEditable: NO];
	[control setSelectable: NO];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddEditTextToView (NSView *view, NSString *label, float x, float y, float w, float h, NSTextField **out)
{
	NSTextField	*control;

	control = [[[NSTextField alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setStringValue: NSLocalizedString(label, @"")];
	[control setBezeled: YES];
	[control setDrawsBackground: YES];
	[control setEditable: YES];
	[control setSelectable: YES];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddMPushBtnToView (NSView *view, NSString *label, float x, float y, float w, float h, NSButton **out)
{
	NSButton	*control;

	control = [[[NSButton alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setTitle: NSLocalizedString(label, @"")];
	[control setBezelStyle: NSRoundedBezelStyle];
	[control setButtonType: NSMomentaryPushInButton];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddCheckBoxToView (NSView *view, NSString *label, float x, float y, float w, float h, NSButton **out)
{
	NSButton	*control;

	control = [[[NSButton alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setTitle: NSLocalizedString(label, @"")];
	[control setButtonType: NSSwitchButton];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}

void CocoaAddPopUpBtnToView (NSView *view, NSArray *array, float x, float y, float w, float h, NSPopUpButton **out)
{
	NSPopUpButton	*control;
	NSMenu			*menu;
	int				n;

	menu = [[[NSMenu alloc] init] autorelease];

	n = [array count];
	for (int i = 0; i < n; i++)
	{
		NSString	*item = [array objectAtIndex: i];
		if ([item isEqualToString: @"---"])
			[menu addItem: [NSMenuItem separatorItem]];
		else
			[menu addItemWithTitle: item action: NULL keyEquivalent: @""];
	}

	control = [[[NSPopUpButton alloc] init] autorelease];

	[[control cell] setControlSize: NSSmallControlSize];
	[control setFont: [NSFont systemFontOfSize: [NSFont systemFontSizeForControlSize: NSSmallControlSize]]];
	[control setPullsDown: NO];
	[control setMenu: menu];

	[view addSubview: control];
	[control setFrame: NSMakeRect(x, y, w, h)];

	if (out != NULL)
		*out = control;
}
