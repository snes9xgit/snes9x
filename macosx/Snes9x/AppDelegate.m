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

#import <Carbon/Carbon.h>
#import <snes9x_framework/snes9x_framework.h>

#import "AppDelegate.h"

#import "S9xPrefsConstants.h"
#import "S9xPrefsViewController.h"

@interface AppDelegate ()
@property (nonatomic, strong) S9xEngine *s9xEngine;
@property (nonatomic, strong) NSMutableDictionary<NSString *, NSNumber *> *keys;
@property (nonatomic, strong) NSWindow *window;
@property (nonatomic, strong, nullable) NSWindowController *prefsWindowController;
@end

static NSWindowFrameAutosaveName const kMainWindowIdentifier = @"s9xMainWindow";

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.s9xEngine = [S9xEngine new];
    [self setupKeyboard];
    [self importRecentItems];

    NSWindow *window = [[NSWindow alloc] initWithContentRect:s9xView.frame styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable backing:NSBackingStoreBuffered defer:NO];

    window.contentView.wantsLayer = YES;
    window.contentView.layer.backgroundColor = NSColor.blackColor.CGColor;

    window.title = @"Snes9x";
    window.restorationClass = [self class];
    window.frameAutosaveName = kMainWindowIdentifier;
    window.releasedWhenClosed = NO;

    [window.contentView addSubview:s9xView];
    [s9xView.topAnchor constraintEqualToAnchor:window.contentView.topAnchor].active = YES;
    [s9xView.bottomAnchor constraintEqualToAnchor:window.contentView.bottomAnchor].active = YES;
    [s9xView.centerXAnchor constraintEqualToAnchor:window.contentView.centerXAnchor].active = YES;
    [s9xView.leftAnchor constraintGreaterThanOrEqualToAnchor:window.contentView.leftAnchor].active = YES;
    [s9xView.rightAnchor constraintLessThanOrEqualToAnchor:window.contentView.rightAnchor].active = YES;

    if ( ![window setFrameUsingName:kMainWindowIdentifier] )
    {
        [window center];
    }

    [NSNotificationCenter.defaultCenter addObserverForName:NSWindowWillCloseNotification object:window queue:NSOperationQueue.mainQueue usingBlock:^(NSNotification *notification)
    {
        [self.s9xEngine stop];
    }];

    self.window = window;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (void)setupKeyboard
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
        kShowFPSPref: @(NO),
        kVideoModePref:@(VIDEOMODE_BLOCKY)
    };

    [defaults registerDefaults:defaultSettings];

    self.keys = [[defaults objectForKey:kKeyboardPrefs] mutableCopy];

    for (NSString *control in [self.keys copy])
    {
        NSInteger key = control.integerValue;
        NSInteger player = key / kNumButtons;
        S9xButtonCode buttonCode = (S9xButtonCode)(key - (kNumButtons * player));
        [self setButtonCode:buttonCode forKeyCode:self.keys[control].integerValue player:player];
    }

    [self importKeySettings];
    [self importGraphicsSettings];
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

    data = [self objectForPrefOSCode:'Vmod'];

    if ( data != nil)
    {
        [NSUserDefaults.standardUserDefaults setInteger:((data.length >= 0 && ((char *)data.bytes)[0]) ? VIDEOMODE_SMOOTH : VIDEOMODE_BLOCKY) forKey:kVideoModePref];
    }
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

        [self.window makeKeyAndOrderFront:self];
        [NSDocumentController.sharedDocumentController noteNewRecentDocumentURL:url];
        return YES;
    }

    return NO;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return [self.s9xEngine isPaused] || ![self.s9xEngine isRunning];
}

- (IBAction)terminate:(id)sender
{
    [self.s9xEngine stop];
    [NSApp terminate:sender];
}

- (IBAction)openPrefs:(id)sender
{
    if ( self.prefsWindowController == nil )
    {
        NSWindow *prefsWindow = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 100, 100) styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];
        self.prefsWindowController = [[NSWindowController alloc] initWithWindow:prefsWindow];

        prefsWindow.contentViewController = [[S9xPrefsViewController alloc] initWithNibName:@"S9xPrefsViewController" bundle:nil];
        [prefsWindow center];
    }

    [self.prefsWindowController.window makeKeyAndOrderFront:self];
}

@end
