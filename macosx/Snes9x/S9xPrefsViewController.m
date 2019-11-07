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
    for (NSView *subview in self.view.subviews)
    {
        if ( [subview isKindOfClass:[S9xButtonConfigTextField class]] )
        {
            S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)subview;
            [field addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
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

    NSMutableDictionary<NSNumber *, NSNumber *> *controlsDict = [NSMutableDictionary new];
    if (self.devicePopUp.selectedItem.tag < 0)
    {
        NSDictionary *keyboardDict = [NSUserDefaults.standardUserDefaults objectForKey:kKeyboardPrefs];
        NSInteger playerNum = self.playerPopUp.selectedItem.tag;

        for (NSUInteger i = 0; i < kNumButtons; ++i)
        {
            controlsDict[@(i)] = keyboardDict[@(i + playerNum).stringValue];
        }
    }

    for (NSView *subview in self.view.subviews)
    {
        if ( [subview isKindOfClass:[S9xButtonConfigTextField class]] )
        {
            S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)subview;

            [field removeObserver:self forKeyPath:@"keyCode"];

            NSNumber *keyCode = controlsDict[@(field.tag)];

            if ( keyCode != nil )
            {
                field.keyCode = keyCode.intValue;
            }
            else
            {
                field.stringValue = @"";
            }

            [field addObserver:self forKeyPath:@"keyCode" options:NSKeyValueObservingOptionNew context:NULL];
        }
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ( [keyPath isEqualToString:@"keyCode"] )
    {
        S9xButtonConfigTextField *field = (S9xButtonConfigTextField *)object;
        S9xButtonCode buttonCode = (S9xButtonCode)field.tag;
        uint16_t keyCode = field.keyCode;
        int8_t player = self.playerPopUp.selectedItem.tag;

        [((AppDelegate *) NSApp.delegate) setButtonCode:buttonCode forKeyCode:keyCode player:player];
        [NSUserDefaults.standardUserDefaults synchronize];

        [self refresh];
    }
}

@end
