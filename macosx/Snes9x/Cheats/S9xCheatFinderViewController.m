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

#import "S9xCheatFinderViewController.h"
#import "S9xCheatEditViewController.h"
#import "S9xHexNumberFormatter.h"

#define	MAIN_MEMORY_SIZE 0x20000
#define MAIN_MEMORY_BASE 0x7E0000
#define MAX_WATCHES 16

typedef NS_ENUM(uint32_t, S9xCheatFinderByteSize) {
	S9xCheatFinderByteSize1 = 1,
	S9xCheatFinderByteSize2 = 2,
	S9xCheatFinderByteSize3 = 3,
	S9xCheatFinderByteSize4 = 4
};

typedef NS_ENUM(uint32_t, S9xCheatFinderValueType) {
	S9xCheatFinderValueTypeUnsigned = 101,
	S9xCheatFinderValueTypeSigned = 102,
	S9xCheatFinderValueTypeHexadecimal = 103
};

typedef NS_ENUM(uint32_t, S9xCheatFinderComparisonType) {
	S9xCheatFinderComparisonTypeEqual = 0,
	S9xCheatFinderComparisonTypeNotEqual = 1,
	S9xCheatFinderComparisonTypeGreaterThan = 2,
	S9xCheatFinderComparisonTypeGreaterThanOrEqual = 3,
	S9xCheatFinderComparisonTypeLessThan = 4,
	S9xCheatFinderComparisonTypeLessThanOrEqual = 5
};

typedef NS_ENUM(uint32_t, S9xCheatFinderCompareTo) {
	S9xCheatFinderCompareToCustom = 200,
	S9xCheatFinderCompareToPrevious = 201,
	S9xCheatFinderCompareToStored = 202
};

typedef NS_ENUM(uint32_t, S9xCheatFinderWatchSegments) {
	S9xCheatFinderWatchSegmentsAdd = 0,
	S9xCheatFinderWatchSegmentsRemove = 1,
	S9xCheatFinderWatchSegmentsSave = 2,
	S9xCheatFinderWatchSegmentsLoad = 3
};

@interface S9xCheatFinderTableView : NSTableView
@property (nonatomic, strong, nullable) void (^deleteBlock)(NSIndexSet *);
@end

@interface S9xCheatFinderViewController () <NSTableViewDataSource, NSTableViewDelegate, NSTextFieldDelegate, S9xEmulationDelegate>

@property (nonatomic, weak) IBOutlet S9xCheatFinderTableView *addressTableView;
@property (nonatomic, weak) IBOutlet NSLayoutConstraint *addressTableWidthConstraint;

@property (nonatomic, weak) IBOutlet NSPopUpButton *byteSizePopUp;

@property (nonatomic, weak) IBOutlet NSPopUpButton *comparisonTypePopUp;
@property (nonatomic, weak) IBOutlet NSTextField *customValueField;

@property (nonatomic, weak) IBOutlet NSButton *resetButon;
@property (nonatomic, weak) IBOutlet NSButton *searchButton;
@property (nonatomic, weak) IBOutlet NSButton *addCheatButton;

@property (nonatomic, weak) IBOutlet S9xCheatFinderTableView *watchTableView;
@property (nonatomic, weak) IBOutlet NSSegmentedControl *watchButtonsSegmentedControl;

@property (nonatomic, readonly, assign) S9xCheatFinderByteSize byteSize;
@property (nonatomic, readonly, assign) S9xCheatFinderValueType valueType;
@property (nonatomic, readonly, assign) S9xCheatFinderComparisonType compareType;
@property (nonatomic, readonly, assign) S9xCheatFinderCompareTo compareTo;
@property (nonatomic, readonly, strong) NSNumber *customValue;

@end

@interface S9xCheatFinderEntry : NSObject
@property (nonatomic, assign) uint32_t address;
@property (nonatomic, assign) uint32_t value;
@property (nonatomic, assign) uint32_t prevValue;
@property (nonatomic, assign) uint32_t storedValue;
@end

@implementation S9xCheatFinderViewController
{
	BOOL _hasStored;
	BOOL _hasPrevious;

	uint8_t _storedRAM[MAIN_MEMORY_SIZE];
	uint8_t _previousRAM[MAIN_MEMORY_SIZE];
	uint8_t _currentRAM[MAIN_MEMORY_SIZE];
	uint8_t _statusFlag[MAIN_MEMORY_SIZE];

	NSMutableArray<S9xCheatFinderEntry *> *_addressRows;

	NSMutableArray<S9xWatchPoint *> *_watchRows;

	NSNumberFormatter *_unsignedFormatter;
	NSNumberFormatter *_signedFormatter;
	NSNumberFormatter *_hexFormatter;

	BOOL _shouldReopen;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

	_unsignedFormatter = [NSNumberFormatter new];
	_unsignedFormatter.minimum = @(0);
	_unsignedFormatter.maximum = @(UINT_MAX);

	_signedFormatter = [NSNumberFormatter new];
	_signedFormatter.minimum = @(INT_MIN);
	_signedFormatter.maximum = @(INT_MAX);

	_hexFormatter = [S9xHexNumberFormatter new];

	[NSNotificationCenter.defaultCenter addObserverForName:NSPreferredScrollerStyleDidChangeNotification object:nil queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *notification)
	{
		[self updateScrollBar];
	}];

	__weak S9xCheatFinderViewController *weakSelf = self;

	self.addressTableView.deleteBlock = ^(NSIndexSet *deletedIndexes)
	{
		__strong S9xCheatFinderViewController *strongSelf = weakSelf;
		[deletedIndexes enumerateIndexesUsingBlock:^(NSUInteger i, BOOL *stop)
		{
			S9xCheatFinderEntry *entry = strongSelf->_addressRows[i];
			strongSelf->_statusFlag[entry.address] = 0;
			strongSelf.resetButon.enabled = YES;
		}];
		[strongSelf reloadData];
	};

	self.watchTableView.deleteBlock = ^(NSIndexSet *deletedIndexes)
	{
		__strong S9xCheatFinderViewController *strongSelf = weakSelf;
		[strongSelf->_watchRows removeObjectsAtIndexes:deletedIndexes];
		[strongSelf.engine setWatchPoints:self->_watchRows];
		[strongSelf reloadWatchPoints];
	};

	[self resetAll];
}

- (void)viewWillAppear
{
	[super viewWillAppear];
	[self updateScrollBar];
	[self.engine copyRAM:(uint8_t *)_currentRAM length:sizeof(_currentRAM)];
	[self reloadData];
	[self reloadWatchPoints];
}

- (void)viewWillDisappear
{
	[super viewWillDisappear];
	[self copyPrevious];
}

- (void)resetAll
{
	[self.byteSizePopUp selectItemWithTag:S9xCheatFinderByteSize2];

	((NSButton *)[self.view viewWithTag:S9xCheatFinderValueTypeUnsigned]).state = NSControlStateValueOn;
	((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToCustom]).state = NSControlStateValueOn;

	((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToPrevious]).enabled = NO;
	((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToStored]).enabled = NO;

	self.resetButon.enabled = NO;
	self.searchButton.enabled = NO;
	self.addCheatButton.enabled = NO;

	[self.comparisonTypePopUp selectItemWithTag:S9xCheatFinderComparisonTypeEqual];
	self.customValueField.stringValue = @"";
	self.customValueField.formatter = _unsignedFormatter;

	_hasStored = NO;
	_hasPrevious = NO;
	_shouldReopen = NO;

	self.engine.emulationDelegate = self;
	[self.engine copyRAM:(uint8_t *)_currentRAM length:sizeof(_currentRAM)];

	[self.engine setWatchPoints:@[]];

	[self reset:self];
}

- (void)copyPrevious
{
	memcpy(_previousRAM, _currentRAM, MAIN_MEMORY_SIZE);
	_hasPrevious = YES;
	((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToPrevious]).enabled = YES;
}

- (void)reloadData
{
	NSMutableSet<NSNumber *> *selectedAddresses = [NSMutableSet new];
	[self.addressTableView.selectedRowIndexes enumerateIndexesUsingBlock:^(NSUInteger i, BOOL *stop)
	{
		[selectedAddresses addObject:@(_addressRows[i].address)];
	}];

	_addressRows = [NSMutableArray new];

	for ( NSUInteger i = 0; i < MAIN_MEMORY_SIZE; ++i )
	{
		if ( _statusFlag[i] )
		{
			S9xCheatFinderEntry *entry = [S9xCheatFinderEntry new];
			entry.address = (uint32_t)i;
			entry.value = [self copyValueFrom:_currentRAM index:i];
			entry.prevValue = [self copyValueFrom:_previousRAM index:i];
			entry.storedValue = [self copyValueFrom:_storedRAM index:i];

			[_addressRows addObject:entry];
		}
	}

	[self.addressTableView reloadData];

	NSIndexSet *newRows = [_addressRows indexesOfObjectsPassingTest:^(S9xCheatFinderEntry *entry, NSUInteger i, BOOL *stop)
	{
		return [selectedAddresses containsObject:@(entry.address)];
	}];

	[self.addressTableView selectRowIndexes:newRows byExtendingSelection:NO];
}

- (void)reloadWatchPoints
{
	_watchRows = [[self.engine getWatchPoints] mutableCopy];
	[self.watchTableView reloadData];
}

- (uint32_t)copyValueFrom:(uint8_t *)buffer index:(NSUInteger)index
{
	uint32_t value = 0;
	NSInteger size = self.byteSize;

	if (size + index >= MAIN_MEMORY_SIZE)
	{
		size = MAIN_MEMORY_SIZE - index;
	}

	memcpy(&value, &(buffer[index]), size);

	return value;
}

- (NSString *)stringFor:(uint8_t *)buffer index:(NSUInteger)index
{
	uint32_t value = [self copyValueFrom:buffer index:index];
	NSString *string = nil;

	switch (self.valueType)
	{
		case S9xCheatFinderValueTypeUnsigned:
			string = [NSString stringWithFormat:@"%u", value];
			break;

		case S9xCheatFinderValueTypeSigned:
			switch (self.byteSize)
			{
				case S9xCheatFinderByteSize1:
					string = [NSString stringWithFormat:@"%d", (int8_t)value];
					break;

				case S9xCheatFinderByteSize2:
					string = [NSString stringWithFormat:@"%d", (int16_t)value];
					break;

				case S9xCheatFinderByteSize3:
					if (value >= (1 << 23))
					{
						value = (int32_t)value - (1 << 24);
					}

					string = [NSString stringWithFormat:@"%d", value];
					break;

				case S9xCheatFinderByteSize4:
					string = [NSString stringWithFormat:@"%d", value];
					break;
			}

			break;

		case S9xCheatFinderValueTypeHexadecimal:
		{
			switch(self.byteSize)
			{
				case S9xCheatFinderByteSize1:
					string = [NSString stringWithFormat:@"%02X", value];
					break;

				case S9xCheatFinderByteSize2:
					string = [NSString stringWithFormat:@"%04X", value];
					break;

				case S9xCheatFinderByteSize3:
					string = [NSString stringWithFormat:@"%06X", value];
					break;

				case S9xCheatFinderByteSize4:
					string = [NSString stringWithFormat:@"%08X", value];
					break;
			}

			break;
		}
	}

	return string;
}

- (void)updateSearchButton
{
	switch (self.compareTo)
	{
		case S9xCheatFinderCompareToCustom:
			self.searchButton.enabled = self.customValueField.stringValue.length > 0;
			break;

		case S9xCheatFinderCompareToPrevious:
			self.searchButton.enabled = _hasPrevious;
			break;

		case S9xCheatFinderCompareToStored:
			self.searchButton.enabled = _hasStored;
			break;
	}
}

- (void)updateWatchButtons
{
	NSInteger numAddressesSelected = self.addressTableView.numberOfSelectedRows;
	NSUInteger numWatches = _watchRows.count;
	NSInteger numWatchesSelected = self.watchTableView.numberOfSelectedRows;

	[self.watchButtonsSegmentedControl setEnabled:numAddressesSelected > 0 && numAddressesSelected + numWatches <= MAX_WATCHES forSegment:S9xCheatFinderWatchSegmentsAdd];

	[self.watchButtonsSegmentedControl setEnabled:numWatchesSelected > 0 forSegment:(NSInteger)S9xCheatFinderWatchSegmentsRemove];

	[self.watchButtonsSegmentedControl setEnabled:numWatches != 0 forSegment:S9xCheatFinderWatchSegmentsSave];
}

- (void)updateScrollBar
{
	const CGFloat defaultWidth = 393.0;
	const CGFloat scrollbarWidth = 15.0;

	if (NSScroller.preferredScrollerStyle == NSScrollerStyleLegacy)
	{
		self.addressTableWidthConstraint.constant = defaultWidth + scrollbarWidth;
	}
	else
	{
		self.addressTableWidthConstraint.constant = defaultWidth;
	}

	[self.view layout];
	[self.addressTableView sizeLastColumnToFit];
	[self.watchTableView sizeLastColumnToFit];
}

#pragma mark - Actions

- (IBAction)storeValues:(id)sender
{
	memcpy(_storedRAM, _currentRAM, MAIN_MEMORY_SIZE);
	_hasStored = YES;
	((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToStored]).enabled = YES;

	[self reloadData];
	[self updateSearchButton];
}

- (IBAction)reset:(id)sender
{
	memset(_statusFlag, 1, sizeof(_statusFlag));
	self.resetButon.enabled = NO;

	[self reloadData];
	[self updateSearchButton];

	[self.addressTableView deselectAll:self];
	[self.watchTableView deselectAll:self];

	[self updateWatchButtons];
}

- (IBAction)search:(id)sender
{
	NSMutableIndexSet *rowsToRemove = [NSMutableIndexSet new];
	NSNumber *customValue = self.customValue;

	S9xCheatFinderValueType valueType = self.valueType;
	S9xCheatFinderComparisonType comparisonType = self.compareType;
	S9xCheatFinderCompareTo compareTo = self.compareTo;

	for (S9xCheatFinderEntry *entry in _addressRows)
	{
		switch(valueType)
		{
			case S9xCheatFinderValueTypeUnsigned:
			case S9xCheatFinderValueTypeHexadecimal:
			{
				uint32_t value = [self copyValueFrom:_currentRAM index:entry.address];
				uint32_t comparedValue;

				switch(compareTo)
				{
					case S9xCheatFinderCompareToCustom:
						comparedValue = customValue.unsignedIntValue;
						break;

					case S9xCheatFinderCompareToStored:
						comparedValue = [self copyValueFrom:_storedRAM index:entry.address];
						break;

					case S9xCheatFinderCompareToPrevious:
						comparedValue = [self copyValueFrom:_previousRAM index:entry.address];
						break;
				}

				switch(comparisonType)
				{
					case S9xCheatFinderComparisonTypeEqual:
						_statusFlag[entry.address] = (value == comparedValue);
						break;

					case S9xCheatFinderComparisonTypeNotEqual:
						_statusFlag[entry.address] = (value != comparedValue);
						break;

					case S9xCheatFinderComparisonTypeGreaterThan:
						_statusFlag[entry.address] = (value > comparedValue);
						break;

					case S9xCheatFinderComparisonTypeGreaterThanOrEqual:
						_statusFlag[entry.address] = (value >= comparedValue);
						break;

					case S9xCheatFinderComparisonTypeLessThan:
						_statusFlag[entry.address] = (value < comparedValue);
						break;

					case S9xCheatFinderComparisonTypeLessThanOrEqual:
						_statusFlag[entry.address] = (value <= comparedValue);
						break;
				}

				break;
			}

			case S9xCheatFinderValueTypeSigned:
			{
				int32_t value = [self copyValueFrom:_currentRAM index:entry.address];
				int32_t comparedValue;

				switch(compareTo)
				{
					case S9xCheatFinderCompareToCustom:
						comparedValue = customValue.intValue;
						break;

					case S9xCheatFinderCompareToStored:
						comparedValue = [self copyValueFrom:_storedRAM index:entry.address];
						break;

					case S9xCheatFinderCompareToPrevious:
						comparedValue = [self copyValueFrom:_previousRAM index:entry.address];
						break;
				}

				switch(comparisonType)
				{
					case S9xCheatFinderComparisonTypeEqual:
						_statusFlag[entry.address] = (value == comparedValue);
						break;

					case S9xCheatFinderComparisonTypeNotEqual:
						_statusFlag[entry.address] = (value != comparedValue);
						break;

					case S9xCheatFinderComparisonTypeGreaterThan:
						_statusFlag[entry.address] = (value > comparedValue);
						break;

					case S9xCheatFinderComparisonTypeGreaterThanOrEqual:
						_statusFlag[entry.address] = (value >= comparedValue);
						break;

					case S9xCheatFinderComparisonTypeLessThan:
						_statusFlag[entry.address] = (value < comparedValue);
						break;

					case S9xCheatFinderComparisonTypeLessThanOrEqual:
						_statusFlag[entry.address] = (value <= comparedValue);
						break;
				}

				break;
			}
		}
	}

	[_addressRows removeObjectsAtIndexes:rowsToRemove];

	[self reloadData];

	[self updateWatchButtons];

	self.resetButon.enabled = YES;
}

- (IBAction)reload:(id)sender
{
	[self reloadData];
}

- (IBAction)setFormat:(id)sender
{
	[self reloadData];

	S9xCheatFinderValueType valueType = (S9xCheatFinderValueType)[sender tag];
	NSNumber *value = self.customValue;

	switch (valueType)
	{
		case S9xCheatFinderValueTypeUnsigned:
			self.customValueField.formatter = _unsignedFormatter;
			break;

		case S9xCheatFinderValueTypeSigned:
			self.customValueField.formatter = _signedFormatter;
			break;

		case S9xCheatFinderValueTypeHexadecimal:
			self.customValueField.formatter = _hexFormatter;
			break;
	}

	self.customValueField.stringValue = [self.customValueField.formatter stringForObjectValue:value];
}

- (IBAction)setCompareTarget:(id)sender
{
	[self reloadData];
	[self updateSearchButton];
	[self updateWatchButtons];
}

- (IBAction)addCheat:(id)sender
{
	S9xCheatFinderEntry *entry = _addressRows[self.addressTableView.selectedRow];

	S9xCheatEditViewController *vc = [[S9xCheatEditViewController alloc] initWithCheatItem:nil saveCallback:^
	{
		[self.delegate addedCheat];
	}];

	[self presentViewControllerAsSheet:vc];

	vc.addressField.objectValue = @(entry.address + MAIN_MEMORY_BASE);
	vc.valueField.objectValue = @(_currentRAM[entry.address]);
	[vc.view.window makeFirstResponder:vc.descriptionField];
	[vc updateSaveButton];
}

- (IBAction)watchesAction:(NSSegmentedControl *)sender
{
	switch((S9xCheatFinderWatchSegments)sender.selectedSegment)
	{
		case S9xCheatFinderWatchSegmentsAdd:
		{
			NSIndexSet *selectedIndexes = self.addressTableView.selectedRowIndexes;

			[selectedIndexes enumerateIndexesUsingBlock:^(NSUInteger i, BOOL *stop)
			{
				S9xWatchPoint *watchPoint = [S9xWatchPoint new];
				S9xCheatFinderEntry *entry = _addressRows[i];

				watchPoint.address = entry.address + MAIN_MEMORY_BASE;
				watchPoint.size = self.byteSize;
				watchPoint.format = self.valueType - 100;

				[_watchRows addObject:watchPoint];
			}];

			[self.engine setWatchPoints:_watchRows];
			[self reloadWatchPoints];

			break;
		}

		case S9xCheatFinderWatchSegmentsRemove:
		{
			[_watchRows removeObjectsAtIndexes:self.watchTableView.selectedRowIndexes];
			[self.engine setWatchPoints:_watchRows];
			[self reloadWatchPoints];

			break;
		}

		case S9xCheatFinderWatchSegmentsSave:
		{
			NSSavePanel *savePanel = [NSSavePanel new];
			savePanel.nameFieldStringValue = @"watches.txt";

			[savePanel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse response)
			{
				if (response == NSModalResponseOK)
				{
					FILE *file = fopen(savePanel.URL.path.UTF8String, "wb");

					if (file != NULL)
					{
						for ( S9xWatchPoint *watchPoint in self->_watchRows)
						{
							fprintf(file, "address = 0x%x, name = \"?\", size = %d, format = %d\n", watchPoint.address, watchPoint.size, watchPoint.format);
						}

						fclose(file);
					}
				}
			}];

			break;
		}

		case S9xCheatFinderWatchSegmentsLoad:
		{
			NSOpenPanel *openPanel = [NSOpenPanel new];
			openPanel.allowsMultipleSelection = NO;
			openPanel.canChooseDirectories = NO;

			[openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse response)
			{
				FILE *file = fopen(openPanel.URL.path.UTF8String, "rb");

				if (file != NULL)
				{
					self->_watchRows = [NSMutableArray new];

					for (NSUInteger i = 0; i < MAX_WATCHES; ++i)
					{
						uint32_t address;
						uint32_t size;
						uint32_t format;
						char ignore[32];

						fscanf(file, "address = 0x%x, name = \"%31[^\"]\", size = %d, format = %d\n", &address, ignore, &size, &format);

						if (ferror(file))
						{
							break;
						}

						S9xWatchPoint *watchPoint = [S9xWatchPoint new];
						watchPoint.address = address;
						watchPoint.size = size;
						watchPoint.format = format;

						[self->_watchRows addObject:watchPoint];

						if(feof(file))
						{
							break;
						}
					}

					fclose(file);

					[self.engine setWatchPoints:self->_watchRows];
					[self reloadWatchPoints];
				}
			}];
		}
	}
}

#pragma mark - Emulation Delegate

- (void)gameLoaded
{
	[self resetAll];
	[self emulationResumed];
}

- (void)emulationPaused
{
	if (!NSThread.isMainThread)
	{
		dispatch_sync(dispatch_get_main_queue(), ^
		{
			[self emulationPaused];
		});
	}
	else if (_shouldReopen)
	{
		[self.view.window makeKeyAndOrderFront:self];
		_shouldReopen = NO;
	}
}

- (void)emulationResumed
{
	if (!NSThread.isMainThread)
	{
		dispatch_sync(dispatch_get_main_queue(), ^
		{
			[self emulationResumed];
		});
	}
	else
	{
		if ([self.view.window isVisible])
		{
			[self.view.window close];
			_shouldReopen = YES;
		}

		[self reloadData];
	}
}

#pragma mark - Table View Delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
	if (tableView == self.addressTableView)
	{
		return _addressRows.count;
	}
	else
	{
		return _watchRows.count;
	}
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	NSString *value = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
	NSTableCellView *cell = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
	cell.textField.stringValue = value;
	return cell;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	if (tableView == self.addressTableView)
	{
		S9xCheatFinderEntry *entry = _addressRows[row];
		if ([tableColumn.identifier isEqualToString:@"address"])
		{
			return [NSString stringWithFormat:@"%06X", entry.address + MAIN_MEMORY_BASE];
		}
		else if ([tableColumn.identifier isEqualToString:@"currentValue"])
		{
			return [self stringFor:_currentRAM index:entry.address];
		}
		else if ([tableColumn.identifier isEqualToString:@"previousValue"])
		{
			if (!_hasPrevious)
			{
				return @"";
			}

			return [self stringFor:_previousRAM index:entry.address];
		}
		else if ( [tableColumn.identifier isEqualToString:@"storedValue"])
		{
			if (!_hasStored)
			{
				return @"";
			}

			return [self stringFor:_storedRAM index:entry.address];
		}

		return @"";
	}
	else
	{
		S9xWatchPoint *watchPoint = _watchRows[row];
		NSString *formatString;

		switch((S9xCheatFinderValueType)(watchPoint.format + 100))
		{
			case S9xCheatFinderValueTypeSigned:
				formatString = @"s";
				break;

			case S9xCheatFinderValueTypeUnsigned:
				formatString = @"u";
				break;

			case S9xCheatFinderValueTypeHexadecimal:
				formatString = @"x";
				break;
		}

		return [NSString stringWithFormat:@"%06X,%d%@", watchPoint.address, watchPoint.size, formatString];
	}
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
	self.addCheatButton.enabled = self.addressTableView.numberOfSelectedRows == 1;
	[self updateWatchButtons];
}

#pragma mark - Text Field Delegate

- (void)controlTextDidChange:(NSNotification *)obj
{
	[self updateSearchButton];
}

#pragma mark - Accessors

@dynamic byteSize;
- (S9xCheatFinderByteSize)byteSize
{
	return (S9xCheatFinderByteSize)[self.byteSizePopUp selectedTag];
}

- (S9xCheatFinderValueType)valueType
{
	S9xCheatFinderValueType valueType;

	if (((NSButton *)[self.view viewWithTag:S9xCheatFinderValueTypeUnsigned]).state == NSOnState)
	{
		valueType = S9xCheatFinderValueTypeUnsigned;
	}
	else if (((NSButton *)[self.view viewWithTag:S9xCheatFinderValueTypeSigned]).state == NSOnState)
	{
		valueType = S9xCheatFinderValueTypeSigned;
	}
	else
	{
		valueType = S9xCheatFinderValueTypeHexadecimal;
	}

	return valueType;
}

@dynamic compareType;
- (S9xCheatFinderComparisonType)compareType
{
	return (S9xCheatFinderComparisonType)self.comparisonTypePopUp.selectedTag;
}

@dynamic compareTo;
- (S9xCheatFinderCompareTo)compareTo
{
	S9xCheatFinderCompareTo compareTo;

	if (((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToCustom]).state == NSOnState)
	{
		compareTo = S9xCheatFinderCompareToCustom;
	}
	else if (((NSButton *)[self.view viewWithTag:S9xCheatFinderCompareToStored]).state == NSOnState)
	{
		compareTo = S9xCheatFinderCompareToStored;
	}
	else
	{
		compareTo = S9xCheatFinderCompareToPrevious;
	}

	return compareTo;
}

@dynamic customValue;
- (NSNumber *)customValue
{
	return [((NSNumberFormatter *)self.customValueField.formatter) numberFromString:self.customValueField.stringValue];
}

@end

@implementation S9xCheatFinderTableView

- (void)keyDown:(NSEvent *)event
{
	if ( [event.characters isEqualToString:@"\b"] || [event.characters isEqualToString:@"\x7f"] )
	{
		if (self.deleteBlock != nil && self.numberOfSelectedRows > 0)
		{
			self.deleteBlock(self.selectedRowIndexes);
		}
	}
	else
	{
		[super keyDown:event];
	}
}

@end

@implementation S9xCheatFinderEntry
@end
