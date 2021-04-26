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
  (c) Copyright 2019 - 2021  Michael Donald Buckley
 ***********************************************************************************/

#import "S9xPreferencesTabViewController.h"

@interface S9xPreferencesTabViewController ()

@end

@implementation S9xPreferencesTabViewController
{
	BOOL _didHide;
}

- (void)tabView:(NSTabView *)tabView willSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	[super tabView:tabView willSelectTabViewItem:tabViewItem];

	NSTabViewItem *currentTabViewItem = tabView.selectedTabViewItem;

	if ( currentTabViewItem == nil || currentTabViewItem == tabViewItem )
	{
		return;
	}

	currentTabViewItem.view.hidden = YES;
	tabViewItem.view.hidden = YES;
	_didHide = YES;

	NSWindow *window = self.view.window;
	NSSize tabViewSize = tabViewItem.view.fittingSize;
	NSSize newWindowSize = [window frameRectForContentRect:(CGRect){CGPointZero, tabViewSize}].size;

	NSRect windowFrame = window.frame;
	windowFrame.origin.y += windowFrame.size.height;
	windowFrame.origin.y -= newWindowSize.height;
	windowFrame.size = newWindowSize;

	if ( self.isViewLoaded )
	{
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context)
		{
			[self.view.window.animator setFrame:windowFrame display:YES];
		} completionHandler:^
		{
			tabViewItem.view.hidden = NO;
		}];
	}
}

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
	[super tabView:tabView didSelectTabViewItem:tabViewItem];

	if ( _didHide )
	{
		tabViewItem.view.hidden = YES;
		_didHide = NO;
	}
}

@end
