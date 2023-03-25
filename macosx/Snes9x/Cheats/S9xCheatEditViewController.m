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
  (c) Copyright 2019 - 2022  Michael Donald Buckley
 ***********************************************************************************/

#import <snes9x_framework/snes9x_framework.h>

#import "S9xCheatEditViewController.h"

NS_ASSUME_NONNULL_BEGIN

@interface S9xCheatEditViewController () <NSTextFieldDelegate>
@property (nonatomic, weak) IBOutlet NSTextField *invalidCodeLabel;
@property (nonatomic, weak) IBOutlet NSButton *saveButton;

@property (nonatomic, strong) S9xCheatItem *cheatItem;
@property (nonatomic, copy) void (^saveBlock)(void);
@end

@implementation S9xCheatEditViewController

- (instancetype)initWithCheatItem:(nullable S9xCheatItem *)cheatItem saveCallback:(void (^)(void))saveCallback
{
	if (self = [super initWithNibName:@"S9xCheatEditViewController" bundle:nil])
	{
		_cheatItem = cheatItem;
		_saveBlock = saveCallback;
	}

	return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

	if (self.cheatItem != nil)
	{
		self.descriptionField.stringValue = self.cheatItem.cheatDescription;
		self.addressField.objectValue = self.cheatItem.address;
		self.valueField.objectValue = self.cheatItem.value;
	}

	[self updateSaveButton];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
	if (notification.object == self.codeField)
	{
		if (self.codeField.stringValue.length > 0 )
		{
			uint32 address = 0;
			uint8 value = 0;

			if ( CheatValuesFromCode(self.codeField.stringValue, &address, &value) )
			{
				self.addressField.objectValue = @(address);
				self.valueField.objectValue = @(value);
				self.invalidCodeLabel.hidden = YES;
			}
			else
			{
				self.addressField.stringValue = @"";
				self.valueField.stringValue = @"";
				self.invalidCodeLabel.hidden = NO;
			}
		}
		else
		{
			if (self.cheatItem != nil)
			{
				self.addressField.objectValue = self.cheatItem.address;
				self.valueField.objectValue = self.cheatItem.value;
			}
			else
			{
				self.addressField.stringValue = @"";
				self.valueField.stringValue = @"";
			}

			self.invalidCodeLabel.hidden = YES;
		}
	}

	else if (notification.object == self.addressField || notification.object == self.valueField)
	{
		self.codeField.stringValue = @"";
	}

	[self updateSaveButton];
}

- (void)updateSaveButton
{
	self.saveButton.enabled = (self.addressField.stringValue.length > 0 && self.valueField.stringValue.length > 0);
}

- (IBAction)save:(id)sender
{
	if ( self.cheatItem != nil )
	{
		self.cheatItem.address = self.addressField.objectValue;
		self.cheatItem.value = self.valueField.objectValue;
		self.cheatItem.cheatDescription = self.descriptionField.stringValue;
	}
	else
	{
		if (self.codeField.stringValue.length > 0)
		{
			CreateCheatFromCode(self.codeField.stringValue, self.descriptionField.stringValue);
		}
		else
		{
			CreateCheatFromAddress(self.addressField.objectValue, self.valueField.objectValue, self.descriptionField.stringValue);
		}
	}

	[self dismissViewController:self];

	self.saveBlock();
}

- (IBAction)cancelOperation:(nullable id)sender
{
	[self dismissViewController:self];
}

@end

NS_ASSUME_NONNULL_END
