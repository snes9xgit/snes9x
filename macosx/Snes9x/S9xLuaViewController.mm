
#ifdef HAVE_LUA

#import <snes9x_framework/snes9x_framework.h>

#import "AppDelegate.h"
#import "S9xLuaViewController.h"
#import "lua-engine.h"

#import <map>
#import <pthread.h>

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

    pthread_mutex_lock(&mainLoopLock);
    OpenLuaContext(self->uniqueID, PrintToWindowConsole, OnStart, OnStop);
    pthread_mutex_unlock(&mainLoopLock);
}

- (void)viewWillDisappear
{
    pthread_mutex_lock(&mainLoopLock);
    StopLuaScript(self->uniqueID);
    CloseLuaContext(self->uniqueID);
    pthread_mutex_unlock(&mainLoopLock);

    s_LuaViewControllers.erase(self->uniqueID);
}

- (void)refresh
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *luaScriptPath = [self.luaScriptPath stringValue];

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
    pthread_mutex_lock(&mainLoopLock);
    StopLuaScript(self->uniqueID);
    pthread_mutex_unlock(&mainLoopLock);
}

- (IBAction)runLuaScript:(NSButton *)sender
{
    NSString *invalidPath = @"cannot open : No such file or directory";

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *luaScriptPath = [self.luaScriptPath stringValue];
    if ([fileManager fileExistsAtPath:luaScriptPath])
    {
        pthread_mutex_lock(&mainLoopLock);
        RunLuaScriptFile(self->uniqueID, [luaScriptPath UTF8String]);
        pthread_mutex_unlock(&mainLoopLock);
    }
    else
    {
        [self.luaScriptConsole setStringValue:invalidPath];
    }
}

static void PrintToWindowConsole(int uniqueID, const char* str)
{
    NSString *errorMsg = [[NSString alloc] initWithCString:str encoding:NSUTF8StringEncoding];
    dispatch_async(dispatch_get_main_queue(), ^{
        LuaViewControllerMap_t::iterator iController = s_LuaViewControllers.find(uniqueID);
        if (iController != s_LuaViewControllers.end())
        {
            S9xLuaViewController *luaController = iController->second;
            NSString *contentString = [luaController.luaScriptConsole stringValue];
            NSArray *contentArray = [[NSArray alloc] initWithObjects:contentString, errorMsg, nil];
            NSString *newContentString = [contentArray componentsJoinedByString:@""];
            [luaController.luaScriptConsole setStringValue:newContentString];
        }
    });
}


static void OnStart(int uniqueID)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        LuaViewControllerMap_t::iterator iController = s_LuaViewControllers.find(uniqueID);
        if (iController != s_LuaViewControllers.end())
		{
            S9xLuaViewController *luaController = iController->second;
            [luaController.luaBrowseButton setEnabled:false];
            [luaController.luaStopButton setEnabled:true];
            [luaController.luaRunButton setTitle:@"Restart"];
            [luaController.luaScriptConsole setStringValue:@""];
		}
    });
}

static void OnStop(int uniqueID, bool statusOK)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        LuaViewControllerMap_t::iterator iController = s_LuaViewControllers.find(uniqueID);
        if (iController != s_LuaViewControllers.end())
        {
            S9xLuaViewController *luaController = iController->second;
            [luaController.luaBrowseButton setEnabled:true];
            [luaController.luaStopButton setEnabled:false];
            [luaController.luaRunButton setTitle:@"Run"];
        }
    });
}

@end

#endif // HAVE_LUA
