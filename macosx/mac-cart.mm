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


#import "snes9x.h"
#import "memmap.h"
#import "snapshot.h"
#import "movie.h"

#import <wchar.h>
#import <Cocoa/Cocoa.h>
#import <objc/objc-runtime.h>

#import "mac-cocoatools.h"
#import "mac-prefix.h"
#import "mac-dialog.h"
#import "mac-os.h"
#import "mac-screenshot.h"
#import "mac-snes9x.h"
#import "mac-stringtools.h"
#import "mac-cart.h"

typedef struct
{
	NSURL		    *ref;
	Boolean			reply;
	char			name[PATH_MAX + 1];
#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
	IBNibRef		customNib;
	WindowRef		customWindow;
	HIViewRef		customPane;
	short			customWidth, customHeight;
#endif
}	NavState;

extern wchar_t	macRecordWChar[MOVIE_MAX_METADATA];

static NSPopUpButton *gPopM, *gPopI, *gPopV, *gPopH;

@interface NavController : NSObject

- (id) init;
- (IBAction) handleCompressButton: (id) sender;

@end

@implementation NavController

- (id) init
{
	self = [super init];
	return (self);
}

- (IBAction) handleCompressButton: (id) sender
{
	//MacQTVideoConfig();
}

@end


NSURL *NavOpenROMImage ()
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.title = NSLocalizedString(@"OpenROMMes", @"Open");
    openPanel.allowsMultipleSelection = NO;

    NSView				*view;
    NSTextField			*txtM, *txtI, *txtV, *txtH;
    NSPopUpButton		*popM, *popI, *popV, *popH;
    NSArray				*aryM, *aryI, *aryV, *aryH;

    view = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 52)];

    aryM = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM6", @""), NSLocalizedString(@"OpenROM7", @""), nil];
    aryI = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMa", @""), NSLocalizedString(@"OpenROMb", @""), NSLocalizedString(@"OpenROMc", @""), NSLocalizedString(@"OpenROMd", @""), nil];
    aryV = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM8", @""), NSLocalizedString(@"OpenROM9", @""), nil];
    aryH = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMe", @""), NSLocalizedString(@"OpenROMf", @""), nil];

    CocoaAddStatTextToView(view, @"OpenROM1",   5, 32,  84, 14, &txtM);
    CocoaAddStatTextToView(view, @"OpenROM3", 214, 32,  96, 14, &txtI);
    CocoaAddStatTextToView(view, @"OpenROM2",   5,  7,  84, 14, &txtV);
    CocoaAddStatTextToView(view, @"OpenROM4", 214,  7,  96, 14, &txtH);
    CocoaAddPopUpBtnToView(view, aryM,         91, 27, 116, 22, &popM);
    CocoaAddPopUpBtnToView(view, aryI,        312, 27, 116, 22, &popI);
    CocoaAddPopUpBtnToView(view, aryV,         91,  2, 116, 22, &popV);
    CocoaAddPopUpBtnToView(view, aryH,        312,  2, 116, 22, &popH);

    [txtM setAlignment: NSTextAlignmentRight];
    [txtI setAlignment: NSTextAlignmentRight];
    [txtV setAlignment: NSTextAlignmentRight];
    [txtH setAlignment: NSTextAlignmentRight];

    [popM selectItemAtIndex: romDetect       ];
    [popI selectItemAtIndex: interleaveDetect];
    [popV selectItemAtIndex: videoDetect     ];
    [popH selectItemAtIndex: headerDetect    ];

        NSModalResponse response = [openPanel runModal];

    if ( response == NSModalResponseOK )
    {
        romDetect        = [popM indexOfSelectedItem];
        interleaveDetect = [popI indexOfSelectedItem];
        videoDetect      = [popV indexOfSelectedItem];
        headerDetect     = [popH indexOfSelectedItem];

        return openPanel.URL;
    }

    return nil;
}

NSURL *NavOpenROMImageSheet (NSWindow *parent, NSString *mes)
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.title = NSLocalizedString(@"OpenROMMes", @"Open");
    openPanel.allowsMultipleSelection = NO;

    NSView                *view;
    NSTextField            *txtM, *txtI, *txtV, *txtH;
    NSPopUpButton        *popM, *popI, *popV, *popH;
    NSArray                *aryM, *aryI, *aryV, *aryH;

    view = [[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 52)];

    aryM = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM6", @""), NSLocalizedString(@"OpenROM7", @""), nil];
    aryI = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMa", @""), NSLocalizedString(@"OpenROMb", @""), NSLocalizedString(@"OpenROMc", @""), NSLocalizedString(@"OpenROMd", @""), nil];
    aryV = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROM8", @""), NSLocalizedString(@"OpenROM9", @""), nil];
    aryH = [NSArray arrayWithObjects: NSLocalizedString(@"OpenROM5", @""), @"---", NSLocalizedString(@"OpenROMe", @""), NSLocalizedString(@"OpenROMf", @""), nil];

    CocoaAddStatTextToView(view, @"OpenROM1",   5, 32,  84, 14, &txtM);
    CocoaAddStatTextToView(view, @"OpenROM3", 214, 32,  96, 14, &txtI);
    CocoaAddStatTextToView(view, @"OpenROM2",   5,  7,  84, 14, &txtV);
    CocoaAddStatTextToView(view, @"OpenROM4", 214,  7,  96, 14, &txtH);
    CocoaAddPopUpBtnToView(view, aryM,         91, 27, 116, 22, &popM);
    CocoaAddPopUpBtnToView(view, aryI,        312, 27, 116, 22, &popI);
    CocoaAddPopUpBtnToView(view, aryV,         91,  2, 116, 22, &popV);
    CocoaAddPopUpBtnToView(view, aryH,        312,  2, 116, 22, &popH);

    [txtM setAlignment: NSTextAlignmentRight];
    [txtI setAlignment: NSTextAlignmentRight];
    [txtV setAlignment: NSTextAlignmentRight];
    [txtH setAlignment: NSTextAlignmentRight];

    [popM selectItemAtIndex: romDetect       ];
    [popI selectItemAtIndex: interleaveDetect];
    [popV selectItemAtIndex: videoDetect     ];
    [popH selectItemAtIndex: headerDetect    ];

    NSModalResponse response = [openPanel runModal];

    if ( response == NSModalResponseOK )
    {
        romDetect        = [popM indexOfSelectedItem];
        interleaveDetect = [popI indexOfSelectedItem];
        videoDetect      = [popV indexOfSelectedItem];
        headerDetect     = [popH indexOfSelectedItem];

        return openPanel.URL;
    }

    return nil;
}

NSURL *NavFreezeTo ()
{
//    OSStatus                    err;
//    NavState                    nav;
//    CFStringRef                    numRef, romRef, baseRef;
//    CFMutableStringRef            mesRef, saveRef;
//    SInt32                        replaceAt;
//    char                        drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//
//    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//    romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
//    numRef  = CFCopyLocalizedString(CFSTR("FreezeToPos"), "1");
//    baseRef = CFCopyLocalizedString(CFSTR("FreezeToMes"), "Freeze");
//    mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
//    saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
//    replaceAt = CFStringGetIntValue(numRef);
//    CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
//    CFStringAppendCString(saveRef, ".frz", CFStringGetSystemEncoding());
//
//    err = NavGetDefaultDialogCreationOptions(&dialogOptions);
//    dialogOptions.preferenceKey = 4;
//    dialogOptions.clientName = kMacS9XCFString;
//    dialogOptions.windowTitle = mesRef;
//    dialogOptions.saveFileName = saveRef;
//    dialogOptions.modality = kWindowModalityAppModal;
//    dialogOptions.parentWindow = NULL;
//    nav.parent = NULL;
//
//    eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);
//
//    err = NavCreatePutFileDialog(&dialogOptions, 'SAVE', '~9X~', eventUPP, &nav, &(nav.nref));
//    if (err == noErr)
//    {
//        err = NavDialogRun(nav.nref);
//        if (err)
//            NavDialogDispose(nav.nref);
//    }
//
//    DisposeNavEventUPP(eventUPP);
//
//    CFRelease(saveRef);
//    CFRelease(mesRef);
//    CFRelease(baseRef);
//    CFRelease(numRef);
//    CFRelease(romRef);
//
//    if (err)
//        return (false);
//    else
//    {
//        if (nav.reply)
//        {
//            char    s[PATH_MAX + 1];
//
//            err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
//            snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);
//
//            return (true);
//        }
//        else
//            return (false);
//    }
    return nil;
}

NSURL *NavDefrostFrom ()
{
//    OSStatus                    err;
//    NavDialogCreationOptions    dialogOptions;
//    NavEventUPP                    eventUPP;
//    NavObjectFilterUPP            filterUPP;
//    NavState                    nav;
//    CFStringRef                    numRef, romRef, baseRef;
//    CFMutableStringRef            mesRef;
//    SInt32                        replaceAt;
//    char                        drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    NavPreviewUPP                previewUPP;
//#endif
//
//    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//    romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
//    numRef  = CFCopyLocalizedString(CFSTR("DefrostFromPos"), "1");
//    baseRef = CFCopyLocalizedString(CFSTR("DefrostFromMes"), "Defrost");
//    mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
//    replaceAt = CFStringGetIntValue(numRef);
//    CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
//
//    err = NavGetDefaultDialogCreationOptions(&dialogOptions);
//    dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
//    dialogOptions.preferenceKey = 5;
//    dialogOptions.clientName = kMacS9XCFString;
//    dialogOptions.windowTitle = mesRef;
//    dialogOptions.modality = kWindowModalityAppModal;
//    dialogOptions.parentWindow = NULL;
//    nav.parent = NULL;
//
//    eventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
//    filterUPP = NewNavObjectFilterUPP(NavDefrostFromFilter);
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    previewUPP = NewNavPreviewUPP(NavDefrostFromPreview);
//#endif
//
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, previewUPP, filterUPP, &nav, &(nav.nref));
//#else
//    err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, NULL, filterUPP, &nav, &(nav.nref));
//#endif
//    if (err == noErr)
//    {
//        err = NavDialogRun(nav.nref);
//        if (err)
//            NavDialogDispose(nav.nref);
//    }
//
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    DisposeNavPreviewUPP(previewUPP);
//#endif
//    DisposeNavObjectFilterUPP(filterUPP);
//    DisposeNavEventUPP(eventUPP);
//
//    CFRelease(mesRef);
//    CFRelease(baseRef);
//    CFRelease(numRef);
//    CFRelease(romRef);
//
//    if (err)
//        return (false);
//    else
//    {
//        if (nav.reply)
//        {
//            err = FSRefMakePath(&(nav.ref), (unsigned char *) path, PATH_MAX);
//            return (true);
//        }
//        else
//            return (false);
//    }
    return nil;
}

NSURL *NavRecordMovieTo ()
{
//    OSStatus                    err;
//    NavDialogCreationOptions    dialogOptions;
//    NavEventUPP                    eventUPP;
//    NavState                    nav;
//    CFStringRef                    numRef, romRef, baseRef;
//    CFMutableStringRef            mesRef, saveRef;
//    SInt32                        replaceAt;
//    char                        drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    if (systemVersion < 0x1060)
//    {
//        HIViewID    cid = { 'PANE', 1000 };
//        Rect        rct;
//
//        err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
//        if (err)
//            return (false);
//
//        err = CreateWindowFromNib(nav.customNib, CFSTR("RecordSMV"), &(nav.customWindow));
//        if (err)
//        {
//            DisposeNibReference(nav.customNib);
//            return (false);
//        }
//
//        GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
//        nav.customWidth  = rct.right  - rct.left;
//        nav.customHeight = rct.bottom - rct.top;
//
//        HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
//    }
//#endif
//
//    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//    romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
//    numRef  = CFCopyLocalizedString(CFSTR("RecordToPos"), "1");
//    baseRef = CFCopyLocalizedString(CFSTR("RecordToMes"), "Record");
//    mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
//    saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
//    replaceAt = CFStringGetIntValue(numRef);
//    CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
//    CFStringAppendCString(saveRef, ".smv", CFStringGetSystemEncoding());
//
//    err = NavGetDefaultDialogCreationOptions(&dialogOptions);
//    dialogOptions.preferenceKey = 6;
//    dialogOptions.clientName = kMacS9XCFString;
//    dialogOptions.windowTitle = mesRef;
//    dialogOptions.saveFileName = saveRef;
//    dialogOptions.modality = kWindowModalityAppModal;
//    dialogOptions.parentWindow = NULL;
//    nav.parent = NULL;
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavRecordMovieToEventHandler : NavGenericSaveEventHandler);
//#else
//    eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);
//#endif
//
//    err = NavCreatePutFileDialog(&dialogOptions, 'SMOV', '~9X~', eventUPP, &nav, &(nav.nref));
//    if (err == noErr)
//    {
//        if (systemVersion >= 0x1060)
//        {
//            NSAutoreleasePool    *pool;
//            NSView                *view;
//            NSTextField            *text;
//            NSButton            *cb1p, *cb2p, *cb3p, *cb4p, *cb5p, *cbre;
//            NSString            *comment;
//
//            pool = [[NSAutoreleasePool alloc] init];
//
//            view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 54)] autorelease];
//
//            CocoaAddStatTextToView(view, @"RecordSMV6",   5, 33, 104, 14, NULL);
//            CocoaAddStatTextToView(view, @"RecordSMV7",   5,  8,  63, 14, NULL);
//            CocoaAddEditTextToView(view, @"",            74,  6, 351, 19, &text);
//            CocoaAddCheckBoxToView(view, @"RecordSMV1", 112, 31,  29, 18, &cb1p);
//            CocoaAddCheckBoxToView(view, @"RecordSMV2", 150, 31,  29, 18, &cb2p);
//            CocoaAddCheckBoxToView(view, @"RecordSMV3", 188, 31,  29, 18, &cb3p);
//            CocoaAddCheckBoxToView(view, @"RecordSMV4", 226, 31,  29, 18, &cb4p);
//            CocoaAddCheckBoxToView(view, @"RecordSMV5", 264, 31,  29, 18, &cb5p);
//            CocoaAddCheckBoxToView(view, @"RecordSMV8", 356, 31,  51, 18, &cbre);
//
//            [cb1p setState: (macRecordFlag & (1 << 0) ? NSOnState : NSOffState)];
//            [cb2p setState: (macRecordFlag & (1 << 1) ? NSOnState : NSOffState)];
//            [cb3p setState: (macRecordFlag & (1 << 2) ? NSOnState : NSOffState)];
//            [cb4p setState: (macRecordFlag & (1 << 3) ? NSOnState : NSOffState)];
//            [cb5p setState: (macRecordFlag & (1 << 4) ? NSOnState : NSOffState)];
//            [cbre setState: (macRecordFlag & (1 << 5) ? NSOnState : NSOffState)];
//
//            [(NSOpenPanel *) nav.nref setAccessoryView: view];
//
//            err = NavDialogRun(nav.nref);
//            if (err)
//                NavDialogDispose(nav.nref);
//
//            macRecordFlag  = 0;
//            macRecordFlag |= ((([cb1p state] == NSOnState) ? 1 : 0) << 0);
//            macRecordFlag |= ((([cb2p state] == NSOnState) ? 1 : 0) << 1);
//            macRecordFlag |= ((([cb3p state] == NSOnState) ? 1 : 0) << 2);
//            macRecordFlag |= ((([cb4p state] == NSOnState) ? 1 : 0) << 3);
//            macRecordFlag |= ((([cb5p state] == NSOnState) ? 1 : 0) << 4);
//            macRecordFlag |= ((([cbre state] == NSOnState) ? 1 : 0) << 5);
//
//            comment = [text stringValue];
//            if (comment)
//            {
//                NSUInteger    cflen;
//                unichar        unistr[MOVIE_MAX_METADATA];
//
//                cflen = [comment length];
//                if (cflen > MOVIE_MAX_METADATA - 1)
//                    cflen = MOVIE_MAX_METADATA - 1;
//
//                [comment getCharacters: unistr range: NSMakeRange(0, cflen)];
//
//                for (unsigned i = 0; i < cflen; i++)
//                    macRecordWChar[i] = (wchar_t) unistr[i];
//                macRecordWChar[cflen] = 0;
//            }
//            else
//                macRecordWChar[0] = 0;
//
//            [pool release];
//        }
//    #ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//        else
//        {
//            err = NavDialogRun(nav.nref);
//            if (err)
//                NavDialogDispose(nav.nref);
//        }
//    #endif
//    }
//
//    DisposeNavEventUPP(eventUPP);
//
//    CFRelease(saveRef);
//    CFRelease(mesRef);
//    CFRelease(baseRef);
//    CFRelease(numRef);
//    CFRelease(romRef);
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    if (systemVersion < 0x1060)
//    {
//        CFRelease(nav.customWindow);
//        DisposeNibReference(nav.customNib);
//    }
//#endif
//
//    if (err)
//        return (false);
//    else
//    {
//        if (nav.reply)
//        {
//            char    s[PATH_MAX + 1];
//
//            err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
//            snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);
//
//            return (true);
//        }
//        else
//            return (false);
//    }
    return nil;
}

NSURL *NavPlayMovieFrom ()
{
//    OSStatus                    err;
//    NavDialogCreationOptions    dialogOptions;
//    NavEventUPP                    eventUPP;
//    NavObjectFilterUPP            filterUPP;
//    NavState                    nav;
//    CFStringRef                    numRef, romRef, baseRef;
//    CFMutableStringRef            mesRef;
//    SInt32                        replaceAt;
//    char                        drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    NavPreviewUPP                previewUPP;
//#endif
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    if (systemVersion < 0x1060)
//    {
//        HIViewID    cid = { 'PANE', 1000 };
//        Rect        rct;
//
//        err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
//        if (err)
//            return (false);
//
//        err = CreateWindowFromNib(nav.customNib, CFSTR("PlaySMV"), &(nav.customWindow));
//        if (err)
//        {
//            DisposeNibReference(nav.customNib);
//            return (false);
//        }
//
//        GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
//        nav.customWidth  = rct.right  - rct.left;
//        nav.customHeight = rct.bottom - rct.top;
//
//        HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
//    }
//#endif
//
//    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//    romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
//    numRef  = CFCopyLocalizedString(CFSTR("PlayFromPos"), "1");
//    baseRef = CFCopyLocalizedString(CFSTR("PlayFromMes"), "Play");
//    mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
//    replaceAt = CFStringGetIntValue(numRef);
//    CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
//
//    err = NavGetDefaultDialogCreationOptions(&dialogOptions);
//    dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
//    dialogOptions.preferenceKey = 6;
//    dialogOptions.clientName = kMacS9XCFString;
//    dialogOptions.windowTitle = mesRef;
//    dialogOptions.modality = kWindowModalityAppModal;
//    dialogOptions.parentWindow = NULL;
//    nav.parent = NULL;
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavPlayMovieFromEventHandler : NavGenericOpenEventHandler);
//#else
//    eventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
//#endif
//    filterUPP = NewNavObjectFilterUPP(NavPlayMovieFromFilter);
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    previewUPP = NewNavPreviewUPP(NavPlayMovieFromPreview);
//#endif
//
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, previewUPP, filterUPP, &nav, &(nav.nref));
//#else
//    err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, NULL, filterUPP, &nav, &(nav.nref));
//#endif
//    if (err == noErr)
//    {
//        if (systemVersion >= 0x1060)
//        {
//            NSAutoreleasePool    *pool;
//            NSView                *view;
//            NSButton            *cbro, *cbqt;
//
//            pool = [[NSAutoreleasePool alloc] init];
//
//            view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 326, 29)] autorelease];
//
//            CocoaAddCheckBoxToView(view, @"PlaySMV1",   5, 6,  76, 18, &cbro);
//            CocoaAddCheckBoxToView(view, @"PlaySMV2", 100, 6, 220, 18, &cbqt);
//
//            [cbro setState: (macPlayFlag & (1 << 0) ? NSOnState : NSOffState)];
//            [cbqt setState: (macPlayFlag & (1 << 1) ? NSOnState : NSOffState)];
//
//            [(NSOpenPanel *) nav.nref setAccessoryView: view];
//
//            err = NavDialogRun(nav.nref);
//            if (err)
//                NavDialogDispose(nav.nref);
//
//            macPlayFlag  = 0;
//            macPlayFlag |= ((([cbro state] == NSOnState) ? 1 : 0) << 0);
//            macPlayFlag |= ((([cbqt state] == NSOnState) ? 1 : 0) << 1);
//
//            [pool release];
//        }
//    #ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//        else
//        {
//            err = NavDialogRun(nav.nref);
//            if (err)
//                NavDialogDispose(nav.nref);
//        }
//    #endif
//    }
//
//#ifdef MAC_TIGER_PANTHER_SUPPORT
//    DisposeNavPreviewUPP(previewUPP);
//#endif
//    DisposeNavObjectFilterUPP(filterUPP);
//    DisposeNavEventUPP(eventUPP);
//
//    CFRelease(mesRef);
//    CFRelease(baseRef);
//    CFRelease(numRef);
//    CFRelease(romRef);
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    if (systemVersion < 0x1060)
//    {
//        CFRelease(nav.customWindow);
//        DisposeNibReference(nav.customNib);
//    }
//#endif
//
//    if (err)
//        return (false);
//    else
//    {
//        if (nav.reply)
//        {
//            err = FSRefMakePath(&(nav.ref), (unsigned char *) path, PATH_MAX);
//            return (true);
//        }
//        else
//            return (false);
//    }
    return nil;
}

NSURL *NavQTMovieRecordTo ()
{
//    OSStatus                    err;
//    NavDialogCreationOptions    dialogOptions;
//    NavEventUPP                    eventUPP;
//    NavState                    nav;
//    CFStringRef                    numRef, romRef, baseRef;
//    CFMutableStringRef            mesRef, saveRef;
//    SInt32                        replaceAt;
//    char                        drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    if (systemVersion < 0x1060)
//    {
//        HIViewID    cid = { 'PANE', 1000 };
//        Rect        rct;
//
//        err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
//        if (err)
//            return (false);
//
//        err = CreateWindowFromNib(nav.customNib, CFSTR("QTMovie"), &(nav.customWindow));
//        if (err)
//        {
//            DisposeNibReference(nav.customNib);
//            return (false);
//        }
//
//        GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
//        nav.customWidth  = rct.right  - rct.left;
//        nav.customHeight = rct.bottom - rct.top;
//
//        HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));
//    }
//#endif
//
//    _splitpath(Memory.ROMFilename, drive, dir, fname, ext);
//    romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, kCFStringEncodingUTF8);
//    numRef  = CFCopyLocalizedString(CFSTR("QTRecordPos"), "1");
//    baseRef = CFCopyLocalizedString(CFSTR("QTRecordMes"), "QT");
//    mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
//    saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
//    replaceAt = CFStringGetIntValue(numRef);
//    CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
//    CFStringAppendCString(saveRef, ".mov", CFStringGetSystemEncoding());
//
//    err = NavGetDefaultDialogCreationOptions(&dialogOptions);
//    dialogOptions.preferenceKey = 7;
//    dialogOptions.clientName = kMacS9XCFString;
//    dialogOptions.windowTitle = mesRef;
//    dialogOptions.saveFileName = saveRef;
//    dialogOptions.modality = kWindowModalityAppModal;
//    dialogOptions.parentWindow = NULL;
//    nav.parent = NULL;
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    eventUPP = NewNavEventUPP((systemVersion < 0x1060) ? NavQTMovieRecordToEventHandler : NavGenericSaveEventHandler);
//#else
//    eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);
//#endif
//
//    err = NavCreatePutFileDialog(&dialogOptions, 'MooV', 'TVOD', eventUPP, &nav, &(nav.nref));
//    if (err == noErr)
//    {
//        if (systemVersion >= 0x1060)
//        {
//            NSAutoreleasePool    *pool;
//            NSView                *view;
//            NSPopUpButton        *fpop;
//            NSButton            *cbds, *cbov, *comp;
//            NSArray                *array;
//            NavController        *controller;
//
//            pool = [[NSAutoreleasePool alloc] init];
//
//            controller = [[[NavController alloc] init] autorelease];
//            view = [[[NSView alloc] initWithFrame: NSMakeRect(0, 0, 433, 31)] autorelease];
//
//            array = [NSArray arrayWithObjects: @"0", @"1", @"2", @"3", @"4", @"5", nil];
//            CocoaAddStatTextToView(view, @"QTRecord3", 185, 9,  70, 14, NULL);
//            CocoaAddCheckBoxToView(view, @"QTRecord1",   5, 7,  85, 18, &cbds);
//            CocoaAddCheckBoxToView(view, @"QTRecord2",  97, 7,  71, 18, &cbov);
//            CocoaAddMPushBtnToView(view, @"QTRecord4", 313, 1, 117, 28, &comp);
//            CocoaAddPopUpBtnToView(view, array,        255, 4,  50, 22, &fpop);
//
//            [cbds setState: (macQTMovFlag & (1 << 0) ? NSOnState : NSOffState)];
//            [cbov setState: (macQTMovFlag & (1 << 1) ? NSOnState : NSOffState)];
//            [fpop selectItemAtIndex: (macQTMovFlag >> 8)];
//
//            [comp setTarget: controller];
//            [comp setAction: @selector(handleCompressButton:)];
//
//            [(NSOpenPanel *) nav.nref setAccessoryView: view];
//
//            err = NavDialogRun(nav.nref);
//            if (err)
//                NavDialogDispose(nav.nref);
//
//            macQTMovFlag  = 0;
//            macQTMovFlag |= ((([cbds state] == NSOnState) ? 1 : 0) << 0);
//            macQTMovFlag |= ((([cbov state] == NSOnState) ? 1 : 0) << 1);
//            macQTMovFlag |= ([fpop indexOfSelectedItem] << 8);
//
//            [pool release];
//        }
//    #ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//        else
//        {
//            err = NavDialogRun(nav.nref);
//            if (err)
//                NavDialogDispose(nav.nref);
//        }
//    #endif
//    }
//
//    DisposeNavEventUPP(eventUPP);
//
//    CFRelease(saveRef);
//    CFRelease(mesRef);
//    CFRelease(baseRef);
//    CFRelease(numRef);
//    CFRelease(romRef);
//
//#ifdef MAC_LEOPARD_TIGER_PANTHER_SUPPORT
//    if (systemVersion < 0x1060)
//    {
//        CFRelease(nav.customWindow);
//        DisposeNibReference(nav.customNib);
//    }
//#endif
//
//    if (err)
//        return (false);
//    else
//    {
//        if (nav.reply)
//        {
//            char    s[PATH_MAX + 1];
//
//            err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
//            snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);
//
//            return (true);
//        }
//        else
//            return (false);
//    }
    return nil;
}

//bool8 NavChooseFolderSheet (WindowRef parent)
//{
//    OSStatus                    err;
//    NavDialogCreationOptions    dialogOptions;
//
//    err = NavGetDefaultDialogCreationOptions(&dialogOptions);
//    dialogOptions.optionFlags &= ~kNavAllowMultipleFiles;
//    dialogOptions.preferenceKey = 8;
//    dialogOptions.clientName = kMacS9XCFString;
//    dialogOptions.modality = kWindowModalityWindowModal;
//    dialogOptions.parentWindow = parent;
//    gSheetNav.parent = parent;
//
//    gSheetEventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
//
//    err = NavCreateChooseFolderDialog(&dialogOptions, gSheetEventUPP, NULL, &gSheetNav, &(gSheetNav.nref));
//    if (err == noErr)
//    {
//        err = NavDialogRun(gSheetNav.nref);
//        if (err)
//        {
//            NavDialogDispose(gSheetNav.nref);
//            DisposeNavEventUPP(gSheetEventUPP);
//            return (false);
//        }
//    }
//    else
//    {
//        DisposeNavEventUPP(gSheetEventUPP);
//        return (false);
//    }
//
//    return (true);
//}
//
//bool8 NavEndChooseFolderSheet (FSRef *ref)
//{
//    DisposeNavEventUPP(gSheetEventUPP);
//
//    if (gSheetNav.reply)
//    {
//        *ref = gSheetNav.ref;
//        return (true);
//    }
//    else
//        return (false);
//}
