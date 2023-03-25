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
  (c) Copyright 2019 - 2023  Michael Donald Buckley
 ***********************************************************************************/

#import <snes9x_framework/snes9x_framework.h>

#import "S9xCheatsViewController.h"
#import "S9xCheatEditViewController.h"
#import "S9xHexNumberFormatter.h"

@interface S9xCheatsViewController () <NSTableViewDataSource, NSTableViewDelegate>

@property (nonatomic, weak) IBOutlet NSTableView *tableView;
@property (nonatomic, weak) IBOutlet NSSegmentedControl *segmentedControl;

@property (nonatomic, strong) NSArray<S9xCheatItem *> *cheats;

@end

@implementation S9xCheatsViewController

- (void)viewDidLoad
{
	[super viewDidLoad];

	[self.segmentedControl setEnabled:YES forSegment:0];
	[self.segmentedControl setEnabled:NO forSegment:1];
	[self.segmentedControl setEnabled:NO forSegment:2];

	self.tableView.doubleAction = @selector(editSelectedCheat);
	self.tableView.target = self;

	[self reloadData];
}

- (void)viewWillAppear
{
	[super viewWillAppear];
	[self.tableView sizeLastColumnToFit];
}

- (void)deselectAll
{
	[self.tableView deselectAll:nil];
}

- (void)reloadData
{
	NSSet<NSNumber *> *selectedIDs = [NSSet setWithArray:[[self.cheats objectsAtIndexes:self.tableView.selectedRowIndexes] valueForKey:@"cheatID"]];

	CGPoint scrollPosition = self.tableView.visibleRect.origin;

	self.cheats = [GetAllCheats() sortedArrayUsingDescriptors:self.tableView.sortDescriptors];
	[self.tableView reloadData];

	NSMutableIndexSet *indexSet = [NSMutableIndexSet new];
	for (NSUInteger i = 0; i < self.tableView.numberOfRows; ++i)
	{
		if ([selectedIDs containsObject:@(self.cheats[i].cheatID)])
		{
			[indexSet addIndex:i];
		}
	}

	[self.tableView scrollPoint:scrollPosition];

	[self.tableView selectRowIndexes:indexSet byExtendingSelection:NO];
}

- (void)tableView:(NSTableView *)tableView sortDescriptorsDidChange:(NSArray<NSSortDescriptor *> *)oldDescriptors
{
	[self reloadData];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	return self.cheats.count;
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	NSTableCellView *view = [self.tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
	view.objectValue = self.cheats[row];
	return view;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	[self.segmentedControl setEnabled:self.tableView.selectedRowIndexes.count > 0 forSegment:1];
	[self.segmentedControl setEnabled:self.tableView.selectedRowIndexes.count == 1 forSegment:2];
}

- (IBAction)segmentedControlAction:(NSSegmentedControl *)sender
{
	switch(sender.selectedSegment)
	{
		case 0:
			[self addNewCheat];
			break;

		case 1:
			[self deleteSelectedCheats];
			break;

		case 2:
			[self editSelectedCheat];
			break;
	}
}

- (void)addNewCheat
{
	S9xCheatEditViewController *vc = [[S9xCheatEditViewController alloc] initWithCheatItem:nil saveCallback:^
	{
		[self reloadData];
	}];

	[self presentViewControllerAsSheet:vc];
}

- (void)deleteSelectedCheats
{

	NSIndexSet *selectedRows = self.tableView.selectedRowIndexes;

	if (selectedRows.count > 0)
	{
		NSArray *cheatsToDelete = [[self.cheats objectsAtIndexes:selectedRows] sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"cheatID" ascending:NO]]];

		for (S9xCheatItem *item in cheatsToDelete)
		{
			[item delete];
		}
	}

	[self.tableView deselectAll:self];
	[self reloadData];
}

- (void)editSelectedCheat
{
	NSIndexSet *selectedRows = self.tableView.selectedRowIndexes;

	if (selectedRows.count == 1)
	{
		S9xCheatItem *item = self.cheats[selectedRows.firstIndex];

		S9xCheatEditViewController *vc = [[S9xCheatEditViewController alloc] initWithCheatItem:item saveCallback:^
		{
			[self reloadData];
		}];

		[self presentViewControllerAsSheet:vc];
	}
}

@end

@interface S9xAddressFormatter : S9xHexNumberFormatter
@end

@implementation S9xAddressFormatter

+ (NSUInteger)maxLength
{
	return 6;
}

@end

@interface S9xValueFormatter : S9xHexNumberFormatter
@end

@implementation S9xValueFormatter

+ (NSUInteger)maxLength
{
	return 2;
}

@end
