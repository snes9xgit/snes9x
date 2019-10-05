//
//  AppDelegate.m
//  Snes9x
//
//  Created by Buckley on 8/21/19.
//

#import "AppDelegate.h"

#import <Carbon/Carbon.h>
#import <snes9x_framework/snes9x_framework.h>

@interface AppDelegate ()
@property (nonatomic, strong) S9xEngine *s9xEngine;
@property (nonatomic, strong) NSMutableDictionary<NSString *, NSNumber *> *keys;
@property (nonatomic, strong) NSWindow *window;
@end

static NSWindowFrameAutosaveName const kMainWindowIdentifier = @"s9xMainWindow";
static NSString * const kKeyboardPrefs = @"KeyboardConfig";

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

    NSDictionary *defaultKeyBindings = @{
        kKeyboardPrefs : @{
            @(k1PUp).stringValue : @(kVK_UpArrow),
            @(k1PDown).stringValue : @(kVK_DownArrow),
            @(k1PLeft).stringValue : @(kVK_LeftArrow),
            @(k1PRight).stringValue : @(kVK_RightArrow),
            @(k1PY).stringValue : @(kVK_ANSI_X),
            @(k1PB).stringValue : @(kVK_ANSI_C),
            @(k1PX).stringValue : @(kVK_ANSI_D),
            @(k1PA).stringValue : @(kVK_ANSI_V),
            @(k1PL).stringValue : @(kVK_ANSI_A),
            @(k1PR).stringValue : @(kVK_ANSI_S),
            @(k1PStart).stringValue : @(kVK_Space),
            @(k1PSelect).stringValue : @(kVK_Return),

            @(k2PUp).stringValue : @(kVK_ANSI_Keypad8),
            @(k2PDown).stringValue : @(kVK_ANSI_Keypad2),
            @(k2PLeft).stringValue : @(kVK_ANSI_Keypad4),
            @(k2PRight).stringValue : @(kVK_ANSI_Keypad6),
            @(k2PY).stringValue : @(kVK_PageDown),
            @(k2PB).stringValue : @(kVK_PageUp),
            @(k2PX).stringValue : @(kVK_End),
            @(k2PA).stringValue : @(kVK_Home),
            @(k2PL).stringValue : @(kVK_ANSI_Keypad0),
            @(k2PR).stringValue : @(kVK_ANSI_KeypadDecimal),
            @(k2PStart).stringValue : @(kVK_ANSI_KeypadEnter),
            @(k2PSelect).stringValue : @(kVK_ANSI_KeypadPlus),

            @(k1PKeyFastForward).stringValue : @(kVK_ANSI_Backslash),
            @(k1PKeyFreeze).stringValue : @(kVK_ANSI_1),
            @(k1PKeyDefrost).stringValue : @(kVK_ANSI_0),
            @(k1PKeyScreenshot).stringValue : @(kVK_ANSI_Grave),
            @(k1PKeySPC).stringValue : @(kVK_ANSI_R),
            @(k1PKeyScopeTurbo).stringValue : @(kVK_ANSI_B),
            @(k1PKeyScopePause).stringValue : @(kVK_ANSI_N),
            @(k1PKeyScopeCursor).stringValue : @(kVK_ANSI_Q),
            @(k1PKeyOffScreen).stringValue : @(kVK_Tab),
            @(k1PKeyFunction).stringValue : @(kVK_ANSI_Slash),
            @(k1PKeyAlt).stringValue : @(kVK_ANSI_Period),
            @(k1PKeyFFDown).stringValue : @(kVK_ANSI_Q),
            @(k1PKeyFFUp).stringValue : @(kVK_ANSI_W),
            @(k1PKeyEsc).stringValue : @(kVK_Escape),
            @(k1PKeyTC).stringValue : @(kVK_ANSI_Comma)
        }
    };

    [defaults registerDefaults:defaultKeyBindings];

    self.keys = [[defaults objectForKey:kKeyboardPrefs] mutableCopy];

    for (NSString *control in [self.keys copy])
    {
        [self setControl:@(control.integerValue) forKey:self.keys[control]];
    }

    [self importKeySettings];
    [defaults synchronize];
}

- (void)setControl:(NSNumber *)control forKey:(NSNumber *)key
{
    if (key == nil || control == nil)
    {
        return;
    }

    self.keys[control.stringValue] = key;

    S9xKey oldControl = kNumButtons;
    [self.s9xEngine setControl:(S9xKey)control.intValue forKey:key.intValue oldControl:&oldControl oldKey:NULL];

    if (oldControl >= 0 && oldControl < kNumButtons)
    {
        [self.keys removeObjectForKey:@(oldControl).stringValue];
    }

    [NSUserDefaults.standardUserDefaults setObject:[self.keys copy] forKey:kKeyboardPrefs];
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
    NSData *data = [NSUserDefaults.standardUserDefaults objectForKey:@"Preferences_byek"];

    if (data == nil)
    {
        data = [NSUserDefaults.standardUserDefaults objectForKey:@"Preferences_keyb"];

        if (data != nil)
        {
            [NSUserDefaults.standardUserDefaults removeObjectForKey:@"Preferences_keyb"];
        }
    }
    else
    {
        [NSUserDefaults.standardUserDefaults removeObjectForKey:@"Preferences_byek"];
    }

    NSUInteger length = data.length;
    char *bytes = (char*)data.bytes;
    for ( NSUInteger i = 0; i < length; ++i )
    {
        // The enum values for controls changed between the Carbon and Cocoa versions.
        // The first 24 enum values are the same, but we have to adjust after that.
        if ( i < 24 )
        {
            [self setControl:@(i) forKey:@(bytes[i])];
        }
        else
        {
            [self setControl:@(i - 24 + k1PKeyFastForward) forKey:@(bytes[i])];
        }
    }
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

@end
