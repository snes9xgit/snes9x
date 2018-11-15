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
#include "mac-cart.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-multicart.h"

#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
#define truncEnd 0
#endif

static pascal OSStatus MultiCartEventHandler (EventHandlerCallRef, EventRef, void *);
static pascal OSStatus MultiCartPaneEventHandler (EventHandlerCallRef, EventRef, void *);

static int		multiCartDragHilite;
static Boolean	multiCartDialogResult;

void InitMultiCart (void)
{
	CFStringRef	keyRef, pathRef;
	char		key[32];

	multiCartPath[0] = multiCartPath[1] = NULL;

	for (int i = 0; i < 2; i++)
	{
		sprintf(key, "MultiCartPath_%02d", i);
		keyRef = CFStringCreateWithCString(kCFAllocatorDefault, key, CFStringGetSystemEncoding());
		if (keyRef)
		{
			pathRef = (CFStringRef) CFPreferencesCopyAppValue(keyRef, kCFPreferencesCurrentApplication);
			if (pathRef)
				multiCartPath[i] = pathRef;

			CFRelease(keyRef);
		}
	}
}

void DeinitMultiCart (void)
{
	CFStringRef	keyRef;
	char		key[32];

	for (int i = 0; i < 2; i++)
	{
		sprintf(key, "MultiCartPath_%02d", i);
		keyRef = CFStringCreateWithCString(kCFAllocatorDefault, key, CFStringGetSystemEncoding());
		if (keyRef)
		{
			if (multiCartPath[i])
			{
				CFPreferencesSetAppValue(keyRef, multiCartPath[i], kCFPreferencesCurrentApplication);
				CFRelease(multiCartPath[i]);
			}
			else
				CFPreferencesSetAppValue(keyRef, NULL, kCFPreferencesCurrentApplication);

			CFRelease(keyRef);
		}
	}

	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

Boolean MultiCartDialog (void)
{
	OSStatus	err;
	IBNibRef	nibRef;

	multiCartDragHilite = -1;
	multiCartDialogResult = false;

	err = CreateNibReference(kMacS9XCFString, &nibRef);
	if (err == noErr)
	{
		WindowRef	window;

		err = CreateWindowFromNib(nibRef, CFSTR("MultiCart"), &window);
		if (err == noErr)
		{
			static int	index[2] = { 0, 1 };

			EventHandlerRef	wRef, cRef[2];
			EventHandlerUPP	wUPP, cUPP[2];
			EventTypeSpec	wEvent[] = { { kEventClassCommand, kEventCommandProcess      },
										 { kEventClassCommand, kEventCommandUpdateStatus } },
							cEvent[] = { { kEventClassControl, kEventControlDraw         },
										 { kEventClassControl, kEventControlDragEnter    },
										 { kEventClassControl, kEventControlDragWithin   },
										 { kEventClassControl, kEventControlDragLeave    },
										 { kEventClassControl, kEventControlDragReceive  } };
			HIViewRef		ctl, root, pane[2];
			HIViewID		cid;

			root = HIViewGetRoot(window);

			wUPP = NewEventHandlerUPP(MultiCartEventHandler);
			err = InstallWindowEventHandler(window, wUPP, GetEventTypeCount(wEvent), wEvent, (void *) window, &wRef);
			err = SetAutomaticControlDragTrackingEnabledForWindow(window, true);

			for (int i = 0; i < 2; i++)
			{
				cid.id = i;

				cid.signature = 'MPan';
				HIViewFindByID(root, cid, &pane[i]);
				cUPP[i] = NewEventHandlerUPP(MultiCartPaneEventHandler);
				err = InstallControlEventHandler(pane[i], cUPP[i], GetEventTypeCount(cEvent), cEvent, (void *) &index[i], &cRef[i]);
				err = SetControlDragTrackingEnabled(pane[i], true);

				cid.signature = 'MNAM';
				HIViewFindByID(root, cid, &ctl);
				SetStaticTextTrunc(ctl, truncEnd, false);
				if (multiCartPath[i])
				{
					CFStringRef	str;
					CFURLRef	url;

					url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, multiCartPath[i], kCFURLPOSIXPathStyle, false);
					str = CFURLCopyLastPathComponent(url);
					SetStaticTextCFString(ctl, str, false);
					CFRelease(str);
					CFRelease(url);
				}
				else
					SetStaticTextCFString(ctl, CFSTR(""), false);
			}

			MoveWindowPosition(window, kWindowMultiCart, false);
			ShowWindow(window);
			err = RunAppModalLoopForWindow(window);
			HideWindow(window);
			SaveWindowPosition(window, kWindowMultiCart);

			for (int i = 0; i < 2; i++)
			{
				err = RemoveEventHandler(cRef[i]);
				DisposeEventHandlerUPP(cUPP[i]);
			}

			err = RemoveEventHandler(wRef);
			DisposeEventHandlerUPP(wUPP);

			CFRelease(window);
		}

		DisposeNibReference(nibRef);
	}

	return (multiCartDialogResult);
}

static pascal OSStatus MultiCartEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus	err, result = eventNotHandledErr;
	WindowRef	window = (WindowRef) inUserData;
	static int	index = -1;

	switch (GetEventClass(inEvent))
	{
		case kEventClassCommand:
		{
			switch (GetEventKind(inEvent))
			{
				HICommand	tHICommand;

				case kEventCommandUpdateStatus:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr && tHICommand.commandID == 'clos')
					{
						UpdateMenuCommandStatus(false);
						result = noErr;
					}

					break;
				}

				case kEventCommandProcess:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &tHICommand);
					if (err == noErr)
					{
						HIViewRef	ctl, root;
						HIViewID	cid;
						FSRef		ref;
						bool8		r;

						root = HIViewGetRoot(window);

						switch (tHICommand.commandID)
						{
							case 'Cho0':
							case 'Cho1':
							{
								index = (tHICommand.commandID & 0xFF) - '0';
								r = NavBeginOpenROMImageSheet(window, NULL);
								result = noErr;
								break;
							}

							case 'NvDn':
							{
								r = NavEndOpenROMImageSheet(&ref);
								if (r)
								{
									CFStringRef	str;
									CFURLRef	url;

									url = CFURLCreateFromFSRef(kCFAllocatorDefault, &ref);
									str = CFURLCopyLastPathComponent(url);
									cid.signature = 'MNAM';
									cid.id = index;
									HIViewFindByID(root, cid, &ctl);
									SetStaticTextCFString(ctl, str, true);
									CFRelease(str);
									str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
									if (multiCartPath[index])
										CFRelease(multiCartPath[index]);
									multiCartPath[index] = str;
									CFRelease(url);
								}

								index = -1;
								result = noErr;
								break;
							}

							case 'Cle0':
							case 'Cle1':
							{
								index = (tHICommand.commandID & 0xFF) - '0';
								cid.signature = 'MNAM';
								cid.id = index;
								HIViewFindByID(root, cid, &ctl);
								SetStaticTextCFString(ctl, CFSTR(""), true);
								if (multiCartPath[index])
								{
									CFRelease(multiCartPath[index]);
									multiCartPath[index] = NULL;
								}

								index = -1;
								result = noErr;
								break;
							}

							case 'SWAP':
							{
								CFStringRef	str;
								CFURLRef	url;

								str = multiCartPath[0];
								multiCartPath[0] = multiCartPath[1];
								multiCartPath[1] = str;

								cid.signature = 'MNAM';

								for (int i = 0; i < 2; i++)
								{
									cid.id = i;
									HIViewFindByID(root, cid, &ctl);

									if (multiCartPath[i])
									{
										url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, multiCartPath[i], kCFURLPOSIXPathStyle, false);
										str = CFURLCopyLastPathComponent(url);
										SetStaticTextCFString(ctl, str, true);
										CFRelease(str);
										CFRelease(url);
									}
									else
										SetStaticTextCFString(ctl, CFSTR(""), true);
								}

								result = noErr;
								break;
							}

							case 'ok  ':
							{
								QuitAppModalLoopForWindow(window);
								multiCartDialogResult = true;
								result = noErr;
								break;
							}

							case 'not!':
							{
								QuitAppModalLoopForWindow(window);
								multiCartDialogResult = false;
								result = noErr;
								break;
							}
						}
					}
				}
			}
		}
	}

	return (result);
}

static pascal OSStatus MultiCartPaneEventHandler (EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
	OSStatus			err, result = eventNotHandledErr;
	HIViewRef			view;
	DragRef				drag;
	PasteboardRef		pasteboard;
	PasteboardItemID	itemID;
	CFArrayRef			array;
	CFStringRef			flavorType;
	CFIndex				numFlavors;
	ItemCount			numItems;
	int					index = *((int *) inUserData);

	switch (GetEventClass(inEvent))
	{
		case kEventClassControl:
		{
			switch (GetEventKind(inEvent))
			{
				case kEventControlDraw:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						CGContextRef	ctx;

						err = GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef, NULL, sizeof(CGContextRef), NULL, &ctx);
						if (err == noErr)
						{
							HIThemeFrameDrawInfo	info;
							HIRect					bounds, frame;

							HIViewGetBounds(view, &bounds);

							CGContextSetRGBFillColor(ctx, 1.0f, 1.0f, 1.0f, 1.0f);
							CGContextFillRect(ctx, bounds);

							info.version   = 0;
							info.kind      = kHIThemeFrameTextFieldSquare;
							info.state     = kThemeStateInactive;
							info.isFocused = false;
							err = HIThemeDrawFrame(&bounds, &info, ctx, kHIThemeOrientationNormal);

							if (multiCartDragHilite == index && systemVersion >= 0x1040)
							{
								err = HIThemeSetStroke(kThemeBrushDragHilite, NULL, ctx, kHIThemeOrientationNormal);
								frame = CGRectInset(bounds, 1, 1);
								CGContextBeginPath(ctx);
								CGContextAddRect(ctx, frame);
								CGContextStrokePath(ctx);
							}
						}
					}

					result = noErr;
					break;
				}

				case kEventControlDragEnter:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						err = GetEventParameter(inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof(DragRef), NULL, &drag);
						if (err == noErr)
						{
							err = GetDragPasteboard(drag, &pasteboard);
							if (err == noErr)
							{
								err = PasteboardGetItemCount(pasteboard, &numItems);
								if (err == noErr && numItems == 1)
								{
									err = PasteboardGetItemIdentifier(pasteboard, 1, &itemID);
									if (err == noErr)
									{
										err = PasteboardCopyItemFlavors(pasteboard, itemID, &array);
										if (err == noErr)
										{
											numFlavors = CFArrayGetCount(array);
											for (CFIndex i = 0; i < numFlavors; i++)
											{
												flavorType = (CFStringRef) CFArrayGetValueAtIndex(array, i);
												if (UTTypeConformsTo(flavorType, CFSTR("public.file-url")))
												{
													Boolean	accept = true;

													err = SetEventParameter(inEvent, kEventParamControlWouldAcceptDrop, typeBoolean, sizeof(Boolean), &accept);
													if (err == noErr)
													{
														multiCartDragHilite = index;
														HIViewSetNeedsDisplay(view, true);
														result = noErr;
													}
												}
											}

											CFRelease(array);
										}
									}
								}
							}
						}
					}

					break;
				}

				case kEventControlDragWithin:
				{
					result = noErr;
					break;
				}

				case kEventControlDragLeave:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						multiCartDragHilite = -1;
						HIViewSetNeedsDisplay(view, true);
					}

					result = noErr;
					break;
				}

				case kEventControlDragReceive:
				{
					err = GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof(ControlRef), NULL, &view);
					if (err == noErr)
					{
						err = GetEventParameter(inEvent, kEventParamDragRef, typeDragRef, NULL, sizeof(DragRef), NULL, &drag);
						if (err == noErr)
						{
							multiCartDragHilite = -1;
							HIViewSetNeedsDisplay(view, true);

							err = GetDragPasteboard(drag, &pasteboard);
							if (err == noErr)
							{
								err = PasteboardGetItemIdentifier(pasteboard, 1, &itemID);
								if (err == noErr)
								{
									err = PasteboardCopyItemFlavors(pasteboard, itemID, &array);
									if (err == noErr)
									{
										numFlavors = CFArrayGetCount(array);
										for (CFIndex i = 0; i < numFlavors; i++)
										{
											flavorType = (CFStringRef) CFArrayGetValueAtIndex(array, i);
											if (UTTypeConformsTo(flavorType, CFSTR("public.file-url")))
											{
												CFDataRef	flavorData;

												err = PasteboardCopyItemFlavorData(pasteboard, itemID, flavorType, &flavorData);
												if (err == noErr)
												{
													CFIndex	dataSize;
													UInt8	*data;

													dataSize = CFDataGetLength(flavorData);
													data = (UInt8 *) malloc(dataSize);
													if (data)
													{
														CFDataGetBytes(flavorData, CFRangeMake(0, dataSize), data);

														HIViewRef	ctl;
														HIViewID	cid;
														CFStringRef	str;
														CFURLRef	url;

														GetControlID(view, &cid);
														cid.signature = 'MNAM';
														HIViewFindByID(view, cid, &ctl);

														url = CFURLCreateWithBytes(kCFAllocatorDefault, data, dataSize, kCFStringEncodingUTF8, NULL);
														str = CFURLCopyLastPathComponent(url);
														SetStaticTextCFString(ctl, str, true);
														CFRelease(str);
														str = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
														if (multiCartPath[cid.id])
															CFRelease(multiCartPath[cid.id]);
														multiCartPath[cid.id] = str;
														CFRelease(url);

														result = noErr;

														free(data);
													}

													CFRelease(flavorData);
												}
											}
										}

										CFRelease(array);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return (result);
}
