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


#include "port.h"

#include <IOKit/hid/IOHIDUsageTables.h>
#include "HID_Utilities_External.h"

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-joypad.h"

#define	kUp(i)					(i * 4)
#define	kDn(i)					(i * 4 + 1)
#define	kLf(i)					(i * 4 + 2)
#define	kRt(i)					(i * 4 + 3)

#define	kPadElemTypeNone		0
#define	kPadElemTypeHat4		1
#define	kPadElemTypeHat8		2
#define	kPadElemTypeAxis		3
#define	kPadElemTypeButton		4
#define	kPadElemTypeOtherHat4	5
#define	kPadElemTypeOtherHat8	6

#define	kPadXAxis				1
#define	kPadYAxis				0
#define	kPadHat					0

#define	kMaskUp					0x0800
#define	kMaskDn					0x0400
#define	kMaskLf					0x0200
#define	kMaskRt					0x0100

typedef	hu_device_t		*pRecDevice;
typedef	hu_element_t	*pRecElement;

typedef struct	actionStruct
{
	pRecDevice 	fDevice;
	pRecElement	fElement;
	long		fValue;
	long		fOldValue;
}	actionRec, *actionPtr;

typedef struct	padDirectionInfo
{
	int			type;
	pRecDevice	device [2];
	pRecElement	element[2];
	long		max    [2];
	long		maxmid [2];
	long		mid    [2];
	long		midmin [2];
	long		min    [2];
}	directionInfo;

static actionRec		gActionRecs[kNumButtons];
static directionInfo	gDirectionInfo[MAC_MAX_PLAYERS];
static int				gDirectionHint[MAC_MAX_PLAYERS];

static void JoypadSetDirectionInfo (void);
//static void IdleTimer (EventLoopTimerRef, void *);
//static OSStatus ControllerEventHandler (EventHandlerCallRef, EventRef, void *);


void SaveControllerSettings (void)
{
//    CFStringRef    keyCFStringRef;
//     Boolean        syncFlag;
//
//    JoypadSetDirectionInfo();
//
//    for (int a = 0; a < kNeedCount; a++)
//    {
//        char    needCStr[64], num[10];
//
//        strcpy(needCStr, gNeeds[a]);
//        if (padSetting > 1)
//        {
//            sprintf(num, "_%d", padSetting);
//            strcat(needCStr, num);
//        }
//
//        keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s"), needCStr);
//        if (keyCFStringRef)
//        {
//            if (gActionRecs[a].fDevice && gActionRecs[a].fElement)
//                syncFlag = HIDSaveElementPref(keyCFStringRef, kCFPreferencesCurrentApplication, gActionRecs[a].fDevice, gActionRecs[a].fElement);
//            else
//                CFPreferencesSetAppValue(keyCFStringRef, NULL, kCFPreferencesCurrentApplication);
//
//            CFRelease(keyCFStringRef);
//        }
//    }
//
//    for (int a = 0; a < MAC_MAX_PLAYERS; a++)
//    {
//        keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("DirectionHint_%d_%d"), a, padSetting);
//        if (keyCFStringRef)
//        {
//            CFNumberRef    numRef;
//            CFIndex        v;
//
//            v = (CFIndex) gDirectionHint[a];
//            numRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberCFIndexType, &v);
//            if (numRef)
//            {
//                CFPreferencesSetAppValue(keyCFStringRef, numRef, kCFPreferencesCurrentApplication);
//                CFRelease(numRef);
//            }
//
//            CFRelease(keyCFStringRef);
//        }
//    }
//
//    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

void LoadControllerSettings (void)
{
//    CFStringRef    keyCFStringRef;
//
//    for (int a = 0; a < kNumButtons; a++)
//    {
//        pRecDevice    pDevice  = NULL;
//        pRecElement    pElement = NULL;
//        Boolean        r = false;
//        char        needCStr[64], num[10];
//
//        strcpy(needCStr, gNeeds[a]);
//        if (padSetting > 1)
//        {
//            sprintf(num, "_%d", padSetting);
//            strcat(needCStr, num);
//        }
//
//        keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s"), needCStr);
//        if (keyCFStringRef)
//        {
//            // r = HIDRestoreElementPref(keyCFStringRef, kCFPreferencesCurrentApplication, &pDevice, &pElement);
//            if (r && pDevice && pElement)
//            {
//                gActionRecs[a].fDevice  = pDevice;
//                gActionRecs[a].fElement = pElement;
//            }
//            else
//            {
//                gActionRecs[a].fDevice  = NULL;
//                gActionRecs[a].fElement = NULL;
//            }
//
//            CFRelease(keyCFStringRef);
//        }
//    }
//
//    for (int a = 0; a < MAC_MAX_PLAYERS; a++)
//    {
//        keyCFStringRef = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("DirectionHint_%d_%d"), a, padSetting);
//        if (keyCFStringRef)
//        {
//            Boolean    r;
//
//            gDirectionHint[a] = (int) CFPreferencesGetAppIntegerValue(keyCFStringRef, kCFPreferencesCurrentApplication, &r);
//            if (!r)
//                gDirectionHint[a] = kPadElemTypeNone;
//
//            CFRelease(keyCFStringRef);
//        }
//        else
//            gDirectionHint[a] = kPadElemTypeNone;
//    }
//
//    JoypadSetDirectionInfo();
}

//static OSStatus ControllerEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
//{
//    OSStatus    err, result = eventNotHandledErr;
//    WindowRef    tWindowRef;
//
//    tWindowRef = (WindowRef) inUserData;
//
//    switch (GetEventClass(inEvent))
//    {
//        case kEventClassWindow:
//            switch (GetEventKind(inEvent))
//            {
//                case kEventWindowClose:
//                    QuitAppModalLoopForWindow(tWindowRef);
//                    result = noErr;
//                    break;
//            }
//
//            break;
//
//        case kEventClassCommand:
//            switch (GetEventKind(inEvent))
//            {
//                HICommand    tHICommand;
//
//                case kEventCommandUpdateStatus:
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
//                    if (err == noErr && tHICommand.commandID == 'clos')
//                    {
//                        UpdateMenuCommandStatus(true);
//                        result = noErr;
//                    }
//
//                    break;
//
//                case kEventCommandProcess:
//                    err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
//                    if (err == noErr)
//                    {
//                        if (tHICommand.commandID == 'CLRa')
//                        {
//                            ClearPadSetting();
//                            result = noErr;
//                        }
//                        else
//                        {
//                            SInt32    command = -1, count;
//
//                            for (count = 0; count < kNeedCount; count++)
//                                if (tHICommand.commandID == gControlIDs[count].signature)
//                                    command = count;
//
//                            if (command >= 0)
//                            {
//                                pRecDevice    pDevice;
//                                pRecElement    pElement;
//
//                                FlushEventQueue(GetCurrentEventQueue());
//
//                                if (HIDConfigureAction(&pDevice, &pElement, 2.5f))
//                                {
//                                    if (command < MAC_MAX_PLAYERS * 4)    // Direction
//                                    {
//                                        int        i    = command >> 2;    // Player
//                                        long    curv = HIDGetElementValue(pDevice, pElement);
//
//                                        if (pElement->usage == kHIDUsage_GD_Hatswitch)    // Hat Switch
//                                        {
//                                            gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = pDevice;
//                                            gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = pElement;
//
//                                            if (pDevice->vendorID == 1103)    // Thrustmaster
//                                                gDirectionInfo[i].type = (pElement->max > 4) ? kPadElemTypeOtherHat8 : kPadElemTypeOtherHat4;
//                                            else
//                                            {
//                                                if (pElement->max > 4)
//                                                {
//                                                    if (((command % 4 == 0) && (curv == 0)) ||    // Up    : 0
//                                                        ((command % 4 == 1) && (curv == 4)) ||    // Down  : 4
//                                                        ((command % 4 == 2) && (curv == 6)) ||    // Left  : 6
//                                                        ((command % 4 == 3) && (curv == 2)))    // Right : 2
//                                                        gDirectionInfo[i].type = kPadElemTypeOtherHat8;
//                                                    else
//                                                        gDirectionInfo[i].type = kPadElemTypeHat8;
//                                                }
//                                                else
//                                                {
//                                                    if (((command % 4 == 0) && (curv == 0)) ||    // Up    : 0
//                                                        ((command % 4 == 1) && (curv == 2)) ||    // Down  : 2
//                                                        ((command % 4 == 2) && (curv == 3)) ||    // Left  : 3
//                                                        ((command % 4 == 3) && (curv == 1)))    // Right : 1
//                                                        gDirectionInfo[i].type = kPadElemTypeOtherHat4;
//                                                    else
//                                                        gDirectionInfo[i].type = kPadElemTypeHat4;
//                                                }
//                                            }
//
//                                            gDirectionInfo[i].device [kPadHat] = pDevice;
//                                            gDirectionInfo[i].element[kPadHat] = pElement;
//                                            gDirectionInfo[i].max    [kPadHat] = pElement->max;
//                                            gDirectionInfo[i].min    [kPadHat] = pElement->min;
//                                        }
//                                        else
//                                        if (pElement->max - pElement->min > 1)            // Axis (maybe)
//                                        {
//                                            if ((command % 4 == 0) || (command % 4 == 1))    // Up or Dn
//                                            {
//                                                gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = pDevice;
//                                                gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = pElement;
//
//                                                gDirectionInfo[i].type               = kPadElemTypeAxis;
//                                                gDirectionInfo[i].device [kPadYAxis] = pDevice;
//                                                gDirectionInfo[i].element[kPadYAxis] = pElement;
//                                                gDirectionInfo[i].max    [kPadYAxis] = pElement->max;
//                                                gDirectionInfo[i].min    [kPadYAxis] = pElement->min;
//                                                gDirectionInfo[i].mid    [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;
//                                                gDirectionInfo[i].maxmid [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].mid[kPadYAxis]) >> 1;
//                                                gDirectionInfo[i].midmin [kPadYAxis] = (gDirectionInfo[i].mid[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;
//                                            }
//                                            else                                            // Lf or Rt
//                                            {
//                                                gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = pDevice;
//                                                gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = pElement;
//
//                                                gDirectionInfo[i].type               = kPadElemTypeAxis;
//                                                gDirectionInfo[i].device [kPadXAxis] = pDevice;
//                                                gDirectionInfo[i].element[kPadXAxis] = pElement;
//                                                gDirectionInfo[i].max    [kPadXAxis] = pElement->max;
//                                                gDirectionInfo[i].min    [kPadXAxis] = pElement->min;
//                                                gDirectionInfo[i].mid    [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
//                                                gDirectionInfo[i].maxmid [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].mid[kPadXAxis]) >> 1;
//                                                gDirectionInfo[i].midmin [kPadXAxis] = (gDirectionInfo[i].mid[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
//                                            }
//                                        }
//                                        else                                            // Button (maybe)
//                                        {
//                                            gActionRecs[command].fDevice  = pDevice;
//                                            gActionRecs[command].fElement = pElement;
//                                            gDirectionInfo[i].type = kPadElemTypeButton;
//                                        }
//
//                                        gDirectionHint[i] = gDirectionInfo[i].type;
//                                    }
//                                    else
//                                    {
//                                        gActionRecs[command].fDevice  = pDevice;
//                                        gActionRecs[command].fElement = pElement;
//                                    }
//                                }
//                                else
//                                {
//                                    if (command < MAC_MAX_PLAYERS * 4)    // Direction
//                                    {
//                                        int    i = command >> 2;    // Player
//
//                                        gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = NULL;
//                                        gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = NULL;
//
//                                        gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeNone;
//                                        gDirectionInfo[i].device [0] = gDirectionInfo[i].device [1] = NULL;
//                                        gDirectionInfo[i].element[0] = gDirectionInfo[i].element[1] = NULL;
//                                    }
//                                    else
//                                    {
//                                        gActionRecs[command].fDevice  = NULL;
//                                        gActionRecs[command].fElement = NULL;
//                                    }
//                                }
//
//                                gActionRecs[command].fValue    = 0;
//                                gActionRecs[command].fOldValue = -2;
//
//                                FlushEventQueue(GetCurrentEventQueue());
//
//                                result = noErr;
//                            }
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

//static void IdleTimer (EventLoopTimerRef inTimer, void *userData)
//{
//    static uint32    old[MAC_MAX_PLAYERS] = { ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0 };
//
//    HIViewRef    ctl, root;
//    uint32        pad[MAC_MAX_PLAYERS];
//
//    root = HIViewGetRoot((WindowRef) userData);
//
//    for (int i = 0; i < MAC_MAX_PLAYERS; i++)
//    {
//        pad[i] = 0;
//        JoypadScanDirection(i, &(pad[i]));
//
//        if (old[i] != pad[i])
//        {
//            old[i]  = pad[i];
//
//            HIViewFindByID(root, gControlIDs[kUp(i)], &ctl);
//            SetControl32BitValue(ctl, (pad[i] & kMaskUp) ? 1 : 0);
//            HIViewFindByID(root, gControlIDs[kDn(i)], &ctl);
//            SetControl32BitValue(ctl, (pad[i] & kMaskDn) ? 1 : 0);
//            HIViewFindByID(root, gControlIDs[kLf(i)], &ctl);
//            SetControl32BitValue(ctl, (pad[i] & kMaskLf) ? 1 : 0);
//            HIViewFindByID(root, gControlIDs[kRt(i)], &ctl);
//            SetControl32BitValue(ctl, (pad[i] & kMaskRt) ? 1 : 0);
//        }
//    }
//
//    for (int i = MAC_MAX_PLAYERS * 4; i < kNeedCount; i++)
//    {
//        gActionRecs[i].fValue = ISpKeyIsPressed(i);
//
//        if (gActionRecs[i].fOldValue != gActionRecs[i].fValue)
//        {
//            gActionRecs[i].fOldValue  = gActionRecs[i].fValue;
//
//            HIViewFindByID(root, gControlIDs[i], &ctl);
//            SetControl32BitValue(ctl, (gActionRecs[i].fValue ? 1 : 0));
//        }
//    }
//}

void SetUpHID (void)
{
//    pRecDevice    device;
//
//    HIDBuildDeviceList(NULL, NULL);
//    device = HIDGetFirstDevice();
//    if (!device)
//    {
//        hidExist = false;
//        return;
//    }
//
//    hidExist = true;
//
//    ClearPadSetting();

    LoadControllerSettings();
}

void ClearPadSetting (void)
{
    for (int i = 0; i < MAC_MAX_PLAYERS; i++)
    {
        gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeNone;
        gDirectionInfo[i].device [0] = gDirectionInfo[i].device [1] = NULL;
        gDirectionInfo[i].element[0] = gDirectionInfo[i].element[1] = NULL;
    }

    for (int i = 0; i < kNumButtons; i++)
    {
        gActionRecs[i].fDevice   = NULL;
        gActionRecs[i].fElement  = NULL;
        gActionRecs[i].fValue    = 0;
        gActionRecs[i].fOldValue = -2;
    }
}

void ReleaseHID (void)
{
//    if (hidExist)
//        HIDReleaseDeviceList();
}

//void ConfigureHID (void)
//{
//    OSStatus    err;
//    IBNibRef    nibRef;
//
//    if (!hidExist)
//        return;
//
//    err = CreateNibReference(kMacS9XCFString, &nibRef);
//    if (err == noErr)
//    {
//        WindowRef    tWindowRef;
//
//        err = CreateWindowFromNib(nibRef, CFSTR("Controllers"), &tWindowRef);
//        if (err == noErr)
//        {
//            EventHandlerRef                eref;
//            EventLoopTimerRef            tref;
//            EventHandlerUPP                eventUPP;
//            EventLoopTimerUPP            timerUPP;
//            EventTypeSpec                windowEvents[] = { { kEventClassCommand, kEventCommandProcess      },
//                                                           { kEventClassCommand, kEventCommandUpdateStatus },
//                                                           { kEventClassWindow,  kEventWindowClose         } };
//            HIViewRef                    ctl, root;
//            HIViewID                    cid;
//            CFStringRef                    str1, str2;
//            ControlButtonContentInfo    info;
//
//            LoadControllerSettings();
//
//            root = HIViewGetRoot(tWindowRef);
//            cid.id = 0;
//            cid.signature = 'PRES';
//            HIViewFindByID(root, cid, &ctl);
//            str1 = CFCopyLocalizedString(CFSTR("PresetNum"), "PresetNum");
//            str2 = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, str1, padSetting);
//            SetStaticTextCFString(ctl, str2, false);
//            CFRelease(str2);
//            CFRelease(str1);
//
//            if (systemVersion >= 0x1040)
//            {
//                info.contentType = kControlContentCGImageRef;
//                for (int i = 0; i < kNeedCount; i++)
//                {
//                    HIViewFindByID(root, gControlIDs[i], &ctl);
//                    info.u.imageRef = macIconImage[gIconNumber[i]];
//                    err = SetBevelButtonContentInfo(ctl, &info);
//                }
//            }
//        #ifdef MAC_PANTHER_SUPPORT
//            else
//            {
//                info.contentType = kControlContentIconRef;
//                for (int i = 0; i < kNeedCount; i++)
//                {
//                    HIViewFindByID(root, gControlIDs[i], &ctl);
//                    info.u.iconRef = macIconRef[gIconNumber[i]];
//                    err = SetBevelButtonContentInfo(ctl, &info);
//                }
//            }
//        #endif
//
//            eventUPP = NewEventHandlerUPP(ControllerEventHandler);
//            err = InstallWindowEventHandler(tWindowRef, eventUPP, GetEventTypeCount(windowEvents), windowEvents, (void *) tWindowRef, &eref);
//
//            timerUPP = NewEventLoopTimerUPP(IdleTimer);
//            err = InstallEventLoopTimer(GetCurrentEventLoop(), 0.0f, 0.1f, timerUPP, (void *) tWindowRef, &tref);
//
//            MoveWindowPosition(tWindowRef, kWindowControllers, false);
//            ShowWindow(tWindowRef);
//            err = RunAppModalLoopForWindow(tWindowRef);
//            HideWindow(tWindowRef);
//            SaveWindowPosition(tWindowRef, kWindowControllers);
//
//            err = RemoveEventLoopTimer(tref);
//            DisposeEventLoopTimerUPP(timerUPP);
//
//            err = RemoveEventHandler(eref);
//            DisposeEventHandlerUPP(eventUPP);
//
//            CFRelease(tWindowRef);
//
//            SaveControllerSettings();
//        }
//
//        DisposeNibReference(nibRef);
//    }
//}

void JoypadScanDirection (int i, uint32 *pad)
{
//    long    state;
//
//    switch (gDirectionInfo[i].type)
//    {
//        case kPadElemTypeAxis:                            // Axis (maybe)
//            if (gDirectionInfo[i].device[kPadYAxis])    // Y-Axis
//            {
//                state = HIDGetElementValue(gDirectionInfo[i].device[kPadYAxis], gDirectionInfo[i].element[kPadYAxis]);
//                if (state >= gDirectionInfo[i].maxmid[kPadYAxis])
//                    *pad |= kMaskDn;
//                else
//                if (state <= gDirectionInfo[i].midmin[kPadYAxis])
//                    *pad |= kMaskUp;
//            }
//
//            if (gDirectionInfo[i].device[kPadXAxis])    // X-Axis
//            {
//                state = HIDGetElementValue(gDirectionInfo[i].device[kPadXAxis], gDirectionInfo[i].element[kPadXAxis]);
//                if (state >= gDirectionInfo[i].maxmid[kPadXAxis])
//                    *pad |= kMaskRt;
//                else
//                if (state <= gDirectionInfo[i].midmin[kPadXAxis])
//                    *pad |= kMaskLf;
//            }
//
//            break;
//
//        case kPadElemTypeHat8:                            // Hat Switch (8 Directions)
//            if (gDirectionInfo[i].device[kPadHat])
//            {
//                state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
//                switch (state)
//                {
//                    case 1:    *pad |=  kMaskUp           ;    break;
//                    case 2:    *pad |= (kMaskUp | kMaskRt);    break;
//                    case 3:    *pad |=  kMaskRt           ;    break;
//                    case 4:    *pad |= (kMaskRt | kMaskDn);    break;
//                    case 5:    *pad |=  kMaskDn           ;    break;
//                    case 6:    *pad |= (kMaskDn | kMaskLf);    break;
//                    case 7:    *pad |=  kMaskLf           ;    break;
//                    case 8:    *pad |= (kMaskLf | kMaskUp);    break;
//                }
//            }
//
//            break;
//
//        case kPadElemTypeHat4:                            // Hat Switch (4 Directions)
//            if (gDirectionInfo[i].device[kPadHat])
//            {
//                state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
//                switch (state)
//                {
//                    case 1:    *pad |=  kMaskUp;    break;
//                    case 2:    *pad |=  kMaskRt;    break;
//                    case 3:    *pad |=  kMaskDn;    break;
//                    case 4:    *pad |=  kMaskLf;    break;
//                }
//            }
//
//            break;
//
//        case kPadElemTypeOtherHat8:                        // Hat Switch (8 Directions, Start at 0)
//            if (gDirectionInfo[i].device[kPadHat])
//            {
//                state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
//                switch (state)
//                {
//                    case 0:    *pad |=  kMaskUp           ;    break;
//                    case 1:    *pad |= (kMaskUp | kMaskRt);    break;
//                    case 2:    *pad |=  kMaskRt           ;    break;
//                    case 3:    *pad |= (kMaskRt | kMaskDn);    break;
//                    case 4:    *pad |=  kMaskDn           ;    break;
//                    case 5:    *pad |= (kMaskDn | kMaskLf);    break;
//                    case 6:    *pad |=  kMaskLf           ;    break;
//                    case 7:    *pad |= (kMaskLf | kMaskUp);    break;
//                }
//            }
//
//            break;
//
//        case kPadElemTypeOtherHat4:                        // Hat Switch (4 Directions, Start at 0)
//            if (gDirectionInfo[i].device[kPadHat])
//            {
//                state = HIDGetElementValue(gDirectionInfo[i].device[kPadHat], gDirectionInfo[i].element[kPadHat]);
//                switch (state)
//                {
//                    case 0:    *pad |=  kMaskUp;    break;
//                    case 1:    *pad |=  kMaskRt;    break;
//                    case 2:    *pad |=  kMaskDn;    break;
//                    case 3:    *pad |=  kMaskLf;    break;
//                }
//            }
//
//            break;
//
//        case kPadElemTypeButton:                        // Button (maybe)
//            if (gActionRecs[kUp(i)].fDevice && HIDGetElementValue(gActionRecs[kUp(i)].fDevice, gActionRecs[kUp(i)].fElement))
//                *pad |= kMaskUp;
//            if (gActionRecs[kDn(i)].fDevice && HIDGetElementValue(gActionRecs[kDn(i)].fDevice, gActionRecs[kDn(i)].fElement))
//                *pad |= kMaskDn;
//            if (gActionRecs[kLf(i)].fDevice && HIDGetElementValue(gActionRecs[kLf(i)].fDevice, gActionRecs[kLf(i)].fElement))
//                *pad |= kMaskLf;
//            if (gActionRecs[kRt(i)].fDevice && HIDGetElementValue(gActionRecs[kRt(i)].fDevice, gActionRecs[kRt(i)].fElement))
//                *pad |= kMaskRt;
//
//            break;
//    }
}

static void JoypadSetDirectionInfo (void)
{
    for (int i = 0; i < MAC_MAX_PLAYERS; i++)
    {
        if (((gActionRecs[kUp(i)].fDevice) && (gActionRecs[kUp(i)].fElement)) &&
            ((gActionRecs[kDn(i)].fDevice) && (gActionRecs[kDn(i)].fElement)) &&
            ((gActionRecs[kLf(i)].fDevice) && (gActionRecs[kLf(i)].fElement)) &&
            ((gActionRecs[kRt(i)].fDevice) && (gActionRecs[kRt(i)].fElement)))
        {
            if ((gActionRecs[kUp(i)].fDevice  == gActionRecs[kDn(i)].fDevice)  &&
                (gActionRecs[kDn(i)].fDevice  == gActionRecs[kLf(i)].fDevice)  &&
                (gActionRecs[kLf(i)].fDevice  == gActionRecs[kRt(i)].fDevice)  &&
                (gActionRecs[kUp(i)].fElement == gActionRecs[kDn(i)].fElement) &&
                (gActionRecs[kDn(i)].fElement == gActionRecs[kLf(i)].fElement) &&
                (gActionRecs[kLf(i)].fElement == gActionRecs[kRt(i)].fElement) &&
                (gActionRecs[kUp(i)].fElement->usage == kHIDUsage_GD_Hatswitch))                // Hat Switch
            {
                if ((gDirectionHint[i] == kPadElemTypeHat8) || (gDirectionHint[i] == kPadElemTypeOtherHat8) ||
                    (gDirectionHint[i] == kPadElemTypeHat4) || (gDirectionHint[i] == kPadElemTypeOtherHat4))
                    gDirectionInfo[i].type = gDirectionHint[i];
                else                                                                            // Assuming...
                {
                    if ((gActionRecs[kUp(i)].fDevice->vendorID == 1103) || (gActionRecs[kUp(i)].fElement->min == 0))
                        gDirectionInfo[i].type = (gActionRecs[kUp(i)].fElement->max > 4) ? kPadElemTypeOtherHat8 : kPadElemTypeOtherHat4;
                    else
                        gDirectionInfo[i].type = (gActionRecs[kUp(i)].fElement->max > 4) ? kPadElemTypeHat8      : kPadElemTypeHat4;

                    gDirectionHint[i] = gDirectionInfo[i].type;
                }

                gDirectionInfo[i].device [kPadHat] =  gActionRecs[kUp(i)].fDevice;
                gDirectionInfo[i].element[kPadHat] =  gActionRecs[kUp(i)].fElement;
                gDirectionInfo[i].max    [kPadHat] =  gActionRecs[kUp(i)].fElement->max;
                gDirectionInfo[i].min    [kPadHat] =  gActionRecs[kUp(i)].fElement->min;
            }
            else
            if ((gActionRecs[kUp(i)].fDevice  == gActionRecs[kDn(i)].fDevice)               &&
                (gActionRecs[kLf(i)].fDevice  == gActionRecs[kRt(i)].fDevice)               &&
                (gActionRecs[kUp(i)].fElement == gActionRecs[kDn(i)].fElement)              &&
                (gActionRecs[kLf(i)].fElement == gActionRecs[kRt(i)].fElement)              &&
                (gActionRecs[kUp(i)].fElement->max - gActionRecs[kUp(i)].fElement->min > 1) &&
                (gActionRecs[kLf(i)].fElement->max - gActionRecs[kLf(i)].fElement->min > 1))    // Axis (maybe)
            {
                gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeAxis;

                gDirectionInfo[i].device [kPadYAxis] = gActionRecs[kUp(i)].fDevice;
                gDirectionInfo[i].element[kPadYAxis] = gActionRecs[kUp(i)].fElement;
                gDirectionInfo[i].max    [kPadYAxis] = gActionRecs[kUp(i)].fElement->max;
                gDirectionInfo[i].min    [kPadYAxis] = gActionRecs[kUp(i)].fElement->min;
                gDirectionInfo[i].mid    [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;
                gDirectionInfo[i].maxmid [kPadYAxis] = (gDirectionInfo[i].max[kPadYAxis] + gDirectionInfo[i].mid[kPadYAxis]) >> 1;
                gDirectionInfo[i].midmin [kPadYAxis] = (gDirectionInfo[i].mid[kPadYAxis] + gDirectionInfo[i].min[kPadYAxis]) >> 1;

                gDirectionInfo[i].device [kPadXAxis] = gActionRecs[kLf(i)].fDevice;
                gDirectionInfo[i].element[kPadXAxis] = gActionRecs[kLf(i)].fElement;
                gDirectionInfo[i].max    [kPadXAxis] = gActionRecs[kLf(i)].fElement->max;
                gDirectionInfo[i].min    [kPadXAxis] = gActionRecs[kLf(i)].fElement->min;
                gDirectionInfo[i].mid    [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
                gDirectionInfo[i].maxmid [kPadXAxis] = (gDirectionInfo[i].max[kPadXAxis] + gDirectionInfo[i].mid[kPadXAxis]) >> 1;
                gDirectionInfo[i].midmin [kPadXAxis] = (gDirectionInfo[i].mid[kPadXAxis] + gDirectionInfo[i].min[kPadXAxis]) >> 1;
            }
            else                                                                                // Button (maybe)
                gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeButton;
        }
        else
        {
            gActionRecs[kUp(i)].fDevice  = gActionRecs[kDn(i)].fDevice  = gActionRecs[kLf(i)].fDevice  = gActionRecs[kRt(i)].fDevice  = NULL;
            gActionRecs[kUp(i)].fElement = gActionRecs[kDn(i)].fElement = gActionRecs[kLf(i)].fElement = gActionRecs[kRt(i)].fElement = NULL;

            gDirectionInfo[i].type = gDirectionHint[i] = kPadElemTypeNone;
            gDirectionInfo[i].device [0] = gDirectionInfo[i].device [1] = NULL;
            gDirectionInfo[i].element[0] = gDirectionInfo[i].element[1] = NULL;
        }
    }
}
