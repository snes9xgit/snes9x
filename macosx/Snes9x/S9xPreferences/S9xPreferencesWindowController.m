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

#import <snes9x_framework/snes9x_framework.h>
#import "AppDelegate.h"
#import "S9xButtonConfigTextField.h"
#import "S9xPreferencesConstants.h"
#import "S9xPreferencesWindowController.h"

#import "S9xPreferencesTabViewController.h"
#import "S9xControlsPreferencesViewController.h"
#import "S9xDisplayPreferencesViewController.h"
#import "S9xSoundPreferencesViewController.h"
#import "S9xEmulationPreferencesViewController.h"
#import "S9xFilesPreferencesViewController.h"

@interface S9xPreferencesWindowController ()
@property (nonatomic, readonly, strong) NSArray<S9xPreferencesViewController *> *tabs;
@property (nonatomic, readonly, strong) NSTabViewController *tabViewController;
@end

@implementation S9xPreferencesWindowController

- (NSArray<S9xPreferencesViewController *> *)tabs
{
	return self.contentViewController.childViewControllers;
}

@dynamic tabViewController;
- (S9xPreferencesTabViewController *)tabViewController
{
	return (S9xPreferencesTabViewController *)self.contentViewController;
}

- (void)windowDidLoad
{
    [self refresh];
}

- (void)awakeFromNib
{
	S9xPreferencesTabViewController *tabViewController = [S9xPreferencesTabViewController new];
	tabViewController.tabStyle = NSTabViewControllerTabStyleToolbar;
	self.contentViewController = tabViewController;

	[self addPane:[S9xControlsPreferencesViewController new] to:tabViewController];
	[self addPane:[S9xDisplayPreferencesViewController new] to:tabViewController];
	[self addPane:[S9xSoundPreferencesViewController new] to:tabViewController];
	[self addPane:[S9xEmulationPreferencesViewController new] to:tabViewController];
	[self addPane:[S9xFilesPreferencesViewController new] to:tabViewController];
}

- (void)addPane:(S9xPreferencesViewController *)pane to:(NSTabViewController *)tabController
{
	[tabController addChildViewController:pane];
	tabController.tabView.tabViewItems.lastObject.image = pane.image;
}

- (void)refresh
{
	for ( S9xPreferencesViewController *tab in self.tabs )
	{
		[tab refresh];
	}
}

- (BOOL)handleInput:(S9xJoypadInput *)input fromJoypad:(S9xJoypad *)joypad
{
	NSTabViewController *tabViewController = self.tabViewController;
	NSUInteger index = self.tabViewController.selectedTabViewItemIndex;
	if ( index >= 0 && index < tabViewController.childViewControllers.count )
	{
		S9xPreferencesViewController *viewController = tabViewController.childViewControllers[index];
		if ( [viewController isKindOfClass:[S9xControlsPreferencesViewController class]] )
		{
			return [((S9xControlsPreferencesViewController *)viewController) handleInput:input fromJoypad:joypad];
		}
	}

	return NO;
}

@end
