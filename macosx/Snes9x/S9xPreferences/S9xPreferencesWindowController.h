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
#import <Cocoa/Cocoa.h>
#import "S9xButtonConfigTextField.h"

NS_ASSUME_NONNULL_BEGIN

static NSString * const kKeyboardPrefs = @"KeyboardConfig";
static NSString * const kJoypadInputPrefs = @"JoypadInputs";
static NSString * const kJoypadPlayerPrefs = @"JoypadPlayers";
static NSString * const kShowFPSPref = @"ShowFPS";
static NSString * const kVideoModePref = @"VideoMode";
static NSString * const kMacFrameSkipPref = @"FrameSkip";

@interface S9xPreferencesWindowController : NSWindowController<S9xInputDelegate>
@property (nonatomic, weak) IBOutlet NSTabView *tabView;
@property (nonatomic, weak) IBOutlet NSPopUpButton *videoModePopup;
@property (nonatomic, weak) IBOutlet NSButton *showFPSCheckbox;
@property (nonatomic, weak) IBOutlet NSPopUpButton *devicePopUp;
@property (nonatomic, weak) IBOutlet NSPopUpButton *playerPopUp;
@property (nonatomic, weak) IBOutlet NSTextField *macFrameSkipTextField;
@property (nonatomic, weak) IBOutlet NSStepper *macFrameSkipStepper;
@property (nonatomic, weak) IBOutlet NSButton *macFrameSkipAutomaticButton;
@property (nonatomic, strong) NSArray *configTextFields;
@end

NS_ASSUME_NONNULL_END
