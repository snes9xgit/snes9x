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

#import <Carbon/Carbon.h>
#import "AppDelegate.h"
#import "S9xPreferencesConstants.h"

NSWindowFrameAutosaveName const kMainWindowIdentifier = @"s9xMainWindow";
NSWindowFrameAutosaveName const kCheatsWindowIdentifier = @"s9xCheatsWindow";

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.s9xEngine = [S9xEngine new];
    self.s9xEngine.inputDelegate = self;
    [self setupDefaults];
    [self importRecentItems];

    NSWindow *gameWindow = [[NSWindow alloc] initWithContentRect:s9xView.frame styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];

    gameWindow.contentView.wantsLayer = YES;
    gameWindow.contentView.layer.backgroundColor = NSColor.blackColor.CGColor;

    gameWindow.title = @"Snes9x";
    gameWindow.restorationClass = [self class];
    gameWindow.frameAutosaveName = kMainWindowIdentifier;
    gameWindow.releasedWhenClosed = NO;
    gameWindow.backgroundColor = NSColor.clearColor;

    if ( ![gameWindow setFrameUsingName:kMainWindowIdentifier] )
    {
        [gameWindow center];
    }

    self.gameWindow = gameWindow;

    [NSNotificationCenter.defaultCenter addObserverForName:NSWindowWillCloseNotification object:gameWindow queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *notification)
    {
        [self.s9xEngine quit];
    }];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [self.s9xEngine quit];
}

- (void)setupDefaults
{
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;

    NSDictionary *defaultSettings = @{
        kKeyboardPrefs : @{
            @(kUp).stringValue : @(kVK_UpArrow),
            @(kDown).stringValue : @(kVK_DownArrow),
            @(kLeft).stringValue : @(kVK_LeftArrow),
            @(kRight).stringValue : @(kVK_RightArrow),
            @(kY).stringValue : @(kVK_ANSI_X),
            @(kB).stringValue : @(kVK_ANSI_C),
            @(kX).stringValue : @(kVK_ANSI_D),
            @(kA).stringValue : @(kVK_ANSI_V),
            @(kL).stringValue : @(kVK_ANSI_A),
            @(kR).stringValue : @(kVK_ANSI_S),
            @(kStart).stringValue : @(kVK_Space),
            @(kSelect).stringValue : @(kVK_Return),

            @(kNumButtons + kUp).stringValue : @(kVK_ANSI_Keypad8),
            @(kNumButtons + kDown).stringValue : @(kVK_ANSI_Keypad2),
            @(kNumButtons + kLeft).stringValue : @(kVK_ANSI_Keypad4),
            @(kNumButtons + kRight).stringValue : @(kVK_ANSI_Keypad6),
            @(kNumButtons + kY).stringValue : @(kVK_PageDown),
            @(kNumButtons + kB).stringValue : @(kVK_PageUp),
            @(kNumButtons + kX).stringValue : @(kVK_End),
            @(kNumButtons + kA).stringValue : @(kVK_Home),
            @(kNumButtons + kL).stringValue : @(kVK_ANSI_Keypad0),
            @(kNumButtons + kR).stringValue : @(kVK_ANSI_KeypadDecimal),
            @(kNumButtons + kStart).stringValue : @(kVK_ANSI_KeypadEnter),
            @(kNumButtons + kSelect).stringValue : @(kVK_ANSI_KeypadPlus),

            @(kKeyFastForward).stringValue : @(kVK_ANSI_Backslash),
            @(kKeyFreeze).stringValue : @(kVK_ANSI_1),
            @(kKeyDefrost).stringValue : @(kVK_ANSI_0),
            @(kKeyScreenshot).stringValue : @(kVK_ANSI_Grave),
            @(kKeySPC).stringValue : @(kVK_ANSI_R),
            @(kKeyScopeTurbo).stringValue : @(kVK_ANSI_B),
            @(kKeyScopePause).stringValue : @(kVK_ANSI_N),
            @(kKeyScopeCursor).stringValue : @(kVK_ANSI_M),
            @(kKeyOffScreen).stringValue : @(kVK_Tab),
            @(kKeyFunction).stringValue : @(kVK_ANSI_Slash),
            @(kKeyAlt).stringValue : @(kVK_ANSI_Period),
            @(kKeyFFDown).stringValue : @(kVK_ANSI_Q),
            @(kKeyFFUp).stringValue : @(kVK_ANSI_W),
            @(kKeyEsc).stringValue : @(kVK_Escape),
            @(kKeyTC).stringValue : @(kVK_ANSI_Comma)
        },
        kShowFPSPref : @(NO),
        kVideoModePref : @(VIDEOMODE_BLOCKY),
        kMacFrameSkipPref : @(macFrameSkip),

        kSuperFXClockSpeedPercentPref : @(100),
        kSoundInterpolationTypePref: @(2),
        kCPUOverclockPref : @(0),

        kApplyGameSpecificHacksPref : (@YES),
        kAllowInvalidVRAMAccessPref : @(NO),
        kSeparateEchoBufferFromRAMPref : @(NO),
        kDisableSpriteLimitPref : @(NO),
    };

    [defaults registerDefaults:defaultSettings];
	[defaults setBool:NO forKey:@"NSWindowAssertWhenDisplayCycleLimitReached"];
	[defaults synchronize];

    self.keys = [[defaults objectForKey:kKeyboardPrefs] mutableCopy];

    for (NSString *control in [self.keys copy])
    {
        NSInteger key = control.integerValue;
        NSInteger player = key / kNumButtons;
        S9xButtonCode buttonCode = (S9xButtonCode)(key - (kNumButtons * player));
        [self setButtonCode:buttonCode forKeyCode:self.keys[control].integerValue player:player];
    }

	NSDictionary *joypadPlayerPrefs = [defaults objectForKey:kJoypadPlayerPrefs];

    for ( S9xJoypad *joypad in [self listJoypads])
    {
        NSMutableDictionary *joypadPrefs = [[defaults objectForKey:kJoypadInputPrefs] mutableCopy];

        if (joypadPrefs == nil)
        {
            joypadPrefs = [NSMutableDictionary new];
            [defaults synchronize];
        }

        NSString *key = [self prefsKeyForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];

		for ( NSString *playerString in joypadPlayerPrefs )
		{
			if ( [key isEqualToString:joypadPlayerPrefs[playerString]] )
			{
				[self setPlayer:playerString.intValue forVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
			}
		}

        NSMutableDictionary *devicePrefs = [joypadPrefs[key] mutableCopy];
        if (devicePrefs == nil)
        {
            devicePrefs = [NSMutableDictionary new];
            for (S9xJoypadInput *input in [self.s9xEngine getInputsForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index])
            {
                devicePrefs[@(input.buttonCode).stringValue] = [self prefValueForCookie:input.cookie value:input.value];
            }

            joypadPrefs[key] = devicePrefs;
            [defaults setObject:joypadPrefs forKey:kJoypadInputPrefs];
            [defaults synchronize];
        }
        else
        {
            [self.s9xEngine clearJoypadForVendorID:joypad.vendorID productID:joypad.productID index:joypad.index];
            for (NSString *buttonCodeString in devicePrefs)
            {
                S9xButtonCode buttonCode = (S9xButtonCode)buttonCodeString.intValue;
                NSString *str = devicePrefs[buttonCodeString];
                uint32 cookie = 0;
                int32 value = -1;

                if ([self getValuesFromString:str cookie:&cookie value:&value])
                {
                    [self setButton:buttonCode forVendorID:joypad.vendorID productID:joypad.productID index:joypad.index cookie:cookie value:value];
                }
            }
        }
    }

	self.deviceSetting = Gamepads;

    [self importKeySettings];
    [self importGraphicsSettings];
    [self applyEmulationSettings];

    self.s9xEngine.cheatsEnabled = [defaults boolForKey:kEnableCheatsPref];

    [defaults synchronize];
}

- (void)setButtonCode:(S9xButtonCode)buttonCode forKeyCode:(int16)keyCode player:(int8)player
{
    if (keyCode < 0)
    {
        return;
    }

    self.keys[@(buttonCode + (kNumButtons * player)).stringValue] = @(keyCode);

    S9xButtonCode oldButton = kNumButtons;
    int8 oldPlayer = -1;
    if ([self.s9xEngine setButton:buttonCode forKey:keyCode player:player oldButton:&oldButton oldPlayer:&oldPlayer oldKey:NULL])
    {
        if (oldButton >= 0 && oldButton < kNumButtons && oldPlayer >= 0 && oldPlayer < MAC_MAX_PLAYERS && (oldPlayer != player || oldButton != buttonCode))
        {
            [self.keys removeObjectForKey:@(oldButton + (kNumButtons * oldPlayer)).stringValue];
        }

        [NSUserDefaults.standardUserDefaults setObject:[self.keys copy] forKey:kKeyboardPrefs];
    }
}

- (void)clearButton:(S9xButtonCode)button forPlayer:(int8)player
{
    [self.s9xEngine clearButton:button forPlayer:player];
    NSMutableDictionary *keyDict = [[NSUserDefaults.standardUserDefaults objectForKey:kKeyboardPrefs] mutableCopy];
    [keyDict removeObjectForKey:@(button + (kNumButtons * player)).stringValue];
    [NSUserDefaults.standardUserDefaults setObject:[keyDict copy] forKey:kKeyboardPrefs];
    [NSUserDefaults.standardUserDefaults synchronize];
}

- (NSArray<S9xJoypad *> *)listJoypads
{
    return [self.s9xEngine listJoypads];
}

- (NSString *)prefsKeyForVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index
{
    return [NSString stringWithFormat:@"%@:%@:%@", @(vendorID).stringValue, @(productID).stringValue, @(index).stringValue];
}

- (BOOL)getValuesFromString:(NSString *)str vendorID:(uint32 *)vendorID productID:(uint32 *)productID index:(uint32 *)index
{
    if (vendorID == NULL || productID == NULL || index == NULL)
    {
        return NO;

    }

    NSArray<NSString *> *components = [str componentsSeparatedByString:@":"];

    if (components.count != 3)
    {
        return NO;
    }

    *vendorID = components[0].intValue;
    *productID = components[1].intValue;
    *index = components[2].intValue;

    return YES;
}

- (NSString *)prefValueForCookie:(uint32)cookie value:(int32)value
{
    return [NSString stringWithFormat:@"%@:%@", @(cookie).stringValue, @(value).stringValue];
}

- (BOOL)getValuesFromString:(NSString *)str cookie:(uint32 *)cookie value:(int32 *)value
{
    if (cookie == NULL || value == NULL)
    {
        return NO;
    }

    NSArray<NSString *> *components = [str componentsSeparatedByString:@":"];

    if (components.count != 2)
    {
        return NO;
    }

    *cookie = components.firstObject.intValue;
    *value = components.lastObject.intValue;

    return YES;
}

- (void)setPlayer:(int8)player forVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index
{
    int8 oldPlayer = -1;
    [self.s9xEngine setPlayer:player forVendorID:vendorID productID:productID index:index oldPlayer:&oldPlayer];

    NSMutableDictionary *playersDict = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadPlayerPrefs] mutableCopy];
    if (playersDict == nil)
    {
        playersDict = [NSMutableDictionary new];
    }

    if (oldPlayer >= 0 && player != oldPlayer)
    {
        [playersDict removeObjectForKey:@(oldPlayer).stringValue];
    }

    playersDict[@(player).stringValue] = [self prefsKeyForVendorID:vendorID productID:productID index:index];

    [NSUserDefaults.standardUserDefaults setObject:[playersDict copy] forKey:kJoypadPlayerPrefs];
}

- (BOOL)setButton:(S9xButtonCode)button forVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index cookie:(uint32)cookie value:(int32)value
{
    S9xButtonCode oldButton = (S9xButtonCode)-1;
    BOOL result = [self.s9xEngine setButton:button forVendorID:vendorID productID:productID index:index cookie:cookie value:value oldButton:&oldButton];

    NSMutableDictionary *prefsDict = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadInputPrefs] mutableCopy];
    NSString *key = [self prefsKeyForVendorID:vendorID productID:productID index:index];
    NSMutableDictionary *joypadDict = [prefsDict[key] mutableCopy];

    if (result && button != oldButton)
    {
        [joypadDict removeObjectForKey:@(oldButton).stringValue];
    }

    joypadDict[@(button).stringValue] = [self prefValueForCookie:cookie value:value];

    prefsDict[key] = [joypadDict copy];

    [NSUserDefaults.standardUserDefaults setObject:[prefsDict copy] forKey:kJoypadInputPrefs];

    return result;
}

- (void)clearJoypadForVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index buttonCode:(S9xButtonCode)buttonCode
{
    [self.s9xEngine clearJoypadForVendorID:vendorID productID:productID index:index buttonCode:buttonCode];
    NSString *key = [self prefsKeyForVendorID:vendorID productID:productID index:index];
    NSMutableDictionary *joypadsDict = [[NSUserDefaults.standardUserDefaults objectForKey:kJoypadInputPrefs] mutableCopy];
    NSMutableDictionary *deviceDict = [joypadsDict[key] mutableCopy];
    [deviceDict removeObjectForKey:@(buttonCode).stringValue];
    joypadsDict[key] = deviceDict;
    [NSUserDefaults.standardUserDefaults setObject:[joypadsDict copy] forKey:kJoypadInputPrefs];
    [NSUserDefaults.standardUserDefaults synchronize];
}

- (NSString *)labelForVendorID:(uint32)vendorID productID:(uint32)productID cookie:(uint32)cookie value:(int32)value
{
    return [self.s9xEngine labelForVendorID:vendorID productID:productID cookie:cookie value:value];
}

- (void)importRecentItems
{
    const NSInteger maxRecents = 20;

    for (NSInteger i = maxRecents - 1; i >= 0; --i)
    {
        NSString *key = [NSString stringWithFormat:@"RecentItem_%02tu", i];
        NSString *recentItem = [NSUserDefaults.standardUserDefaults objectForKey:key];

        if (recentItem != nil)
        {
            [NSDocumentController.sharedDocumentController noteNewRecentDocumentURL:[NSURL fileURLWithPath:recentItem]];
            [NSUserDefaults.standardUserDefaults removeObjectForKey:key];
        }
    }

    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)importKeySettings
{
    NSData *data = [self objectForPrefOSCode:'keyb'];

    NSUInteger length = data.length;
    char *bytes = (char*)data.bytes;
    for ( NSUInteger i = 0; i < length; ++i )
    {
        // The enum values for controls changed between the Carbon and Cocoa versions.
        // The first 24 enum values are the same, but we have to adjust after that.
        if ( i < 24 )
        {
            [self setButtonCode:(S9xButtonCode)(i - (i / 12)) forKeyCode:bytes[i] player:i / 12];
        }
        else
        {
            [self setButtonCode:(S9xButtonCode)(i - 24 + kKeyFastForward) forKeyCode:bytes[i] player:0];
        }
    }
}

- (void)importGraphicsSettings
{
    NSData *data = [self objectForPrefOSCode:'dfps'];

    if (data != nil)
    {
        [NSUserDefaults.standardUserDefaults setBool:(data.length > 0 && ((char *)data.bytes)[0]) forKey:kShowFPSPref];
    }

    [self setShowFPS:[NSUserDefaults.standardUserDefaults boolForKey:kShowFPSPref]];

    data = [self objectForPrefOSCode:'Vmod'];

    if ( data != nil)
    {
        [NSUserDefaults.standardUserDefaults setInteger:((data.length >= 0 && ((char *)data.bytes)[0]) ? VIDEOMODE_SMOOTH : VIDEOMODE_BLOCKY) forKey:kVideoModePref];
    }

    [self setVideoMode:(int)[NSUserDefaults.standardUserDefaults integerForKey:kVideoModePref]];
}

- (id)objectForPrefOSCode:(uint32_t)osCode
{
    NSString *key = [@"Preferences_" stringByAppendingString:[[NSString alloc] initWithBytes:(char *)&osCode length:sizeof(uint32_t) encoding:NSASCIIStringEncoding]];

    id obj = [NSUserDefaults.standardUserDefaults objectForKey:key];

    if (obj == nil)
    {
        osCode =CFSwapInt32(osCode);
        key = [@"Preferences_" stringByAppendingString:[[NSString alloc] initWithBytes:(char *)&osCode length:sizeof(uint32_t) encoding:NSASCIIStringEncoding]];

        obj = [NSUserDefaults.standardUserDefaults objectForKey:key];

        if (obj != nil)
        {
            [NSUserDefaults.standardUserDefaults removeObjectForKey:key];
        }
    }
    else
    {
        [NSUserDefaults.standardUserDefaults removeObjectForKey:key];
    }

    return obj;
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
    return [self openURL:[NSURL fileURLWithPath:filename]];
}

- (IBAction)openDocument:(id)sender
{
    NSOpenPanel* panel = [NSOpenPanel new];
    NSModalResponse response = [panel runModal];

    if ( response == NSModalResponseOK )
    {
        [self openURL:panel.URL];
    }
}

- (BOOL)openURL:(NSURL *)url
{
    if ([self.s9xEngine loadROM:url])
    {
		[self.s9xEngine recreateS9xView];
		
		NSWindow *gameWindow = self.gameWindow;
		[gameWindow.contentView addSubview:s9xView];
		[s9xView.topAnchor constraintEqualToAnchor:gameWindow.contentView.topAnchor].active = YES;
		[s9xView.bottomAnchor constraintEqualToAnchor:gameWindow.contentView.bottomAnchor].active = YES;
		[s9xView.centerXAnchor constraintEqualToAnchor:gameWindow.contentView.centerXAnchor].active = YES;
		[s9xView.leftAnchor constraintGreaterThanOrEqualToAnchor:gameWindow.contentView.leftAnchor].active = YES;
		[s9xView.rightAnchor constraintLessThanOrEqualToAnchor:gameWindow.contentView.rightAnchor].active = YES;

        if (self.cheatsWindowController != nil)
        {
            [((S9xCheatsViewController *)self.cheatsWindowController.contentViewController) deselectAll];
            [((S9xCheatsViewController *)self.cheatsWindowController.contentViewController) reloadData];
        }
		
        [gameWindow makeKeyAndOrderFront:self];
        [NSDocumentController.sharedDocumentController noteNewRecentDocumentURL:url];
        return YES;
    }

    return NO;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
	SEL action = menuItem.action;
	if (action == @selector(resume:) || action == @selector(softwareReset:) || action == @selector(hardwareReset:)) {
		return [self.s9xEngine isRunning] && [self.s9xEngine isPaused];
	}
	else if (action == @selector(updateDeviceSetting:)) {
		menuItem.state = (self.deviceSetting == (S9xDeviceSetting)menuItem.tag) ? NSOnState : NSOffState;
	}
    else if (action == @selector(toggleCheats:))
    {
        if (self.s9xEngine.cheatsEnabled)
        {
            menuItem.title = NSLocalizedString(@"Disable Cheats", nil);
        }
        else
        {
            menuItem.title = NSLocalizedString(@"Enable Cheats", nil);
        }
    }

    return !self.isRunningEmulation;
}

@dynamic isRunningEmulation;
- (BOOL)isRunningEmulation
{
    return [self.s9xEngine isRunning] && ![self.s9xEngine isPaused];
}

- (IBAction)terminate:(id)sender
{
    [self.s9xEngine stop];
    [NSApp terminate:sender];
}

- (IBAction)openPreferencesWindow:(id)sender
{
    if ( self.preferencesWindowController == nil )
    {
		self.preferencesWindowController = [[S9xPreferencesWindowController alloc] initWithWindowNibName:@"S9xPreferencesWindowController"];
	}
	
	[self.preferencesWindowController showWindow:nil];
	[self.preferencesWindowController.window makeKeyAndOrderFront:nil];
	[self.preferencesWindowController.window makeKeyWindow];
}

- (void)setVideoMode:(int)videoMode
{
    [self.s9xEngine setVideoMode:videoMode];
    [NSUserDefaults.standardUserDefaults setObject:@(videoMode) forKey:kVideoModePref];
    [NSUserDefaults.standardUserDefaults synchronize];
}

- (void)setMacFrameSkip:(int)_macFrameSkip
{
    [self.s9xEngine setMacFrameSkip:_macFrameSkip];
    [NSUserDefaults.standardUserDefaults setObject:@(_macFrameSkip) forKey:kMacFrameSkipPref];
    [NSUserDefaults.standardUserDefaults synchronize];
}

- (void)setShowFPS:(BOOL)showFPS
{
    [self.s9xEngine setShowFPS:showFPS];
    [NSUserDefaults.standardUserDefaults setObject:@(showFPS) forKey:kShowFPSPref];
    [NSUserDefaults.standardUserDefaults synchronize];
}

- (void)applyEmulationSettings
{
    S9xEngine *engine = self.s9xEngine;
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;

    [engine setSuperFXClockSpeedPercent:(uint32_t)[defaults integerForKey:kSuperFXClockSpeedPercentPref]];
    [engine setSoundInterpolationType:(int)[defaults integerForKey:kSoundInterpolationTypePref]];
    [engine setCPUOverclockMode:(int)[defaults integerForKey:kCPUOverclockPref]];

    [engine setApplySpecificGameHacks:[defaults boolForKey:kApplyGameSpecificHacksPref]];
    [engine setAllowInvalidVRAMAccess:[defaults boolForKey:kAllowInvalidVRAMAccessPref]];
    [engine setSeparateEchoBufferFromRAM:[defaults boolForKey:kSeparateEchoBufferFromRAMPref]];
    [engine setDisableSpriteLimit:[defaults boolForKey:kDisableSpriteLimitPref]];
}

- (IBAction)resume:(id)sender
{
    [self.s9xEngine resume];
}

- (IBAction)softwareReset:(id)sender
{
	[self.s9xEngine softwareReset];
}

- (IBAction)hardwareReset:(id)sender
{
	[self.s9xEngine hardwareReset];
}

- (IBAction)updateDeviceSetting:(id)sender
{
	self.deviceSetting = (S9xDeviceSetting)[sender tag];
}

- (void)setDeviceSetting:(S9xDeviceSetting)deviceSetting
{
	_deviceSetting = deviceSetting;
	[self.s9xEngine setDeviceSetting:deviceSetting];
}

- (BOOL)handleInput:(S9xJoypadInput *)input fromJoypad:(S9xJoypad *)joypad
{
    if (NSApp.keyWindow != nil && NSApp.keyWindow == self.preferencesWindowController.window)
    {
        return [((S9xPreferencesWindowController *) self.preferencesWindowController) handleInput:input fromJoypad:joypad];
    }

    return NO;
}

- (void)deviceSettingChanged:(S9xDeviceSetting)deviceSetting
{
    _deviceSetting = deviceSetting;
}


- (IBAction)openCheatsWindow:(id)sender
{
    if (self.cheatsWindowController == nil)
    {
        NSWindow *window = [NSWindow windowWithContentViewController:[[S9xCheatsViewController alloc] initWithNibName:@"S9xCheatsViewController" bundle:nil]];
        self.cheatsWindowController = [[NSWindowController alloc] initWithWindow:window];

        window = self.cheatsWindowController.window;

        window.title = NSLocalizedString(@"Cheats", nil);
        window.restorationClass = self.class;
        window.frameAutosaveName = kCheatsWindowIdentifier;
        window.releasedWhenClosed = NO;

        if ( ![window setFrameUsingName:kCheatsWindowIdentifier] )
        {
            [window center];
        }
    }

    [self.cheatsWindowController showWindow:nil];
    [self.cheatsWindowController.window makeKeyAndOrderFront:nil];
    [self.cheatsWindowController.window makeKeyWindow];
}

- (IBAction)toggleCheats:(id)sender
{
    self.s9xEngine.cheatsEnabled = !self.s9xEngine.cheatsEnabled;
    [NSUserDefaults.standardUserDefaults setBool:self.s9xEngine.cheatsEnabled forKey:kEnableCheatsPref];
}

@end
