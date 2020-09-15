
#import <snes9x_framework/snes9x_framework.h>

#import "AppDelegate.h"
#import "S9xLuaViewController.h"
#import "lua-engine.h"

#import <map>

@interface S9xLuaViewController ()
@property (nonatomic, weak) IBOutlet NSTextField *luaScriptPath;
@property (nonatomic, weak) IBOutlet NSTextField *luaScriptConsole;
@property (nonatomic, weak) IBOutlet NSButton *luaBrowseButton;
@property (nonatomic, weak) IBOutlet NSButton *luaStopButton;
@property (nonatomic, weak) IBOutlet NSButton *luaRunButton;
@end

@implementation S9xLuaViewController
{
    int uniqueID;
}

static int s_UniqueIDs = 0;

typedef std::map<int, S9xLuaViewController*> LuaViewControllerMap_t;
static LuaViewControllerMap_t s_LuaViewControllers;

- (void)awakeFromNib
{
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    [self.luaScriptConsole setEditable:false];
    [self.luaStopButton setEnabled:false];
    [self.luaBrowseButton setEnabled:true];

    self->uniqueID = s_UniqueIDs;
    s_LuaViewControllers.insert(std::make_pair(self->uniqueID, self));

    ++s_UniqueIDs;

    OpenLuaContext(self->uniqueID, PrintToWindowConsole, OnStart, OnStop);
}

- (void)viewWillDisappear
{
    StopLuaScript(self->uniqueID);
    CloseLuaContext(self->uniqueID);

    s_LuaViewControllers.erase(self->uniqueID);
}

- (void)refresh
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *luaScriptPath = [self.luaScriptPath stringValue];
    NSString *invalidPath = @"cannot open : No such file or directory";

    if ([fileManager fileExistsAtPath:luaScriptPath])
    {
        NSError* error = nil;
        NSString *luaScriptContent = [NSString stringWithContentsOfFile:luaScriptPath encoding:NSUTF8StringEncoding error:&error];

        if(!error)
        {
            [self.luaScriptConsole setStringValue:luaScriptContent];
        }
    }
}

- (IBAction)browseLuaScript:(NSButton *)sender
{
    NSOpenPanel* panel = [NSOpenPanel new];
    NSModalResponse response = [panel runModal];

    if ( response == NSModalResponseOK )
    {
        [self.luaScriptPath setStringValue:panel.URL.path];
        [self refresh];
    }
}

- (IBAction)editLuaScript:(NSButton *)sender
{
    [NSWorkspace.sharedWorkspace openFile:[self.luaScriptPath stringValue] withApplication:@"TextEdit"];
}

- (IBAction)closeLuaScript:(NSButton *)sender
{
    StopLuaScript(self->uniqueID);
}

- (IBAction)runLuaScript:(NSButton *)sender
{
    NSString *invalidPath = @"cannot open : No such file or directory";

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *luaScriptPath = [self.luaScriptPath stringValue];
    if ([fileManager fileExistsAtPath:luaScriptPath])
    {
        RunLuaScriptFile(self->uniqueID, [luaScriptPath UTF8String]);
    }
    else
    {
        [self.luaScriptConsole setStringValue:invalidPath];
    }
}

static void PrintToWindowConsole(int uniqueID, const char* str)
{
    S9xLuaViewController *luaController = s_LuaViewControllers[uniqueID];

    dispatch_async(dispatch_get_main_queue(), ^{
        NSString *errorMsg = [[NSString alloc] initWithCString:str encoding:NSUTF8StringEncoding];
        NSString *contentString = [luaController.luaScriptConsole stringValue];
        NSArray *contentArray = [[NSArray alloc] initWithObjects:contentString, errorMsg, nil];
        NSString *newContentString = [contentArray componentsJoinedByString:@""];
        [luaController.luaScriptConsole setStringValue:newContentString];
    });
}


static void OnStart(int uniqueID)
{
    S9xLuaViewController *luaController = s_LuaViewControllers[uniqueID];

    dispatch_async(dispatch_get_main_queue(), ^{
        [luaController.luaBrowseButton setEnabled:false];
        [luaController.luaStopButton setEnabled:true];
        [luaController.luaRunButton setTitle:@"Restart"];
        [luaController.luaScriptConsole setStringValue:@""];
    });
}

static void OnStop(int uniqueID, bool statusOK)
{
    S9xLuaViewController *luaController = s_LuaViewControllers[uniqueID];

    dispatch_async(dispatch_get_main_queue(), ^{
        [luaController.luaBrowseButton setEnabled:true];
        [luaController.luaStopButton setEnabled:false];
        [luaController.luaRunButton setTitle:@"Run"];
    });
}

@end
