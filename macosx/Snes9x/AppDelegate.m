//
//  AppDelegate.m
//  Snes9x
//
//  Created by Buckley on 8/21/19.
//

#import "AppDelegate.h"
#import <snes9x_framework/snes9x_framework.h>

@interface AppDelegate ()
@property (nonatomic, strong) S9xEngine *s9xEngine;
@property (nonatomic, strong) NSWindow *window;
@end

static NSWindowFrameAutosaveName const kMainWindowIdentifier = @"s9xMainWindow";

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.s9xEngine = [S9xEngine new];
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
