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

#import "S9xSoundPreferencesViewController.h"
#import "AppDelegate.h"
#import "S9xPreferencesConstants.h"

@interface S9xSoundPreferencesViewController ()
@property (nonatomic, strong) NSSlider *volumeSlider;
@property (nonatomic, strong) NSTextField *volumeValueLabel;
@property (nonatomic, strong) NSButton *muteCheckbox;
@property (nonatomic, strong) NSButton *soundSyncCheckbox;
@property (nonatomic, strong) NSButton *dynamicRateCheckbox;
@property (nonatomic, strong) NSPopUpButton *bufferPopUp;
@end

@implementation S9xSoundPreferencesViewController

- (instancetype)init
{
	if (self = [super initWithNibName:nil bundle:nil])
	{
		self.title = NSLocalizedString(@"Sound", nil);
		self.image = [NSImage imageNamed:@"sound"];
	}

	return self;
}

- (void)loadView
{
	NSView *container = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 460, 210)];

	NSTextField *volumeLabel = [NSTextField labelWithString:NSLocalizedString(@"Volume:", nil)];
	volumeLabel.alignment = NSTextAlignmentRight;

	self.volumeSlider = [NSSlider sliderWithValue:80 minValue:0 maxValue:100 target:self action:@selector(onVolumeChanged:)];
	self.volumeSlider.continuous = YES;
	self.volumeSlider.translatesAutoresizingMaskIntoConstraints = NO;
	[self.volumeSlider.widthAnchor constraintEqualToConstant:210].active = YES;

	self.volumeValueLabel = [NSTextField labelWithString:@"80%"];
	self.volumeValueLabel.font = [NSFont monospacedDigitSystemFontOfSize:NSFont.systemFontSize weight:NSFontWeightRegular];
	[self.volumeValueLabel.widthAnchor constraintEqualToConstant:44].active = YES;

	NSStackView *volumeRow = [NSStackView stackViewWithViews:@[self.volumeSlider, self.volumeValueLabel]];
	volumeRow.orientation = NSUserInterfaceLayoutOrientationHorizontal;
	volumeRow.spacing = 8.0;

	self.muteCheckbox = [NSButton checkboxWithTitle:NSLocalizedString(@"Mute audio output", nil) target:self action:@selector(onMuteChanged:)];
	self.soundSyncCheckbox = [NSButton checkboxWithTitle:NSLocalizedString(@"Sync emulation speed to audio (Sound Sync)", nil) target:self action:@selector(onSoundSyncChanged:)];
	self.dynamicRateCheckbox = [NSButton checkboxWithTitle:NSLocalizedString(@"Dynamic rate control", nil) target:self action:@selector(onDynamicRateChanged:)];

	NSTextField *bufferLabel = [NSTextField labelWithString:NSLocalizedString(@"Audio Buffer:", nil)];
	bufferLabel.alignment = NSTextAlignmentRight;

	self.bufferPopUp = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];
	self.bufferPopUp.target = self;
	self.bufferPopUp.action = @selector(onBufferChanged:);
	for (NSNumber *ms in @[@40, @60, @80, @100, @120, @160])
	{
		NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"%@ ms", ms] action:nil keyEquivalent:@""];
		item.tag = ms.integerValue;
		[self.bufferPopUp.menu addItem:item];
	}

	NSGridView *grid = [NSGridView gridViewWithViews:@[
		@[volumeLabel, volumeRow],
		@[[NSGridCell emptyContentView], self.muteCheckbox],
		@[bufferLabel, self.bufferPopUp],
		@[[NSGridCell emptyContentView], self.soundSyncCheckbox],
		@[[NSGridCell emptyContentView], self.dynamicRateCheckbox],
	]];
	grid.translatesAutoresizingMaskIntoConstraints = NO;
	grid.rowSpacing = 12.0;
	grid.columnSpacing = 12.0;
	[grid columnAtIndex:0].xPlacement = NSGridCellPlacementTrailing;
	[grid columnAtIndex:1].xPlacement = NSGridCellPlacementLeading;
	[grid rowAtIndex:0].yPlacement = NSGridCellPlacementCenter;
	[grid rowAtIndex:2].yPlacement = NSGridCellPlacementCenter;

	[container addSubview:grid];
	[NSLayoutConstraint activateConstraints:@[
		[grid.topAnchor constraintEqualToAnchor:container.topAnchor constant:24.0],
		[grid.bottomAnchor constraintEqualToAnchor:container.bottomAnchor constant:-24.0],
		[grid.leadingAnchor constraintEqualToAnchor:container.leadingAnchor constant:28.0],
		[grid.trailingAnchor constraintEqualToAnchor:container.trailingAnchor constant:-28.0],
	]];

	self.view = container;
}

- (void)viewDidLoad
{
	[super viewDidLoad];
	[self refresh];
}

- (void)refresh
{
	NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
	NSInteger volume = [defaults objectForKey:kSoundVolumePref] ? [defaults integerForKey:kSoundVolumePref] : 80;
	BOOL muted = [defaults boolForKey:kSoundMutePref];
	BOOL soundSync = [defaults boolForKey:kSoundSyncPref];
	BOOL dynamicRate = [defaults boolForKey:kDynamicRateControlPref];
	NSInteger bufferMs = [defaults objectForKey:kSoundBufferMsPref] ? [defaults integerForKey:kSoundBufferMsPref] : 80;

	self.volumeSlider.integerValue = volume;
	self.volumeValueLabel.stringValue = [NSString stringWithFormat:@"%ld%%", (long)volume];
	self.muteCheckbox.state = muted ? NSControlStateValueOn : NSControlStateValueOff;
	self.soundSyncCheckbox.state = soundSync ? NSControlStateValueOn : NSControlStateValueOff;
	self.dynamicRateCheckbox.state = dynamicRate ? NSControlStateValueOn : NSControlStateValueOff;
	[self.bufferPopUp selectItemWithTag:bufferMs];
}

- (IBAction)onVolumeChanged:(NSSlider *)sender
{
	NSInteger vol = sender.integerValue;
	self.volumeValueLabel.stringValue = [NSString stringWithFormat:@"%ld%%", (long)vol];
	[NSUserDefaults.standardUserDefaults setInteger:vol forKey:kSoundVolumePref];
	[((AppDelegate *)NSApp.delegate).s9xEngine setSoundVolume:vol];
}

- (IBAction)onMuteChanged:(NSButton *)sender
{
	BOOL muted = (sender.state == NSControlStateValueOn);
	[NSUserDefaults.standardUserDefaults setBool:muted forKey:kSoundMutePref];
	[((AppDelegate *)NSApp.delegate).s9xEngine setSoundMuted:muted];
}

- (IBAction)onSoundSyncChanged:(NSButton *)sender
{
	BOOL sync = (sender.state == NSControlStateValueOn);
	[NSUserDefaults.standardUserDefaults setBool:sync forKey:kSoundSyncPref];
	[((AppDelegate *)NSApp.delegate).s9xEngine setSoundSync:sync];
}

- (IBAction)onDynamicRateChanged:(NSButton *)sender
{
	BOOL enabled = (sender.state == NSControlStateValueOn);
	[NSUserDefaults.standardUserDefaults setBool:enabled forKey:kDynamicRateControlPref];
	[((AppDelegate *)NSApp.delegate).s9xEngine setDynamicRateControl:enabled];
}

- (IBAction)onBufferChanged:(NSPopUpButton *)sender
{
	uint32_t ms = (uint32_t)sender.selectedTag;
	[NSUserDefaults.standardUserDefaults setInteger:ms forKey:kSoundBufferMsPref];
	[((AppDelegate *)NSApp.delegate).s9xEngine setSoundBufferMs:ms];
}

@end
