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

    bool8 keys[MAC_MAX_PLAYERS][kNumButtons];
    bool8 gamepadButtons[MAC_MAX_PLAYERS][kNumButtons];

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
                case 0: *pressed = KeyIsPressed(keys, gamepadButtons, 1, kStart);    break;
                case 1: *pressed = KeyIsPressed(keys, gamepadButtons, 1, kB);        break;
                case 2: *pressed = KeyIsPressed(keys, gamepadButtons, 1, kA);
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

long ISpKeyIsPressed (bool8 keys[MAC_MAX_PLAYERS][kNumButtons], bool8 gamepadButtons[MAC_MAX_PLAYERS][kNumButtons], ISpKey key)
{
    switch (key)
    {
        case kISpFastForward:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyFastForward) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyFastForward);

        case kISpFreeze:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyFreeze) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyFreeze);

        case kISpDefrost:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyDefrost) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyDefrost);

        case kISpScreenshot:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyScreenshot) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyScreenshot);

        case kISpSPC:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeySPC) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeySPC);

        case kISpScopeTurbo:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyScopeTurbo) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyScopeTurbo);

        case kISpScopePause:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyScopePause) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyScopePause);

        case kISpScopeCursor:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyScopeCursor) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyScopeCursor);

        case kISpOffScreen:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyOffScreen) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyOffScreen);

        case kISpFunction:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyFunction) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyFunction);

        case kISpAlt:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyAlt) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyAlt);

        case kISpFFDown:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyFFDown) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyFFDown);

        case kISpFFUp:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyFFUp) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyFFUp);

        case kISpEsc:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyEsc) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyEsc);

        case kISpTC:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyTC) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyTC);

        case kISpMouseLeft:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyMouseLeft) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyMouseLeft);

        case kISpMouseRight:
            return
            KeyIsPressed(keys, gamepadButtons, 0, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 1, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 2, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 3, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 4, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 5, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 6, kKeyMouseRight) ||
            KeyIsPressed(keys, gamepadButtons, 7, kKeyMouseRight);

        default:
            break;
    }
}
