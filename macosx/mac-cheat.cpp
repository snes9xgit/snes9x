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


#include "port.h"
#include "cheats.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-stringtools.h"
#include "mac-cheat.h"

#define	kDataBrowser	'BRSR'
#define	kCmCheckBox		'CHK_'
#define	kCmAddress		'ADDR'
#define	kCmValue		'VALU'
#define	kCmDescription	'DESC'
#define	kNewButton		'NEW_'
#define	kDelButton		'DEL_'
#define	kAllButton		'ALL_'

extern SCheatData	Cheat;

typedef struct
{
	uint32	id;
	uint32	address;
	uint8	value;
	bool8	valid;
	bool8	enabled;
	char	description[22];
}	CheatItem;

static WindowRef	wRef;
static HIViewRef	dbRef;
static CheatItem	citem[MAC_MAX_CHEATS];
static uint32		numofcheats;

static void InitCheatItems (void);
static void ImportCheatItems (void);
static void DetachCheatItems (void);
static void AddCheatItem (void);
static void DeleteCheatItem (void);
static void EnableAllCheatItems (void);
static pascal void DBItemNotificationCallBack (HIViewRef, DataBrowserItemID, DataBrowserItemNotification);
static pascal Boolean DBCompareCallBack (HIViewRef, DataBrowserItemID, DataBrowserItemID, DataBrowserPropertyID);
static pascal OSStatus DBClientDataCallback (HIViewRef, DataBrowserItemID, DataBrowserPropertyID, DataBrowserItemDataRef, Boolean);
static pascal OSStatus CheatEventHandler (EventHandlerCallRef, EventRef, void *);


static void InitCheatItems (void)
{
	for (unsigned int i = 0; i < MAC_MAX_CHEATS; i++)
	{
		citem[i].id      = i + 1;
		citem[i].valid   = false;
		citem[i].enabled = false;
		citem[i].address = 0;
		citem[i].value   = 0;
		sprintf(citem[i].description, "Cheat %03" PRIu32, citem[i].id);
	}
}

static void ImportCheatItems (void)
{
    int cheat_num = std::min((int)Cheat.g.size(), MAC_MAX_CHEATS);
	for (unsigned int i = 0; i < cheat_num; i++)
	{
		citem[i].valid   = true;
		citem[i].enabled = Cheat.g[i].enabled;
		citem[i].address = Cheat.g[i].c[0].address; // mac dialog only supports one cheat per group at the moment
		citem[i].value   = Cheat.g[i].c[0].byte;
		strncpy(citem[i].description, Cheat.g[i].name, 21);
		citem[i].description[21] = '\0';
	}
}

static void DetachCheatItems (void)
{
	S9xDeleteCheats();

	for (unsigned int i = 0; i < MAC_MAX_CHEATS; i++)
	{
		if (citem[i].valid)
		{
			char code[10];
			snprintf(code, 10, "%x=%x", citem[i].address, citem[i].value);
			int index = S9xAddCheatGroup(citem[i].description, code);
			if(citem[i].enabled && index >= 0)
				S9xEnableCheatGroup(index);
		}
	}
}

void ConfigureCheat (void)
{
	if (!cartOpen)
		return;

	OSStatus	err;
	IBNibRef	nibRef;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		err = CreateWindowFromNib(nibRef, CFSTR("CheatEntry"), &wRef);
		if (err == noErr)
		{
			DataBrowserCallbacks	callbacks;
			EventHandlerRef			eref;
			EventHandlerUPP			eUPP;
			EventTypeSpec			events[] = { { kEventClassCommand, kEventCommandProcess      },
												 { kEventClassCommand, kEventCommandUpdateStatus },
												 { kEventClassWindow,  kEventWindowClose         } };
			HIViewRef				ctl, root;
			HIViewID				cid;

			root = HIViewGetRoot(wRef);
			cid.id = 0;
			cid.signature = kDataBrowser;
			HIViewFindByID(root, cid, &dbRef);

		#ifdef MAC_PANTHER_SUPPORT
			if (systemVersion < 0x1040)
			{
				HISize	minSize;
				Rect	rct;

				GetWindowBounds(wRef, kWindowContentRgn, &rct);
				minSize.width  = (float) (rct.right  - rct.left);
				minSize.height = (float) (rct.bottom - rct.top );
				err = SetWindowResizeLimits(wRef, &minSize, NULL);
			}
		#endif

			callbacks.version = kDataBrowserLatestCallbacks;
			err = InitDataBrowserCallbacks(&callbacks);
			callbacks.u.v1.itemDataCallback = NewDataBrowserItemDataUPP(DBClientDataCallback);
			callbacks.u.v1.itemCompareCallback = NewDataBrowserItemCompareUPP(DBCompareCallBack);
			callbacks.u.v1.itemNotificationCallback = NewDataBrowserItemNotificationUPP(DBItemNotificationCallBack);
			err = SetDataBrowserCallbacks(dbRef, &callbacks);

			if (systemVersion >= 0x1040)
				err = DataBrowserChangeAttributes(dbRef, kDataBrowserAttributeListViewAlternatingRowColors, kDataBrowserAttributeNone);

			InitCheatItems();
			ImportCheatItems();

			DataBrowserItemID	*id;

			id = new DataBrowserItemID[MAC_MAX_CHEATS];
			if (!id)
				QuitWithFatalError(0, "cheat 01");

			numofcheats = 0;

			for (unsigned int i = 0; i < MAC_MAX_CHEATS; i++)
			{
				if (citem[i].valid)
				{
					id[numofcheats] = citem[i].id;
					numofcheats++;
				}
			}

			if (numofcheats)
				err = AddDataBrowserItems(dbRef, kDataBrowserNoItem, numofcheats, id, kDataBrowserItemNoProperty);

			delete [] id;

			cid.signature = kNewButton;
			HIViewFindByID(root, cid, &ctl);
			if (numofcheats == MAC_MAX_CHEATS)
				err = DeactivateControl(ctl);
			else
				err = ActivateControl(ctl);

			cid.signature = kAllButton;
			HIViewFindByID(root, cid, &ctl);
			if (numofcheats == 0)
				err = DeactivateControl(ctl);
			else
				err = ActivateControl(ctl);

			cid.signature = kDelButton;
			HIViewFindByID(root, cid, &ctl);
			err = DeactivateControl(ctl);

			eUPP = NewEventHandlerUPP(CheatEventHandler);
			err = InstallWindowEventHandler(wRef, eUPP, GetEventTypeCount(events), events, (void *) wRef, &eref);

			err = SetKeyboardFocus(wRef, dbRef, kControlFocusNextPart);

			MoveWindowPosition(wRef, kWindowCheatEntry, true);
			ShowWindow(wRef);
			err = RunAppModalLoopForWindow(wRef);
			HideWindow(wRef);
			SaveWindowPosition(wRef, kWindowCheatEntry);

			err = RemoveEventHandler(eref);
			DisposeEventHandlerUPP(eUPP);

			DisposeDataBrowserItemNotificationUPP(callbacks.u.v1.itemNotificationCallback);
			DisposeDataBrowserItemCompareUPP(callbacks.u.v1.itemCompareCallback);
			DisposeDataBrowserItemDataUPP(callbacks.u.v1.itemDataCallback);

			CFRelease(wRef);

			DetachCheatItems();
		}

		DisposeNibReference(nibRef);
	}
}

static void AddCheatItem (void)
{
	OSStatus			err;
	HIViewRef			ctl, root;
	HIViewID			cid;
	DataBrowserItemID	id[1];
	unsigned int		i;

	if (numofcheats == MAC_MAX_CHEATS)
		return;

	for (i = 0; i < MAC_MAX_CHEATS; i++)
		if (citem[i].valid == false)
			break;

	if (i == MAC_MAX_CHEATS)
		return;

	numofcheats++;
	citem[i].valid   = true;
	citem[i].enabled = false;
	citem[i].address = 0;
	citem[i].value   = 0;
	sprintf(citem[i].description, "Cheat %03" PRIu32, citem[i].id);

	id[0] = citem[i].id;
	err = AddDataBrowserItems(dbRef, kDataBrowserNoItem, 1, id, kDataBrowserItemNoProperty);
	err = RevealDataBrowserItem(dbRef, id[0], kCmAddress, true);

	root = HIViewGetRoot(wRef);
	cid.id = 0;

	if (numofcheats == MAC_MAX_CHEATS)
	{
		cid.signature = kNewButton;
		HIViewFindByID(root, cid, &ctl);
		err = DeactivateControl(ctl);
	}

	if (numofcheats)
	{
		cid.signature = kAllButton;
		HIViewFindByID(root, cid, &ctl);
		err = ActivateControl(ctl);
	}
}

static void DeleteCheatItem (void)
{
	OSStatus	err;
	HIViewRef	ctl, root;
	HIViewID	cid;
	Handle		selectedItems;
	ItemCount	selectionCount;

	selectedItems = NewHandle(0);
	if (!selectedItems)
		return;

	err = GetDataBrowserItems(dbRef, kDataBrowserNoItem, true, kDataBrowserItemIsSelected, selectedItems);
	selectionCount = (GetHandleSize(selectedItems) / sizeof(DataBrowserItemID));

	if (selectionCount == 0)
	{
		DisposeHandle(selectedItems);
		return;
	}

	err = RemoveDataBrowserItems(dbRef, kDataBrowserNoItem, selectionCount, (DataBrowserItemID *) *selectedItems, kDataBrowserItemNoProperty);

	for (unsigned int i = 0; i < selectionCount; i++)
	{
		citem[((DataBrowserItemID *) (*selectedItems))[i] - 1].valid   = false;
		citem[((DataBrowserItemID *) (*selectedItems))[i] - 1].enabled = false;
		numofcheats--;
	}

	DisposeHandle(selectedItems);

	root = HIViewGetRoot(wRef);
	cid.id = 0;

	if (numofcheats < MAC_MAX_CHEATS)
	{
		cid.signature = kNewButton;
		HIViewFindByID(root, cid, &ctl);
		err = ActivateControl(ctl);
	}

	if (numofcheats == 0)
	{
		cid.signature = kAllButton;
		HIViewFindByID(root, cid, &ctl);
		err = DeactivateControl(ctl);
	}
}

static void EnableAllCheatItems (void)
{
	OSStatus	err;

	for (unsigned int i = 0; i < MAC_MAX_CHEATS; i++)
		if (citem[i].valid)
			citem[i].enabled = true;

	err = UpdateDataBrowserItems(dbRef, kDataBrowserNoItem, kDataBrowserNoItem, NULL, kDataBrowserItemNoProperty, kCmCheckBox);
}

static pascal OSStatus DBClientDataCallback (HIViewRef browser, DataBrowserItemID itemID, DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue)
{
	OSStatus 	err, result;
	CFStringRef	str;
	Boolean		r;
	uint32		address;
	uint8		value;
	char		code[256];

	result = noErr;

	switch (property)
	{
		case kCmCheckBox:
			ThemeButtonValue	buttonValue;

            if (changeValue)
			{
				err = GetDataBrowserItemDataButtonValue(itemData, &buttonValue);
				citem[itemID - 1].enabled = (buttonValue == kThemeButtonOn) ? true : false;
 	        }
			else
				err = SetDataBrowserItemDataButtonValue(itemData, citem[itemID - 1].enabled ? kThemeButtonOn : kThemeButtonOff);

			break;

		case kCmAddress:
			if (changeValue)
			{
				err = GetDataBrowserItemDataText(itemData, &str);
				r = CFStringGetCString(str, code, 256, CFStringGetSystemEncoding());
				CFRelease(str);
				if (r)
				{
					Boolean	translated;

					if (S9xProActionReplayToRaw(code, address, value) == NULL)
						translated = true;
					else
					if (S9xGameGenieToRaw(code, address, value) == NULL)
						translated = true;
					else
					{
						translated = false;
						if (sscanf(code, "%" SCNx32, &address) != 1)
							address = 0;
						else
							address &= 0xFFFFFF;
					}

					citem[itemID - 1].address = address;
					sprintf(code, "%06" PRIX32, address);
					str = CFStringCreateWithCString(kCFAllocatorDefault, code, CFStringGetSystemEncoding());
					err = SetDataBrowserItemDataText(itemData, str);
					CFRelease(str);

					if (translated)
					{
						DataBrowserItemID	id[1];

						citem[itemID - 1].value = value;
						id[0] = itemID;
						err = UpdateDataBrowserItems(browser, kDataBrowserNoItem, 1, id, kDataBrowserItemNoProperty, kCmValue);
					}
				}
			}
			else
			{
				sprintf(code, "%06" PRIX32, citem[itemID - 1].address);
				str = CFStringCreateWithCString(kCFAllocatorDefault, code, CFStringGetSystemEncoding());
				err = SetDataBrowserItemDataText(itemData, str);
				CFRelease(str);
			}

			break;

		case kCmValue:
			if (changeValue)
			{
				err = GetDataBrowserItemDataText(itemData, &str);
				r = CFStringGetCString(str, code, 256, CFStringGetSystemEncoding());
				CFRelease(str);
				if (r)
				{
					uint32	byte;

					if (sscanf(code, "%" SCNx32, &byte) == 1)
						citem[itemID - 1].value = (uint8) byte;
					else
					{
						citem[itemID - 1].value = 0;
						err = SetDataBrowserItemDataText(itemData, CFSTR("00"));
					}
				}
			}
			else
			{
				sprintf(code, "%02" PRIX8, citem[itemID - 1].value);
				str = CFStringCreateWithCString(kCFAllocatorDefault, code, CFStringGetSystemEncoding());
				err = SetDataBrowserItemDataText(itemData, str);
				CFRelease(str);
			}

			break;

		case kCmDescription:
			if (changeValue)
			{
				code[0] = 0;
				err = GetDataBrowserItemDataText(itemData, &str);
				strcpy(code, GetMultiByteCharacters(str, 19));
				CFRelease(str);

				if (code[0] == 0)
				{
					code[0] = ' ';
					code[1] = 0;
				}

				strcpy(citem[itemID - 1].description, code);
			}
			else
			{
				str = CFStringCreateWithCString(kCFAllocatorDefault, citem[itemID - 1].description, CFStringGetSystemEncoding());
				err = SetDataBrowserItemDataText(itemData, str);
				CFRelease(str);
			}

			break;

		case kDataBrowserItemIsActiveProperty:
			err = SetDataBrowserItemDataBooleanValue(itemData, true);
			break;

		case kDataBrowserItemIsEditableProperty:
			err = SetDataBrowserItemDataBooleanValue(itemData, true);
			break;

		default:
			result = errDataBrowserPropertyNotSupported;
	}

	return (result);
}

static pascal Boolean DBCompareCallBack (HIViewRef browser, DataBrowserItemID itemOne, DataBrowserItemID itemTwo, DataBrowserPropertyID sortProperty)
{
	Boolean	result = false;

	switch (sortProperty)
	{
		case kCmCheckBox:
			result = (citem[itemOne - 1].enabled && !(citem[itemTwo - 1].enabled)) ? true : false;
			break;

		case kCmAddress:
			result = (citem[itemOne - 1].address <    citem[itemTwo - 1].address)  ? true : false;
			break;

		case kCmValue:
			result = (citem[itemOne - 1].value   <    citem[itemTwo - 1].value)    ? true : false;
			break;

		case kCmDescription:
			result = (strcmp(citem[itemOne - 1].description, citem[itemTwo - 1].description) < 0) ? true : false;
	}

	return (result);
}

static pascal void DBItemNotificationCallBack (HIViewRef browser, DataBrowserItemID itemID, DataBrowserItemNotification message)
{
	OSStatus	err;
	HIViewRef	ctl;
	HIViewID	cid = { kDelButton, 0 };
	ItemCount	selectionCount;

	switch (message)
	{
		case kDataBrowserSelectionSetChanged:
			HIViewFindByID(HIViewGetRoot(wRef), cid, &ctl);

			err = GetDataBrowserItemCount(browser, kDataBrowserNoItem, true, kDataBrowserItemIsSelected, &selectionCount);
			if (selectionCount == 0)
				err = DeactivateControl(ctl);
			else
				err = ActivateControl(ctl);
	}
}

static pascal OSStatus CheatEventHandler (EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	tWindowRef;

	tWindowRef = (WindowRef) inUserData;

	switch (GetEventClass(inEvent))
	{
		case kEventClassWindow:
			switch (GetEventKind(inEvent))
			{
				case kEventWindowClose:
					QuitAppModalLoopForWindow(tWindowRef);
					result = noErr;
					break;
			}

			break;

		case kEventClassCommand:
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(true);
						result = noErr;
					}

					break;

				case kEventCommandProcess:
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						switch (tHICommand.commandID)
						{
							case kNewButton:
								AddCheatItem();
								result = noErr;
								break;

							case kDelButton:
								DeleteCheatItem();
								result = noErr;
								break;

							case kAllButton:
								EnableAllCheatItems();
								result = noErr;
						}
					}
			}
	}

	return (result);
}
