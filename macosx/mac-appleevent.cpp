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

#include "mac-prefix.h"
#include "mac-os.h"
#include "mac-snes9x.h"
#include "mac-appleevent.h"

static AEEventHandlerUPP	oappUPP, rappUPP, pdocUPP, quitUPP, odocUPP;

static pascal OSErr AEoapp (const AppleEvent *, AppleEvent *, long);
static pascal OSErr AErapp (const AppleEvent *, AppleEvent *, long);
static pascal OSErr AEpdoc (const AppleEvent *, AppleEvent *, long);
static pascal OSErr AEquit (const AppleEvent *, AppleEvent *, long);
static pascal OSErr AEodoc (const AppleEvent *, AppleEvent *, long);


static pascal OSErr AEoapp (const AppleEvent *theEvent, AppleEvent *theReply, long refCon)
{
	if (running)
		return (noErr);

	if (startopendlog)
	{
		if (SNES9X_OpenCart(NULL))
		{
			SNES9X_Go();
			QuitApplicationEventLoop();
		}
		else
			AdjustMenus();
	}

	return (noErr);
}

static pascal OSErr AErapp (const AppleEvent *theEvent, AppleEvent *theReply, long refCon)
{
	if (running)
		return (noErr);

	if (!cartOpen)
	{
		if (startopendlog)
		{
			if (SNES9X_OpenCart(NULL))
			{
				SNES9X_Go();
				QuitApplicationEventLoop();
			}
			else
				AdjustMenus();
		}
	}
	else
	{
		SNES9X_Go();
		QuitApplicationEventLoop();
	}

	return (noErr);
}

static pascal OSErr AEpdoc (const AppleEvent *theEvent, AppleEvent *theReply, long refCon)
{
	return (errAEEventNotHandled);
}

static pascal OSErr AEquit (const AppleEvent *theEvent, AppleEvent *theReply, long refCon)
{
	if (running)
		return (noErr);

	SNES9X_Quit();
	QuitApplicationEventLoop();

	return (noErr);
}

static pascal OSErr AEodoc (const AppleEvent *theEvent, AppleEvent *theReply, long refCon)
{
	OSErr 		err;
	FSRef		ref;
	AEDescList	docList;
	AEKeyword	keywd;
	DescType	rtype;
	Size		acsize;
	long		count;

	if (running)
		return (noErr);

	err = AEGetParamDesc(theEvent, keyDirectObject, typeAEList, &docList);
	if (err)
		return (noErr);

	err = AECountItems(&docList, &count);
	if (err || (count != 1))
	{
		err = AEDisposeDesc(&docList);
		return (noErr);
	}

	err = AEGetNthPtr(&docList, 1, typeFSRef, &keywd, &rtype, &ref, sizeof(FSRef), &acsize);
	if (err == noErr)
	{
		if (SNES9X_OpenCart(&ref))
		{
			SNES9X_Go();
			QuitApplicationEventLoop();
		}
		else
			AdjustMenus();
	}

	err = AEDisposeDesc(&docList);

	return (noErr);
}

void InitAppleEvents (void)
{
	OSErr	err;

	oappUPP = NewAEEventHandlerUPP(AEoapp);
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,   oappUPP, 0L, false);

	odocUPP = NewAEEventHandlerUPP(AEodoc);
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,     odocUPP, 0L, false);

	pdocUPP = NewAEEventHandlerUPP(AEpdoc);
	err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,    pdocUPP, 0L, false);

	quitUPP = NewAEEventHandlerUPP(AEquit);
	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,   quitUPP, 0L, false);

	rappUPP = NewAEEventHandlerUPP(AErapp);
	err = AEInstallEventHandler(kCoreEventClass, kAEReopenApplication, rappUPP, 0L, false);
}

void DeinitAppleEvents (void)
{
	DisposeAEEventHandlerUPP(oappUPP);
	DisposeAEEventHandlerUPP(odocUPP);
	DisposeAEEventHandlerUPP(pdocUPP);
	DisposeAEEventHandlerUPP(quitUPP);
	DisposeAEEventHandlerUPP(rappUPP);
}
