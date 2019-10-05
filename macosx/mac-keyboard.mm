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

#include <Cocoa/Cocoa.h>

#include "port.h"

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-gworld.h"
#include "mac-os.h"
#include "mac-keyboard.h"

int16 keyCodes[MAC_NUM_KEYCODES];

void InitKeyboard (void)
{
    for (int i = 0; i < MAC_NUM_KEYCODES; ++i)
    {
        keyCodes[i] = -1;
    }
}

void DeinitKeyboard (void)
{
    for (int i = 0; i < MAC_NUM_KEYCODES; ++i)
    {
        keyCodes[i] = -1;
    }
}

void SetKeyCode(int16 keyCode, S9xKey control, int16 *oldKeyCode, S9xKey *oldControl)
{
    for ( int i = 0; i < MAC_NUM_KEYCODES; ++i)
    {
        if (keyCodes[i] == control && i != keyCode)
        {
            keyCodes[i] = -1;

            if (oldKeyCode !=NULL)
            {
                *oldKeyCode = i;
            }
        }
    }

    if (oldControl != NULL)
    {
        *oldControl = (S9xKey)keyCodes[keyCode];
    }

    keyCodes[keyCode] = control;
}
