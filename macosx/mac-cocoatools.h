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


#ifndef _mac_cocoatools_h_
#define _mac_cocoatools_h_

void CocoaPlayFreezeDefrostSound (void);
void CocoaAddStatTextToView (NSView *, NSString *, float, float, float, float, NSTextField **);
void CocoaAddEditTextToView (NSView *, NSString *, float, float, float, float, NSTextField **);
void CocoaAddMPushBtnToView (NSView *, NSString *, float, float, float, float, NSButton **);
void CocoaAddCheckBoxToView (NSView *, NSString *, float, float, float, float, NSButton **);
void CocoaAddPopUpBtnToView (NSView *, NSArray *, float, float, float, float, NSPopUpButton **);

#endif
