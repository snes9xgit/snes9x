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


#ifndef _mac_keyboard_h_
#define _mac_keyboard_h_

#define	kKeys	39

extern uint8   keyCode[kKeys];

enum
{
	k1PUp,
	k1PDown,
	k1PLeft,
	k1PRight,
	k1PY,
	k1PB,
	k1PX,
	k1PA,
	k1PL,
	k1PR,
	k1PStart,
	k1PSelect,

	k2PUp,
	k2PDown,
	k2PLeft,
	k2PRight,
	k2PY,
	k2PB,
	k2PX,
	k2PA,
	k2PL,
	k2PR,
	k2PStart,
	k2PSelect,

	kKeyFastForward,
	kKeyFreeze,
	kKeyDefrost,
	kKeyScreenshot,
	kKeySPC,
	kKeyScopeTurbo,
	kKeyScopePause,
	kKeyScopeCursor,
	kKeyOffScreen,
	kKeyFunction,
	kKeyAlt,
	kKeyFFDown,
	kKeyFFUp,
	kKeyEsc,
	kKeyTC
};

void InitKeyboard (void);
void DeinitKeyboard (void);
void ConfigureKeyboard (void);

#endif
