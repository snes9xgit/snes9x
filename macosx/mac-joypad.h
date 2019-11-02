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


#ifndef _mac_joypad_h_
#define _mac_joypad_h_

#include "mac-controls.h"

void SetUpHID (void);
void ReleaseHID (void);

void SetPlayerForJoypad(int8 playerNum, uint32 vendorID, uint32 productID, uint8 index, int8 *oldPlayerNum);
void SetButtonCodeForJoypadControl(uint32 vendorID, uint32 productID, uint8 index, uint32 cookie, int32 value, S9xButtonCode buttonCode, bool overwrite, S9xButtonCode *oldButtonCode);

#endif
