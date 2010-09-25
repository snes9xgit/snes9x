/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2010  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2010  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2010  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2010  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2010  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#include "snes9x.h"
#include "memmap.h"
#include "snapshot.h"
#include "movie.h"

#include <wchar.h>

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-quicktime.h"
#include "mac-screenshot.h"
#include "mac-snes9x.h"
#include "mac-stringtools.h"
#include "mac-cart.h"

typedef struct
{
	NavDialogRef	nref;
	FSRef			ref;
	Boolean			reply;
	char			name[PATH_MAX + 1];
	IBNibRef		customNib;
	WindowRef		customWindow;
	HIViewRef		customPane;
	CFStringRef		customSRef;
	short			customWidth, customHeight;
	Boolean			isSheet;
}	NavState;

extern wchar_t	macRecordWChar[MOVIE_MAX_METADATA];

static NavEventUPP			gSheetEventUPP;
static NavObjectFilterUPP	gSheetFilterUPP;
static NavState				gSheetNav;
static WindowRef			gSheetParent;

static void GlobalPointToWindowLocalPoint (Point *, WindowRef);
static pascal void NavOpenCartEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavRecordMovieToEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavPlayMovieFromEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavQTMovieRecordToEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavGenericOpenEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal void NavGenericSaveEventHandler (const NavEventCallbackMessage, NavCBRecPtr, NavCallBackUserData);
static pascal Boolean NavOpenCartFilter (AEDesc *, void *, NavCallBackUserData, NavFilterModes);
static pascal Boolean NavDefrostFromFilter (AEDesc *, void *, NavCallBackUserData, NavFilterModes);
static pascal Boolean NavPlayMovieFromFilter (AEDesc *, void *, NavCallBackUserData, NavFilterModes);
static pascal Boolean NavDefrostFromPreview (NavCBRecPtr, NavCallBackUserData);
static pascal Boolean NavPlayMovieFromPreview (NavCBRecPtr, NavCallBackUserData);


bool8 NavOpenROMImage (FSRef *ref)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavObjectFilterUPP			filterUPP;
	NavState					nav;
	Rect						rct;
	HIViewID					cid = { 'PANE', 1000 };

	err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
	if (err)
		return (false);

	err = CreateWindowFromNib(nav.customNib, CFSTR("OpenROM"), &(nav.customWindow));
	if (err)
	{
		DisposeNibReference(nav.customNib);
		return (false);
	}

	nav.isSheet = false;

	GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
	nav.customWidth  = rct.right  - rct.left;
	nav.customHeight = rct.bottom - rct.top;

	HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowPreviews & ~kNavAllowMultipleFiles & ~kNavAllowStationery;
	dialogOptions.preferenceKey = 3;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = CFCopyLocalizedString(CFSTR("OpenROMMes"), "Open");
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;

	eventUPP = NewNavEventUPP(NavOpenCartEventHandler);
	filterUPP = NewNavObjectFilterUPP(NavOpenCartFilter);

	gSheetParent = NULL;

	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, NULL, filterUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
		if (err)
			NavDialogDispose(nav.nref);
	}

	DisposeNavObjectFilterUPP(filterUPP);
	DisposeNavEventUPP(eventUPP);

	CFRelease(dialogOptions.windowTitle);

	CFRelease(nav.customWindow);
	DisposeNibReference(nav.customNib);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			*ref = nav.ref;
			return (true);
		}
		else
			return (false);
	}
}

static pascal void NavOpenCartEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	MenuRef			menu;
	AEDesc			resultDesc;
	Point			pt;
	SInt32 			count;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'CCTL')
							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'CCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			cid.id = 101;
			HIViewFindByID(nav->customPane, cid, &ctl);
			menu = HIMenuViewGetMenu(ctl);
			for (int i = 1; i <= CountMenuItems(menu); i++)
				CheckMenuItem(menu, i, false);
			switch (romDetect)
			{
				case kAutoROMType:			SetControl32BitValue(ctl, 1);	break;
				case kLoROMForce:  			SetControl32BitValue(ctl, 3);	break;
				case kHiROMForce:  			SetControl32BitValue(ctl, 4);	break;
			}

			cid.id = 102;
			HIViewFindByID(nav->customPane, cid, &ctl);
			menu = HIMenuViewGetMenu(ctl);
			for (int i = 1; i <= CountMenuItems(menu); i++)
				CheckMenuItem(menu, i, false);
			switch (interleaveDetect)
			{
				case kAutoInterleave:		SetControl32BitValue(ctl, 1);	break;
				case kNoInterleaveForce:	SetControl32BitValue(ctl, 3);	break;
				case kInterleaveForce:		SetControl32BitValue(ctl, 4);	break;
				case kInterleave2Force:		SetControl32BitValue(ctl, 5);	break;
				case kInterleaveGD24:		SetControl32BitValue(ctl, 6);	break;
			}

			cid.id = 103;
			HIViewFindByID(nav->customPane, cid, &ctl);
			menu = HIMenuViewGetMenu(ctl);
			for (int i = 1; i <= CountMenuItems(menu); i++)
				CheckMenuItem(menu, i, false);
			switch (videoDetect)
			{
				case kAutoVideo:			SetControl32BitValue(ctl, 1);	break;
				case kPALForce:				SetControl32BitValue(ctl, 3);	break;
				case kNTSCForce:			SetControl32BitValue(ctl, 4);	break;
			}

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			menu = HIMenuViewGetMenu(ctl);
			for (int i = 1; i <= CountMenuItems(menu); i++)
				CheckMenuItem(menu, i, false);
			switch (headerDetect)
			{
				case kAutoHeader:			SetControl32BitValue(ctl, 1);	break;
				case kNoHeaderForce:		SetControl32BitValue(ctl, 3);	break;
				case kHeaderForce:			SetControl32BitValue(ctl, 4);	break;
			}

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			cid.signature = 'CCTL';

			cid.id = 101;
			HIViewFindByID(nav->customPane, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case 1: romDetect        = kAutoROMType; 		break;
				case 3: romDetect        = kLoROMForce;  		break;
				case 4: romDetect        = kHiROMForce;			break;
			}

			cid.id = 102;
			HIViewFindByID(nav->customPane, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case 1: interleaveDetect = kAutoInterleave;		break;
				case 3: interleaveDetect = kNoInterleaveForce;	break;
				case 4: interleaveDetect = kInterleaveForce;	break;
				case 5: interleaveDetect = kInterleave2Force;   break;
				case 6: interleaveDetect = kInterleaveGD24;		break;
			}

			cid.id = 103;
			HIViewFindByID(nav->customPane, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case 1: videoDetect      = kAutoVideo; 			break;
				case 3: videoDetect      = kPALForce;  			break;
				case 4: videoDetect      = kNTSCForce;			break;
			}

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			switch (GetControl32BitValue(ctl))
			{
				case 1: headerDetect     = kAutoHeader; 		break;
				case 3: headerDetect     = kNoHeaderForce;  	break;
				case 4: headerDetect     = kHeaderForce;		break;
			}

			break;

	  case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionChoose:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AECountItems(&(reply.selection), &count);
						if ((err == noErr) && (count == 1))
						{
							err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
							if (err == noErr)
							{
								err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
								if (err == noErr)
									nav->reply = true;

								err = AEDisposeDesc(&resultDesc);
							}
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);

			if (nav->isSheet)
			{
				EventRef	event;
				HICommand   cmd;

				cmd.commandID          = 'NvDn';
				cmd.attributes         = kEventAttributeUserEvent;
				cmd.menu.menuRef       = NULL;
				cmd.menu.menuItemIndex = 0;

				err = CreateEvent(kCFAllocatorDefault, kEventClassCommand, kEventCommandProcess, GetCurrentEventTime(), kEventAttributeUserEvent, &event);
				if (err == noErr)
				{
					err = SetEventParameter(event, kEventParamDirectObject, typeHICommand, sizeof(HICommand), &cmd);
					if (err == noErr)
						err = SendEventToWindow(event, gSheetParent);

					ReleaseEvent(event);
				}
			}

			break;
	}
}

static pascal Boolean NavOpenCartFilter (AEDesc *theItem, void *ninfo, NavCallBackUserData callbackUD, NavFilterModes filterMode)
{
	OSStatus	err;
	AEDesc 		resultDesc;
	Boolean		result = true;

	err = AECoerceDesc(theItem, typeFSRef, &resultDesc);
	if (err == noErr)
	{
		FSRef	ref;

		err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
		if (err == noErr)
		{
			FSCatalogInfo	catinfo;
			HFSUniStr255	unistr;

			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags, &catinfo, &unistr, NULL, NULL);
			if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask) && (unistr.length > 4))
			{
				UInt16	i = unistr.length;

				if  ((unistr.unicode[i - 4] == '.') &&
				   (((unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'r') && (unistr.unicode[i - 1] == 'm')) ||
					((unistr.unicode[i - 3] == 'f') && (unistr.unicode[i - 2] == 'r') && (unistr.unicode[i - 1] == 'z')) ||
					((unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'm') && (unistr.unicode[i - 1] == 'v')) ||
					((unistr.unicode[i - 3] == 'm') && (unistr.unicode[i - 2] == 'o') && (unistr.unicode[i - 1] == 'v')) ||
					((unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'p') && (unistr.unicode[i - 1] == 'c')) ||
					((unistr.unicode[i - 3] == 'r') && (unistr.unicode[i - 2] == 't') && (unistr.unicode[i - 1] == 'c')) ||
					((unistr.unicode[i - 3] == 'd') && (unistr.unicode[i - 2] == 'a') && (unistr.unicode[i - 1] == 't')) ||
					((unistr.unicode[i - 3] == 'o') && (unistr.unicode[i - 2] == 'u') && (unistr.unicode[i - 1] == 't')) ||
					((unistr.unicode[i - 3] == 'c') && (unistr.unicode[i - 2] == 'h') && (unistr.unicode[i - 1] == 't')) ||
					((unistr.unicode[i - 3] == 'i') && (unistr.unicode[i - 2] == 'p') && (unistr.unicode[i - 1] == 's')) ||
					((unistr.unicode[i - 3] == 'p') && (unistr.unicode[i - 2] == 'n') && (unistr.unicode[i - 1] == 'g'))))
					result = false;
			}
		}

		AEDisposeDesc(&resultDesc);
	}

	return (result);
}

bool8 NavFreezeTo (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef, saveRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, MAC_PATH_ENCODING);
	numRef  = CFCopyLocalizedString(CFSTR("FreezeToPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("FreezeToMes"), "Freeze");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
	CFStringAppendCString(saveRef, ".frz", CFStringGetSystemEncoding());

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.preferenceKey = 4;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.saveFileName = saveRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;

	eventUPP = NewNavEventUPP(NavGenericSaveEventHandler);

	err = NavCreatePutFileDialog(&dialogOptions, 'SAVE', '~9X~', eventUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
        if (err)
        	NavDialogDispose(nav.nref);
	}

	DisposeNavEventUPP(eventUPP);

	CFRelease(saveRef);
	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			char	s[PATH_MAX + 1];

			err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
			snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);

			return (true);
		}
		else
			return (false);
	}
}

static pascal void NavGenericSaveEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	AEDesc			resultDesc;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			break;

		case kNavCBStart:
			nav->reply = false;
			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionSaveAs:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
						if (err == noErr)
						{
							err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
							if (err == noErr)
							{
								Boolean	r;

								r = CFStringGetCString(reply.saveFileName, nav->name, PATH_MAX, MAC_PATH_ENCODING);
								if (r)
									nav->reply = true;
							}

							err = AEDisposeDesc(&resultDesc);
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

bool8 NavDefrostFrom (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavObjectFilterUPP			filterUPP;
	NavPreviewUPP				previewUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, MAC_PATH_ENCODING);
	numRef  = CFCopyLocalizedString(CFSTR("DefrostFromPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("DefrostFromMes"), "Defrost");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles & ~kNavAllowStationery;
	dialogOptions.preferenceKey = 5;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;

	eventUPP = NewNavEventUPP(NavGenericOpenEventHandler);
	filterUPP = NewNavObjectFilterUPP(NavDefrostFromFilter);
	previewUPP = NewNavPreviewUPP(NavDefrostFromPreview);

	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, previewUPP, filterUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
        if (err)
        	NavDialogDispose(nav.nref);
	}

	DisposeNavPreviewUPP(previewUPP);
	DisposeNavObjectFilterUPP(filterUPP);
	DisposeNavEventUPP(eventUPP);

	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			err = FSRefMakePath(&(nav.ref), (unsigned char *) path, PATH_MAX);
			return (true);
		}
		else
			return (false);
	}
}

static pascal void NavGenericOpenEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	AEDesc			resultDesc;
	SInt32			count;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			break;

		case kNavCBStart:
			nav->reply = false;
			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionChoose:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AECountItems(&(reply.selection), &count);
						if ((err == noErr) && (count == 1))
						{
							err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
							if (err == noErr)
							{
								err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
								if (err == noErr)
									nav->reply = true;

								err = AEDisposeDesc(&resultDesc);
							}
						}

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

static pascal Boolean NavDefrostFromFilter (AEDesc *theItem, void *ninfo, NavCallBackUserData callbackUD, NavFilterModes filterMode)
{
	OSStatus	err;
	AEDesc 		resultDesc;
	Boolean		result = true;

	err = AECoerceDesc(theItem, typeFSRef, &resultDesc);
	if (err == noErr)
	{
		FSRef	ref;

		err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
		if (err == noErr)
		{
			FSCatalogInfo	catinfo;
			HFSUniStr255	unistr;

			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &catinfo, &unistr, NULL, NULL);
			if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask))
			{
				if (((FileInfo *) &catinfo.finderInfo)->fileType != 'SAVE')
				{
					UInt16	i = unistr.length;

					if (i < 4)
						result = false;
					else
					if ((unistr.unicode[i - 4] == '.') && (unistr.unicode[i - 3] == 'f') && (unistr.unicode[i - 2] == 'r'))
						result = true;
					else
						result = false;
				}
			}
		}

		AEDisposeDesc(&resultDesc);
	}

	return (result);
}

static pascal Boolean NavDefrostFromPreview (NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
#ifndef MAC_TIGER_PANTHER_SUPPORT
	return (true);
#else
	OSStatus	err;
	Boolean		previewShowing, result = false;

	err = NavCustomControl(callBackParms->context, kNavCtlIsPreviewShowing, &previewShowing);
	if ((err == noErr) && previewShowing)
	{
		AEDesc	resultDesc;

		err = AECoerceDesc((AEDesc *) callBackParms->eventData.eventDataParms.param, typeFSRef, &resultDesc);
		if (err == noErr)
		{
			FSRef	ref;

			err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
			if (err == noErr)
			{
				FSCatalogInfo	catinfo;

				err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags | kFSCatInfoContentMod, &catinfo, NULL, NULL, NULL);
				if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask))
				{
					CFMutableStringRef	sref;

					sref = CFStringCreateMutable(kCFAllocatorDefault, 0);
					if (sref)
					{
						CFAbsoluteTime		at;
						CFDateFormatterRef	format;
						CFLocaleRef			locale;
						CFStringRef			datstr;

						err = UCConvertUTCDateTimeToCFAbsoluteTime(&(catinfo.contentModDate), &at);
						locale = CFLocaleCopyCurrent();
						format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterMediumStyle);
						datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
						CFStringAppend(sref, datstr);
						CFRelease(datstr);
						CFRelease(format);
						CFRelease(locale);

						CGContextRef	ctx;
						CGRect			bounds;
						CGrafPtr		port;
						Rect			rct;
						int				rightedge;

						port = GetWindowPort(callBackParms->window);
						GetWindowBounds(callBackParms->window, kWindowContentRgn, &rct);

						err = QDBeginCGContext(port, &ctx);

						rightedge = (callBackParms->previewRect.right < (rct.right - rct.left - 11)) ? callBackParms->previewRect.right : (rct.right - rct.left - 11);

						bounds.origin.x = (float) callBackParms->previewRect.left;
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.bottom);
						bounds.size.width  = (float) (rightedge - callBackParms->previewRect.left);
						bounds.size.height = (float) (callBackParms->previewRect.bottom - callBackParms->previewRect.top);

						CGContextClipToRect(ctx, bounds);

						CGContextSetRGBFillColor(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
						CGContextFillRect(ctx, bounds);
						CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);

						bounds.origin.x = (float) (callBackParms->previewRect.left + ((callBackParms->previewRect.right - callBackParms->previewRect.left - 128) >> 1));
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 120 - 25);
						bounds.size.width  = 128.0f;
						bounds.size.height = 120.0f;

						DrawThumbnailResource(&ref, ctx, bounds);

						HIThemeTextInfo	textinfo;

						textinfo.version             = 0;
						textinfo.state               = kThemeStateActive;
						textinfo.fontID              = kThemeSmallSystemFont;
						textinfo.verticalFlushness   = kHIThemeTextVerticalFlushTop;
						textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushCenter;
						textinfo.options             = 0;
						textinfo.truncationPosition  = kHIThemeTextTruncationMiddle;
						textinfo.truncationMaxLines  = 0;
						textinfo.truncationHappened  = false;

						bounds.origin.x = (float) (callBackParms->previewRect.left + 10);
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 153 - 20);
						bounds.size.width  = (float) (callBackParms->previewRect.right - callBackParms->previewRect.left - 20);
						bounds.size.height = 20.0f;

						err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);

						CGContextSynchronize(ctx);

						err = QDEndCGContext(port, &ctx);

						CFRelease(sref);

						result = true;
					}
				}
			}

			AEDisposeDesc(&resultDesc);
		}
	}

	return (result);
#endif
}

bool8 NavBeginOpenROMImageSheet (WindowRef parent, CFStringRef mes)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	Rect						rct;
	HIViewID					cid = { 'PANE', 1000 };

	err = CreateNibReference(kMacS9XCFString, &(gSheetNav.customNib));
	if (err)
		return (false);

	err = CreateWindowFromNib(gSheetNav.customNib, CFSTR("OpenROM"), &(gSheetNav.customWindow));
	if (err)
	{
		DisposeNibReference(gSheetNav.customNib);
		return (false);
	}

	gSheetNav.isSheet = true;

	GetWindowBounds(gSheetNav.customWindow, kWindowContentRgn, &rct);
	gSheetNav.customWidth  = rct.right  - rct.left;
	gSheetNav.customHeight = rct.bottom - rct.top;

	HIViewFindByID(HIViewGetRoot(gSheetNav.customWindow), cid, &(gSheetNav.customPane));

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowPreviews & ~kNavAllowMultipleFiles & ~kNavAllowStationery;
	dialogOptions.preferenceKey = 3;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = CFCopyLocalizedString(CFSTR("OpenROMMes"), "Open");
	dialogOptions.parentWindow = parent;
	dialogOptions.modality = kWindowModalityWindowModal;
	dialogOptions.message = mes;

	gSheetEventUPP = NewNavEventUPP(NavOpenCartEventHandler);
	gSheetFilterUPP = NewNavObjectFilterUPP(NavOpenCartFilter);

	gSheetParent = parent;

	err = NavCreateChooseFileDialog(&dialogOptions, NULL, gSheetEventUPP, NULL, gSheetFilterUPP, &gSheetNav, &(gSheetNav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(gSheetNav.nref);
		if (err)
		{
			NavDialogDispose(gSheetNav.nref);
			DisposeNavObjectFilterUPP(gSheetFilterUPP);
			DisposeNavEventUPP(gSheetEventUPP);
			CFRelease(gSheetNav.customWindow);
			DisposeNibReference(gSheetNav.customNib);
			CFRelease(dialogOptions.windowTitle);
			return (false);
		}
	}
	else
	{
		DisposeNavObjectFilterUPP(gSheetFilterUPP);
		DisposeNavEventUPP(gSheetEventUPP);
		CFRelease(gSheetNav.customWindow);
		DisposeNibReference(gSheetNav.customNib);
		CFRelease(dialogOptions.windowTitle);
		return (false);
	}

	CFRelease(dialogOptions.windowTitle);
	return (true);
}

bool8 NavEndOpenROMImageSheet (FSRef *ref)
{
	DisposeNavObjectFilterUPP(gSheetFilterUPP);
	DisposeNavEventUPP(gSheetEventUPP);

	CFRelease(gSheetNav.customWindow);
	DisposeNibReference(gSheetNav.customNib);

	if (gSheetNav.reply)
	{
		*ref = gSheetNav.ref;
		return (true);
	}
	else
		return (false);
}

bool8 NavRecordMovieTo (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef, saveRef;
	Rect						rct;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
	HIViewID					cid = { 'PANE', 1000 };

	err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
	if (err)
		return (false);

	err = CreateWindowFromNib(nav.customNib, CFSTR("RecordSMV"), &(nav.customWindow));
	if (err)
	{
		DisposeNibReference(nav.customNib);
		return (false);
	}

	nav.isSheet = false;

	GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
	nav.customWidth  = rct.right  - rct.left;
	nav.customHeight = rct.bottom - rct.top;

	HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, MAC_PATH_ENCODING);
	numRef  = CFCopyLocalizedString(CFSTR("RecordToPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("RecordToMes"), "Record");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
	CFStringAppendCString(saveRef, ".smv", CFStringGetSystemEncoding());

	nav.customSRef = romRef;

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.preferenceKey = 6;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.saveFileName = saveRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;

	eventUPP = NewNavEventUPP(NavRecordMovieToEventHandler);

	gSheetParent = NULL;

	err = NavCreatePutFileDialog(&dialogOptions, 'SMOV', '~9X~', eventUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
        if (err)
        	NavDialogDispose(nav.nref);
	}

	DisposeNavEventUPP(eventUPP);

	CFRelease(saveRef);
	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	CFRelease(nav.customWindow);
	DisposeNibReference(nav.customNib);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			char	s[PATH_MAX + 1];

			err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
			snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);

			return (true);
		}
		else
			return (false);
	}
}

static pascal void NavRecordMovieToEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef 		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	CFStringRef		sref;
	CFIndex			cflen;
	AEDesc			resultDesc;
	Point			pt;
	UniChar			unistr[MOVIE_MAX_METADATA];

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'RCTL')
						{
							if (cid.id == 107)
							{
								HIViewRef	tmp;

								err = GetKeyboardFocus(callBackParms->window, &tmp);
								if ((err == noErr) && (tmp != ctl))
									err = SetKeyboardFocus(callBackParms->window, ctl, kControlFocusNextPart);
							}

							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);
						}
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'RCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			for (cid.id = 101; cid.id <= 106; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				SetControl32BitValue(ctl, (macRecordFlag & (1 << (cid.id - 101))) ? true : false);
			}

			cid.id = 107;
			HIViewFindByID(nav->customPane, cid, &ctl);
			if (nav->customSRef)
				SetEditTextCFString(ctl, nav->customSRef, false);

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			macRecordFlag = 0;
			cid.signature = 'RCTL';

			for (cid.id = 101; cid.id <= 106; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				if (GetControl32BitValue(ctl))
					macRecordFlag |= (1 << (cid.id - 101));
			}

			cid.id = 107;
			HIViewFindByID(nav->customPane, cid, &ctl);
			CopyEditTextCFString(ctl, &sref);
			if (sref)
			{
				cflen = CFStringGetLength(sref);
				if (cflen > MOVIE_MAX_METADATA - 1)
					cflen = MOVIE_MAX_METADATA - 1;

				CFStringGetCharacters(sref, CFRangeMake(0, cflen), unistr);

				for (int i = 0; i < cflen; i++)
					macRecordWChar[i] = (wchar_t) unistr[i];
				macRecordWChar[cflen] = 0;

				CFRelease(sref);
			}
			else
				macRecordWChar[0] = 0;

			break;

	  case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionSaveAs:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
						if (err == noErr)
						{
							err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
							if (err == noErr)
							{
								Boolean	r;

								r = CFStringGetCString(reply.saveFileName, nav->name, PATH_MAX, MAC_PATH_ENCODING);
								if (r)
									nav->reply = true;
							}

							err = AEDisposeDesc(&resultDesc);
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

bool8 NavPlayMovieFrom (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavObjectFilterUPP			filterUPP;
	NavPreviewUPP				previewUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef;
	Rect						rct;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
	HIViewID					cid = { 'PANE', 1000 };

	err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
	if (err)
		return (false);

	err = CreateWindowFromNib(nav.customNib, CFSTR("PlaySMV"), &(nav.customWindow));
	if (err)
	{
		DisposeNibReference(nav.customNib);
		return (false);
	}

	nav.isSheet = false;

	GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
	nav.customWidth  = rct.right  - rct.left;
	nav.customHeight = rct.bottom - rct.top;

	HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, MAC_PATH_ENCODING);
	numRef  = CFCopyLocalizedString(CFSTR("PlayFromPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("PlayFromMes"), "Play");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.optionFlags &= ~kNavAllowMultipleFiles & ~kNavAllowStationery;
	dialogOptions.preferenceKey = 6;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;

	eventUPP = NewNavEventUPP(NavPlayMovieFromEventHandler);
	filterUPP = NewNavObjectFilterUPP(NavPlayMovieFromFilter);
	previewUPP = NewNavPreviewUPP(NavPlayMovieFromPreview);

	gSheetParent = NULL;

	err = NavCreateChooseFileDialog(&dialogOptions, NULL, eventUPP, previewUPP, filterUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
        if (err)
        	NavDialogDispose(nav.nref);
	}

	DisposeNavPreviewUPP(previewUPP);
	DisposeNavObjectFilterUPP(filterUPP);
	DisposeNavEventUPP(eventUPP);

	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	CFRelease(nav.customWindow);
	DisposeNibReference(nav.customNib);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			err = FSRefMakePath(&(nav.ref), (unsigned char *) path, PATH_MAX);
			return (true);
		}
		else
			return (false);
	}
}

static pascal void NavPlayMovieFromEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef 		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	AEDesc			resultDesc;
	Point			pt;
	SInt32 			count;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'PCTL')
							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'PCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				SetControl32BitValue(ctl, (macPlayFlag & (1 << (cid.id - 101))) ? true : false);
			}

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			macPlayFlag = 0;
			cid.signature = 'PCTL';

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				if (GetControl32BitValue(ctl))
					macPlayFlag |= (1 << (cid.id - 101));
			}

			break;

	  case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionChoose:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AECountItems(&(reply.selection), &count);
						if ((err == noErr) && (count == 1))
						{
							err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
							if (err == noErr)
							{
								err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
								if (err == noErr)
									nav->reply = true;

								err = AEDisposeDesc(&resultDesc);
							}
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

static pascal Boolean NavPlayMovieFromFilter (AEDesc *theItem, void *ninfo, NavCallBackUserData callbackUD, NavFilterModes filterMode)
{
	OSStatus	err;
	AEDesc 		resultDesc;
	Boolean		result = true;

	err = AECoerceDesc(theItem, typeFSRef, &resultDesc);
	if (err == noErr)
	{
		FSRef	ref;

		err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
		if (err == noErr)
		{
			FSCatalogInfo	catinfo;
			HFSUniStr255	unistr;

			err = FSGetCatalogInfo(&ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &catinfo, &unistr, NULL, NULL);
			if ((err == noErr) && !(catinfo.nodeFlags & kFSNodeIsDirectoryMask))
			{
				if (((FileInfo *) &catinfo.finderInfo)->fileType != 'SMOV')
				{
					UInt16	i = unistr.length;

					if (i < 4)
						result = false;
					else
					if  ((unistr.unicode[i - 4] == '.') && (unistr.unicode[i - 3] == 's') && (unistr.unicode[i - 2] == 'm') && (unistr.unicode[i - 1] == 'v'))
						result = true;
					else
						result = false;
				}
			}
		}

		AEDisposeDesc(&resultDesc);
	}

	return (result);
}

static pascal Boolean NavPlayMovieFromPreview (NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
#ifndef MAC_TIGER_PANTHER_SUPPORT
	return (true);
#else
	OSStatus	err;
	Boolean		previewShowing, result = false;

	err = NavCustomControl(callBackParms->context, kNavCtlIsPreviewShowing, &previewShowing);
	if ((err == noErr) && previewShowing)
	{
		AEDesc	resultDesc;

		err = AECoerceDesc((AEDesc *) callBackParms->eventData.eventDataParms.param, typeFSRef, &resultDesc);
		if (err == noErr)
		{
			FSRef	ref;

			err = AEGetDescData(&resultDesc, &ref, sizeof(FSRef));
			if (err == noErr)
			{
				char	path[PATH_MAX + 1];

				err = FSRefMakePath(&ref, (unsigned char *) path, PATH_MAX);
				if (err == noErr)
				{
					MovieInfo	movinfo;
					int			r;

					r = S9xMovieGetInfo(path, &movinfo);
					if (r == SUCCESS)
					{
						UTCDateTime		utctime;
						CGContextRef	ctx;
						CGRect			bounds;
						CGrafPtr		port;
						Rect			rct;
						CFStringRef		sref;
						UInt64			t;
						int				rightedge, border, width, l, sec, min, hr, n;
						char			cstr[256], cbuf[512];
						UniChar			unistr[MOVIE_MAX_METADATA];

						sref = CFCopyLocalizedString(CFSTR("MoviePrevBorder"), "1");
						if (sref)
						{
							border = CFStringGetIntValue(sref);
							CFRelease(sref);
						}
						else
							border = 1;

						sref = CFCopyLocalizedString(CFSTR("MoviePrevWidth"), "1");
						if (sref)
						{
							width = CFStringGetIntValue(sref);
							CFRelease(sref);
						}
						else
							width = 1;

						// Date

						t = (UInt64) movinfo.TimeCreated + ((365 * 66 + 17) * 24 * 60 * 60);
						utctime.highSeconds = (UInt16) (t >> 32);
						utctime.lowSeconds  = (UInt32) (t & 0xFFFFFFFF);
						utctime.fraction    = 0;

						CFAbsoluteTime		at;
						CFDateFormatterRef	format;
						CFLocaleRef			locale;
						CFStringRef			datstr;
						Boolean				e;

						err = UCConvertUTCDateTimeToCFAbsoluteTime(&utctime, &at);
						locale = CFLocaleCopyCurrent();
						format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterShortStyle, kCFDateFormatterNoStyle);
						datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
						e = CFStringGetCString(datstr, cbuf, sizeof(cbuf), CFStringGetSystemEncoding());
						CFRelease(datstr);
						CFRelease(format);
						strcat(cbuf, "\n");
						format = CFDateFormatterCreate(kCFAllocatorDefault, locale, kCFDateFormatterNoStyle, kCFDateFormatterMediumStyle);
						datstr = CFDateFormatterCreateStringWithAbsoluteTime(kCFAllocatorDefault, format, at);
						e = CFStringGetCString(datstr, cstr, sizeof(cstr), CFStringGetSystemEncoding());
						CFRelease(datstr);
						CFRelease(format);
						strcat(cbuf, cstr);
						strcat(cbuf, "\n");
						CFRelease(locale);

						// Length

						l = (movinfo.LengthFrames + 30) / ((movinfo.Opts & MOVIE_OPT_PAL) ? 50 : 60);
						sec = l % 60;	l /= 60;
						min = l % 60;	l /= 60;
						hr  = l % 60;

						sprintf(cstr, "%02d:%02d:%02d\n", hr, min, sec);
						strcat(cbuf, cstr);

						// Frames

						sprintf(cstr, "%d\n", movinfo.LengthFrames);
						strcat(cbuf, cstr);

						//

						port = GetWindowPort(callBackParms->window);
						GetWindowBounds(callBackParms->window, kWindowContentRgn, &rct);

						err = QDBeginCGContext(port, &ctx);

						rightedge = (callBackParms->previewRect.right < (rct.right - rct.left - 11)) ? callBackParms->previewRect.right : (rct.right - rct.left - 11);

						bounds.origin.x = (float) callBackParms->previewRect.left;
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.bottom);
						bounds.size.width  = (float) (rightedge - callBackParms->previewRect.left);
						bounds.size.height = (float) (callBackParms->previewRect.bottom - callBackParms->previewRect.top);

						CGContextClipToRect(ctx, bounds);

						CGContextSetRGBFillColor(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
						CGContextFillRect(ctx, bounds);
						CGContextSetRGBFillColor(ctx, 0.0f, 0.0f, 0.0f, 1.0f);

						// Thumbnail

						bounds.origin.x  = (float) (callBackParms->previewRect.left + ((callBackParms->previewRect.right - callBackParms->previewRect.left - 128) >> 1));
						bounds.origin.y  = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 120 - 25);
						bounds.size.width  = 128.0f;
						bounds.size.height = 120.0f;

						DrawThumbnailResource(&ref, ctx, bounds);

						// Text

						HIThemeTextInfo	textinfo;

						textinfo.version            = 0;
						textinfo.state              = kThemeStateActive;
						textinfo.fontID             = kThemeSmallSystemFont;
						textinfo.verticalFlushness  = kHIThemeTextVerticalFlushTop;
						textinfo.options            = 0;
						textinfo.truncationPosition = kHIThemeTextTruncationMiddle;
						textinfo.truncationMaxLines = 0;
						textinfo.truncationHappened = false;

						bounds.origin.x = (float) (((callBackParms->previewRect.right - callBackParms->previewRect.left - width) >> 1) + callBackParms->previewRect.left + border + 7);
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 153 - 60);
						bounds.size.width  = (float) callBackParms->previewRect.right - bounds.origin.x;
						bounds.size.height = 60.0f;

						sref = CFStringCreateWithCString(kCFAllocatorDefault, cbuf, CFStringGetSystemEncoding());
						if (sref)
						{
							textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
							err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);
							CFRelease(sref);
						}

						bounds.origin.x = (float) callBackParms->previewRect.left;
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.top - 153 - 60);
						bounds.size.width  = (float) (((callBackParms->previewRect.right - callBackParms->previewRect.left - width) >> 1) + border);
						bounds.size.height = 60.0f;

						sref = CFCopyLocalizedString(CFSTR("MoviePrevMes"), "MovieInfo");
						if (sref)
						{
							textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushRight;
							err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);
							CFRelease(sref);
						}

						bounds.origin.x = (float) (((callBackParms->previewRect.right - callBackParms->previewRect.left - 132) >> 1) + callBackParms->previewRect.left);
						bounds.origin.y = (float) (rct.bottom - rct.top - callBackParms->previewRect.bottom + 10);
						bounds.size.width  = 132.0f;
						bounds.size.height = (float) (callBackParms->previewRect.bottom - callBackParms->previewRect.top - 223 - 10);

						n = wcslen(movinfo.Metadata);

						for (int i = 0; i < n; i++)
							unistr[i] = (UniChar) movinfo.Metadata[i];
						unistr[n] = 0;

						sref = CFStringCreateWithCharacters(kCFAllocatorDefault, unistr, n);
						if (sref)
						{
							textinfo.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
							err = HIThemeDrawTextBox(sref, &bounds, &textinfo, ctx, kHIThemeOrientationInverted);
							CFRelease(sref);
						}

						CGContextSynchronize(ctx);

						err = QDEndCGContext(port, &ctx);

						result = true;
					}
				}
			}

			AEDisposeDesc(&resultDesc);
		}
	}

	return (result);
#endif
}

bool8 NavQTMovieRecordTo (char *path)
{
	OSStatus					err;
	NavDialogCreationOptions	dialogOptions;
	NavEventUPP					eventUPP;
	NavState					nav;
	CFStringRef					numRef, romRef, baseRef;
	CFMutableStringRef			mesRef, saveRef;
	Rect						rct;
	SInt32						replaceAt;
	char						drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], fname[_MAX_FNAME + 1], ext[_MAX_EXT + 1];
	HIViewID					cid = { 'PANE', 1000 };

	err = CreateNibReference(kMacS9XCFString, &(nav.customNib));
	if (err)
		return (false);

	err = CreateWindowFromNib(nav.customNib, CFSTR("QTMovie"), &(nav.customWindow));
	if (err)
	{
		DisposeNibReference(nav.customNib);
		return (false);
	}

	nav.isSheet = false;

	GetWindowBounds(nav.customWindow, kWindowContentRgn, &rct);
	nav.customWidth  = rct.right  - rct.left;
	nav.customHeight = rct.bottom - rct.top;

	HIViewFindByID(HIViewGetRoot(nav.customWindow), cid, &(nav.customPane));

	_splitpath(Memory.ROMFilename, drive, dir, fname, ext);
	romRef  = CFStringCreateWithCString(kCFAllocatorDefault, fname, MAC_PATH_ENCODING);
	numRef  = CFCopyLocalizedString(CFSTR("QTRecordPos"), "1");
	baseRef = CFCopyLocalizedString(CFSTR("QTRecordMes"), "QT");
	mesRef  = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, baseRef);
	saveRef = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, romRef);
	replaceAt = CFStringGetIntValue(numRef);
	CFStringReplace(mesRef, CFRangeMake(replaceAt - 1, 1), romRef);
	CFStringAppendCString(saveRef, ".mov", CFStringGetSystemEncoding());

	nav.customSRef = romRef;

	err = NavGetDefaultDialogCreationOptions(&dialogOptions);
	dialogOptions.preferenceKey = 7;
	dialogOptions.clientName = kMacS9XCFString;
	dialogOptions.windowTitle = mesRef;
	dialogOptions.saveFileName = saveRef;
	dialogOptions.modality = kWindowModalityAppModal;
	dialogOptions.parentWindow = NULL;

	eventUPP = NewNavEventUPP(NavQTMovieRecordToEventHandler);

	gSheetParent = NULL;

	err = NavCreatePutFileDialog(&dialogOptions, 'MooV', 'TVOD', eventUPP, &nav, &(nav.nref));
	if (err == noErr)
	{
		err = NavDialogRun(nav.nref);
        if (err)
        	NavDialogDispose(nav.nref);
	}

	DisposeNavEventUPP(eventUPP);

	CFRelease(saveRef);
	CFRelease(mesRef);
	CFRelease(baseRef);
	CFRelease(numRef);
	CFRelease(romRef);

	CFRelease(nav.customWindow);
	DisposeNibReference(nav.customNib);

	if (err)
		return (false);
	else
	{
		if (nav.reply)
		{
			char	s[PATH_MAX + 1];

			err = FSRefMakePath(&(nav.ref), (unsigned char *) s, PATH_MAX);
			snprintf(path, PATH_MAX + 1, "%s%s%s", s, MAC_PATH_SEPARATOR, nav.name);

			return (true);
		}
		else
			return (false);
	}
}

static pascal void NavQTMovieRecordToEventHandler (const NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD)
{
	static Boolean	embedded = false;

	OSStatus		err;
	NavReplyRecord	reply;
	NavUserAction	userAction;
	NavState		*nav;
	HIViewRef 		ctl;
	HIViewID		cid;
	HIViewPartCode	part;
	MenuRef			menu;
	AEDesc			resultDesc;
	Point			pt;

	nav = (NavState *) callBackUD;

	switch (callBackSelector)
	{
		case kNavCBEvent:
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case mouseDown:
					pt = callBackParms->eventData.eventDataParms.event->where;
					GlobalPointToWindowLocalPoint(&pt, callBackParms->window);

					ctl = FindControlUnderMouse(pt, callBackParms->window, &part);
					if (ctl)
					{
						GetControlID(ctl, &cid);
						if (cid.signature == 'QCTL')
							part = HandleControlClick(ctl, pt, callBackParms->eventData.eventDataParms.event->modifiers, (ControlActionUPP) -1L);

						if (cid.id == 103)
							MacQTVideoConfig(NavDialogGetWindow(callBackParms->context));
					}

					break;
			}

			break;

		case kNavCBStart:
			nav->reply = false;
			cid.signature = 'QCTL';
			err = NavCustomControl(callBackParms->context, kNavCtlAddControl, nav->customPane);

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				SetControl32BitValue(ctl, (macQTMovFlag & (1 << (cid.id - 101))) ? true : false);
			}

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			menu = HIMenuViewGetMenu(ctl);
			for (int i = 1; i <= CountMenuItems(menu); i++)
				CheckMenuItem(menu, i, false);
			SetControl32BitValue(ctl, ((macQTMovFlag & 0xFF00) >> 8) + 1);

			MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
										 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			embedded = true;

			break;

		case kNavCBAccept:
			macQTMovFlag = 0;
			cid.signature = 'QCTL';

			for (cid.id = 101; cid.id <= 102; cid.id++)
			{
				HIViewFindByID(nav->customPane, cid, &ctl);
				if (GetControl32BitValue(ctl))
					macQTMovFlag |= (1 << (cid.id - 101));
			}

			cid.id = 104;
			HIViewFindByID(nav->customPane, cid, &ctl);
			macQTMovFlag |= ((GetControl32BitValue(ctl) - 1) << 8);

			break;

	  case kNavCBCustomize:
			if ((callBackParms->customRect.right == 0) && (callBackParms->customRect.bottom == 0))	// First call
			{
				embedded = false;

				callBackParms->customRect.right  = callBackParms->customRect.left + nav->customWidth;
				callBackParms->customRect.bottom = callBackParms->customRect.top  + nav->customHeight;
			}

			break;

		case kNavCBAdjustRect:
			if (embedded)
				MoveControl(nav->customPane, ((callBackParms->customRect.right  - callBackParms->customRect.left) - nav->customWidth ) / 2 + callBackParms->customRect.left,
											 ((callBackParms->customRect.bottom - callBackParms->customRect.top ) - nav->customHeight) / 2 + callBackParms->customRect.top);

			break;

		case kNavCBUserAction:
			userAction = NavDialogGetUserAction(callBackParms->context);
			switch (userAction)
			{
				case kNavUserActionSaveAs:
					err = NavDialogGetReply(callBackParms->context, &reply);
					if (err == noErr)
					{
						err = AEGetNthDesc(&(reply.selection), 1, typeFSRef, NULL, &resultDesc);
						if (err == noErr)
						{
							err = AEGetDescData(&resultDesc, &(nav->ref), sizeof(FSRef));
							if (err == noErr)
							{
								Boolean	r;

								r = CFStringGetCString(reply.saveFileName, nav->name, PATH_MAX, MAC_PATH_ENCODING);
								if (r)
									nav->reply = true;
							}

							err = AEDisposeDesc(&resultDesc);
				        }

						err = NavDisposeReply(&reply);
					}

					break;
            }

            break;

		case kNavCBTerminate:
			NavDialogDispose(nav->nref);
			break;
	}
}

static void GlobalPointToWindowLocalPoint (Point *pt, WindowRef window)
{
	if (systemVersion >= 0x1040)
	{
		HIViewRef	view;
		HIPoint		cpt = CGPointMake((float) pt->h, (float) pt->v);

		HIViewFindByID(HIViewGetRoot(window), kHIViewWindowContentID, &view);
		HIPointConvert(&cpt, kHICoordSpace72DPIGlobal, NULL, kHICoordSpaceView, view);
		pt->h = (short) cpt.x;
		pt->v = (short) cpt.y;
	}
#ifdef MAC_PANTHER_SUPPORT
	else
		QDGlobalToLocalPoint(GetWindowPort(window), pt);
#endif
}
