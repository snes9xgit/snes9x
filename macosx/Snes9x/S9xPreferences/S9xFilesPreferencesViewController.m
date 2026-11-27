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

#import "S9xFilesPreferencesViewController.h"
#import "AppDelegate.h"
#import "S9xPreferencesConstants.h"

@interface S9xFilesPreferencesViewController ()
@property (nonatomic, strong) NSPopUpButton *saveLocationPopUp;
@property (nonatomic, strong) NSPathControl *customFolderPathControl;
@property (nonatomic, strong) NSButton *chooseFolderButton;
@property (nonatomic, strong) NSButton *openFolderButton;
@end

@implementation S9xFilesPreferencesViewController

- (instancetype)init
{
	if (self = [super initWithNibName:nil bundle:nil])
	{
		self.title = NSLocalizedString(@"Files", nil);
		self.image = [NSImage imageNamed:@"files"];
	}

	return self;
}

- (void)loadView
{
	NSView *container = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 500, 170)];

	NSTextField *locationLabel = [NSTextField labelWithString:NSLocalizedString(@"Save Data Location:", nil)];
	locationLabel.alignment = NSTextAlignmentRight;

	self.saveLocationPopUp = [[NSPopUpButton alloc] initWithFrame:NSZeroRect pullsDown:NO];
	self.saveLocationPopUp.target = self;
	self.saveLocationPopUp.action = @selector(onSaveLocationChanged:);

	NSMenuItem *appSupportItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Application Support (Default)", nil) action:nil keyEquivalent:@""];
	appSupportItem.tag = 2;
	NSMenuItem *romFolderItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Same Folder as ROM", nil) action:nil keyEquivalent:@""];
	romFolderItem.tag = 1;
	NSMenuItem *customFolderItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Custom Folder…", nil) action:nil keyEquivalent:@""];
	customFolderItem.tag = 4;

	[self.saveLocationPopUp.menu addItem:appSupportItem];
	[self.saveLocationPopUp.menu addItem:romFolderItem];
	[self.saveLocationPopUp.menu addItem:customFolderItem];

	NSTextField *customLabel = [NSTextField labelWithString:NSLocalizedString(@"Custom Folder:", nil)];
	customLabel.alignment = NSTextAlignmentRight;

	self.customFolderPathControl = [NSPathControl new];
	self.customFolderPathControl.pathStyle = NSPathStyleStandard;
	self.customFolderPathControl.editable = NO;
	self.customFolderPathControl.translatesAutoresizingMaskIntoConstraints = NO;
	[self.customFolderPathControl.widthAnchor constraintGreaterThanOrEqualToConstant:220].active = YES;

	self.chooseFolderButton = [NSButton buttonWithTitle:NSLocalizedString(@"Choose…", nil) target:self action:@selector(onChooseFolder:)];

	NSStackView *customRow = [NSStackView stackViewWithViews:@[self.customFolderPathControl, self.chooseFolderButton]];
	customRow.orientation = NSUserInterfaceLayoutOrientationHorizontal;
	customRow.spacing = 8.0;

	self.openFolderButton = [NSButton buttonWithTitle:NSLocalizedString(@"Open Saves Folder in Finder", nil) target:self action:@selector(onOpenSavesFolder:)];

	NSGridView *grid = [NSGridView gridViewWithViews:@[
		@[locationLabel, self.saveLocationPopUp],
		@[customLabel, customRow],
		@[[NSGridCell emptyContentView], self.openFolderButton],
	]];
	grid.translatesAutoresizingMaskIntoConstraints = NO;
	grid.rowSpacing = 14.0;
	grid.columnSpacing = 12.0;
	[grid columnAtIndex:0].xPlacement = NSGridCellPlacementTrailing;
	[grid columnAtIndex:1].xPlacement = NSGridCellPlacementLeading;
	[grid rowAtIndex:0].yPlacement = NSGridCellPlacementCenter;
	[grid rowAtIndex:1].yPlacement = NSGridCellPlacementCenter;

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
	NSInteger mode = [defaults objectForKey:kSaveLocationModePref] ? [defaults integerForKey:kSaveLocationModePref] : 2;
	NSString *customPath = [defaults stringForKey:kCustomSaveFolderPref];

	[self.saveLocationPopUp selectItemWithTag:mode];
	if (customPath.length > 0)
	{
		self.customFolderPathControl.URL = [NSURL fileURLWithPath:customPath];
	}
	else
	{
		self.customFolderPathControl.URL = nil;
	}

	BOOL isCustom = (mode == 4);
	self.chooseFolderButton.enabled = isCustom;
	self.customFolderPathControl.enabled = isCustom;
	self.openFolderButton.enabled = (mode != 1);
}

- (IBAction)onSaveLocationChanged:(NSPopUpButton *)sender
{
	uint8_t mode = (uint8_t)sender.selectedTag;
	[NSUserDefaults.standardUserDefaults setInteger:mode forKey:kSaveLocationModePref];
	[((AppDelegate *)NSApp.delegate).s9xEngine setSaveLocationMode:mode];

	if (mode == 4 && [NSUserDefaults.standardUserDefaults stringForKey:kCustomSaveFolderPref].length == 0)
	{
		[self onChooseFolder:nil];
	}

	[self refresh];
}

- (IBAction)onChooseFolder:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];
	panel.canChooseFiles = NO;
	panel.canChooseDirectories = YES;
	panel.canCreateDirectories = YES;
	panel.allowsMultipleSelection = NO;

	[panel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse result)
	{
		if (result == NSModalResponseOK && panel.URL != nil)
		{
			NSString *path = panel.URL.path;
			[NSUserDefaults.standardUserDefaults setObject:path forKey:kCustomSaveFolderPref];
			[((AppDelegate *)NSApp.delegate).s9xEngine setCustomSaveFolderPath:path];
			[self refresh];
		}
	}];
}

- (IBAction)onOpenSavesFolder:(id)sender
{
	NSURL *url = [((AppDelegate *)NSApp.delegate).s9xEngine currentSaveRootURL];
	if (url != nil)
	{
		[NSWorkspace.sharedWorkspace openURL:url];
	}
}

@end
