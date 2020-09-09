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
  (c) Copyright 2019         Michael Donald Buckley
 ***********************************************************************************/

#import <snes9x_framework/snes9x_framework.h>

#import "AppDelegate.h"
#import "S9xPrefsConstants.h"
#import "S9xPrefsViewController.h"
#import "S9xButtonConfigTextField.h"

@interface S9xPrefsViewController ()
@property (nonatomic, weak) IBOutlet NSPopUpButton *videoModePopup;
@property (nonatomic, weak) IBOutlet NSButton *showFPSCheckbox;
@property (nonatomic, weak) IBOutlet NSPopUpButton *devicePopUp;
@property (nonatomic, weak) IBOutlet NSPopUpButton *playerPopUp;
@end

@implementation S9xPrefsViewController

- (void)awakeFromNib
{
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

    [self selectDeviceForPlayer:0];

    for (NSView *subview in self.view.subviews)
    {
        if ( [subview isKindOfClass:[S9xButtonConfigTextField class]] )
        {
            S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)subview;
            [field addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
            [field addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];
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

- (void)viewDidAppear
{
    [super viewDidAppear];
    [self.view.window makeFirstResponder:self.view];
    [self refresh];
}

- (void)refresh
{
    NSUInteger index = MIN([NSUserDefaults.standardUserDefaults integerForKey:kVideoModePref], 1);
    [self.videoModePopup selectItemAtIndex:index];
    self.showFPSCheckbox.state = [NSUserDefaults.standardUserDefaults boolForKey:kShowFPSPref];

    if (self.devicePopUp.selectedItem.tag < 0)
    {
        NSMutableDictionary<NSNumber *, NSNumber *> *controlsDict = [NSMutableDictionary new];

        NSDictionary *keyboardDict = [NSUserDefaults.standardUserDefaults objectForKey:kKeyboardPrefs];
        NSInteger playerNum = self.playerPopUp.selectedItem.tag;

        for (NSUInteger i = 0; i < kNumButtons; ++i)
        {
            controlsDict[@(i)] = keyboardDict[@(i + (playerNum * kNumButtons)).stringValue];
        }

        for (NSView *subview in self.view.subviews)
        {
            if ([subview isKindOfClass:[S9xButtonConfigTextField class]])
            {
                S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)subview;

                [field removeObserver:self forKeyPath:@"keyCode"];
                [field removeObserver:self forKeyPath:@"joypadInput"];

                NSNumber *keyCode = controlsDict[@(field.tag)];

                field.joypadInput = nil;

                if ( keyCode != nil )
                {
                    field.keyCode = keyCode.intValue;
                }
                else
                {
                    field.keyCode = -1;
                }

                [field addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
                [field addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];

                field.disableKeyboardInput = NO;
            }
        }
    }
    else
    {
        AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
        S9xJoypad *joypad = self.devicePopUp.selectedItem.representedObject;
        NSString *joypadKey = [appDelegate prefsKeyForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
        NSDictionary *joypadDIct = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadInputPrefs] objectForKey:joypadKey];

        for (NSView *subview in self.view.subviews)
        {
            if ([subview isKindOfClass:[S9xButtonConfigTextField class]])
            {
                S9xButtonConfigTextField *textField = (S9xButtonConfigTextField *)subview;

                [textField removeObserver:self forKeyPath:@"keyCode"];
                [textField removeObserver:self forKeyPath:@"joypadInput"];

                uint32 cookie = 0;
                int32 value = 0;
                S9xButtonCode buttonCode = (S9xButtonCode)textField.tag;
                NSString *inputString = joypadDIct[@(buttonCode).stringValue];

                textField.keyCode = -1;

                if ([appDelegate getValuesFromString:inputString cookie:&cookie value:&value])
                {
                    S9xJoypadInput *input = [S9xJoypadInput new];
                    input.cookie = cookie;
                    input.value = value;
                    input.buttonCode = buttonCode;
                    textField.joypadInput = input;
                    textField.stringValue = [appDelegate labelForVendorID:joypad.vendorID productID:joypad.productID cookie:cookie value:value];
                }
                else
                {
                    textField.joypadInput = nil;
                    textField.stringValue = @"";
                }

                [textField addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
                [textField addObserver:self forKeyPath:@"joypadInput" options:NSKeyValueObservingOptionNew context:NULL];

                textField.disableKeyboardInput = YES;
            }
        }
    }
}

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

- (IBAction)showFPS:(NSButton *)sender
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
    [appDelegate setShowFPS:sender.state == NSOnState];
}

- (IBAction)setVideoMode:(NSPopUpButton *)sender
{
    AppDelegate *appDelegate = (AppDelegate *)NSApp.delegate;
    [appDelegate setVideoMode:(int)sender.selectedTag];
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

@end
