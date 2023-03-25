//
//  S9xOpenMultipleViewController.m
//  Snes9x
//
//  Created by Buckley on 3/25/23.
//

#import "S9xOpenMultipleViewController.h"

@interface S9xOpenMultipleViewController ()
@property (nonatomic, weak) IBOutlet NSTextField *label1;
@property (nonatomic, weak) IBOutlet NSTextField *label2;
@property (nonatomic, weak) IBOutlet NSButton *okButton;
@end

@implementation S9xOpenMultipleViewController

- (void)open:(void (^)(NSURL *))completionBlock
{
	NSOpenPanel *openPanel = [NSOpenPanel new];
	openPanel.allowsMultipleSelection = NO;
	openPanel.canChooseDirectories = NO;

	[openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse response)
	{
		if (response == NSModalResponseOK)
		{
			completionBlock(openPanel.URL);
		}
	}];
}

- (void)updateOKButton
{
	self.okButton.enabled = (self.fileURL1 != nil);
}

- (IBAction)choose1:(id)sender
{
	[self open:^(NSURL *fileURL)
	{
		self.fileURL1 = fileURL;
	}];
}

- (IBAction)choose2:(id)sender
{
	[self open:^(NSURL *fileURL)
	{
		self.fileURL2 = fileURL;
	}];
}

- (IBAction)go:(id)sender
{
	if (self.completionBlock != nil)
	{
		[self.view.window close];
		[NSApp stopModal];

		if (self.fileURL1 != nil)
		{
			if (self.fileURL2 != nil)
			{
				self.completionBlock(@[self.fileURL1, self.fileURL2]);
			}
			else
			{
				self.completionBlock(@[self.fileURL1]);
			}
		}
	}
}

- (IBAction)cancel:(id)sender
{
	[self.view.window close];
	[NSApp stopModal];
}

- (void)setFileURL1:(NSURL *)fileURL1
{
	_fileURL1 = fileURL1;

	if (fileURL1 != nil)
	{
		self.label1.stringValue = fileURL1.lastPathComponent;
	}
	else
	{
		self.label1.stringValue = @"";
	}

	[self updateOKButton];
}

- (void)setFileURL2:(NSURL *)fileURL2
{
	_fileURL2 = fileURL2;

	if (fileURL2 != nil)
	{
		self.label2.stringValue = fileURL2.lastPathComponent;
	}
	else
	{
		self.label2.stringValue = @"";
	}

	[self updateOKButton];
}

@end
