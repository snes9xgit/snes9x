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


#ifndef _mac_controls_h_
#define _mac_controls_h_

enum
{
	k_HD = 0x80000000,

	k_JP = 0x01000000,
	k_MO = 0x02000000,
	k_SS = 0x04000000,
	k_LG = 0x08000000,

	k_BT = 0x00100000,
	k_PT = 0x00200000,
	k_PS = 0x00400000,

	k_C1 = 0x00000100,
	k_C2 = 0x00000200,
	k_C3 = 0x00000400,
	k_C4 = 0x00000800,
	k_C5 = 0x00001000,
	k_C6 = 0x00002000,
	k_C7 = 0x00004000,
	k_C8 = 0x00008000
};

enum
{
	kMacCMapPad1PX            = k_HD | k_BT | k_JP | k_C1,
	kMacCMapPad1PA,
	kMacCMapPad1PB,
	kMacCMapPad1PY,
	kMacCMapPad1PL,
	kMacCMapPad1PR,
	kMacCMapPad1PSelect,
	kMacCMapPad1PStart,
	kMacCMapPad1PUp,
	kMacCMapPad1PDown,
	kMacCMapPad1PLeft,
	kMacCMapPad1PRight,

	kMacCMapPad2PX            = k_HD | k_BT | k_JP | k_C2,
	kMacCMapPad2PA,
	kMacCMapPad2PB,
	kMacCMapPad2PY,
	kMacCMapPad2PL,
	kMacCMapPad2PR,
	kMacCMapPad2PSelect,
	kMacCMapPad2PStart,
	kMacCMapPad2PUp,
	kMacCMapPad2PDown,
	kMacCMapPad2PLeft,
	kMacCMapPad2PRight,

	kMacCMapPad3PX            = k_HD | k_BT | k_JP | k_C3,
	kMacCMapPad3PA,
	kMacCMapPad3PB,
	kMacCMapPad3PY,
	kMacCMapPad3PL,
	kMacCMapPad3PR,
	kMacCMapPad3PSelect,
	kMacCMapPad3PStart,
	kMacCMapPad3PUp,
	kMacCMapPad3PDown,
	kMacCMapPad3PLeft,
	kMacCMapPad3PRight,

	kMacCMapPad4PX            = k_HD | k_BT | k_JP | k_C4,
	kMacCMapPad4PA,
	kMacCMapPad4PB,
	kMacCMapPad4PY,
	kMacCMapPad4PL,
	kMacCMapPad4PR,
	kMacCMapPad4PSelect,
	kMacCMapPad4PStart,
	kMacCMapPad4PUp,
	kMacCMapPad4PDown,
	kMacCMapPad4PLeft,
	kMacCMapPad4PRight,

	kMacCMapPad5PX            = k_HD | k_BT | k_JP | k_C5,
	kMacCMapPad5PA,
	kMacCMapPad5PB,
	kMacCMapPad5PY,
	kMacCMapPad5PL,
	kMacCMapPad5PR,
	kMacCMapPad5PSelect,
	kMacCMapPad5PStart,
	kMacCMapPad5PUp,
	kMacCMapPad5PDown,
	kMacCMapPad5PLeft,
	kMacCMapPad5PRight,

	kMacCMapPad6PX            = k_HD | k_BT | k_JP | k_C6,
	kMacCMapPad6PA,
	kMacCMapPad6PB,
	kMacCMapPad6PY,
	kMacCMapPad6PL,
	kMacCMapPad6PR,
	kMacCMapPad6PSelect,
	kMacCMapPad6PStart,
	kMacCMapPad6PUp,
	kMacCMapPad6PDown,
	kMacCMapPad6PLeft,
	kMacCMapPad6PRight,

	kMacCMapPad7PX            = k_HD | k_BT | k_JP | k_C7,
	kMacCMapPad7PA,
	kMacCMapPad7PB,
	kMacCMapPad7PY,
	kMacCMapPad7PL,
	kMacCMapPad7PR,
	kMacCMapPad7PSelect,
	kMacCMapPad7PStart,
	kMacCMapPad7PUp,
	kMacCMapPad7PDown,
	kMacCMapPad7PLeft,
	kMacCMapPad7PRight,

	kMacCMapPad8PX            = k_HD | k_BT | k_JP | k_C8,
	kMacCMapPad8PA,
	kMacCMapPad8PB,
	kMacCMapPad8PY,
	kMacCMapPad8PL,
	kMacCMapPad8PR,
	kMacCMapPad8PSelect,
	kMacCMapPad8PStart,
	kMacCMapPad8PUp,
	kMacCMapPad8PDown,
	kMacCMapPad8PLeft,
	kMacCMapPad8PRight,

	kMacCMapMouse1PL          = k_HD | k_BT | k_MO | k_C1,
	kMacCMapMouse1PR,
	kMacCMapMouse2PL          = k_HD | k_BT | k_MO | k_C2,
	kMacCMapMouse2PR,

	kMacCMapScopeOffscreen    = k_HD | k_BT | k_SS | k_C1,
	kMacCMapScopeFire,
	kMacCMapScopeCursor,
	kMacCMapScopeTurbo,
	kMacCMapScopePause,

	kMacCMapLGun1Offscreen    = k_HD | k_BT | k_LG | k_C1,
	kMacCMapLGun1Trigger,
	kMacCMapLGun1Start,
	kMacCMapLGun2Offscreen    = k_HD | k_BT | k_LG | k_C2,
	kMacCMapLGun2Trigger,
	kMacCMapLGun2Start,

	kMacCMapMouse1Pointer     = k_HD | k_PT | k_MO | k_C1,
	kMacCMapMouse2Pointer     = k_HD | k_PT | k_MO | k_C2,
	kMacCMapSuperscopePointer = k_HD | k_PT | k_SS | k_C1,
	kMacCMapJustifier1Pointer = k_HD | k_PT | k_LG | k_C1,

	kMacCMapPseudoPtrBase     = k_HD | k_PS | k_LG | k_C2	// for Justifier 2P
};

void ControlPadFlagsToS9xReportButtons (int, uint32);
void ControlPadFlagsToS9xPseudoPointer (uint32);

#endif
