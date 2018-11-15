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
 ***********************************************************************************/


#ifndef _mac_dialog_h_
#define _mac_dialog_h_

enum
{
	kS9xMacAlertFolderNotFound = 1,
	kS9xMacAlertFolderNotFoundHint,
	kS9xMacAlertFolderFailToCreate,
	kS9xMacAlertFolderFailToCreateHint,
	kS9xMacAlertkBadRom,
	kS9xMacAlertkBadRomHint,
	kS9xMacAlertCFCantAddEntry,
	kS9xMacAlertCFCantAddEntryHint,
	kS9xMacAlertRequiredSystem,
	kS9xMacAlertRequiredSystemHint
};

extern int	autofireLastTabIndex;

void AboutDialog (void);
void RomInfoDialog (void);
void ConfigureAutofire (void);
void StartCarbonModalDialog (void);
void FinishCarbonModalDialog (void);
void RegisterHelpBook (void);
void MoveWindowPosition (WindowRef, int, Boolean);
void SaveWindowPosition (WindowRef, int);
void AppearanceAlert (AlertType, int, int);
void SetHIViewID (HIViewID *, OSType, SInt32);
OSStatus SetStaticTextCStr (HIViewRef, char *, Boolean);
OSStatus SetStaticTextCFString (HIViewRef, CFStringRef, Boolean);
OSStatus SetStaticTextTrunc (HIViewRef, TruncCode, Boolean);
OSStatus GetEditTextCStr (HIViewRef, char *);
OSStatus SetEditTextCStr (HIViewRef, char *, Boolean);
OSStatus CopyEditTextCFString (HIViewRef, CFStringRef *);
OSStatus SetEditTextCFString (HIViewRef, CFStringRef, Boolean);
OSStatus SetEditTextSelection (HIViewRef, SInt16, SInt16);
pascal OSStatus DefaultEventHandler (EventHandlerCallRef, EventRef, void *);

#endif
