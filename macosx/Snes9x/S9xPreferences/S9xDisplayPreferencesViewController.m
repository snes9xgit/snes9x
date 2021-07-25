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

#import "S9xDisplayPreferencesViewController.h"

#import "AppDelegate.h"
#import "S9xPreferencesConstants.h"

@interface S9xDisplayPreferencesViewController ()

@property (nonatomic, weak) IBOutlet NSPopUpButton *videoModePopup;
@property (nonatomic, weak) IBOutlet NSButton *showFPSCheckbox;
@property (nonatomic, weak) IBOutlet NSTextField *macFrameSkipTextField;
@property (nonatomic, weak) IBOutlet NSStepper *macFrameSkipStepper;
@property (nonatomic, weak) IBOutlet NSButton *macFrameSkipAutomaticButton;

@end

@implementation S9xDisplayPreferencesViewController

- (instancetype)init
{
	if (self = [super initWithNibName:@"S9xDisplayPreferencesViewController" bundle:nil])
	{
		self.title = NSLocalizedString(@"Display", nil);
		self.image = [NSImage imageNamed:@"display"];
	}

	return self;
}

- (void)refresh
{
	NSUInteger index = MIN([NSUserDefaults.standardUserDefaults integerForKey:kVideoModePref], 1);
	[self.videoModePopup selectItemAtIndex:index];

	NSInteger macFrameSkipDefault = [NSUserDefaults.standardUserDefaults integerForKey:kMacFrameSkipPref];
	// if macFrameSkip is equal to -1, set automatic checkbox
	// to be checked, disable the ability to change frame skip values
	// from stepper/text field, else leave automatic checkbox
	// unchecked and set textfield to value
	if (macFrameSkipDefault == -1) {
		[self.macFrameSkipTextField setEnabled: false];
		[self.macFrameSkipStepper setEnabled: false];
		[self.macFrameSkipTextField setIntValue: 0]; // show something at least
		[self.macFrameSkipAutomaticButton setIntValue: 1];
	} else {
		[self.macFrameSkipTextField setIntValue: (int)macFrameSkipDefault];
		[self.macFrameSkipAutomaticButton setIntValue: 0];
	}

	self.showFPSCheckbox.state = [NSUserDefaults.standardUserDefaults boolForKey:kShowFPSPref];
}

- (void)setShowFPS:(BOOL)value
{
	AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
	[appDelegate setShowFPS:value];
}

- (void)setVideoMode:(int)value
{
	AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
	[appDelegate setVideoMode:value];
}

- (void)setMacFrameSkip:(int)value
{
	AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
	[appDelegate setMacFrameSkip:value];
}

- (IBAction)onCheckShowFPS:(NSButton *)sender
{
	[self setShowFPS:sender.state == NSOnState];
}

- (IBAction)onSelectVideoMode:(NSPopUpButton *)sender
{
	[self setVideoMode:(int)sender.selectedTag];
}

- (IBAction)bumpMacFrameSkip:(NSStepper *)sender
{
	int bumpValue = sender.intValue;   // 1 or -1
	int nextValue = self.macFrameSkipTextField.intValue + bumpValue;

	// constrain value
	if (nextValue < 0) {
		nextValue = 0;
	}
	if (nextValue > 200) {
		nextValue = 200;
	}

	[self.macFrameSkipTextField setIntValue: nextValue];
	[sender setIntValue:0];	// reset stepper value
	[self setMacFrameSkip:self.macFrameSkipTextField.intValue]; // execute setter
}

- (IBAction)onChangeMacFrameSkipTextField:(NSTextField *)sender
{
	[self setMacFrameSkip:sender.intValue];
}

- (IBAction)onCheckMacFrameSkipAutomaticButton:(NSButton *)sender
{
	if (sender.intValue == 1) {
		// when automatic is checked, disable macFrameSkipTextField and
		// macFrameSkipStepper, then set macFrameSkip to -1 (automatic)
		[self.macFrameSkipTextField setEnabled:false];
		[self.macFrameSkipStepper setEnabled:false];
		[self setMacFrameSkip:-1];
	} else {
		// when automatic is unchecked, enable macFrameSkipTextField and
		// macFrameSkipStepper, then set macFrameSkip to value of text field
		[self.macFrameSkipTextField setEnabled:true];
		[self.macFrameSkipStepper setEnabled:true];
		[self setMacFrameSkip:self.macFrameSkipTextField.intValue];
	}
}

@end
