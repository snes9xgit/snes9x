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
#import <mach/mach_time.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLRenderers.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>

#import "snes9x.h"
#import "memmap.h"
#import "apu.h"
#import "controls.h"
#import "crosshairs.h"
#import "cheats.h"
#import "movie.h"
#import "snapshot.h"
#import "display.h"
#import "blit.h"

#ifdef DEBUGGER
#import "debug.h"
#endif

#import <pthread.h>

#import "mac-prefix.h"
#import "mac-audio.h"
#import "mac-cheat.h"
#import "mac-cheatfinder.h"
#import "mac-cocoatools.h"
#import "mac-controls.h"
#import "mac-dialog.h"
#import "mac-file.h"
#import "mac-gworld.h"
#import "mac-joypad.h"
#import "mac-keyboard.h"
#import "mac-multicart.h"
#import "mac-musicbox.h"
#import "mac-netplay.h"
#import "mac-render.h"
#import "mac-screenshot.h"
#import "mac-snes9x.h"
#import "mac-stringtools.h"
#import "mac-os.h"

#define	kRecentMenu_MAX		20

volatile bool8		running             = false;
volatile bool8		s9xthreadrunning    = false;

volatile bool8		windowExtend        = true;

uint32				controlPad[MAC_MAX_PLAYERS];

uint8				romDetect           = 0,
					interleaveDetect    = 0,
					videoDetect         = 0,
					headerDetect        = 0;

WindowRef			gWindow             = NULL;
uint32				glScreenW,
					glScreenH;
CGRect				glScreenBounds;

CGImageRef			macIconImage[118];
int					macPadIconIndex,
					macLegendIconIndex,
					macMusicBoxIconIndex, 
					macFunctionIconIndex;

int					macFrameSkip        = -1;
int32				skipFrames          = 3;
int64				lastFrame           = 0;

int64               machTimeNumerator   = 0;
int64               machTimeDenominator = 0;

int					macFastForwardRate  = 5,
					macFrameAdvanceRate = 1000000;

unsigned long		spcFileCount        = 0,
					pngFileCount        = 0;

bool8				cartOpen            = false,
					autofire            = false;

bool8				fullscreen          = false,
					autoRes             = false,
					glstretch           = true,
					gl32bit             = true,
					vsync               = true,
					drawoverscan        = false;
int					videoMode           = VIDEOMODE_BLOCKY;

SInt32				macSoundVolume      = 80;	// %
uint32				macSoundBuffer_ms   = 80;	// ms
uint32				macSoundInterval_ms = 16;   // ms
bool8				macSoundLagEnable   = false;
uint16				aueffect            = 0;

uint8				saveInROMFolder     = 2;	// 0 : Snes9x  1 : ROM  2 : Application Support
NSString   			*saveFolderPath;

int					macCurvatureWarp    = 15,
					macAspectRatio      = 0;

bool8				startopendlog       = false,
					showtimeinfrz       = false,
					enabletoggle        = true,
					savewindowpos       = false,
					onscreeninfo        = true;
int					inactiveMode        = 2;
int					musicboxmode        = kMBXSoundEmulation;

bool8				applycheat          = false;
int					padSetting          = 1,
					deviceSetting       = 1,
					deviceSettingMaster = 1;
int					macControllerOption = SNES_JOYPAD;
AutoFireState		autofireRec[MAC_MAX_PLAYERS];

bool8				macQTRecord         = false;
uint16				macQTMovFlag        = 0;

uint16				macRecordFlag       = 0x3,
					macPlayFlag         = 0x1;
wchar_t				macRecordWChar[MOVIE_MAX_METADATA];

char				npServerIP[256],
					npName[256];

bool8				lastoverscan        = false;

CGPoint				unlimitedCursor;

CFStringRef			multiCartPath[2];

#ifdef MAC_PANTHER_SUPPORT
IconRef				macIconRef[118];
#endif

id<S9xInputDelegate>    inputDelegate = nil;

typedef enum
{
    ToggleBG0,
    ToggleBG1,
    ToggleBG2,
    ToggleBG3,
    ToggleSprites,
    SwapJoypads,
    SoundChannel0,
    SoundChannel1,
    SoundChannel2,
    SoundChannel3,
    SoundChannel4,
    SoundChannel5,
    SoundChannel6,
    SoundChannel7,
    SoundChannelsOn,
    ToggleDisplayPressedKeys,
    ToggleDisplayMovieFrame,
    IncreaseFrameAdvanceRate,
    DecreaseFrameAdvanceRate,
    ToggleEmulationPause,
    AdvanceFrame,
    kNumFunctionButtons
} S9xFunctionButtonCommand;

uint8 functionButtons[kNumFunctionButtons] = {
    kVK_F1,
    kVK_F2,
    kVK_F3,
    kVK_F4,
    kVK_F5,
    kVK_F6,
    kVK_ANSI_1,
    kVK_ANSI_2,
    kVK_ANSI_3,
    kVK_ANSI_4,
    kVK_ANSI_5,
    kVK_ANSI_6,
    kVK_ANSI_7,
    kVK_ANSI_8,
    kVK_ANSI_9,
    kVK_ANSI_0,
    kVK_ANSI_Minus,
    kVK_ANSI_Q,
    kVK_ANSI_W,
    kVK_ANSI_O,
    kVK_ANSI_P
};

bool8               pressedKeys[MAC_MAX_PLAYERS][kNumButtons] = { 0 };
bool8               pressedGamepadButtons[MAC_MAX_PLAYERS][kNumButtons] = { 0 };
bool8               pressedFunctionButtons[kNumFunctionButtons] = { 0 };
bool8               pressedRawKeyboardButtons[MAC_NUM_KEYCODES] = { 0 };
bool8               heldFunctionButtons[kNumFunctionButtons] = { 0 };
pthread_mutex_t     keyLock;

S9xView             *s9xView;

enum
{
       mApple          = 128,
       iAbout          = 1,

       mFile           = 129,
       iOpen           = 1,
       iOpenMulti      = 2,
       iOpenRecent     = 3,
       iClose          = 5,
       iRomInfo        = 7,

       mControl        = 134,
       iKeyboardLayout = 1,
       iISpLayout      = 2,
       iAutoFire       = 4,
       iISpPreset      = 6,

       mEdit           = 130,

       mEmulation      = 131,
       iResume         = 1,
       iSoftReset      = 3,
       iReset          = 4,
       iDevice         = 6,

       mCheat          = 132,
       iApplyCheats    = 1,
       iGameGenie      = 3,
       iCheatFinder    = 4,

       mOption         = 133,
       iFreeze         = 1,
       iDefrost        = 2,
       iFreezeTo       = 4,
       iDefrostFrom    = 5,
       iRecordMovie    = 7,
       iPlayMovie      = 8,
       iQTMovie        = 10,
       iSaveSPC        = 12,
       iSaveSRAM       = 13,
       iCIFilter       = 15,
       iMusicBox       = 17,

       mNetplay        = 135,
       iServer         = 1,
       iClient         = 2,

       mPresets        = 201,

       mDevice         = 202,
       iPad            = 1,
       iMouse                  = 2,
       iMouse2         = 3,
       iSuperScope     = 4,
       iMultiPlayer5   = 5,
       iMultiPlayer5_2 = 6,
       iJustifier1     = 7,
       iJustifier2     = 8,

       mRecentItem     = 203
};

struct GameViewInfo
{
	int		globalLeft;
	int		globalTop;
	int		width;
	int		height;
};

static volatile bool8	rejectinput     = false;

static bool8			pauseEmulation  = false,
						frameAdvance    = false;

static int				frameCount      = 0;

static bool8			frzselecting    = false;

static uint16			changeAuto[2] = { 0x0000, 0x0000 };

static GameViewInfo		scopeViewInfo;

static void Initialize (void);
static void Deinitialize (void);
static void InitAutofire (void);
static void ProcessInput (void);
static void ChangeAutofireSettings (int, int);
static void ChangeTurboRate (int);
static void UpdateFreezeDefrostScreen (int, CGImageRef, uint8 *, CGContextRef);
static void * MacSnes9xThread (void *);
static inline void EmulationLoop (void);

int main (int argc, const char *argv[])
{
    return NSApplicationMain(argc, argv);
}

static void * MacSnes9xThread (void *)
{
    Settings.StopEmulation = false;
    s9xthreadrunning = true;

    EmulationLoop();

    s9xthreadrunning = false;
    Settings.StopEmulation = true;

    return (NULL);
}

void CopyPressedKeys(bool8 keys[MAC_MAX_PLAYERS][kNumButtons], bool8 gamepadButtons[MAC_MAX_PLAYERS][kNumButtons])
{
    pthread_mutex_lock(&keyLock);
    memcpy(keys, pressedKeys, sizeof(pressedKeys));
    memcpy(gamepadButtons, pressedGamepadButtons, sizeof(pressedGamepadButtons));
    pthread_mutex_unlock(&keyLock);
}

static inline void EmulationLoop (void)
{
    bool8    olddisplayframerate = false;
    int        storedMacFrameSkip  = macFrameSkip;

    pauseEmulation = false;
    frameAdvance   = false;
	[s9xView updatePauseOverlay];


    if (macQTRecord)
    {
        olddisplayframerate = Settings.DisplayFrameRate;
        Settings.DisplayFrameRate = false;
    }

    MacStartSound();

    if (Settings.NetPlay)
    {
//        if (Settings.NetPlayServer)
//        {
//            NPServerDetachNetPlayThread();
//            NPServerStartClients();
//
//            while (running)
//            {
//                NPServerProcessInput();
//                S9xMainLoop();
//            }
//
//            NPServerStopNetPlayThread();
//            NPServerStopServer();
//        }
//        else
//        {
//            NPClientDetachNetPlayThread();
//            NPClientNetPlayWaitStart();
//
//            while (running)
//            {
//                NPClientProcessInput();
//                S9xMainLoop();
//            }
//
//            NPClientStopNetPlayThread();
//            NPClientDisconnect();
//
//            NPClientRestoreConfig();
//        }
    }
    else
    {
        while (running)
        {
            ProcessInput();

            if (!pauseEmulation)
            {
                S9xMainLoop();
            }
            else
            {
                if (frameAdvance)
                {
                    macFrameSkip = 1;
                    skipFrames = 1;
                    frameAdvance = false;
                    S9xMainLoop();
                    macFrameSkip = storedMacFrameSkip;
                }

                usleep(Settings.FrameTime);
            }
        }
    }

    MacStopSound();

    if (macQTRecord)
    {
//        MacQTStopRecording();
//        macQTRecord = false;

        Settings.DisplayFrameRate = olddisplayframerate;
    }

    S9xMovieShutdown();
}
//
//static OSStatus MainEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
//{
//    OSStatus    err, result = eventNotHandledErr;
//    Boolean        done = false;
//
//    if (frzselecting)
//        return (result);
//
//    switch (GetEventClass(inEvent))
//    {
//        case kEventClassCommand:
//            switch (GetEventKind(inEvent))
//            {
//                HICommand    cmd;
//
//                case kEventCommandUpdateStatus:
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &cmd);
//                    if (err == noErr && cmd.commandID == 'clos')
//                    {
//                        UpdateMenuCommandStatus(false);
//                        result = noErr;
//                    }
//
//                    break;
//
//                case kEventCommandProcess:
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &cmd);
//                    if (err == noErr)
//                    {
//                        UInt32    modifierkey;
//
//                        err = GetEventParameter(inEvent, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifierkey);
//                        if (err == noErr)
//                        {
//                            if ((cmd.commandID == 'pref') && (modifierkey & optionKey))
//                                cmd.commandID = 'EXTR';
//
//                            result = HandleMenuChoice(cmd.commandID, &done);
//
//                            if (done)
//                                QuitApplicationEventLoop();
//                        }
//                    }
//
//                    break;
//            }
//
//            break;
//    }
//
//    return (result);
//}
//
//static OSStatus SubEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
//{
//    OSStatus    err, result = eventNotHandledErr;
//
//    if (frzselecting)
//        return (result);
//
//    switch (GetEventClass(inEvent))
//    {
//        case kEventClassCommand:
//            switch (GetEventKind(inEvent))
//            {
//                HICommand    cmd;
//
//                case kEventCommandUpdateStatus:
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &cmd);
//                    if (err == noErr && cmd.commandID == 'clos')
//                    {
//                        UpdateMenuCommandStatus(false);
//                        result = noErr;
//                    }
//
//                    break;
//
//                case kEventCommandProcess:
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &cmd);
//                    if (err == noErr)
//                    {
//                        switch (cmd.commandID)
//                        {
//                            case 'Erun':    // Pause
//                            case 'SubQ':    // Queue from emulation thread
//                                running = false;
//                                while (s9xthreadrunning)
//                                    sleep(0);
//                                QuitApplicationEventLoop();
//                                result = noErr;
//                                break;
//
//                            case 'Ocif':    // Core Image Filter
//                                HiliteMenu(0);
//                                ConfigureCoreImageFilter();
//                                result = noErr;
//                                break;
//                        }
//                    }
//
//                    break;
//            }
//
//            break;
//
//        case kEventClassMouse:
//            if (fullscreen)
//            {
//                if ((macControllerOption == SNES_JOYPAD) || (macControllerOption == SNES_MULTIPLAYER5) || (macControllerOption == SNES_MULTIPLAYER5_2))
//                {
//                    if (!(Settings.NetPlay && !Settings.NetPlayServer))
//                    {
//                        switch (GetEventKind(inEvent))
//                        {
//                            case kEventMouseUp:
//                                HIPoint    hipt;
//
//                                err = GetEventParameter(inEvent, kEventParamMouseLocation, typeHIPoint, NULL, sizeof(HIPoint), NULL, &hipt);
//                                if (err == noErr)
//                                {
//                                    if (CGRectContainsPoint(glScreenBounds, hipt))
//                                    {
//                                        running = false;
//                                        while (s9xthreadrunning)
//                                            sleep(0);
//                                        QuitApplicationEventLoop();
//                                        result = noErr;
//                                    }
//                                }
//
//                                break;
//                        }
//                    }
//                }
//                else
//                if ((macControllerOption == SNES_MOUSE) || (macControllerOption == SNES_MOUSE_SWAPPED))
//                {
//                    switch (GetEventKind(inEvent))
//                    {
//                        case kEventMouseMoved:
//                        case kEventMouseDragged:
//                            HIPoint    hipt;
//
//                            err = GetEventParameter(inEvent, kEventParamMouseDelta, typeHIPoint, NULL, sizeof(HIPoint), NULL, &hipt);
//                            if (err == noErr)
//                            {
//                                unlimitedCursor.x += hipt.x;
//                                unlimitedCursor.y += hipt.y;
//                            }
//
//                            break;
//                    }
//                }
//            }
//
//            break;
//    }
//
//    return (result);
//}
//
//void PostQueueToSubEventLoop (void)
//{
//    OSStatus    err;
//    EventRef    event;
//
//    err = CreateEvent(kCFAllocatorDefault, kEventClassCommand, kEventCommandProcess, 0, kEventAttributeUserEvent, &event);
//    if (err == noErr)
//    {
//        HICommand    cmd;
//
//        cmd.commandID          = 'SubQ';
//        cmd.attributes         = kEventAttributeUserEvent;
//        cmd.menu.menuRef       = NULL;
//        cmd.menu.menuItemIndex = 0;
//
//        err = SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(HICommand), &cmd);
//        if (err == noErr)
//            err = PostEventToQueue(GetMainEventQueue(), event, kEventPriorityStandard);
//
//        ReleaseEvent(event);
//    }
//}
//
//void InitGameWindow (void)
//{
//    OSStatus            err;
//    IBNibRef            nibRef;
//    WindowAttributes    attr;
//    CFStringRef            ref;
//    HIViewRef            ctl;
//    HIViewID            cid = { 'Pict', 0 };
//    Rect                rct;
//    char                drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//    EventTypeSpec        wupaneEvents[] = { { kEventClassControl, kEventControlClick            },
//                                           { kEventClassControl, kEventControlDraw             } },
//                        windowEvents[] = { { kEventClassWindow,  kEventWindowDeactivated       },
//                                           { kEventClassWindow,  kEventWindowActivated         },
//                                           { kEventClassWindow,  kEventWindowBoundsChanging    },
//                                           { kEventClassWindow,  kEventWindowBoundsChanged     },
//                                           { kEventClassWindow,  kEventWindowZoom              },
//                                           { kEventClassWindow,  kEventWindowToolbarSwitchMode } };
//
//    if (gWindow)
//        return;
//
//    err = CreateNibReference(kMacS9XCFString, &nibRef);
//    if (err)
//        QuitWithFatalError(err, "os 02");
//
//    err = CreateWindowFromNib(nibRef, CFSTR("GameWindow"), &gWindow);
//    if (err)
//        QuitWithFatalError(err, "os 03");
//
//    DisposeNibReference(nibRef);
//
//    HIViewFindByID(HIViewGetRoot(gWindow), cid, &ctl);
//
//    gameWindowUPP = NewEventHandlerUPP(GameWindowEventHandler);
//    err = InstallWindowEventHandler(gWindow, gameWindowUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) gWindow, &gameWindowEventRef);
//
//    gameWUPaneUPP = NewEventHandlerUPP(GameWindowUserPaneEventHandler);
//    err = InstallControlEventHandler(ctl, gameWUPaneUPP, GetEventTypeCount(wupaneEvents), wupaneEvents, (void *) gWindow, &gameWUPaneEventRef);
//
//    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//    ref = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
//    if (ref)
//    {
//        SetWindowTitleWithCFString(gWindow, ref);
//        CFRelease(ref);
//    }
//
//    attr = kWindowFullZoomAttribute | kWindowResizableAttribute | kWindowLiveResizeAttribute;
//    err = ChangeWindowAttributes(gWindow, attr, kWindowNoAttributes);
//
//    attr = kWindowToolbarButtonAttribute;
//    if (!drawoverscan)
//        err = ChangeWindowAttributes(gWindow, attr, kWindowNoAttributes);
//    else
//        err = ChangeWindowAttributes(gWindow, kWindowNoAttributes, attr);
//
//    if (savewindowpos)
//    {
//        MoveWindow(gWindow, windowPos[kWindowScreen].h, windowPos[kWindowScreen].v, false);
//
//        if ((windowSize[kWindowScreen].width <= 0) || (windowSize[kWindowScreen].height <= 0))
//        {
//            windowExtend = true;
//            windowSize[kWindowScreen].width  = 512;
//            windowSize[kWindowScreen].height = kMacWindowHeight;
//        }
//
//        if (!lastoverscan && !windowExtend && drawoverscan)
//        {
//            windowExtend = true;
//            windowSize[kWindowScreen].height = (int) ((float) (windowSize[kWindowScreen].height + 0.5) * SNES_HEIGHT_EXTENDED / SNES_HEIGHT);
//        }
//
//        SizeWindow(gWindow, (short) windowSize[kWindowScreen].width, (short) windowSize[kWindowScreen].height, false);
//    }
//    else
//    {
//        if (drawoverscan)
//            windowExtend = true;
//
//        SizeWindow(gWindow, 512, (windowExtend ? kMacWindowHeight : (SNES_HEIGHT << 1)), false);
//        RepositionWindow(gWindow, NULL, kWindowCenterOnMainScreen);
//    }
//
//    windowZoomCount = 0;
//
//    GetWindowBounds(gWindow, kWindowContentRgn, &rct);
//    gWindowRect = CGRectMake((float) rct.left, (float) rct.top, (float) (rct.right - rct.left), (float) (rct.bottom - rct.top));
//
//    ActivateWindow(gWindow, true);
//}
//
//void UpdateGameWindow (void)
//{
//    OSStatus    err;
//    HIViewRef    ctl;
//    HIViewID    cid = { 'Pict', 0 };
//
//    if (!gWindow)
//        return;
//
//    HIViewFindByID(HIViewGetRoot(gWindow), cid, &ctl);
//    err = HIViewSetNeedsDisplay(ctl, true);
//}
//
//static void ResizeGameWindow (void)
//{
//    Rect    rct;
//    int        ww, wh;
//
//    if (!gWindow)
//        return;
//
//    GetWindowBounds(gWindow, kWindowContentRgn, &rct);
//
//    wh = (windowExtend ? SNES_HEIGHT_EXTENDED : SNES_HEIGHT) * ((windowZoomCount >> 1) + 1);
//
//    if (windowZoomCount % 2)
//        ww = SNES_NTSC_OUT_WIDTH(SNES_WIDTH) * ((windowZoomCount >> 1) + 1) / 2;
//    else
//        ww = SNES_WIDTH * ((windowZoomCount >> 1) + 1);
//
//    rct.right  = rct.left + ww;
//    rct.bottom = rct.top  + wh;
//
//    SetWindowBounds(gWindow, kWindowContentRgn, &rct);
//
//    printf("Window Size: %d, %d\n", ww, wh);
//
//    windowZoomCount++;
//    if (windowZoomCount == 8)
//        windowZoomCount = 0;
//}
//
//void DeinitGameWindow (void)
//{
//    OSStatus    err;
//
//    if (!gWindow)
//        return;
//
//    SaveWindowPosition(gWindow, kWindowScreen);
//    lastoverscan = drawoverscan;
//
//    err = RemoveEventHandler(gameWUPaneEventRef);
//    DisposeEventHandlerUPP(gameWUPaneUPP);
//
//    err = RemoveEventHandler(gameWindowEventRef);
//    DisposeEventHandlerUPP(gameWindowUPP);
//
//    CFRelease(gWindow);
//    gWindow = NULL;
//}
//
//static OSStatus GameWindowEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
//{
//    OSStatus    err, result = eventNotHandledErr;
//    HIRect        rct;
//    Rect        r;
//    UInt32        attr;
//
//    switch (GetEventClass(inEvent))
//    {
//        case kEventClassWindow:
//            switch (GetEventKind(inEvent))
//            {
//                case kEventWindowDeactivated:
//                    if (running)
//                    {
//                        if (!(Settings.NetPlay && !Settings.NetPlayServer))
//                        {
//                            if (inactiveMode == 3)
//                            {
//                                running = false;
//                                while (s9xthreadrunning)
//                                    sleep(0);
//                                QuitApplicationEventLoop();
//                                result = noErr;
//                            }
//                            else
//                            if (inactiveMode == 2)
//                            {
//                                rejectinput = true;
//                                result = noErr;
//                            }
//                        }
//                    }
//
//                    break;
//
//                case kEventWindowActivated:
//                    if (running)
//                    {
//                        if (!(Settings.NetPlay && !Settings.NetPlayServer))
//                        {
//                            ForceChangingKeyScript();
//
//                            if (inactiveMode == 2)
//                            {
//                                rejectinput = false;
//                                result = noErr;
//                            }
//                        }
//                    }
//
//                    break;
//
//                case kEventWindowBoundsChanging:
//                    windowResizeCount = 0x7FFFFFFF;
//
//                    err = GetEventParameter(inEvent, kEventParamAttributes, typeUInt32, NULL, sizeof(UInt32), NULL, &attr);
//                    if ((err == noErr) && (attr & kWindowBoundsChangeSizeChanged))
//                    {
//                        err = GetEventParameter(inEvent, kEventParamCurrentBounds, typeHIRect, NULL, sizeof(HIRect), NULL, &rct);
//                        if (err == noErr)
//                        {
//                            if (GetCurrentEventKeyModifiers() & shiftKey)
//                            {
//                                HIRect    origRct;
//
//                                err = GetEventParameter(inEvent, kEventParamOriginalBounds, typeHIRect, NULL, sizeof(HIRect), NULL, &origRct);
//                                if (err == noErr)
//                                {
//                                    rct.size.width = (float) (int) (origRct.size.width * rct.size.height / origRct.size.height);
//                                    err = SetEventParameter(inEvent, kEventParamCurrentBounds, typeHIRect, sizeof(HIRect), &rct);
//                                }
//                            }
//
//                            gWindowRect = rct;
//                        }
//                    }
//
//                    result = noErr;
//                    break;
//
//                case kEventWindowBoundsChanged:
//                    windowResizeCount = 3;
//                    result = noErr;
//                    break;
//
//                case kEventWindowZoom:
//                    ResizeGameWindow();
//                    result = noErr;
//                    break;
//
//                case kEventWindowToolbarSwitchMode:
//                    windowExtend = !windowExtend;
//
//                    GetWindowBounds(gWindow, kWindowContentRgn, &r);
//
//                    if (windowExtend)
//                        r.bottom = r.top + (int) (((float) (r.bottom - r.top) + 0.5) * SNES_HEIGHT_EXTENDED / SNES_HEIGHT);
//                    else
//                        r.bottom = r.top + (int) (((float) (r.bottom - r.top) + 0.5) * SNES_HEIGHT / SNES_HEIGHT_EXTENDED);
//
//                    SetWindowBounds(gWindow, kWindowContentRgn, &r);
//
//                    result = noErr;
//                    break;
//            }
//
//            break;
//    }
//
//    return (result);
//}
//
//static OSStatus GameWindowUserPaneEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
//{
//    OSStatus    err, result = eventNotHandledErr;
//
//    switch (GetEventClass(inEvent))
//    {
//        case kEventClassControl:
//            switch (GetEventKind(inEvent))
//            {
//                case kEventControlClick:
//                    if (running)
//                    {
//                        if ((macControllerOption == SNES_JOYPAD) || (macControllerOption == SNES_MULTIPLAYER5) || (macControllerOption == SNES_MULTIPLAYER5_2))
//                        {
//                            if (!(Settings.NetPlay && !Settings.NetPlayServer))
//                            {
//                                if (!frzselecting)
//                                {
//                                    running = false;
//                                    while (s9xthreadrunning)
//                                        sleep(0);
//                                    QuitApplicationEventLoop();
//                                    result = noErr;
//                                }
//                            }
//                        }
//                    }
//                    else
//                    {
//                        UInt32    count;
//
//                        err = GetEventParameter(inEvent, kEventParamClickCount, typeUInt32, NULL, sizeof(UInt32), NULL, &count);
//                        if ((err == noErr) && (count == 2))
//                        {
//                            SNES9X_Go();
//                            QuitApplicationEventLoop();
//                            result = noErr;
//                        }
//                    }
//
//                    break;
//
//                case kEventControlDraw:
//                    CGContextRef    ctx;
//                    HIViewRef        view;
//                    HIRect            bounds;
//
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
//                    if (err == noErr)
//                    {
//                        err = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx);
//                        if (err == noErr)
//                        {
//                            if (!running)
//                            {
//                                HIViewGetBounds(view, &bounds);
//                                CGContextTranslateCTM(ctx, 0, bounds.size.height);
//                                CGContextScaleCTM(ctx, 1.0f, -1.0f);
//                                DrawPauseScreen(ctx, bounds);
//                            }
//                        }
//                    }
//
//                    result = noErr;
//                    break;
//            }
//
//            break;
//    }
//
//    return (result);
//}
//
//
//static void InitRecentMenu (void)
//{
//    OSStatus    err;
//
//    err = CreateNewMenu(mRecentItem, 0, &recentMenu);
//    err = SetMenuItemHierarchicalMenu(GetMenuRef(mFile), iOpenRecent, recentMenu);
//}
//
//static void DeinitRecentMenu (void)
//{
//    CFRelease(recentMenu);
//}
//
//void BuildRecentMenu (void)
//{
//    OSStatus    err;
//    CFStringRef    str;
//
//    err = DeleteMenuItems(recentMenu, 1, CountMenuItems(recentMenu));
//
//    for (int i = 0; i < kRecentMenu_MAX; i++)
//    {
//        if (!recentItem[i])
//            break;
//
//        Boolean    r;
//        char    path[PATH_MAX + 1];
//
//        r = CFStringGetCString(recentItem[i], path, PATH_MAX, kCFStringEncodingUTF8);
//        if (r)
//        {
//            CFStringRef    nameRef;
//            char        drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//
//            _splitpath(path, drive, dir, fname, ext);
//            snprintf(path, PATH_MAX + 1, "%s%s", fname, ext);
//            nameRef = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
//            if (nameRef)
//            {
//                err = AppendMenuItemTextWithCFString(recentMenu, nameRef, 0, 'FRe0' + i, NULL);
//                CFRelease(nameRef);
//            }
//        }
//    }
//
//    err = AppendMenuItemTextWithCFString(recentMenu, NULL, kMenuItemAttrSeparator, 'FR__', NULL);
//
//    str = CFCopyLocalizedString(CFSTR("ClearMenu"), "ClearMenu");
//    if (str)
//    {
//        err = AppendMenuItemTextWithCFString(recentMenu, str, 0, 'FRcr', NULL);
//        CFRelease(str);
//    }
//}
//
//void AdjustMenus (void)
//{
//    OSStatus    err;
//    MenuRef        menu;
//    CFStringRef    str;
//
//    if (running)
//    {
//        menu = GetMenuRef(mApple);
//        DisableMenuItem(menu, iAbout);
//        DisableMenuCommand(NULL, kHICommandPreferences);
//        DisableMenuCommand(NULL, kHICommandQuit);
//
//        menu = GetMenuRef(mFile);
//        DisableMenuItem(menu, iOpen);
//        DisableMenuItem(menu, iOpenMulti);
//        DisableMenuItem(menu, iOpenRecent);
//        DisableMenuItem(menu, iRomInfo);
//
//        menu = GetMenuRef(mControl);
//        DisableMenuItem(menu, iKeyboardLayout);
//        DisableMenuItem(menu, iISpLayout);
//        DisableMenuItem(menu, iAutoFire);
//        DisableMenuItem(menu, iISpPreset);
//
//        menu = GetMenuRef(mEmulation);
//        str = CFCopyLocalizedString(CFSTR("PauseMenu"), "pause");
//        err = SetMenuItemTextWithCFString(menu, iResume, str);
//        CFRelease(str);
//        DisableMenuItem(menu, iSoftReset);
//        DisableMenuItem(menu, iReset);
//        DisableMenuItem(menu, iDevice);
//
//        if (Settings.NetPlay)
//        {
//            if (Settings.NetPlayServer)
//                EnableMenuItem(menu, iResume);
//            else
//                DisableMenuItem(menu, iResume);
//        }
//        else
//            EnableMenuItem(menu, iResume);
//
//        menu = GetMenuRef(mCheat);
//        DisableMenuItem(menu, iApplyCheats);
//        DisableMenuItem(menu, iGameGenie);
//        DisableMenuItem(menu, iCheatFinder);
//
//        menu = GetMenuRef(mOption);
//        DisableMenuItem(menu, iFreeze);
//        DisableMenuItem(menu, iDefrost);
//        DisableMenuItem(menu, iFreezeTo);
//        DisableMenuItem(menu, iDefrostFrom);
//        DisableMenuItem(menu, iRecordMovie);
//        DisableMenuItem(menu, iPlayMovie);
//        DisableMenuItem(menu, iQTMovie);
//        DisableMenuItem(menu, iSaveSPC);
//        DisableMenuItem(menu, iSaveSRAM);
//        DisableMenuItem(menu, iMusicBox);
//        if (ciFilterEnable)
//            EnableMenuItem(menu, iCIFilter);
//        else
//            DisableMenuItem(menu, iCIFilter);
//
//        menu = GetMenuRef(mNetplay);
//        DisableMenuItem(menu, iServer);
//        DisableMenuItem(menu, iClient);
//    }
//    else
//    {
//        menu = GetMenuRef(mApple);
//        EnableMenuItem(menu, iAbout);
//        EnableMenuCommand(NULL, kHICommandPreferences);
//        EnableMenuCommand(NULL, kHICommandQuit);
//
//        menu = GetMenuRef(mFile);
//        EnableMenuItem(menu, iOpen);
//        EnableMenuItem(menu, iOpenMulti);
//        EnableMenuItem(menu, iOpenRecent);
//        if (cartOpen)
//            EnableMenuItem(menu, iRomInfo);
//        else
//            DisableMenuItem(menu, iRomInfo);
//
//        menu = GetMenuRef(mControl);
//        EnableMenuItem(menu, iKeyboardLayout);
//        EnableMenuItem(menu, iAutoFire);
//
//        menu = GetMenuRef(mEmulation);
//        str = CFCopyLocalizedString(CFSTR("RunMenu"), "run");
//        err = SetMenuItemTextWithCFString(menu, iResume, str);
//        CFRelease(str);
//        EnableMenuItem(menu, iDevice);
//        if (cartOpen)
//        {
//            EnableMenuItem(menu, iResume);
//            EnableMenuItem(menu, iSoftReset);
//            EnableMenuItem(menu, iReset);
//        }
//        else
//        {
//            DisableMenuItem(menu, iResume);
//            DisableMenuItem(menu, iSoftReset);
//            DisableMenuItem(menu, iReset);
//        }
//
//        menu = GetMenuRef(mCheat);
//        if (cartOpen)
//        {
//            EnableMenuItem(menu, iApplyCheats);
//            EnableMenuItem(menu, iGameGenie);
//            EnableMenuItem(menu, iCheatFinder);
//        }
//        else
//        {
//            DisableMenuItem(menu, iApplyCheats);
//            DisableMenuItem(menu, iGameGenie);
//            DisableMenuItem(menu, iCheatFinder);
//        }
//
//        menu = GetMenuRef(mOption);
//        DisableMenuItem(menu, iCIFilter);
//        if (cartOpen)
//        {
//            EnableMenuItem(menu, iFreeze);
//            EnableMenuItem(menu, iDefrost);
//            EnableMenuItem(menu, iFreezeTo);
//            EnableMenuItem(menu, iDefrostFrom);
//            EnableMenuItem(menu, iRecordMovie);
//            EnableMenuItem(menu, iPlayMovie);
//            EnableMenuItem(menu, iQTMovie);
//            EnableMenuItem(menu, iSaveSPC);
//            EnableMenuItem(menu, iSaveSRAM);
//            EnableMenuItem(menu, iMusicBox);
//        }
//        else
//        {
//            DisableMenuItem(menu, iFreeze);
//            DisableMenuItem(menu, iDefrost);
//            DisableMenuItem(menu, iFreezeTo);
//            DisableMenuItem(menu, iDefrostFrom);
//            DisableMenuItem(menu, iRecordMovie);
//            DisableMenuItem(menu, iPlayMovie);
//            DisableMenuItem(menu, iQTMovie);
//            DisableMenuItem(menu, iSaveSPC);
//            DisableMenuItem(menu, iSaveSRAM);
//            DisableMenuItem(menu, iMusicBox);
//        }
//
//        menu = GetMenuRef(mNetplay);
//        EnableMenuItem(menu, iClient);
//        if (cartOpen)
//            EnableMenuItem(menu, iServer);
//        else
//            DisableMenuItem(menu, iServer);
//    }
//
//    DrawMenuBar();
//}
//
//void UpdateMenuCommandStatus (Boolean closeMenu)
//{
//    if (closeMenu)
//        EnableMenuItem(GetMenuRef(mFile), iClose);
//    else
//        DisableMenuItem(GetMenuRef(mFile), iClose);
//}
//
//static OSStatus HandleMenuChoice (UInt32 command, Boolean *done)
//{
//    OSStatus    err, result = noErr;
//    MenuRef        mh;
//    int            item;
//    bool8        isok = true;
//
//    if ((command & 0xFFFFFF00) == 'FRe\0')
//    {
//        Boolean    r;
//        int        index;
//        char    path[PATH_MAX + 1];
//
//        index = (int) (command & 0x000000FF) - (int) '0';
//        r = CFStringGetCString(recentItem[index], path, PATH_MAX, kCFStringEncodingUTF8);
//        if (r)
//        {
//            FSRef    ref;
//
//            err = FSPathMakeRef((unsigned char *) path, &ref, NULL);
//            if (err == noErr)
//            {
//                if (SNES9X_OpenCart(&ref))
//                {
//                    SNES9X_Go();
//                    *done = true;
//                }
//                else
//                    AdjustMenus();
//            }
//        }
//    }
//    else
//    {
//        switch (command)
//        {
//            case 'abou':    // About SNES9X
//                StartCarbonModalDialog();
//                AboutDialog();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'pref':    // Preferences...
//                StartCarbonModalDialog();
//                ConfigurePreferences();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'EXTR':    // Extra Options...
//                StartCarbonModalDialog();
//                ConfigureExtraOptions();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'quit':    // Quit SNES9X
//                SNES9X_Quit();
//                *done = true;
//
//                break;
//
//            case 'open':    // Open ROM Image...
//                if (SNES9X_OpenCart(NULL))
//                {
//                    SNES9X_Go();
//                    *done = true;
//                }
//                else
//                    AdjustMenus();
//
//                break;
//
//            case 'Mult':    // Open Multiple ROM Images...
//                if (SNES9X_OpenMultiCart())
//                {
//                    SNES9X_Go();
//                    *done = true;
//                }
//                else
//                    AdjustMenus();
//
//                break;
//
//            case 'FRcr':    // Clear Menu
//                ClearRecentItems();
//                BuildRecentMenu();
//
//                break;
//
//            case 'Finf':    // ROM Information
//                StartCarbonModalDialog();
//                RomInfoDialog();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Ckey':    // Configure Keyboard...
//                StartCarbonModalDialog();
//                ConfigureKeyboard();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Cpad':    // Configure Controllers...
//                StartCarbonModalDialog();
//                ConfigureHID();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Caut':    // Automatic Fire...
//                StartCarbonModalDialog();
//                ConfigureAutofire();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Hapl':    // Apply Cheat Entries
//                mh = GetMenuRef(mCheat);
//                applycheat = !applycheat;
//                CheckMenuItem(mh, iApplyCheats, applycheat);
//                Settings.ApplyCheats = applycheat;
//
//                if (!Settings.ApplyCheats)
//                    S9xCheatsDisable();
//                else
//                    S9xCheatsEnable();
//
//                break;
//
//            case 'Hent':    // Cheat Entry...
//                StartCarbonModalDialog();
//                ConfigureCheat();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Hfnd':    // Cheat Finder...
//                StartCarbonModalDialog();
//                CheatFinder();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Erun':    // Run
//                SNES9X_Go();
//                *done = true;
//
//                break;
//
//            case 'Esrs':    // Software Reset
//                SNES9X_SoftReset();
//                SNES9X_Go();
//                *done = true;
//
//                break;
//
//            case 'Erst':    // Hardware Reset
//                SNES9X_Reset();
//                SNES9X_Go();
//                *done = true;
//
//                break;
//
//            case 'Ofrz':    // Freeze State
//                isok = SNES9X_Freeze();
//                *done = true;
//
//                break;
//
//            case 'Odfr':    // Defrost state
//                isok = SNES9X_Defrost();
//                *done = true;
//
//                break;
//
//            case 'Ofrd':    // Freeze State to...
//                StartCarbonModalDialog();
//                isok = SNES9X_FreezeTo();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Odfd':    // Defrost State From...
//                StartCarbonModalDialog();
//                isok = SNES9X_DefrostFrom();
//                if (gWindow)
//                    ActivateWindow(gWindow, true);
//                FinishCarbonModalDialog();
//                *done = true;
//
//                break;
//
//            case 'MVrc':    // Record Movie...
//                StartCarbonModalDialog();
//                isok = SNES9X_RecordMovie();
//                if (gWindow)
//                    ActivateWindow(gWindow, true);
//                FinishCarbonModalDialog();
//                *done = true;
//
//                break;
//
//            case 'MVpl':    // Play Movie...
//                StartCarbonModalDialog();
//                isok = SNES9X_PlayMovie();
//                if (isok && (macPlayFlag & 0x2))
//                {
//                    running = false;
//                    isok = SNES9X_QTMovieRecord();
//                    running = true;
//                }
//
//                if (gWindow)
//                    ActivateWindow(gWindow, true);
//                FinishCarbonModalDialog();
//                *done = true;
//
//                break;
//
//            case 'QTmv':    // Record QuickTime Movie...
//                StartCarbonModalDialog();
//                isok = SNES9X_QTMovieRecord();
//                if (gWindow)
//                    ActivateWindow(gWindow, true);
//                FinishCarbonModalDialog();
//                *done = true;
//
//                break;
//
//            case 'Ospc':    // Save SPC File at Next Note-on
//                S9xDumpSPCSnapshot();
//
//                break;
//
//            case 'Osrm':    // Save SRAM Now
//                SNES9X_SaveSRAM();
//
//                break;
//
//            case 'Ombx':    // Music Box
//                StartCarbonModalDialog();
//                MusicBoxDialog();
//                FinishCarbonModalDialog();
//
//                break;
//
//            case 'Nser':    // Server...
//                bool8    sr;
//
//                Settings.NetPlay = false;
//                Settings.NetPlayServer = false;
//
//                NPServerInit();
//
//                if (!NPServerStartServer(NP_PORT))
//                {
//                    NPServerStopServer();
//                    break;
//                }
//
//                StartCarbonModalDialog();
//                sr = NPServerDialog();
//                FinishCarbonModalDialog();
//
//                if (sr)
//                {
//                    SNES9X_Reset();
//                    SNES9X_Go();
//                    Settings.NetPlay = true;
//                    Settings.NetPlayServer = true;
//
//                    *done = true;
//                }
//                else
//                    NPServerStopServer();
//
//                break;
//
//            case 'Ncli':    // Client...
//                bool8    cr;
//
//                Settings.NetPlay = false;
//                Settings.NetPlayServer = false;
//
//                NPClientInit();
//
//                StartCarbonModalDialog();
//                cr = NPClientDialog();
//                FinishCarbonModalDialog();
//
//                if (cr)
//                {
//                    SNES9X_Go();
//                    Settings.NetPlay = true;
//                    Settings.NetPlayServer = false;
//
//                    *done = true;
//                }
//                else
//                    AdjustMenus();
//
//                break;
//
//            case 'CPr1':    // Controller Preset
//            case 'CPr2':
//            case 'CPr3':
//            case 'CPr4':
//            case 'CPr5':
//                item = (int) (command & 0x000000FF) - (int) '0';
//                err = GetMenuItemHierarchicalMenu(GetMenuRef(mControl), iISpPreset, &mh);
//                CheckMenuItem(mh, padSetting, false);
//                padSetting = item;
//                CheckMenuItem(mh, padSetting, true);
//                ClearPadSetting();
//                LoadControllerSettings();
//
//                break;
//
//            case 'EIp1':    // Input Device
//            case 'EIp2':
//            case 'EIp3':
//            case 'EIp4':
//            case 'EIp5':
//            case 'EIp6':
//            case 'EIp7':
//            case 'EIp8':
//                item = (int) (command & 0x000000FF) - (int) '0';
//                err = GetMenuItemHierarchicalMenu(GetMenuRef(mEmulation), iDevice, &mh);
//                CheckMenuItem(mh, deviceSetting, false);
//                deviceSetting = item;
//                deviceSettingMaster = deviceSetting;
//                CheckMenuItem(mh, deviceSetting, true);
//                ChangeInputDevice();
//
//                break;
//
//            default:
//                result = eventNotHandledErr;
//                break;
//        }
//    }
//
//    return (result);
//}
//
void ChangeInputDevice (void)
{
    switch (deviceSetting)
    {
        case iPad:
            S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
            S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);
            macControllerOption = SNES_JOYPAD;
            break;

        case iMouse:
            S9xSetController(0, CTL_MOUSE,      0, 0, 0, 0);
            S9xSetController(1, CTL_JOYPAD,     1, 0, 0, 0);
            macControllerOption = SNES_MOUSE;
            break;

        case iMouse2:
            S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
            S9xSetController(1, CTL_MOUSE,      1, 0, 0, 0);
            macControllerOption = SNES_MOUSE_SWAPPED;
            break;

        case iSuperScope:
            S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
            S9xSetController(1, CTL_SUPERSCOPE, 0, 0, 0, 0);
            macControllerOption = SNES_SUPERSCOPE;
            break;

        case iMultiPlayer5:
            S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
            S9xSetController(1, CTL_MP5,        1, 2, 3, 4);
            macControllerOption = SNES_MULTIPLAYER5;
            break;

        case iMultiPlayer5_2:
            S9xSetController(0, CTL_MP5,        0, 1, 2, 3);
            S9xSetController(1, CTL_MP5,        4, 5, 6, 7);
            macControllerOption = SNES_MULTIPLAYER5_2;
            break;

        case iJustifier1:
            S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
            S9xSetController(1, CTL_JUSTIFIER,  0, 0, 0, 0);
            macControllerOption = SNES_JUSTIFIER;
            break;

        case iJustifier2:
            S9xSetController(0, CTL_JOYPAD,     0, 0, 0, 0);
            S9xSetController(1, CTL_JUSTIFIER,  1, 0, 0, 0);
            macControllerOption = SNES_JUSTIFIER_2;
            break;
    }
}

void ApplyNSRTHeaderControllers (void)
{
    uint32 valid = 0;
    deviceSetting = deviceSettingMaster;

    if (!strncmp((const char *) Memory.NSRTHeader + 24, "NSRT", 4))
    {
        switch (Memory.NSRTHeader[29])
        {
            case 0x00: // Everything goes
                deviceSetting = iPad;
                valid = (1 << iPad);
                break;

            case 0x10: // Mouse in Port 0
                deviceSetting = iMouse;
                valid = (1 << iMouse);
                break;

            case 0x01: // Mouse in Port 1
                deviceSetting = iMouse2;
                valid = (1 << iMouse2);
                break;

            case 0x03: // Super Scope in Port 1
                deviceSetting = iSuperScope;
                valid = (1 << iSuperScope);
                break;

            case 0x06: // Multitap in Port 1
                deviceSetting = iMultiPlayer5;
                valid = (1 << iPad) | (1 << iMultiPlayer5);
                break;

            case 0x66: // Multitap in Ports 0 and 1
                deviceSetting = iMultiPlayer5_2;
                valid = (1 << iPad) | (1 << iMultiPlayer5) | (1 << iMultiPlayer5_2);
                break;

            case 0x08: // Multitap in Port 1, Mouse in new Port 1
                deviceSetting = iMouse2;
                valid = (1 << iPad) | (1 << iMouse2) | (1 << iMultiPlayer5);
                break;

            case 0x04: // Pad or Super Scope in Port 1
                deviceSetting = iSuperScope;
                valid = (1 << iPad) | (1 << iSuperScope);
                break;

            case 0x05: // Justifier - Must ask user...
                deviceSetting = iJustifier1;
                valid = (1 << iJustifier1) | (1 << iJustifier2);
                break;

            case 0x20: // Pad or Mouse in Port 0
                deviceSetting = iMouse;
                valid = (1 << iPad) | (1 << iMouse);
                break;

            case 0x22: // Pad or Mouse in Port 0 & 1
                deviceSetting = iMouse;
                valid = (1 << iPad) | (1 << iMouse) | (1 << iMouse2);
                break;

            case 0x24: // Pad or Mouse in Port 0, Pad or Super Scope in Port 1
                deviceSetting = iSuperScope;
                valid = (1 << iPad) | (1 << iMouse) | (1 << iSuperScope);
                break;

            case 0x27: // Pad or Mouse in Port 0, Pad or Mouse or Super Scope in Port 1
                deviceSetting = iSuperScope;
                valid = (1 << iPad) | (1 << iMouse) | (1 << iMouse2) | (1 << iSuperScope);
                break;

            case 0x99: // Lasabirdie
                break;

            case 0x0A: // Barcode Battler
                break;

            default:
                break;
        }
    }

    ChangeInputDevice();
}

void DrawString(CGContextRef ctx, NSString *string, CGFloat size, CGFloat x, CGFloat y)
{
    NSAttributedString *astr = [[NSAttributedString alloc] initWithString:string attributes:@{NSFontAttributeName: [NSFont fontWithName:@"Helvetica" size:size], NSForegroundColorAttributeName: NSColor.whiteColor}];

    CTLineRef line = CTLineCreateWithAttributedString((__bridge CFAttributedStringRef)astr);
    CGFloat ascent = 0.0;
    CGFloat descent = 0.0;
    CGFloat leading = 0.0;
    CTLineGetTypographicBounds(line, &ascent, &descent, &leading);

    // Draw the text in the new CoreGraphics Context
    CGContextSetTextPosition(ctx, x, y + descent);
    CTLineDraw(line, ctx);
    CFRelease(line);
}

int PromptFreezeDefrost (Boolean freezing)
{
    OSStatus            err;
    CGContextRef        ctx;
    CGColorSpaceRef     color;
    CGDataProviderRef   prov;
    CGImageRef          image;
    CGRect              rct;
    CFURLRef            url;
    FSCatalogInfo       info;
    bool8               keys[MAC_MAX_PLAYERS][kNumButtons];
    bool8               gamepadButtons[MAC_MAX_PLAYERS][kNumButtons];
    CFAbsoluteTime      newestDate, currentDate;
    int64               startTime;
    float               x, y;
    int                 result, newestIndex, current_selection, oldInactiveMode;
    char                dateC[256];
    uint8               *back, *draw;

    const UInt32        repeatDelay = 200000;
    const int           w = SNES_WIDTH << 1, h = SNES_HEIGHT << 1;
    const char          letters[] = "123456789ABC", *filename;

    frzselecting = true;
	[s9xView updatePauseOverlay];
    oldInactiveMode = inactiveMode;
    if (inactiveMode == 3)
        inactiveMode = 2;

    S9xSetSoundMute(true);

    back = (uint8 *) malloc(w * h * 2);
    draw = (uint8 *) malloc(w * h * 2);
    if (!back || !draw)
        QuitWithFatalError(@"os 04");

    color = CGColorSpaceCreateDeviceRGB();
    if (!color)
        QuitWithFatalError(@"os 05");

    ctx = CGBitmapContextCreate(back, w, h, 5, w * 2, color, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Host);
    if (!ctx)
        QuitWithFatalError(@"os 06");

    rct = CGRectMake(0.0f, 0.0f, (float) w, (float) h);
    CGContextClearRect(ctx, rct);

    image = NULL;
    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.snes9x.macos.snes9x-framework"));

    if (freezing)
        url = CFBundleCopyResourceURL(bundle, CFSTR("logo_freeze"),  CFSTR("png"), NULL);
    else
        url = CFBundleCopyResourceURL(bundle, CFSTR("logo_defrost"), CFSTR("png"), NULL);
    if (url)
    {
        prov = CGDataProviderCreateWithURL(url);
        if (prov)
        {
            image = CGImageCreateWithPNGDataProvider(prov, NULL, true, kCGRenderingIntentDefault);
            CGDataProviderRelease(prov);
        }

        CFRelease(url);
    }

    if (image)
    {
        rct = CGRectMake(0.0f, (float) h - 88.0f, w, 88.0f);
        CGContextDrawImage(ctx, rct, image);
        CGImageRelease(image);
    }

    newestDate  = 0;
    newestIndex = -1;

    CGContextSetLineJoin(ctx, kCGLineJoinRound);

    rct = CGRectMake(0.0f, (float) h - 208.0f, 128.0f, 120.0f);

    for (int count = 0; count < 12; count++)
    {
        url = nil;
        filename = S9xGetFreezeFilename(count);
        CFStringRef cfFilename = CFStringCreateWithCString(kCFAllocatorDefault, filename, kCFStringEncodingUTF8);

        if (cfFilename != NULL)
        {
            url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfFilename, kCFURLPOSIXPathStyle, false);
            CFRelease(cfFilename);
        }

        if (url != NULL)
        {
            CFDateRef date = NULL;
            if (CFURLCopyResourcePropertyForKey(url, kCFURLAttributeModificationDateKey, &date, NULL))
            {
                currentDate = CFDateGetAbsoluteTime(date);
                CFRelease(date);
            }
            else
            {
                currentDate = DBL_MIN;
            }

            if (currentDate > newestDate)
            {
                newestIndex = count;
                newestDate  = currentDate;
            }

            DrawThumbnailFromExtendedAttribute(filename, ctx, rct);

            CGContextSetShouldAntialias(ctx, false);
            CGContextSetLineWidth(ctx, 1.0f);

            CGContextSetRGBStrokeColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
            x = rct.origin.x + 127.0f;
            y = rct.origin.y + 119.0f;
            CGContextBeginPath(ctx);
            CGContextMoveToPoint(ctx, x, y);
            CGContextAddLineToPoint(ctx, x,          y - 119.0f);
            CGContextAddLineToPoint(ctx, x - 127.0f, y - 119.0f);
            CGContextStrokePath(ctx);

            CGContextSetShouldAntialias(ctx, true);
            CGContextSetLineWidth(ctx, 3.0f);

            CGContextSetRGBFillColor(ctx, 1.0, 0.7, 0.7, 1.0);
            x = rct.origin.x +   5.0f;
            y = rct.origin.y + 102.0f;
            DrawString(ctx, [NSString stringWithFormat:@"%c", letters[count]], 12.0, x, y);

            if (showtimeinfrz)
            {
                CFAbsoluteTime        at;
                CFDateFormatterRef    format;
                CFLocaleRef            locale;
                CFStringRef            datstr;
                Boolean                r;

                err = UCConvertUTCDateTimeToCFAbsoluteTime(&(info.contentModDate), &at);
                locale = CFLocaleCopyCurrent();
                format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterMediumStyle);
                datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
                r = CFStringGetCString(datstr, dateC, sizeof(dateC), CFStringGetSystemEncoding());
                CFRelease(datstr);
                CFRelease(format);
                CFRelease(locale);

                x = rct.origin.x +  20.0f;
                y = rct.origin.y + 102.0f;
                DrawString(ctx, [NSString stringWithUTF8String:dateC], 10.0, x, y);
            }
        }
        else
        {
            x = rct.origin.x +   5.0f;
            y = rct.origin.y + 102.0f;
            DrawString(ctx, [NSString stringWithFormat:@"%c", letters[count]], 12.0, x, y);
        }

        if ((count % 4) == 3)
            rct = CGRectOffset(rct, -128.0f * 3.0f, -120.0f);
        else
            rct = CGRectOffset(rct, 128.0f, 0.0f);
    }

    if (newestIndex < 0)
        newestIndex = 0;

    CGContextRelease(ctx);

    image = NULL;

    prov = CGDataProviderCreateWithData(NULL, back, w * h * 2, NULL);
    if (prov)
    {
        image = CGImageCreate(w, h, 5, 16, w * 2, color, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Host, prov, NULL, 0, kCGRenderingIntentDefault);
        CGDataProviderRelease(prov);
    }

    if (!image)
        QuitWithFatalError(@"os 07");

    ctx = CGBitmapContextCreate(draw, w, h, 5, w * 2, color, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder16Host);
    if (!ctx)
        QuitWithFatalError(@"os 08");

    CGContextSetShouldAntialias(ctx, false);

    UpdateFreezeDefrostScreen(newestIndex, image, draw, ctx);

    CocoaPlayFreezeDefrostSound();

    result = -2;
    current_selection = newestIndex;

    do
    {
        if (!rejectinput)
        {
            CopyPressedKeys(keys, gamepadButtons);

            while (pressedRawKeyboardButtons[kVK_ANSI_1])
            {
                result = 0;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_2])
            {
                result = 1;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_3])
            {
                result = 2;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_4])
            {
                result = 3;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_5])
            {
                result = 4;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_6])
            {
                result = 5;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_7])
            {
                result = 6;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_8])
            {
                result = 7;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_9])
            {
                result = 8;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_A])
            {
                result = 9;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_B])
            {
                result = 10;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_ANSI_C])
            {
                result = 11;
                usleep(repeatDelay);
            }

            while (pressedRawKeyboardButtons[kVK_Return] || pressedRawKeyboardButtons[kVK_ANSI_KeypadEnter])
            {
                result = current_selection;
                usleep(repeatDelay);
            }

            while (KeyIsPressed(keys, gamepadButtons, 0, kRight))
            {
                startTime = mach_absolute_time();
                current_selection += 1;
                if (current_selection > 11)
                    current_selection -= 12;
                UpdateFreezeDefrostScreen(current_selection, image, draw, ctx);
                usleep(repeatDelay);
                CopyPressedKeys(keys, gamepadButtons);
            }

            while (KeyIsPressed(keys, gamepadButtons, 0, kLeft))
            {
                startTime = mach_absolute_time();
                current_selection -= 1;
                if (current_selection < 0)
                    current_selection += 12;
                UpdateFreezeDefrostScreen(current_selection, image, draw, ctx);
                usleep(repeatDelay);
                CopyPressedKeys(keys, gamepadButtons);
            }

            while (KeyIsPressed(keys, gamepadButtons, 0, kDown))
            {
                startTime = mach_absolute_time();
                current_selection += 4;
                if (current_selection > 11)
                    current_selection -= 12;
                UpdateFreezeDefrostScreen(current_selection, image, draw, ctx);
                usleep(repeatDelay);
                CopyPressedKeys(keys, gamepadButtons);
            }

            while (KeyIsPressed(keys, gamepadButtons, 0, kUp))
            {
                startTime = mach_absolute_time();
                current_selection -= 4;
                if (current_selection < 0)
                    current_selection += 12;
                UpdateFreezeDefrostScreen(current_selection, image, draw, ctx);
                usleep(repeatDelay);
                CopyPressedKeys(keys, gamepadButtons);
            }

            while (ISpKeyIsPressed(keys, gamepadButtons, kISpEsc))
            {
                result = -1;
                usleep(repeatDelay);
                CopyPressedKeys(keys, gamepadButtons);
            }

            while (KeyIsPressed(keys, gamepadButtons, 0, kA) ||
                   KeyIsPressed(keys, gamepadButtons, 1, kA) ||
                   KeyIsPressed(keys, gamepadButtons, 0, kB) ||
                   KeyIsPressed(keys, gamepadButtons, 1, kB) ||
                   KeyIsPressed(keys, gamepadButtons, 0, kX) ||
                   KeyIsPressed(keys, gamepadButtons, 1, kX) ||
                   KeyIsPressed(keys, gamepadButtons, 0, kY) ||
                   KeyIsPressed(keys, gamepadButtons, 1, kY))
            {
                result = current_selection;
                usleep(repeatDelay);
                CopyPressedKeys(keys, gamepadButtons);
            }
        }

        usleep(30000);

        UpdateFreezeDefrostScreen(current_selection, image, draw, ctx);
    } while (result == -2 && frzselecting);

    CocoaPlayFreezeDefrostSound();

    CGContextRelease(ctx);
    CGImageRelease(image);
    CGColorSpaceRelease(color);
    free(draw);
    free(back);

    S9xSetSoundMute(false);

    inactiveMode = oldInactiveMode;
    frzselecting = false;
	pauseEmulation = false;

	[s9xView updatePauseOverlay];

    return (result);
}

static void UpdateFreezeDefrostScreen (int newIndex, CGImageRef image, uint8 *draw, CGContextRef ctx)
{
    if (newIndex >= 0 && newIndex < 12)
    {
        CGRect      rct;
        const int   w = SNES_WIDTH << 1, h = SNES_HEIGHT << 1;

        CGContextSetLineWidth(ctx, 1.0f);

        rct = CGRectMake(0.0f, 0.0f, (float) w, (float) h);
        CGContextDrawImage(ctx, rct, image);

        rct = CGRectMake(0.0f, (float) h - 208.0f, 128.0f, 120.0f);
        rct = CGRectOffset(rct, (float) (128 * (newIndex % 4)), (float) (-120 * (newIndex / 4)));
        rct.size.width  -= 1.0f;
        rct.size.height -= 1.0f;

        CGContextSetRGBStrokeColor(ctx, 1.0f, 1.0f, 0.0f, 1.0f);
        CGContextStrokeRect(ctx, rct);
        rct = CGRectInset(rct, 1.0f, 1.0f);
        CGContextSetRGBStrokeColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);
        CGContextStrokeRect(ctx, rct);
    }

    DrawFreezeDefrostScreen(draw);
}

static void ProcessInput (void)
{
    bool8           keys[MAC_MAX_PLAYERS][kNumButtons];
    bool8           gamepadButtons[MAC_MAX_PLAYERS][kNumButtons];
    bool8           isok, fnbtn, altbtn, tcbtn;
    static bool8    toggleff = false, lastTimeTT = false, lastTimeFn = false, ffUp = false, ffDown = false;

    if (rejectinput)
        return;

    CopyPressedKeys(keys, gamepadButtons);

    fnbtn  = ISpKeyIsPressed(keys, gamepadButtons, kISpFunction);
    altbtn = ISpKeyIsPressed(keys, gamepadButtons, kISpAlt);

    if (fnbtn)
    {
        if (!lastTimeFn)
        {
            memset(heldFunctionButtons, 0, kNumFunctionButtons);
        }

        lastTimeFn = true;
        lastTimeTT = false;
        ffUp = ffDown = false;

        for (unsigned int i = 0; i < kNumFunctionButtons; i++)
        {
            if (pressedFunctionButtons[i])
            {
                if (!heldFunctionButtons[i])
                {
                    s9xcommand_t    s9xcmd;
                    static char     msg[64];

                    heldFunctionButtons[i] = true;

                    switch ((S9xFunctionButtonCommand) i)
                    {
                        case ToggleBG0:
                            s9xcmd = S9xGetCommandT("ToggleBG0");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case ToggleBG1:
                            s9xcmd = S9xGetCommandT("ToggleBG1");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case ToggleBG2:
                            s9xcmd = S9xGetCommandT("ToggleBG2");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case ToggleBG3:
                            s9xcmd = S9xGetCommandT("ToggleBG3");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case ToggleSprites:
                            s9xcmd = S9xGetCommandT("ToggleSprites");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SwapJoypads:
                            s9xcmd = S9xGetCommandT("SwapJoypads");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel0:
                            s9xcmd = S9xGetCommandT("SoundChannel0");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel1:
                            s9xcmd = S9xGetCommandT("SoundChannel1");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel2:
                            s9xcmd = S9xGetCommandT("SoundChannel2");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel3:
                            s9xcmd = S9xGetCommandT("SoundChannel3");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel4:
                            s9xcmd = S9xGetCommandT("SoundChannel4");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel5:
                            s9xcmd = S9xGetCommandT("SoundChannel5");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel6:
                            s9xcmd = S9xGetCommandT("SoundChannel6");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannel7:
                            s9xcmd = S9xGetCommandT("SoundChannel7");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case SoundChannelsOn:
                            s9xcmd = S9xGetCommandT("SoundChannelsOn");
                            S9xApplyCommand(s9xcmd, 1, 0);
                            break;

                        case ToggleDisplayPressedKeys:
                            Settings.DisplayPressedKeys = !Settings.DisplayPressedKeys;
                            break;

                        case ToggleDisplayMovieFrame:
                            if (S9xMovieActive())
                                Settings.DisplayMovieFrame = !Settings.DisplayMovieFrame;
                            break;

                        case IncreaseFrameAdvanceRate:
                            if (macFrameAdvanceRate < 5000000)
                                macFrameAdvanceRate += 100000;
                            sprintf(msg, "Emulation Speed: 100/%d", macFrameAdvanceRate / 10000);
                            S9xSetInfoString(msg);
                            break;

                        case DecreaseFrameAdvanceRate:
                            if (macFrameAdvanceRate > 500000)
                                macFrameAdvanceRate -= 100000;
                            sprintf(msg, "Emulation Speed: 100/%d", macFrameAdvanceRate / 10000);
                            S9xSetInfoString(msg);
                            break;

                        case ToggleEmulationPause:
                            pauseEmulation = !pauseEmulation;
							[s9xView updatePauseOverlay];
                            break;

                        case AdvanceFrame:
                            frameAdvance = true;
                            break;

                        case kNumFunctionButtons:
                            break;

                    }
                }
            }
        }
    }
    else
    {
        lastTimeFn = false;

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpEsc))
        {
            pauseEmulation = true;
			[s9xView updatePauseOverlay];

            dispatch_async(dispatch_get_main_queue(), ^
            {
                [s9xView setNeedsDisplay:YES];
            });
        }

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpFreeze))
        {
            MacStopSound();
            while (ISpKeyIsPressed(keys, gamepadButtons, kISpFreeze))
                CopyPressedKeys(keys, gamepadButtons);

            isok = SNES9X_Freeze();
            return;
        }

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpDefrost))
        {
            MacStopSound();
            while (ISpKeyIsPressed(keys, gamepadButtons, kISpDefrost))
                CopyPressedKeys(keys, gamepadButtons);

            isok = SNES9X_Defrost();
            return;
        }

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpScreenshot))
        {
            Settings.TakeScreenshot = true;
            while (ISpKeyIsPressed(keys, gamepadButtons, kISpScreenshot))
                CopyPressedKeys(keys, gamepadButtons);
        }

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpSPC))
        {
            S9xDumpSPCSnapshot();
            while (ISpKeyIsPressed(keys, gamepadButtons, kISpSPC))
                CopyPressedKeys(keys, gamepadButtons);
        }

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpFFUp))
        {
            if (!ffUp)
            {
                ChangeTurboRate(+1);
                ffUp = true;
            }
        }
        else
            ffUp = false;

        if (ISpKeyIsPressed(keys, gamepadButtons, kISpFFDown))
        {
            if (!ffDown)
            {
                ChangeTurboRate(-1);
                ffDown = true;
            }
        }
        else
            ffDown = false;

        for (int i = 0; i < MAC_MAX_PLAYERS; ++i)
        {
            controlPad[i] = 0;
            if (KeyIsPressed(keys, gamepadButtons, i, kR     ))    controlPad[i] |= 0x0010;
            if (KeyIsPressed(keys, gamepadButtons, i, kL     ))    controlPad[i] |= 0x0020;
            if (KeyIsPressed(keys, gamepadButtons, i, kX     ))    controlPad[i] |= 0x0040;
            if (KeyIsPressed(keys, gamepadButtons, i, kA     ))    controlPad[i] |= 0x0080;
            if (KeyIsPressed(keys, gamepadButtons, i, kRight ))    controlPad[i] |= 0x0100;
            if (KeyIsPressed(keys, gamepadButtons, i, kLeft  ))    controlPad[i] |= 0x0200;
            if (KeyIsPressed(keys, gamepadButtons, i, kDown  ))    controlPad[i] |= 0x0400;
            if (KeyIsPressed(keys, gamepadButtons, i, kUp    ))    controlPad[i] |= 0x0800;
            if (KeyIsPressed(keys, gamepadButtons, i, kStart ))    controlPad[i] |= 0x1000;
            if (KeyIsPressed(keys, gamepadButtons, i, kSelect))    controlPad[i] |= 0x2000;
            if (KeyIsPressed(keys, gamepadButtons, i, kY     ))    controlPad[i] |= 0x4000;
            if (KeyIsPressed(keys, gamepadButtons, i, kB     ))    controlPad[i] |= 0x8000;
        }

        if (altbtn)
        {
            if (!lastTimeTT)
                changeAuto[0] = changeAuto[1] = 0;

            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 12; j++)
                {
                    uint16    mask = 0x0010 << j;

                    if (controlPad[i] & mask & autofireRec[i].toggleMask)
                    {
                        controlPad[i] &= ~mask;

                        if (!(changeAuto[i] & mask))
                        {
                            changeAuto[i] |= mask;
                            ChangeAutofireSettings(i, j);
                        }
                    }
                    else
                        changeAuto[i] &= ~mask;
                }
            }

            lastTimeTT = true;
        }
        else
            lastTimeTT = false;
    }

    if (enabletoggle)
    {
        if (ISpKeyIsPressed(keys, gamepadButtons, kISpFastForward) && !fnbtn)
        {
            if (!toggleff)
            {
                toggleff = true;
                Settings.TurboMode = !Settings.TurboMode;
                S9xSetInfoString(Settings.TurboMode ? "Turbo mode on" : "Turbo mode off");
                if (!Settings.TurboMode)
                    S9xClearSamples();
            }
        }
        else
            toggleff = false;
    }
    else
    {
        bool8    old = Settings.TurboMode;
        Settings.TurboMode = (ISpKeyIsPressed(keys, gamepadButtons, kISpFastForward) && !fnbtn) ? true : false;
        if (!Settings.TurboMode && old)
            S9xClearSamples();
    }

    for (int i = 0; i < 2; i++)
        controlPad[i] ^= autofireRec[i].invertMask;

    if (autofire)
    {
        long long    currentTime;
        uint16        changeMask;

        currentTime = GetMicroseconds();
        tcbtn = (ISpKeyIsPressed(keys, gamepadButtons, kISpTC));

        for (int i = 0; i < 2; i++)
        {
            changeMask = (lastTimeTT ? (~changeAuto[i]) : 0xFFFF);

            for (int j = 0; j < 12; j++)
            {
                uint16    mask = (0x0010 << j) & changeMask;

                if (autofireRec[i].tcMask & mask)
                {
                    if (!tcbtn)
                        continue;
                }

                if (autofireRec[i].buttonMask & mask)
                {
                    if (controlPad[i] & mask)
                    {
                        if (currentTime > autofireRec[i].nextTime[j])
                        {
                            if (Settings.TurboMode)
                                autofireRec[i].nextTime[j] = currentTime + (long long) ((1.0 / (float) autofireRec[i].frequency) * 1000000.0 / macFastForwardRate);
                            else
                                autofireRec[i].nextTime[j] = currentTime + (long long) ((1.0 / (float) autofireRec[i].frequency) * 1000000.0);
                        }
                        else
                            controlPad[i] &= ~mask;
                    }
                }
            }
        }
    }

	for (int i = 0; i < MAC_MAX_PLAYERS; ++i)
	{
		ControlPadFlagsToS9xReportButtons(i, controlPad[i]);
	}

    if (macControllerOption == SNES_JUSTIFIER_2)
        ControlPadFlagsToS9xPseudoPointer(controlPad[1]);
}

static void ChangeAutofireSettings (int player, int btn)
{
    static char    msg[64];
    uint16        mask, m;

    mask = 0x0010 << btn;
    autofireRec[player].buttonMask ^= mask;
    autofire = (autofireRec[0].buttonMask || autofireRec[1].buttonMask);

    m = autofireRec[player].buttonMask;
    if (m)
        snprintf(msg, sizeof(msg), "Autofire %d:%s%s%s%s%s%s%s%s%s%s%s%s%s", player + 1,
            (m & 0xC0F0 ?   " " : ""),
            (m & 0x0080 ?   "A" : ""),
            (m & 0x8000 ?   "B" : ""),
            (m & 0x0040 ?   "X" : ""),
            (m & 0x4000 ?   "Y" : ""),
            (m & 0x0020 ?   "L" : ""),
            (m & 0x0010 ?   "R" : ""),
            (m & 0x0800 ? " Up" : ""),
            (m & 0x0400 ? " Dn" : ""),
            (m & 0x0200 ? " Lf" : ""),
            (m & 0x0100 ? " Rt" : ""),
            (m & 0x1000 ? " St" : ""),
            (m & 0x2000 ? " Se" : ""));
    else
        snprintf(msg, sizeof(msg), "Autofire %d: Off", player + 1);

    S9xSetInfoString(msg);
}

static void ChangeTurboRate (int d)
{
    static char    msg[64];

    macFastForwardRate += d;
    if (macFastForwardRate < 1)
        macFastForwardRate = 1;
    else
    if (macFastForwardRate > 15)
        macFastForwardRate = 15;

    snprintf(msg, sizeof(msg), "Turbo Rate: %d", macFastForwardRate);
    S9xSetInfoString(msg);
}

void GetGameScreenPointer (int16 *x, int16 *y, bool fullmouse)
{
    int    ph;

    ph = !drawoverscan ? ((IPPU.RenderedScreenHeight > 256) ? IPPU.RenderedScreenHeight : (IPPU.RenderedScreenHeight << 1)) : (SNES_HEIGHT_EXTENDED << 1);

    if (fullscreen)
    {
        if (glstretch)
        {
            float   fpw = (float) glScreenH / (float) ph * 512.0f;

            scopeViewInfo.width      = (int) (fpw + ((float) glScreenW - fpw) * (float) macAspectRatio / 10000.0);
            scopeViewInfo.height     = glScreenH;
            scopeViewInfo.globalLeft = (int) glScreenBounds.origin.x + ((glScreenW - scopeViewInfo.width) >> 1);
            scopeViewInfo.globalTop  = (int) glScreenBounds.origin.y;
        }
        else
        {
            scopeViewInfo.width      = 512;
            scopeViewInfo.height     = ph;
            scopeViewInfo.globalLeft = (int) glScreenBounds.origin.x + ((glScreenW - 512) >> 1);
            scopeViewInfo.globalTop  = (int) glScreenBounds.origin.y + ((glScreenH - ph ) >> 1);
        }
    }
    else
    {
        CGRect frame = s9xView.frame;
        frame = [s9xView convertRect:frame toView:nil];
        frame = [s9xView.window convertRectToScreen:frame];

        scopeViewInfo.width      = frame.size.width;
        scopeViewInfo.globalLeft = frame.origin.x;

        if (windowExtend)
        {
            scopeViewInfo.height    = ph * frame.size.height / kMacWindowHeight;
            scopeViewInfo.globalTop = frame.origin.y + ((kMacWindowHeight - ph) >> 1) * frame.size.height / kMacWindowHeight;
        }
        else
        {
            scopeViewInfo.height    = frame.size.height;
            scopeViewInfo.globalTop = frame.origin.y;
        }
    }

    if (!fullmouse)
    {
        CGPoint point = [NSEvent mouseLocation];

        *x = (int16) (((float) (point.x - scopeViewInfo.globalLeft)) / ((float) scopeViewInfo.width ) * (float) IPPU.RenderedScreenWidth);
        *y = (int16) (((float) (point.y - scopeViewInfo.globalTop )) / ((float) scopeViewInfo.height) * (float) (!drawoverscan ? IPPU.RenderedScreenHeight : SNES_HEIGHT_EXTENDED));
    }
    else
    {
        *x = (int16) (unlimitedCursor.x / (float) scopeViewInfo.width  * (float) IPPU.RenderedScreenWidth);
        *y = (int16) (unlimitedCursor.y / (float) scopeViewInfo.height * (float) (!drawoverscan ? IPPU.RenderedScreenHeight : SNES_HEIGHT_EXTENDED));
    }
}

static void Initialize (void)
{
	bzero(&Settings, sizeof(Settings));
	Settings.MouseMaster = true;
	Settings.SuperScopeMaster = true;
	Settings.JustifierMaster = true;
	Settings.MultiPlayer5Master = true;
	Settings.FrameTimePAL = 20000;
	Settings.FrameTimeNTSC = 16667;
	Settings.SixteenBitSound = true;
	Settings.Stereo = true;
	Settings.SoundPlaybackRate = 32000;
	Settings.SoundInputRate = 31950;
	Settings.SupportHiRes = true;
	Settings.Transparency = true;
	Settings.AutoDisplayMessages = true;
	Settings.InitialInfoStringTimeout = 120;
	Settings.HDMATimingHack = 100;
	Settings.BlockInvalidVRAMAccessMaster = true;
	Settings.StopEmulation = true;
	Settings.WrongMovieStateProtection = true;
	Settings.DumpStreamsMaxFrames = -1;
	Settings.StretchScreenshots = 1;
	Settings.SnapshotScreenshots = true;
	Settings.OpenGLEnable = true;
	Settings.SuperFXClockMultiplier = 100;
	Settings.InterpolationMethod = DSP_INTERPOLATION_GAUSSIAN;
	Settings.MaxSpriteTilesPerLine = 34;
	Settings.OneClockCycle = 6;
	Settings.OneSlowClockCycle = 8;
	Settings.TwoClockCycles = 12;

    mach_timebase_info_data_t info;
    mach_timebase_info(&info);

    machTimeNumerator = info.numer;
    machTimeDenominator = info.denom * 1000;

	npServerIP[0] = 0;
	npName[0] = 0;

	saveFolderPath = NULL;

	CreateIconImages();

	InitKeyboard();
	InitAutofire();
	InitCheatFinder();

	InitGraphics();
	InitMacSound();
	SetUpHID();

	InitMultiCart();

	autofire = (autofireRec[0].buttonMask || autofireRec[1].buttonMask) ? true : false;
	for (int a = 0; a < MAC_MAX_PLAYERS; a++)
		for (int b = 0; b < 12; b++)
			autofireRec[a].nextTime[b] = 0;

	S9xMovieInit();

	S9xUnmapAllControls();
	S9xSetupDefaultKeymap();
	ChangeInputDevice();

	if (!Memory.Init() || !S9xInitAPU() || !S9xGraphicsInit())
    {

    }

	frzselecting = false;
	[s9xView updatePauseOverlay];

	S9xSetControllerCrosshair(X_MOUSE1, 0, NULL, NULL);
	S9xSetControllerCrosshair(X_MOUSE2, 0, NULL, NULL);
}

static void Deinitialize (void)
{
	deviceSetting = deviceSettingMaster;

	DeinitMultiCart();
	ReleaseHID();
	DeinitCheatFinder();
	DeinitGraphics();
	DeinitKeyboard();
	DeinitMacSound();
	ReleaseIconImages();

	S9xGraphicsDeinit();
	S9xDeinitAPU();
	Memory.Deinit();
}

uint64 GetMicroseconds(void)
{
    uint64 ms = mach_absolute_time();
    ms *= machTimeNumerator;
    ms /= machTimeDenominator;

    return ms;
}

static void InitAutofire (void)
{
	autofire = false;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 12; j++)
			autofireRec[i].nextTime[j] = 0;

		autofireRec[i].buttonMask = 0x0000;
		autofireRec[i].toggleMask = 0xFFF0;
		autofireRec[i].tcMask     = 0x0000;
		autofireRec[i].invertMask = 0x0000;
		autofireRec[i].frequency  = 10;
	}
}

void S9xSyncSpeed (void)
{
	long long	currentFrame, adjustment;

	if (Settings.SoundSync)
	{
		while (!S9xSyncSound())
			usleep(0);
	}

	if (!macQTRecord)
	{
		if (macFrameSkip < 0)	// auto skip
		{
			skipFrames--;

			if (skipFrames <= 0)
			{
				adjustment = (Settings.TurboMode ? (macFrameAdvanceRate / macFastForwardRate) : macFrameAdvanceRate) / Memory.ROMFramesPerSecond;
				currentFrame = GetMicroseconds();

				skipFrames = (int32) ((currentFrame - lastFrame) / adjustment);
				lastFrame += frameCount * adjustment;

				if (skipFrames < 1)
					skipFrames = 1;
				else
				if (skipFrames > 7)
				{
					skipFrames = 7;
					lastFrame = GetMicroseconds();
				}

				frameCount = skipFrames;

				if (lastFrame > currentFrame)
					usleep((useconds_t) (lastFrame - currentFrame));

				IPPU.RenderThisFrame = true;
			}
			else
				IPPU.RenderThisFrame = false;
		}
		else					// constant
		{
			skipFrames--;

			if (skipFrames <= 0)
			{
				adjustment = macFrameAdvanceRate * macFrameSkip / Memory.ROMFramesPerSecond;
				currentFrame = GetMicroseconds();

				if (currentFrame - lastFrame < adjustment)
				{
					usleep((useconds_t) (adjustment + lastFrame - currentFrame));
					currentFrame = GetMicroseconds();
				}

				lastFrame = currentFrame;
				skipFrames = macFrameSkip;
				if (Settings.TurboMode)
					skipFrames *= macFastForwardRate;

				IPPU.RenderThisFrame = true;
			}
			else
				IPPU.RenderThisFrame = false;
		}
	}
	else
	{
		//MacQTRecordFrame(IPPU.RenderedScreenWidth, IPPU.RenderedScreenHeight);

		adjustment = macFrameAdvanceRate / Memory.ROMFramesPerSecond;
		currentFrame = GetMicroseconds();

		if (currentFrame - lastFrame < adjustment)
			usleep((useconds_t) (adjustment + lastFrame - currentFrame));

		lastFrame = currentFrame;

		IPPU.RenderThisFrame = true;
	}
}

void S9xAutoSaveSRAM (void)
{
    SNES9X_SaveSRAM();
}

void S9xMessage (int type, int number, const char *message)
{
	static char	mes[256];

	if (!onscreeninfo)
	{
		printf("%s\n", message);

//        if ((type == S9X_INFO) && (number == S9X_ROM_INFO))
//            if (strstr(message, "checksum ok") == NULL)
//                AppearanceAlert(kAlertCautionAlert, kS9xMacAlertkBadRom, kS9xMacAlertkBadRomHint);
	}
	else
	{
		strncpy(mes, message, 255);
		S9xSetInfoString(mes);
	}
}

const char * S9xStringInput (const char *s)
{
	return (NULL);
}

void S9xToggleSoundChannel (int c)
{
    static int	channel_enable = 255;

	if (c == 8)
		channel_enable = 255;
    else
		channel_enable ^= 1 << c;

	S9xSetSoundControl(channel_enable);
}

void S9xExit (void)
{
	NSBeep();

	running = false;
	cartOpen = false;
}

void QuitWithFatalError ( NSString *message)
{
    NSError *error = [NSError errorWithDomain:@"com.snes9x" code:0 userInfo:@{ NSLocalizedFailureReasonErrorKey: message }];
    NSAlert *alert = [NSAlert alertWithError:error];
    [alert runModal];
    [NSApp terminate:nil];
}

@implementation S9xView

+ (void)initialize
{
    keyLock = PTHREAD_MUTEX_INITIALIZER;
}

- (instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];

    if (self)
    {
        NSView *dimmedView = [[NSView alloc] initWithFrame:frameRect];
        dimmedView.wantsLayer = YES;
        dimmedView.layer.backgroundColor = NSColor.blackColor.CGColor;
        dimmedView.layer.opacity = 0.5;
        dimmedView.layer.zPosition = 100.0;
        dimmedView.translatesAutoresizingMaskIntoConstraints = NO;
        [self addSubview:dimmedView];

        [dimmedView.topAnchor constraintEqualToAnchor:self.topAnchor].active = YES;
        [dimmedView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor].active = YES;
        [dimmedView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor].active = YES;
        [dimmedView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor].active = YES;

        dimmedView.hidden = YES;
    }

    return self;
}

- (void)keyDown:(NSEvent *)event
{
    if (!NSApp.isActive)
    {
        return;
    }

    pthread_mutex_lock(&keyLock);
    S9xButton button = keyCodes[event.keyCode];
    if ( button.buttonCode >= 0 && button.buttonCode < kNumButtons && button.player >= 0 && button.player <= MAC_MAX_PLAYERS)
    {
        pressedKeys[button.player][button.buttonCode] = true;
    }

    for ( NSUInteger i = 0; i < kNumFunctionButtons; ++i )
    {
        if ( event.keyCode == functionButtons[i])
        {
            pressedFunctionButtons[i] = true;
            break;
        }
    }

    pressedRawKeyboardButtons[event.keyCode] = true;

    pthread_mutex_unlock(&keyLock);
}

- (void)keyUp:(NSEvent *)event
{
    if (!NSApp.isActive)
    {
        return;
    }

    pthread_mutex_lock(&keyLock);
    S9xButton button = keyCodes[event.keyCode];
    if ( button.buttonCode >= 0 && button.buttonCode < kNumButtons && button.player >= 0 && button.player <= MAC_MAX_PLAYERS)
    {
        pressedKeys[button.player][button.buttonCode] = false;
    }

    for ( NSUInteger i = 0; i < kNumFunctionButtons; ++i )
    {
        if ( event.keyCode == functionButtons[i])
        {
            pressedFunctionButtons[i] = false;
            heldFunctionButtons[i] = false;
            break;
        }
    }

    pressedRawKeyboardButtons[event.keyCode] = false;

    pthread_mutex_unlock(&keyLock);
}

- (void)flagsChanged:(NSEvent *)event
{
    if (!NSApp.isActive)
    {
        return;
    }

    pthread_mutex_lock(&keyLock);

    NSEventModifierFlags flags = event.modifierFlags;

    struct S9xButton button = keyCodes[kVK_Shift];
    if (button.buttonCode >= 0 && button.buttonCode < kNumButtons && button.player >= 0 && button.player < MAC_MAX_PLAYERS)
    {
        pressedKeys[button.player][button.buttonCode] = (flags & NSEventModifierFlagShift) != 0;
    }

    button = keyCodes[kVK_Command];
    if (button.buttonCode >= 0 && button.buttonCode < kNumButtons && button.player >= 0 && button.player < MAC_MAX_PLAYERS)
    {
        pressedKeys[button.player][button.buttonCode] = (flags & NSEventModifierFlagCommand) != 0;
    }

    button = keyCodes[kVK_Control];
    if (button.buttonCode >= 0 && button.buttonCode < kNumButtons && button.player >= 0 && button.player < MAC_MAX_PLAYERS)
    {
        pressedKeys[button.player][button.buttonCode] = (flags & NSEventModifierFlagControl) != 0;
    }

    button = keyCodes[kVK_Option];
    if (button.buttonCode >= 0 && button.buttonCode < kNumButtons && button.player >= 0 && button.player < MAC_MAX_PLAYERS)
    {
        pressedKeys[button.player][button.buttonCode] = (flags & NSEventModifierFlagOption) != 0;
    }

    pthread_mutex_unlock(&keyLock);
}

- (void)mouseDown:(NSEvent *)event
{
    pauseEmulation = true;
	[s9xView updatePauseOverlay];
}

- (void)updatePauseOverlay
{
	dispatch_async(dispatch_get_main_queue(), ^{
		self.subviews[0].hidden = (frzselecting || !pauseEmulation);
		CGFloat scaleFactor = MAX(self.window.backingScaleFactor, 1.0);
		glScreenW = self.frame.size.width * scaleFactor;
		glScreenH = self.frame.size.height * scaleFactor;
	});
}

- (void)setFrame:(NSRect)frame
{
    if ( !NSEqualRects(frame, self.frame) )
    {
        [super setFrame:frame];
    }
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return !( running && !pauseEmulation);
}

@end

@implementation S9xEngine

- (instancetype)init
{
    if (self = [super init])
    {
        Initialize();
		[self recreateS9xView];
    }

    return self;
}

- (void)dealloc
{
    Deinitialize();
}

- (void)recreateS9xView
{
	[s9xView removeFromSuperview];
	S9xDeinitDisplay();
	CGRect frame = NSMakeRect(0, 0, SNES_WIDTH * 2, SNES_HEIGHT * 2);
	s9xView = [[S9xView alloc] initWithFrame:frame];
	s9xView.translatesAutoresizingMaskIntoConstraints = NO;
	s9xView.autoresizingMask = NSViewWidthSizable|NSViewHeightSizable;
	[s9xView addConstraint:[NSLayoutConstraint constraintWithItem:s9xView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:s9xView attribute:NSLayoutAttributeWidth multiplier:(CGFloat)SNES_HEIGHT/(CGFloat)SNES_WIDTH constant:0.0]];
	[s9xView addConstraint:[NSLayoutConstraint constraintWithItem:s9xView attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:SNES_WIDTH * 2.0]];
	[s9xView addConstraint:[NSLayoutConstraint constraintWithItem:s9xView attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationGreaterThanOrEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0 constant:SNES_HEIGHT * 2.0]];
	s9xView.device = MTLCreateSystemDefaultDevice();
	S9xInitDisplay(NULL, NULL);
}

- (void)start
{
#ifdef DEBUGGER
	CPU.Flags |= DEBUG_MODE_FLAG;
	S9xDoDebug();
#endif

	lastFrame = GetMicroseconds();
	frameCount = 0;
	if (macFrameSkip < 0)
		skipFrames = 3;
	else
		skipFrames = macFrameSkip;

	S9xInitDisplay(NULL, NULL);

	[NSThread detachNewThreadWithBlock:^
	{
		MacSnes9xThread(NULL);
	}];
}

- (void)stop
{
	SNES9X_Quit();
    S9xExit();
}

- (void)softwareReset
{
	SNES9X_SoftReset();
	SNES9X_Go();
	[self resume];
}


- (void)hardwareReset
{
	SNES9X_Reset();
	SNES9X_Go();
	[self resume];
}

- (BOOL)isRunning
{
    return running;
}

- (BOOL)isPaused
{
    return running && pauseEmulation;
}

- (void)pause
{
    pauseEmulation = true;
    [s9xView updatePauseOverlay];
}

- (void)quit
{
	SNES9X_Quit();
	[self pause];
}

- (void)resume
{
    pauseEmulation = false;
	[s9xView updatePauseOverlay];
}

- (NSArray<S9xJoypad *> *)listJoypads
{
    pthread_mutex_lock(&keyLock);
    NSMutableArray<S9xJoypad *> *joypads = [NSMutableArray new];
    for (auto joypadStruct : ListJoypads())
    {
        S9xJoypad *joypad = [S9xJoypad new];
        joypad.vendorID = joypadStruct.vendorID;
        joypad.productID = joypadStruct.productID;
        joypad.index = joypadStruct.index;
        joypad.name = [[NSString alloc] initWithUTF8String:NameForDevice(joypadStruct).c_str()];

        [joypads addObject:joypad];
    }

    [joypads sortUsingComparator:^NSComparisonResult(S9xJoypad *a, S9xJoypad *b)
    {
        NSComparisonResult result = [a.name compare:b.name];

        if ( result == NSOrderedSame )
        {
            result = [@(a.vendorID) compare:@(b.vendorID)];
        }

        if ( result == NSOrderedSame )
        {
            result = [@(a.productID) compare:@(b.productID)];
        }

        if ( result == NSOrderedSame )
        {
            result = [@(a.index) compare:@(b.index)];
        }

        return result;
    }];
    pthread_mutex_unlock(&keyLock);

    return joypads;
}

- (void)setPlayer:(int8)player forVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index oldPlayer:(int8 *)oldPlayer
{
    pthread_mutex_lock(&keyLock);
    SetPlayerForJoypad(player, vendorID, productID, index, oldPlayer);
    pthread_mutex_unlock(&keyLock);
}

- (BOOL)setButton:(S9xButtonCode)button forVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index cookie:(uint32)cookie value:(int32)value oldButton:(S9xButtonCode *)oldButton
{
    BOOL result = NO;
    pthread_mutex_lock(&keyLock);
    result = SetButtonCodeForJoypadControl(vendorID, productID, index, cookie, value, button, true, oldButton);
    pthread_mutex_unlock(&keyLock);
    return result;
}

- (void)clearJoypadForVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index
{
    pthread_mutex_lock(&keyLock);
    ClearJoypad(vendorID, productID, index);
    pthread_mutex_unlock(&keyLock);
}

- (void)clearJoypadForVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index buttonCode:(S9xButtonCode)buttonCode
{
    pthread_mutex_lock(&keyLock);
    ClearButtonCodeForJoypad(vendorID, productID, index, buttonCode);
    pthread_mutex_unlock(&keyLock);
}

- (NSArray<S9xJoypadInput *> *)getInputsForVendorID:(uint32)vendorID productID:(uint32)productID index:(uint32)index
{
    pthread_mutex_lock(&keyLock);
    NSMutableArray<S9xJoypadInput *> *inputs = [NSMutableArray new];
    std::unordered_map<struct JoypadInput, S9xButtonCode> buttonCodeMap = GetJuypadButtons(vendorID, productID, index);
    for (auto it = buttonCodeMap.begin(); it != buttonCodeMap.end(); ++it)
    {
        S9xJoypadInput *input = [S9xJoypadInput new];
        input.cookie = it->first.cookie.cookie;
        input.value = it->first.value;
        input.buttonCode = it->second;

        [inputs addObject:input];
    }
    pthread_mutex_unlock(&keyLock);

    return inputs;
}

- (NSString *)labelForVendorID:(uint32)vendorID productID:(uint32)productID cookie:(uint32)cookie value:(int32)value
{
    return [NSString stringWithUTF8String:LabelForInput(vendorID, productID, cookie, value).c_str()];
}

- (BOOL)setButton:(S9xButtonCode)button forKey:(int16)key player:(int8)player oldButton:(S9xButtonCode *)oldButton oldPlayer:(int8 *)oldPlayer oldKey:(int16 *)oldKey
{
    BOOL result = NO;
    pthread_mutex_lock(&keyLock);
    result = SetKeyCode(key, button, player, oldKey, oldButton, oldPlayer);
    pthread_mutex_unlock(&keyLock);
    return result;
}

- (void)clearButton:(S9xButtonCode)button forPlayer:(int8)player
{
    pthread_mutex_lock(&keyLock);
    ClearKeyCode(button, player);
    pthread_mutex_unlock(&keyLock);
}

- (BOOL)loadROM:(NSURL *)fileURL
{
	running = false;
	frzselecting = false;

	while (!Settings.StopEmulation)
	{
		usleep(Settings.FrameTime);
	}

    if ( SNES9X_OpenCart(fileURL) )
    {
        SNES9X_Go();
        s9xView.window.title = fileURL.lastPathComponent.stringByDeletingPathExtension;
        [s9xView.window makeKeyAndOrderFront:nil];

		dispatch_async(dispatch_get_main_queue(), ^
		{
			[s9xView.window makeFirstResponder:s9xView];
		});

        [self start];
        return YES;
    }

    return NO;
}

- (void)setShowFPS:(BOOL)showFPS
{
    Settings.DisplayFrameRate = showFPS;
}

- (void)setVideoMode:(int)mode
{
    videoMode = mode;
}

@dynamic inputDelegate;
- (void)setInputDelegate:(id<S9xInputDelegate>)delegate
{
    inputDelegate = delegate;
}

- (id<S9xInputDelegate>)inputDelegate
{
    return inputDelegate;
}

@end

@implementation S9xJoypad

- (BOOL)isEqual:(id)object
{
    if (![object isKindOfClass:[self class]])
    {
        return NO;
    }

    S9xJoypad *other = (S9xJoypad *)object;
    return (self.vendorID == other.vendorID && self.productID == other.productID && self.index == other.index);
}

@end

@implementation S9xJoypadInput
@end
