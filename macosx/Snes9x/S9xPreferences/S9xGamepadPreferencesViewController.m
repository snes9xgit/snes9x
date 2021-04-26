//
//  S9xGamepadPreferencesViewController.m
//  Snes9x
//
//  Created by Buckley on 4/14/21.
//

#import <snes9x_framework/snes9x_framework.h>

#import "AppDelegate.h"
#import "S9xButtonConfigTextField.h"
#import "S9xPreferencesConstants.h"
#import "S9xControlsPreferencesViewController.h"

@interface S9xControlsPreferencesViewController () <S9xInputDelegate>
@property (nonatomic, weak) IBOutlet NSPopUpButton *devicePopUp;
@property (nonatomic, weak) IBOutlet NSPopUpButton *playerPopUp;
@property (nonatomic, strong) NSArray *configTextFields;
@end

@implementation S9xControlsPreferencesViewController

- (instancetype)init
{
	if (self = [super initWithNibName:@"S9xGamepadPreferencesViewController" bundle:nil])
	{
		self.title = @"Controls";
	}

	return self;
}

@dynamic image;
- (NSImage *)image
{
	return [NSImage imageNamed:@"gamepad"];
}

- (void)viewDidLoad {
    [super viewDidLoad];

	AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
	NSUInteger joypadIndex = 0;
	for (S9xJoypad *joypad in [appDelegate listJoypads])
	{
		NSMenuItem *item = [NSMenuItem new];
		item.title = joypad.name;
		item.tag = joypadIndex++;
		item.representedObject = joypad;
		[self.devicePopUp.menu addItem:item];
	}

	// collect all S9xButtonConfigTextFields within subviews
	NSMutableArray *configTextFields = [[NSMutableArray alloc] init];

	for ( NSView *view in self.view.subviews )
	{
		if ([view isKindOfClass:[S9xButtonConfigTextField class]])
		{
			[configTextFields addObject:view];
		}
	}

	self.configTextFields = configTextFields;

	for (S9xButtonConfigTextField *configTextField in self.configTextFields)
	{
		[configTextField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
		[configTextField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];
	}

	// select Keyboard as default
	[self selectDeviceForPlayer:0];

}

- (void)refresh
{
	if (self.devicePopUp.selectedItem.tag < 0)
	{
		NSMutableDictionary<NSNumber *, NSNumber *> *controlsDict = [NSMutableDictionary new];

		NSDictionary *keyboardDict = [NSUserDefaults.standardUserDefaults objectForKey:kKeyboardPrefs];
		NSInteger playerNum = self.playerPopUp.selectedItem.tag;

		for (NSUInteger i = 0; i < kNumButtons; ++i)
		{
			controlsDict[@(i)] = keyboardDict[@(i + (playerNum * kNumButtons)).stringValue];
		}

		for (S9xButtonConfigTextField *configTextField in self.configTextFields)
		{
			[configTextField removeObserver:self forKeyPath:@"keyCode"];
			[configTextField removeObserver:self forKeyPath:@"joypadInput"];

			NSNumber *keyCode = controlsDict[@(configTextField.tag)];

			configTextField.joypadInput = nil;

			if ( keyCode != nil )
			{
				configTextField.keyCode = keyCode.intValue;
			}
			else
			{
				configTextField.keyCode = -1;
			}

			[configTextField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
			[configTextField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];

			configTextField.disableKeyboardInput = NO;
		}
	}
	else
	{
		AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
		S9xJoypad *joypad = self.devicePopUp.selectedItem.representedObject;
		NSString *joypadKey = [appDelegate prefsKeyForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
		NSDictionary *joypadDIct = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadInputPrefs] objectForKey:joypadKey];

		for (S9xButtonConfigTextField *configTextField in self.configTextFields)
		{
			[configTextField removeObserver:self forKeyPath:@"keyCode"];
			[configTextField removeObserver:self forKeyPath:@"joypadInput"];

			uint32 cookie = 0;
			int32 value = 0;
			S9xButtonCode buttonCode = (S9xButtonCode)configTextField.tag;
			NSString *inputString = joypadDIct[@(buttonCode).stringValue];

			configTextField.keyCode = -1;

			if ([appDelegate getValuesFromString:inputString cookie:&cookie value:&value])
			{
				S9xJoypadInput *input = [S9xJoypadInput new];
				input.cookie = cookie;
				input.value = value;
				input.buttonCode = buttonCode;
				configTextField.joypadInput = input;
				configTextField.stringValue = [appDelegate labelForVendorID:joypad.vendorID productID:joypad.productID cookie:cookie value:value];
			}
			else
			{
				configTextField.joypadInput = nil;
				configTextField.stringValue = @"";
			}

			[configTextField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
			[configTextField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];

			configTextField.disableKeyboardInput = YES;
		}
	}
}

- (void)selectDeviceForPlayer:(int8_t)player
{
	AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
	NSString* joypadKey = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadPlayerPrefs] objectForKey:@(player).stringValue];

	[self.devicePopUp selectItemAtIndex:0];

	if (joypadKey != nil)
	{
		uint32 vendorID = 0;
		uint32 productID = 0;
		uint32 index = 0;

		if ( [appDelegate getValuesFromString:joypadKey vendorID:&vendorID productID:&productID index:&index])
		{
			S9xJoypad *joypad = [S9xJoypad new];
			joypad.vendorID = vendorID;
			joypad.productID = productID;
			joypad.index = index;

			for (NSMenuItem *item in self.devicePopUp.menu.itemArray)
			{
				if ([joypad isEqual:item.representedObject])
				{
					[self.devicePopUp selectItem:item];
					break;
				}
			}
		}
	}
}

- (BOOL)handleInput:(S9xJoypadInput *)input fromJoypad:(S9xJoypad *)joypad
{
	id firstResponder = self.view.window.firstResponder;

	if ([firstResponder respondsToSelector:@selector(isFieldEditor)] && [firstResponder isFieldEditor])
	{
		firstResponder = [firstResponder delegate];
	}

	if ([firstResponder respondsToSelector:@selector(setJoypadInput:)])
	{
		S9xJoypad *currentJoypad = self.devicePopUp.selectedItem.representedObject;

		if ([joypad isEqual:currentJoypad])
		{
			[firstResponder setJoypadInput:input];
			return YES;
		}
	}

	return NO;
}

- (void)deviceSettingChanged:(S9xDeviceSetting)deviceSetting {}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
	if ([keyPath isEqualToString:@"keyCode"])
	{
		S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)object;
		S9xButtonCode buttonCode = (S9xButtonCode)field.tag;
		uint16_t keyCode = field.keyCode;
		int8_t player = self.playerPopUp.selectedItem.tag;

		if (keyCode != (CGKeyCode)-1)
		{
			[((AppDelegate *) NSApp.delegate) setButtonCode:buttonCode forKeyCode:keyCode player:player];
		}
		else
		{
			[((AppDelegate *) NSApp.delegate) clearButton:buttonCode forPlayer:player];
		}

		[NSUserDefaults.standardUserDefaults synchronize];

		[self refresh];
	}
	else if ( [keyPath isEqualToString:@"joypadInput"])
	{
		S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)object;
		S9xButtonCode buttonCode = (S9xButtonCode)field.tag;
		S9xJoypad *joypad = self.devicePopUp.selectedItem.representedObject;

		if ([joypad isKindOfClass:[S9xJoypad class]])
		{
			S9xJoypadInput *input = field.joypadInput;

			if (input != nil)
			{
				[((AppDelegate *)NSApp.delegate) setButton:buttonCode forVendorID:joypad.vendorID productID:joypad.productID index:joypad.index cookie:input.cookie value:input.value];
			}
			else
			{
				[((AppDelegate *)NSApp.delegate) clearJoypadForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index buttonCode:buttonCode];
			}
		}

		[NSUserDefaults.standardUserDefaults synchronize];
		[self refresh];
	}
}

- (IBAction)playerDropdownChanged:(NSPopUpButton *)sender
{
	[self selectDeviceForPlayer:sender.selectedTag];
	[self refresh];
}

- (IBAction)deviceDropdownChanged:(NSPopUpButton *)sender
{
	if (sender.selectedTag >= 0)
	{
		AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
		S9xJoypad *joypad = sender.selectedItem.representedObject;
		[appDelegate setPlayer:self.playerPopUp.selectedTag forVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
		[NSUserDefaults.standardUserDefaults synchronize];
	}
	
	[self refresh];
}


@end
