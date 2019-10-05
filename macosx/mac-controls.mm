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


#include "port.h"
#include "controls.h"

#include "mac-prefix.h"
#include "mac-joypad.h"
#include "mac-keyboard.h"
#include "mac-os.h"
#include "mac-controls.h"

#define	ASSIGN_BUTTONf(n, s)	S9xMapButton (n, cmd = S9xGetCommandT(s), false)
#define	ASSIGN_BUTTONt(n, s)	S9xMapButton (n, cmd = S9xGetCommandT(s), true)
#define	ASSIGN_POINTRf(n, s)	S9xMapPointer(n, cmd = S9xGetCommandT(s), false)
#define	ASSIGN_POINTRt(n, s)	S9xMapPointer(n, cmd = S9xGetCommandT(s), true)

void S9xSetupDefaultKeymap (void)
{
	s9xcommand_t	cmd;

	ASSIGN_BUTTONf(kMacCMapPad1PX,         "Joypad1 X");
	ASSIGN_BUTTONf(kMacCMapPad1PA,         "Joypad1 A");
	ASSIGN_BUTTONf(kMacCMapPad1PB,         "Joypad1 B");
	ASSIGN_BUTTONf(kMacCMapPad1PY,         "Joypad1 Y");
	ASSIGN_BUTTONf(kMacCMapPad1PL,         "Joypad1 L");
	ASSIGN_BUTTONf(kMacCMapPad1PR,         "Joypad1 R");
	ASSIGN_BUTTONf(kMacCMapPad1PSelect,    "Joypad1 Select");
	ASSIGN_BUTTONf(kMacCMapPad1PStart,     "Joypad1 Start");
	ASSIGN_BUTTONf(kMacCMapPad1PUp,        "Joypad1 Up");
	ASSIGN_BUTTONf(kMacCMapPad1PDown,      "Joypad1 Down");
	ASSIGN_BUTTONf(kMacCMapPad1PLeft,      "Joypad1 Left");
	ASSIGN_BUTTONf(kMacCMapPad1PRight,     "Joypad1 Right");

	ASSIGN_BUTTONf(kMacCMapPad2PX,         "Joypad2 X");
	ASSIGN_BUTTONf(kMacCMapPad2PA,         "Joypad2 A");
	ASSIGN_BUTTONf(kMacCMapPad2PB,         "Joypad2 B");
	ASSIGN_BUTTONf(kMacCMapPad2PY,         "Joypad2 Y");
	ASSIGN_BUTTONf(kMacCMapPad2PL,         "Joypad2 L");
	ASSIGN_BUTTONf(kMacCMapPad2PR,         "Joypad2 R");
	ASSIGN_BUTTONf(kMacCMapPad2PSelect,    "Joypad2 Select");
	ASSIGN_BUTTONf(kMacCMapPad2PStart,     "Joypad2 Start");
	ASSIGN_BUTTONf(kMacCMapPad2PUp,        "Joypad2 Up");
	ASSIGN_BUTTONf(kMacCMapPad2PDown,      "Joypad2 Down");
	ASSIGN_BUTTONf(kMacCMapPad2PLeft,      "Joypad2 Left");
	ASSIGN_BUTTONf(kMacCMapPad2PRight,     "Joypad2 Right");

	ASSIGN_BUTTONf(kMacCMapPad3PX,         "Joypad3 X");
	ASSIGN_BUTTONf(kMacCMapPad3PA,         "Joypad3 A");
	ASSIGN_BUTTONf(kMacCMapPad3PB,         "Joypad3 B");
	ASSIGN_BUTTONf(kMacCMapPad3PY,         "Joypad3 Y");
	ASSIGN_BUTTONf(kMacCMapPad3PL,         "Joypad3 L");
	ASSIGN_BUTTONf(kMacCMapPad3PR,         "Joypad3 R");
	ASSIGN_BUTTONf(kMacCMapPad3PSelect,    "Joypad3 Select");
	ASSIGN_BUTTONf(kMacCMapPad3PStart,     "Joypad3 Start");
	ASSIGN_BUTTONf(kMacCMapPad3PUp,        "Joypad3 Up");
	ASSIGN_BUTTONf(kMacCMapPad3PDown,      "Joypad3 Down");
	ASSIGN_BUTTONf(kMacCMapPad3PLeft,      "Joypad3 Left");
	ASSIGN_BUTTONf(kMacCMapPad3PRight,     "Joypad3 Right");

	ASSIGN_BUTTONf(kMacCMapPad4PX,         "Joypad4 X");
	ASSIGN_BUTTONf(kMacCMapPad4PA,         "Joypad4 A");
	ASSIGN_BUTTONf(kMacCMapPad4PB,         "Joypad4 B");
	ASSIGN_BUTTONf(kMacCMapPad4PY,         "Joypad4 Y");
	ASSIGN_BUTTONf(kMacCMapPad4PL,         "Joypad4 L");
	ASSIGN_BUTTONf(kMacCMapPad4PR,         "Joypad4 R");
	ASSIGN_BUTTONf(kMacCMapPad4PSelect,    "Joypad4 Select");
	ASSIGN_BUTTONf(kMacCMapPad4PStart,     "Joypad4 Start");
	ASSIGN_BUTTONf(kMacCMapPad4PUp,        "Joypad4 Up");
	ASSIGN_BUTTONf(kMacCMapPad4PDown,      "Joypad4 Down");
	ASSIGN_BUTTONf(kMacCMapPad4PLeft,      "Joypad4 Left");
	ASSIGN_BUTTONf(kMacCMapPad4PRight,     "Joypad4 Right");

	ASSIGN_BUTTONf(kMacCMapPad5PX,         "Joypad5 X");
	ASSIGN_BUTTONf(kMacCMapPad5PA,         "Joypad5 A");
	ASSIGN_BUTTONf(kMacCMapPad5PB,         "Joypad5 B");
	ASSIGN_BUTTONf(kMacCMapPad5PY,         "Joypad5 Y");
	ASSIGN_BUTTONf(kMacCMapPad5PL,         "Joypad5 L");
	ASSIGN_BUTTONf(kMacCMapPad5PR,         "Joypad5 R");
	ASSIGN_BUTTONf(kMacCMapPad5PSelect,    "Joypad5 Select");
	ASSIGN_BUTTONf(kMacCMapPad5PStart,     "Joypad5 Start");
	ASSIGN_BUTTONf(kMacCMapPad5PUp,        "Joypad5 Up");
	ASSIGN_BUTTONf(kMacCMapPad5PDown,      "Joypad5 Down");
	ASSIGN_BUTTONf(kMacCMapPad5PLeft,      "Joypad5 Left");
	ASSIGN_BUTTONf(kMacCMapPad5PRight,     "Joypad5 Right");

	ASSIGN_BUTTONf(kMacCMapPad6PX,         "Joypad6 X");
	ASSIGN_BUTTONf(kMacCMapPad6PA,         "Joypad6 A");
	ASSIGN_BUTTONf(kMacCMapPad6PB,         "Joypad6 B");
	ASSIGN_BUTTONf(kMacCMapPad6PY,         "Joypad6 Y");
	ASSIGN_BUTTONf(kMacCMapPad6PL,         "Joypad6 L");
	ASSIGN_BUTTONf(kMacCMapPad6PR,         "Joypad6 R");
	ASSIGN_BUTTONf(kMacCMapPad6PSelect,    "Joypad6 Select");
	ASSIGN_BUTTONf(kMacCMapPad6PStart,     "Joypad6 Start");
	ASSIGN_BUTTONf(kMacCMapPad6PUp,        "Joypad6 Up");
	ASSIGN_BUTTONf(kMacCMapPad6PDown,      "Joypad6 Down");
	ASSIGN_BUTTONf(kMacCMapPad6PLeft,      "Joypad6 Left");
	ASSIGN_BUTTONf(kMacCMapPad6PRight,     "Joypad6 Right");

	ASSIGN_BUTTONf(kMacCMapPad7PX,         "Joypad7 X");
	ASSIGN_BUTTONf(kMacCMapPad7PA,         "Joypad7 A");
	ASSIGN_BUTTONf(kMacCMapPad7PB,         "Joypad7 B");
	ASSIGN_BUTTONf(kMacCMapPad7PY,         "Joypad7 Y");
	ASSIGN_BUTTONf(kMacCMapPad7PL,         "Joypad7 L");
	ASSIGN_BUTTONf(kMacCMapPad7PR,         "Joypad7 R");
	ASSIGN_BUTTONf(kMacCMapPad7PSelect,    "Joypad7 Select");
	ASSIGN_BUTTONf(kMacCMapPad7PStart,     "Joypad7 Start");
	ASSIGN_BUTTONf(kMacCMapPad7PUp,        "Joypad7 Up");
	ASSIGN_BUTTONf(kMacCMapPad7PDown,      "Joypad7 Down");
	ASSIGN_BUTTONf(kMacCMapPad7PLeft,      "Joypad7 Left");
	ASSIGN_BUTTONf(kMacCMapPad7PRight,     "Joypad7 Right");

	ASSIGN_BUTTONf(kMacCMapPad8PX,         "Joypad8 X");
	ASSIGN_BUTTONf(kMacCMapPad8PA,         "Joypad8 A");
	ASSIGN_BUTTONf(kMacCMapPad8PB,         "Joypad8 B");
	ASSIGN_BUTTONf(kMacCMapPad8PY,         "Joypad8 Y");
	ASSIGN_BUTTONf(kMacCMapPad8PL,         "Joypad8 L");
	ASSIGN_BUTTONf(kMacCMapPad8PR,         "Joypad8 R");
	ASSIGN_BUTTONf(kMacCMapPad8PSelect,    "Joypad8 Select");
	ASSIGN_BUTTONf(kMacCMapPad8PStart,     "Joypad8 Start");
	ASSIGN_BUTTONf(kMacCMapPad8PUp,        "Joypad8 Up");
	ASSIGN_BUTTONf(kMacCMapPad8PDown,      "Joypad8 Down");
	ASSIGN_BUTTONf(kMacCMapPad8PLeft,      "Joypad8 Left");
	ASSIGN_BUTTONf(kMacCMapPad8PRight,     "Joypad8 Right");

	ASSIGN_BUTTONt(kMacCMapMouse1PL,       "Mouse1 L");
	ASSIGN_BUTTONt(kMacCMapMouse1PR,       "Mouse1 R");
	ASSIGN_BUTTONt(kMacCMapMouse2PL,       "Mouse2 L");
	ASSIGN_BUTTONt(kMacCMapMouse2PR,       "Mouse2 R");

	ASSIGN_BUTTONt(kMacCMapScopeOffscreen, "Superscope AimOffscreen");
	ASSIGN_BUTTONt(kMacCMapScopeFire,      "Superscope Fire");
	ASSIGN_BUTTONt(kMacCMapScopeCursor,    "Superscope Cursor");
	ASSIGN_BUTTONt(kMacCMapScopeTurbo,     "Superscope ToggleTurbo");
	ASSIGN_BUTTONt(kMacCMapScopePause,     "Superscope Pause");

	ASSIGN_BUTTONt(kMacCMapLGun1Offscreen, "Justifier1 AimOffscreen");
	ASSIGN_BUTTONt(kMacCMapLGun1Trigger,   "Justifier1 Trigger");
	ASSIGN_BUTTONt(kMacCMapLGun1Start,     "Justifier1 Start");
	ASSIGN_BUTTONt(kMacCMapLGun2Offscreen, "Justifier2 AimOffscreen");
	ASSIGN_BUTTONt(kMacCMapLGun2Trigger,   "Justifier2 Trigger");
	ASSIGN_BUTTONt(kMacCMapLGun2Start,     "Justifier2 Start");

	ASSIGN_POINTRt(kMacCMapMouse1Pointer,     "Pointer Mouse1");
	ASSIGN_POINTRt(kMacCMapMouse2Pointer,     "Pointer Mouse2");
	ASSIGN_POINTRt(kMacCMapSuperscopePointer, "Pointer Superscope");
	ASSIGN_POINTRt(kMacCMapJustifier1Pointer, "Pointer Justifier1");

	ASSIGN_POINTRf(PseudoPointerBase,         "Pointer Justifier2");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 0, "ButtonToPointer 1u Med");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 1, "ButtonToPointer 1d Med");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 2, "ButtonToPointer 1l Med");
	ASSIGN_BUTTONf(kMacCMapPseudoPtrBase + 3, "ButtonToPointer 1r Med");
}

bool S9xPollButton (uint32 id, bool *pressed)
{
    #define kmControlKey    0x3B

    bool8 keys[MAC_NUM_KEYCODES];
    bool8 gamepadButtons[kNumButtons];

    CopyPressedKeys(keys, gamepadButtons);

    *pressed = false;

    if (id & k_MO)    // mouse
    {
        switch (id & 0xFF)
        {
            case 0: *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpMouseLeft);   break;
            case 1: *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpMouseRight);
        }
    }
    else
    if (id & k_SS)    // superscope
    {
        switch (id & 0xFF)
        {
            case 0:    *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpOffScreen);   break;
            case 2:    *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpScopeCursor); break;
            case 3:    *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpScopeTurbo);  break;
            case 4:    *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpScopePause);  break;
            case 1:    *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpMouseLeft);
        }
    }
    else
    if (id & k_LG)    // justifier
    {
        if (id & k_C1)
        {
            switch (id & 0xFF)
            {
                case 0: *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpOffScreen);    break;
                case 1: *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpMouseLeft);       break;                       break;
                case 2: *pressed = ISpKeyIsPressed(keys, gamepadButtons, kISpMouseRight);
            }
        }
        else
        {
            switch (id & 0xFF)
            {
                case 0: *pressed = KeyIsPressed(keys, gamepadButtons, k2PStart);    break;
                case 1: *pressed = KeyIsPressed(keys, gamepadButtons, k2PB);        break;
                case 2: *pressed = KeyIsPressed(keys, gamepadButtons, k2PA);
            }
        }
    }

	return (true);
}

bool S9xPollPointer (uint32 id, int16 *x, int16 *y)
{
	if (id & k_PT)
	{
		if ((id & k_MO) && fullscreen)
			GetGameScreenPointer(x, y, true);
		else
			GetGameScreenPointer(x, y, false);
	}
	else
		*x = *y = 0;

	return (true);
}

bool S9xPollAxis (uint32 id, int16 *value)
{
	return (false);
}

s9xcommand_t S9xGetPortCommandT (const char *name)
{
	s9xcommand_t	cmd;

    memset(&cmd, 0, sizeof(cmd));
    cmd.type = S9xBadMapping;

	return (cmd);
}

char * S9xGetPortCommandName (s9xcommand_t command)
{
	static char	str[] = "PortCommand";

	return (str);
}

void S9xHandlePortCommand (s9xcommand_t cmd, int16 data1, int16 data2)
{
	return;
}

bool8 S9xMapInput (const char *name, s9xcommand_t *cmd)
{
	return (true);
}

void ControlPadFlagsToS9xReportButtons (int n, uint32 p)
{
	uint32	base = k_HD | k_BT | k_JP | (0x100 << n);

	S9xReportButton(base +  0, (p & 0x0040));
	S9xReportButton(base +  1, (p & 0x0080));
	S9xReportButton(base +  2, (p & 0x8000));
	S9xReportButton(base +  3, (p & 0x4000));
	S9xReportButton(base +  4, (p & 0x0020));
	S9xReportButton(base +  5, (p & 0x0010));
	S9xReportButton(base +  6, (p & 0x2000));
	S9xReportButton(base +  7, (p & 0x1000));
	S9xReportButton(base +  8, (p & 0x0800));
	S9xReportButton(base +  9, (p & 0x0400));
	S9xReportButton(base + 10, (p & 0x0200));
	S9xReportButton(base + 11, (p & 0x0100));
}

void ControlPadFlagsToS9xPseudoPointer (uint32 p)
{
	// prevent screwiness caused by trying to move the pointer left+right or up+down
	if ((p & 0x0c00) == 0x0c00) p &= ~0x0c00;
	if ((p & 0x0300) == 0x0300) p &= ~0x0300;

	// checks added to prevent a lack of right/down movement from breaking left/up movement
	if (!(p & 0x0400))
		S9xReportButton(kMacCMapPseudoPtrBase + 0, (p & 0x0800));
	if (!(p & 0x0800))
		S9xReportButton(kMacCMapPseudoPtrBase + 1, (p & 0x0400));
	if (!(p & 0x0100))
		S9xReportButton(kMacCMapPseudoPtrBase + 2, (p & 0x0200));
	if (!(p & 0x0200))
		S9xReportButton(kMacCMapPseudoPtrBase + 3, (p & 0x0100));
}

long ISpKeyIsPressed (bool8 keys[kNumButtons], bool8 gamepadButtons[kNumButtons], ISpKey key)
{
    switch (key)
    {
        case kISpFastForward:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyFastForward);

        case kISpFreeze:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyFreeze);

        case kISpDefrost:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyDefrost);

        case kISpScreenshot:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyScreenshot);

        case kISpSPC:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeySPC);

        case kISpScopeTurbo:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyScopeTurbo);

        case kISpScopePause:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyScopePause);

        case kISpScopeCursor:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyScopeCursor);

        case kISpOffScreen:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyOffScreen);

        case kISpFunction:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyFunction);

        case kISpAlt:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyAlt);

        case kISpFFDown:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyFFDown);

        case kISpFFUp:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyFFUp);

        case kISpEsc:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyEsc);

        case kISpTC:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, k8PKeyTC);

        case kISpMouseLeft:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyMouseLeft);

        case kISpMouseRight:
            return
            KeyIsPressed(keys, gamepadButtons, k1PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k2PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k3PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k4PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k5PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k6PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, k7PKeyMouseRight);

        default:
            break;
    }
}
