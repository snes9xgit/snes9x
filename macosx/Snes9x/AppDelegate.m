//
//  AppDelegate.m
//  Snes9x
//
//  Created by Buckley on 8/21/19.
//

#import "AppDelegate.h"
#import <snes9x_framework/snes9x_framework.h>

@interface AppDelegate ()

@property (strong) S9xEngine *s9xEngine;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    self.s9xEngine = [S9xEngine new];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (IBAction)openDocument:(id)sender
{
    NSOpenPanel* panel = [NSOpenPanel new];
    NSModalResponse response = [panel runModal];

    if ( response == NSModalResponseOK )
    {
        [self.s9xEngine loadROM:panel.URL];
    }
}

@end
