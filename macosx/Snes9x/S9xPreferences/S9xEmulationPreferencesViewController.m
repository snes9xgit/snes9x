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

#import "S9xEmulationPreferencesViewController.h"

#import "AppDelegate.h"
#import "S9xPreferencesConstants.h"

@interface S9xEmulationPreferencesViewController ()

@end

@implementation S9xEmulationPreferencesViewController

- (instancetype)init
{
	if (self = [super initWithNibName:@"S9xEmulationPreferencesViewController" bundle:nil])
	{
		self.title = NSLocalizedString(@"Emulation", nil);
		self.image = [NSImage imageNamed:@"emulation"];
	}

	return self;
}

- (void)viewWillAppear
{
	[super viewWillAppear];

	[NSNotificationCenter.defaultCenter addObserverForName:NSUserDefaultsDidChangeNotification
													object:NSUserDefaults.standardUserDefaults
													 queue:NSOperationQueue.mainQueue
												usingBlock:^(NSNotification *notification)
	{
		[((AppDelegate *)NSApp.delegate) applyEmulationSettings];
	}];
}

- (void)viewDidDisappear
{
	[super viewDidDisappear];

	[NSNotificationCenter.defaultCenter removeObserver:self];
}

- (IBAction)resetDefaults:(id)sender
{
	NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;

	[defaults setInteger:100 forKey:kSuperFXClockSpeedPercentPref];
	[defaults setInteger:2 forKey:kSoundInterpolationTypePref];
	[defaults setInteger:0 forKey:kCPUOverclockPref];

	[defaults setBool:YES forKey:kApplyGameSpecificHacksPref];
	[defaults setBool:NO forKey:kAllowInvalidVRAMAccessPref];
	[defaults setBool:NO forKey:kSeparateEchoBufferFromRAMPref];
	[defaults setBool:NO forKey:kDisableSpriteLimitPref];

	[defaults synchronize];
}

@end
