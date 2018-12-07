/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifdef __MINGW32__
#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0501
#endif

#define STRICT
#include <windows.h>
#include <io.h>

#if (((defined(_MSC_VER) && _MSC_VER >= 1300)) || defined(__MINGW32__))
	// both MINGW and VS.NET use fstream instead of fstream.h which is deprecated
	#include <fstream>
	using namespace std;
#else
	// for VC++ 6
	#include <fstream.h>
#endif

#include "InputCustom.h"
#include "wsnes9x.h"
#include "wlanguage.h"

static TCHAR szClassName[] = _T("InputCustom");
static TCHAR szHotkeysClassName[] = _T("InputCustomHot");

static LRESULT CALLBACK InputCustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK HotInputCustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


extern SJoyState JoystickF [16];

HWND funky;
//WPARAM tid;


void JoystickChanged( short ID, short Movement)
{
	// don't allow two changes to happen too close together in time
	{
		static bool first = true;
		static DWORD lastTime = 0;
		if(first || timeGetTime() - lastTime > 300) // 0.3 seconds
		{
			first = false;
			lastTime = timeGetTime();
		}
		else
		{
			return; // too soon after last change
		}
	}

	WORD JoyKey;

    JoyKey = 0x8000;
	JoyKey |= (WORD)(ID << 8);
	JoyKey |= Movement;
	SendMessage(funky,WM_USER+45,JoyKey,0);
//	KillTimer(funky,tid);
}

int FunkyNormalize(int cur, int min, int max)
{
	int Result = 0;

    if ((max - min) == 0)

        return (Result);

    Result = cur - min;
    Result = (Result * 200) / (max - min);
    Result -= 100;

    return Result;
}


void CheckAxis (short joy, short control, int val,
                                       int min, int max,
                                       bool &first, bool &second)
{



    if (FunkyNormalize (val, min, max) < -S9X_JOY_NEUTRAL)

    {
        second = false;
        if (!first)
        {
            JoystickChanged (joy, control);
            first = true;

        }
    }
    else
        first = false;

    if (FunkyNormalize (val, min, max) > S9X_JOY_NEUTRAL)
    {
        first = false;
        if (!second)
        {
            JoystickChanged (joy, (short) (control + 1));
            second = true;
        }
    }
    else
        second = false;
}

void FunkyJoyStickTimer()
{
	JOYINFOEX jie;

    for (short C = 0; C != 16; C ++)
    {

        jie.dwSize = sizeof( jie);
        jie.dwFlags = JOY_RETURNALL;

        if (joyGetPosEx (JOYSTICKID1 + C, &jie) != JOYERR_NOERROR)
            continue;

        CheckAxis (C, 0, jie.dwXpos,
                   JoystickF[C].Caps.wXmin, JoystickF[C].Caps.wXmax,
                   JoystickF[C].Left, JoystickF[C].Right);
        CheckAxis (C, 2, jie.dwYpos,
                   JoystickF[C].Caps.wYmin, JoystickF[C].Caps.wYmax,
                   JoystickF[C].Up, JoystickF[C].Down);
        if(JoystickF[C].Caps.wCaps & JOYCAPS_HASZ)
		{
		CheckAxis (C, 41, jie.dwZpos,
                   JoystickF[C].Caps.wZmin, JoystickF[C].Caps.wZmax,
                   JoystickF[C].ZUp, JoystickF[C].ZDown);
		}
		if(JoystickF[C].Caps.wCaps & JOYCAPS_HASR)
		{
        CheckAxis (C, 43, jie.dwRpos,
                   JoystickF[C].Caps.wRmin, JoystickF[C].Caps.wRmax,
                   JoystickF[C].RUp, JoystickF[C].RDown);
		}
		if(JoystickF[C].Caps.wCaps & JOYCAPS_HASU)
		{
        CheckAxis (C, 45, jie.dwUpos,
                   JoystickF[C].Caps.wUmin, JoystickF[C].Caps.wUmax,
                   JoystickF[C].UUp, JoystickF[C].UDown);
		}
        if(JoystickF[C].Caps.wCaps & JOYCAPS_HASV)
		{
		CheckAxis (C, 47, jie.dwVpos,
                   JoystickF[C].Caps.wVmin, JoystickF[C].Caps.wVmax,
                   JoystickF[C].VUp, JoystickF[C].VDown);
		}

        switch (jie.dwPOV)
        {
            case JOY_POVBACKWARD:
                if( !JoystickF[C].PovDown)
                {   JoystickChanged( C, 7); }

                JoystickF[C].PovDown = true;
                JoystickF[C].PovUp = false;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
                break;
			case 4500:
				if( !JoystickF[C].PovUpRight)
                {   JoystickChanged( C, 52); }
				JoystickF[C].PovDown = false;
                JoystickF[C].PovUp = false;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = true;
				break;
			case 13500:
				if( !JoystickF[C].PovDnRight)
                {   JoystickChanged( C, 50); }
				JoystickF[C].PovDown = false;
                JoystickF[C].PovUp = false;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = true;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
				break;
			case 22500:
				if( !JoystickF[C].PovDnLeft)
                {   JoystickChanged( C, 49); }
				JoystickF[C].PovDown = false;
                JoystickF[C].PovUp = false;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = true;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
				break;
			case 31500:
				if( !JoystickF[C].PovUpLeft)
                {   JoystickChanged( C, 51); }
				JoystickF[C].PovDown = false;
                JoystickF[C].PovUp = false;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = true;
				JoystickF[C].PovUpRight = false;
				break;

            case JOY_POVFORWARD:
                if( !JoystickF[C].PovUp)
                {   JoystickChanged( C, 6); }

                JoystickF[C].PovDown = false;
                JoystickF[C].PovUp = true;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
                break;

            case JOY_POVLEFT:
                if( !JoystickF[C].PovLeft)
                {   JoystickChanged( C, 4); }

				JoystickF[C].PovDown = false;
				JoystickF[C].PovUp = false;
				JoystickF[C].PovLeft = true;
				JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
                break;

            case JOY_POVRIGHT:
                if( !JoystickF[C].PovRight)
                {   JoystickChanged( C, 5); }

				JoystickF[C].PovDown = false;
				JoystickF[C].PovUp = false;
				JoystickF[C].PovLeft = false;
				JoystickF[C].PovRight = true;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
                break;

            default:
                JoystickF[C].PovDown = false;
                JoystickF[C].PovUp = false;
                JoystickF[C].PovLeft = false;
                JoystickF[C].PovRight = false;
				JoystickF[C].PovDnLeft = false;
				JoystickF[C].PovDnRight = false;
				JoystickF[C].PovUpLeft = false;
				JoystickF[C].PovUpRight = false;
                break;
        }

        for( short B = 0; B != 32; B ++, jie.dwButtons >>= 1)
        if( (jie.dwButtons&1))
        {
            if( !JoystickF[C].Button[B])
            {
                JoystickChanged( C, (short)(8+B));
                JoystickF[C].Button[B] = true;
            }
        }
        else
        {   JoystickF[C].Button[B] = false; }

    }

}

void TranslateKey(WORD keyz,char *out)
{
//	sprintf(out,"%d",keyz);
//	return;

	char temp[128];
	if(keyz&0x8000)
	{
		sprintf(out,GAMEDEVICE_JOYNUMPREFIX,((keyz>>8)&0xF));
		switch(keyz&0xFF)
		{
		case 0:  strcat(out,GAMEDEVICE_XNEG); break;
		case 1:  strcat(out,GAMEDEVICE_XPOS); break;
        case 2:  strcat(out,GAMEDEVICE_YPOS); break;
		case 3:  strcat(out,GAMEDEVICE_YNEG); break;
		case 4:  strcat(out,GAMEDEVICE_POVLEFT); break;
		case 5:  strcat(out,GAMEDEVICE_POVRIGHT); break;
		case 6:  strcat(out,GAMEDEVICE_POVUP); break;
		case 7:  strcat(out,GAMEDEVICE_POVDOWN); break;
		case 49: strcat(out,GAMEDEVICE_POVDNLEFT); break;
		case 50: strcat(out,GAMEDEVICE_POVDNRIGHT); break;
		case 51: strcat(out,GAMEDEVICE_POVUPLEFT); break;
		case 52: strcat(out,GAMEDEVICE_POVUPRIGHT); break;
		case 41: strcat(out,GAMEDEVICE_ZPOS); break;
		case 42: strcat(out,GAMEDEVICE_ZNEG); break;
		case 43: strcat(out,GAMEDEVICE_RPOS); break;
		case 44: strcat(out,GAMEDEVICE_RNEG); break;
		case 45: strcat(out,GAMEDEVICE_UPOS); break;
		case 46: strcat(out,GAMEDEVICE_UNEG); break;
		case 47: strcat(out,GAMEDEVICE_VPOS); break;
		case 48: strcat(out,GAMEDEVICE_VNEG); break;
		default:
			if ((keyz & 0xff) > 40)
            {
				sprintf(temp,GAMEDEVICE_JOYBUTPREFIX,keyz&0xFF);
				strcat(out,temp);
				break;
            }

			sprintf(temp,GAMEDEVICE_BUTTON,(keyz&0xFF)-8);
			strcat(out,temp);
			break;

		}
		return;
	}
	sprintf(out,GAMEDEVICE_KEY,keyz);
	if((keyz>='0' && keyz<='9')||(keyz>='A' &&keyz<='Z'))
	{
		sprintf(out,"%c",keyz);
		return;
	}

	if( keyz >= VK_NUMPAD0 && keyz <= VK_NUMPAD9)
    {

		sprintf(out,GAMEDEVICE_NUMPADPREFIX,'0'+(keyz-VK_NUMPAD0));

        return ;
    }
	switch(keyz)
    {
        case 0:				sprintf(out,GAMEDEVICE_DISABLED); break;
        case VK_TAB:		sprintf(out,GAMEDEVICE_VK_TAB); break;
        case VK_BACK:		sprintf(out,GAMEDEVICE_VK_BACK); break;
        case VK_CLEAR:		sprintf(out,GAMEDEVICE_VK_CLEAR); break;
        case VK_RETURN:		sprintf(out,GAMEDEVICE_VK_RETURN); break;
        case VK_LSHIFT:		sprintf(out,GAMEDEVICE_VK_LSHIFT); break;
		case VK_RSHIFT:		sprintf(out,GAMEDEVICE_VK_RSHIFT); break;
        case VK_LCONTROL:	sprintf(out,GAMEDEVICE_VK_LCONTROL); break;
		case VK_RCONTROL:	sprintf(out,GAMEDEVICE_VK_RCONTROL); break;
        case VK_LMENU:		sprintf(out,GAMEDEVICE_VK_LMENU); break;
		case VK_RMENU:		sprintf(out,GAMEDEVICE_VK_RMENU); break;
        case VK_PAUSE:		sprintf(out,GAMEDEVICE_VK_PAUSE); break;
        case VK_CANCEL:		sprintf(out,GAMEDEVICE_VK_PAUSE); break; // the Pause key can resolve to either "Pause" or "Cancel" depending on when it's pressed
        case VK_CAPITAL:	sprintf(out,GAMEDEVICE_VK_CAPITAL); break;
        case VK_ESCAPE:		sprintf(out,GAMEDEVICE_VK_ESCAPE); break;
        case VK_SPACE:		sprintf(out,GAMEDEVICE_VK_SPACE); break;
        case VK_PRIOR:		sprintf(out,GAMEDEVICE_VK_PRIOR); break;
        case VK_NEXT:		sprintf(out,GAMEDEVICE_VK_NEXT); break;
        case VK_HOME:		sprintf(out,GAMEDEVICE_VK_HOME); break;
        case VK_END:		sprintf(out,GAMEDEVICE_VK_END); break;
        case VK_LEFT:		sprintf(out,GAMEDEVICE_VK_LEFT ); break;
        case VK_RIGHT:		sprintf(out,GAMEDEVICE_VK_RIGHT); break;
        case VK_UP:			sprintf(out,GAMEDEVICE_VK_UP); break;
        case VK_DOWN:		sprintf(out,GAMEDEVICE_VK_DOWN); break;
        case VK_SELECT:		sprintf(out,GAMEDEVICE_VK_SELECT); break;
        case VK_PRINT:		sprintf(out,GAMEDEVICE_VK_PRINT); break;
        case VK_EXECUTE:	sprintf(out,GAMEDEVICE_VK_EXECUTE); break;
        case VK_SNAPSHOT:	sprintf(out,GAMEDEVICE_VK_SNAPSHOT); break;
        case VK_INSERT:		sprintf(out,GAMEDEVICE_VK_INSERT); break;
        case VK_DELETE:		sprintf(out,GAMEDEVICE_VK_DELETE); break;
        case VK_HELP:		sprintf(out,GAMEDEVICE_VK_HELP); break;
        case VK_LWIN:		sprintf(out,GAMEDEVICE_VK_LWIN); break;
        case VK_RWIN:		sprintf(out,GAMEDEVICE_VK_RWIN); break;
        case VK_APPS:		sprintf(out,GAMEDEVICE_VK_APPS); break;
        case VK_MULTIPLY:	sprintf(out,GAMEDEVICE_VK_MULTIPLY); break;
        case VK_ADD:		sprintf(out,GAMEDEVICE_VK_ADD); break;
        case VK_SEPARATOR:	sprintf(out,GAMEDEVICE_VK_SEPARATOR); break;
		case /*VK_OEM_1*/0xBA:		sprintf(out,GAMEDEVICE_VK_OEM_1); break;
        case /*VK_OEM_2*/0xBF:		sprintf(out,GAMEDEVICE_VK_OEM_2); break;
        case /*VK_OEM_3*/0xC0:		sprintf(out,GAMEDEVICE_VK_OEM_3); break;
        case /*VK_OEM_4*/0xDB:		sprintf(out,GAMEDEVICE_VK_OEM_4); break;
        case /*VK_OEM_5*/0xDC:		sprintf(out,GAMEDEVICE_VK_OEM_5); break;
        case /*VK_OEM_6*/0xDD:		sprintf(out,GAMEDEVICE_VK_OEM_6); break;
		case /*VK_OEM_7*/0xDE:		sprintf(out,GAMEDEVICE_VK_OEM_7); break;
		case /*VK_OEM_COMMA*/0xBC:	sprintf(out,GAMEDEVICE_VK_OEM_COMMA );break;
		case /*VK_OEM_PERIOD*/0xBE:	sprintf(out,GAMEDEVICE_VK_OEM_PERIOD);break;
        case VK_SUBTRACT:	sprintf(out,GAMEDEVICE_VK_SUBTRACT); break;
        case VK_DECIMAL:	sprintf(out,GAMEDEVICE_VK_DECIMAL); break;
        case VK_DIVIDE:		sprintf(out,GAMEDEVICE_VK_DIVIDE); break;
        case VK_NUMLOCK:	sprintf(out,GAMEDEVICE_VK_NUMLOCK); break;
        case VK_SCROLL:		sprintf(out,GAMEDEVICE_VK_SCROLL); break;
        case /*VK_OEM_MINUS*/0xBD:	sprintf(out,GAMEDEVICE_VK_OEM_MINUS); break;
        case /*VK_OEM_PLUS*/0xBB:	sprintf(out,GAMEDEVICE_VK_OEM_PLUS); break;
        case VK_SHIFT:		sprintf(out,GAMEDEVICE_VK_SHIFT); break;
        case VK_CONTROL:	sprintf(out,GAMEDEVICE_VK_CONTROL); break;
        case VK_MENU:		sprintf(out,GAMEDEVICE_VK_MENU); break;
        case VK_F1:			sprintf(out,GAMEDEVICE_VK_F1); break;
        case VK_F2:			sprintf(out,GAMEDEVICE_VK_F2); break;
        case VK_F3:			sprintf(out,GAMEDEVICE_VK_F3); break;
        case VK_F4:			sprintf(out,GAMEDEVICE_VK_F4); break;
        case VK_F5:			sprintf(out,GAMEDEVICE_VK_F5); break;
        case VK_F6:			sprintf(out,GAMEDEVICE_VK_F6); break;
        case VK_F7:			sprintf(out,GAMEDEVICE_VK_F7); break;
        case VK_F8:			sprintf(out,GAMEDEVICE_VK_F8); break;
        case VK_F9:			sprintf(out,GAMEDEVICE_VK_F9); break;
        case VK_F10:		sprintf(out,GAMEDEVICE_VK_F10); break;
        case VK_F11:		sprintf(out,GAMEDEVICE_VK_F11); break;
        case VK_F12:		sprintf(out,GAMEDEVICE_VK_F12); break;
    }

    return ;



}

bool IsReserved (WORD Key, int modifiers)
{
	// keys that do other stuff in Windows
	if(Key == VK_CAPITAL || Key == VK_NUMLOCK || Key == VK_SCROLL || Key == VK_SNAPSHOT
	|| Key == VK_LWIN    || Key == VK_RWIN    || Key == VK_APPS || Key == /*VK_SLEEP*/0x5F
	|| (Key == VK_F4 && (modifiers & CUSTKEY_ALT_MASK) != 0)) // alt-f4 (behaves unlike accelerators)
		return true;

	// menu shortcuts (accelerators) -- TODO: should somehow parse GUI.Accelerators for this information
	if(modifiers == CUSTKEY_CTRL_MASK
	 && (Key == 'O')
	|| modifiers == CUSTKEY_ALT_MASK
	 && (Key == VK_F5 || Key == VK_F7 || Key == VK_F8 || Key == VK_F9
	  || Key == 'R' || Key == 'T' || Key == /*VK_OEM_4*/0xDB || Key == /*VK_OEM_6*/0xDD
	  || Key == 'E' || Key == 'A' || Key == VK_RETURN || Key == VK_DELETE))
		return true;

	return false;
}

int GetNumHotKeysAssignedTo (WORD Key, int modifiers)
{
	int count = 0;
	{
		#define MATCHES_KEY(k) \
			(modifiers == CustomKeys.k.modifiers \
		  && Key != 0 && Key != VK_ESCAPE \
		  && (Key == CustomKeys.k.key \
		   || (Key == VK_SHIFT   && (CustomKeys.k.modifiers & CUSTKEY_SHIFT_MASK) != 0) \
		   || (Key == VK_MENU    && (CustomKeys.k.modifiers & CUSTKEY_ALT_MASK)   != 0) \
		   || (Key == VK_CONTROL && (CustomKeys.k.modifiers & CUSTKEY_CTRL_MASK)  != 0) \
		   || (CustomKeys.k.key == VK_SHIFT) \
		   || (CustomKeys.k.key == VK_MENU) \
		   || (CustomKeys.k.key == VK_CONTROL)))

		if(MATCHES_KEY(SpeedUp))           count++;
		if(MATCHES_KEY(SpeedDown))         count++;
		if(MATCHES_KEY(Pause))             count++;
		if(MATCHES_KEY(FrameAdvance))      count++;
		if(MATCHES_KEY(SkipUp))            count++;
		if(MATCHES_KEY(SkipDown))          count++;
		if(MATCHES_KEY(ScopeTurbo))        count++;
		if(MATCHES_KEY(ScopePause))        count++;
		if(MATCHES_KEY(FrameCount))        count++;
		if(MATCHES_KEY(ReadOnly))          count++;
		for(int i = 0 ; i < 10 ; i++) {
			if(MATCHES_KEY(Save[i]))       count++;
			if(MATCHES_KEY(Load[i]))       count++;
			if(MATCHES_KEY(SelectSave[i])) count++;
		}
		if(MATCHES_KEY(FastForward))       count++;
		if(MATCHES_KEY(ShowPressed))       count++;
		if(MATCHES_KEY(SaveScreenShot))    count++;
		if(MATCHES_KEY(SlotPlus))          count++;
		if(MATCHES_KEY(SlotMinus))         count++;
		if(MATCHES_KEY(SlotSave))          count++;
		if(MATCHES_KEY(SlotLoad))          count++;
		if(MATCHES_KEY(BGL1))              count++;
		if(MATCHES_KEY(BGL2))              count++;
		if(MATCHES_KEY(BGL3))              count++;
		if(MATCHES_KEY(BGL4))              count++;
		if(MATCHES_KEY(BGL5))              count++;
		if(MATCHES_KEY(ClippingWindows))   count++;
//		if(MATCHES_KEY(BGLHack))           count++;
		if(MATCHES_KEY(Transparency))      count++;
//		if(MATCHES_KEY(GLCube))            count++;
//		if(MATCHES_KEY(InterpMode7))       count++;
		if(MATCHES_KEY(JoypadSwap))        count++;
		if(MATCHES_KEY(SwitchControllers)) count++;
		if(MATCHES_KEY(TurboA))            count++;
		if(MATCHES_KEY(TurboB))            count++;
		if(MATCHES_KEY(TurboY))            count++;
		if(MATCHES_KEY(TurboX))            count++;
		if(MATCHES_KEY(TurboL))            count++;
		if(MATCHES_KEY(TurboR))            count++;
		if(MATCHES_KEY(TurboStart))        count++;
		if(MATCHES_KEY(TurboSelect))       count++;
		if(MATCHES_KEY(TurboLeft))         count++;
		if(MATCHES_KEY(TurboUp))           count++;
		if(MATCHES_KEY(TurboRight))        count++;
		if(MATCHES_KEY(TurboDown))         count++;
		if(MATCHES_KEY(ResetGame))         count++;
		if(MATCHES_KEY(ToggleCheats))      count++;
        if(MATCHES_KEY(QuitS9X))           count++;
        if(MATCHES_KEY(Rewind))            count++;
        if(MATCHES_KEY(SaveFileSelect))    count++;
        if(MATCHES_KEY(LoadFileSelect))    count++;
        if(MATCHES_KEY(Mute))              count++;

		#undef MATCHES_KEY
	}
	return count;
}

int GetNumButtonsAssignedTo (WORD Key)
{
	int count = 0;
	for(int J = 0; J < 5*2; J++)
	{
		// don't want to report conflicts with disabled keys
		if(!Joypad[J%5].Enabled || Key == 0 || Key == VK_ESCAPE)
			continue;

		if(Key == Joypad[J].Left)       count++;
		if(Key == Joypad[J].Right)      count++;
		if(Key == Joypad[J].Left_Up)    count++;
		if(Key == Joypad[J].Left_Down)  count++;
		if(Key == Joypad[J].Right_Up)   count++;
		if(Key == Joypad[J].Right_Down) count++;
		if(Key == Joypad[J].Up)         count++;
		if(Key == Joypad[J].Down)       count++;
		if(Key == Joypad[J].Start)      count++;
		if(Key == Joypad[J].Select)     count++;
		if(Key == Joypad[J].A)          count++;
		if(Key == Joypad[J].B)          count++;
		if(Key == Joypad[J].X)          count++;
		if(Key == Joypad[J].Y)          count++;
		if(Key == Joypad[J].L)          count++;
		if(Key == Joypad[J].R)          count++;
    }
	return count;
}

COLORREF CheckButtonKey( WORD Key)
{
	COLORREF red,magenta,blue,white;
	red =RGB(255,0,0);
	magenta =RGB(255,0,255);
	blue = RGB(0,0,255);
	white = RGB(255,255,255);

	// Check for conflict with reserved windows keys
    if(IsReserved(Key,0))
		return red;

    // Check for conflict with Snes9X hotkeys
	if(GetNumHotKeysAssignedTo(Key,0) > 0)
        return magenta;

    // Check for duplicate button keys
    if(GetNumButtonsAssignedTo(Key) > 1)
        return blue;

    return white;
}

COLORREF CheckHotKey( WORD Key, int modifiers)
{
	COLORREF red,magenta,blue,white;
	red =RGB(255,0,0);
	magenta =RGB(255,0,255);
	blue = RGB(0,0,255);
	white = RGB(255,255,255);

	// Check for conflict with reserved windows keys
    if(IsReserved(Key,modifiers))
		return red;

    // Check for conflict with button keys
    if(modifiers == 0 && GetNumButtonsAssignedTo(Key) > 0)
        return magenta;

	// Check for duplicate Snes9X hotkeys
	if(GetNumHotKeysAssignedTo(Key,modifiers) > 1)
        return blue;

    return white;
}

void InitInputCustomControl()
{

    WNDCLASSEX wc;

    wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szClassName;
    wc.hInstance      = GetModuleHandle(0);
    wc.lpfnWndProc    = InputCustomWndProc;
    wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
    wc.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
    wc.style          = 0;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = sizeof(InputCust *);
    wc.hIconSm        = 0;


    RegisterClassEx(&wc);

}
void InitKeyCustomControl()
{

    WNDCLASSEX wc;

    wc.cbSize         = sizeof(wc);
    wc.lpszClassName  = szHotkeysClassName;
    wc.hInstance      = GetModuleHandle(0);
    wc.lpfnWndProc    = HotInputCustomWndProc;
    wc.hCursor        = LoadCursor (NULL, IDC_ARROW);
    wc.hIcon          = 0;
    wc.lpszMenuName   = 0;
    wc.hbrBackground  = (HBRUSH)GetSysColorBrush(COLOR_BTNFACE);
    wc.style          = 0;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = sizeof(InputCust *);
    wc.hIconSm        = 0;


    RegisterClassEx(&wc);

}
HWND CreateInputCustom(HWND hwndParent)
{
    HWND hwndCtrl;

    hwndCtrl = CreateWindowEx(
                 WS_EX_CLIENTEDGE, // give it a standard border
                 szClassName,
                 _T("A custom control"),
                 WS_VISIBLE | WS_CHILD,
                 0, 0, 100, 100,
                 hwndParent,
                 NULL, GetModuleHandle(0), NULL
               );

    return hwndCtrl;
}
InputCust * GetInputCustom(HWND hwnd)
{
	return (InputCust *)GetWindowLongPtr(hwnd, 0);
}

void SetInputCustom(HWND hwnd, InputCust *icp)
{
    SetWindowLongPtr(hwnd, 0, (LONG_PTR)icp);
}

LRESULT InputCustom_OnPaint(InputCust *ccp, WPARAM wParam, LPARAM lParam)
{
    HDC				hdc;
    PAINTSTRUCT		ps;
    HANDLE			hOldFont;
    TCHAR			szText[200];
    RECT			rect;
	SIZE			sz;
	int				x,y;

    // Get a device context for this window
    hdc = BeginPaint(ccp->hwnd, &ps);

    // Set the font we are going to use
    hOldFont = SelectObject(hdc, ccp->hFont);

    // Set the text colours
    SetTextColor(hdc, ccp->crForeGnd);
    SetBkColor  (hdc, ccp->crBackGnd);

    // Find the text to draw
    GetWindowText(ccp->hwnd, szText, sizeof(szText));

    // Work out where to draw
    GetClientRect(ccp->hwnd, &rect);


    // Find out how big the text will be
    GetTextExtentPoint32(hdc, szText, lstrlen(szText), &sz);

    // Center the text
    x = (rect.right  - sz.cx) / 2;
    y = (rect.bottom - sz.cy) / 2;

    // Draw the text
    ExtTextOut(hdc, x, y, ETO_OPAQUE, &rect, szText, lstrlen(szText), 0);

    // Restore the old font when we have finished
    SelectObject(hdc, hOldFont);

    // Release the device context
    EndPaint(ccp->hwnd, &ps);

    return 0;
}

static LRESULT CALLBACK InputCustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// retrieve the custom structure POINTER for THIS window
    InputCust *icp = GetInputCustom(hwnd);
	HWND pappy = (HWND__ *)GetWindowLongPtr(hwnd,GWLP_HWNDPARENT);
	funky= hwnd;

	static HWND selectedItem = NULL;

	char temp[100];
	COLORREF col;
    switch(msg)
    {

	case WM_GETDLGCODE:
		return DLGC_WANTARROWS|DLGC_WANTALLKEYS|DLGC_WANTCHARS;
		break;


    case WM_NCCREATE:

        // Allocate a new CustCtrl structure for this window.
        icp = (InputCust *) malloc( sizeof(InputCust) );

        // Failed to allocate, stop window creation.
        if(icp == NULL) return FALSE;

        // Initialize the CustCtrl structure.
        icp->hwnd      = hwnd;
        icp->crForeGnd = GetSysColor(COLOR_WINDOWTEXT);
        icp->crBackGnd = GetSysColor(COLOR_WINDOW);
        icp->hFont     = (HFONT__ *) GetStockObject(DEFAULT_GUI_FONT);

        // Assign the window text specified in the call to CreateWindow.
        SetWindowText(hwnd, ((CREATESTRUCT *)lParam)->lpszName);

        // Attach custom structure to this window.
        SetInputCustom(hwnd, icp);

		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);

		selectedItem = NULL;

		SetTimer(hwnd,777,125,NULL);

        // Continue with window creation.
        return TRUE;

    // Clean up when the window is destroyed.
    case WM_NCDESTROY:
        free(icp);
        break;
	case WM_PAINT:
		return InputCustom_OnPaint(icp,wParam,lParam);
		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_USER+45:
	case WM_KEYDOWN:
    {
        UINT scancode = (lParam & 0x00ff0000) >> 16;
        int extended = (lParam & 0x01000000) != 0;

        switch (wParam) {
        case VK_SHIFT:
            wParam = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
            break;
        case VK_CONTROL:
            wParam = extended ? VK_RCONTROL : VK_LCONTROL;
            break;
        case VK_MENU:
            wParam = extended ? VK_RMENU : VK_LMENU;
            break;
        default:
            break;
        }

        TranslateKey(wParam, temp);
        col = CheckButtonKey(wParam);

        icp->crForeGnd = ((~col) & 0x00ffffff);
        icp->crBackGnd = col;
        SetWindowText(hwnd, _tFromChar(temp));
        InvalidateRect(icp->hwnd, NULL, FALSE);
        UpdateWindow(icp->hwnd);
        SendMessage(pappy, WM_USER + 43, wParam, (LPARAM)hwnd);

        break;
    }
	case WM_USER+44:

		TranslateKey(wParam,temp);
		if(IsWindowEnabled(hwnd))
		{
			col = CheckButtonKey(wParam);
		}
		else
		{
			col = RGB( 192,192,192);
		}
		icp->crForeGnd = ((~col) & 0x00ffffff);
		icp->crBackGnd = col;
		SetWindowText(hwnd,_tFromChar(temp));
		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);

		break;

	case WM_SETFOCUS:
	{
		selectedItem = hwnd;
		col = RGB( 0,255,0);
		icp->crForeGnd = ((~col) & 0x00ffffff);
		icp->crBackGnd = col;
		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);
//		tid = wParam;

		break;
	}
	case WM_KILLFOCUS:
	{
		selectedItem = NULL;
		SendMessage(pappy,WM_USER+46,wParam,(LPARAM)hwnd); // refresh fields on deselect
		break;
	}

	case WM_TIMER:
		if(hwnd == selectedItem)
		{
			FunkyJoyStickTimer();
		}
		SetTimer(hwnd,777,125,NULL);
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		break;
	case WM_ENABLE:
		COLORREF col;
		if(wParam)
		{
			col = RGB( 255,255,255);
			icp->crForeGnd = ((~col) & 0x00ffffff);
			icp->crBackGnd = col;
		}
		else
		{
			col = RGB( 192,192,192);
			icp->crForeGnd = ((~col) & 0x00ffffff);
			icp->crBackGnd = col;
		}
		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);
		return true;
    default:
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void TranslateKeyWithModifiers(int wParam, int modifiers, char * outStr)
{

	// if the key itself is a modifier, special case output:
	if(wParam == VK_SHIFT)
		strcpy(outStr, "Shift");
	else if(wParam == VK_MENU)
		strcpy(outStr, "Alt");
	else if(wParam == VK_CONTROL)
		strcpy(outStr, "Control");
	else
	{
		// otherwise, prepend the modifier(s)
		if(wParam != VK_ESCAPE && wParam != 0)
		{
			if((modifiers & CUSTKEY_CTRL_MASK) != 0)
			{
				sprintf(outStr,HOTKEYS_CONTROL_MOD);
				outStr += strlen(HOTKEYS_CONTROL_MOD);
			}
			if((modifiers & CUSTKEY_ALT_MASK) != 0)
			{
				sprintf(outStr,HOTKEYS_ALT_MOD);
				outStr += strlen(HOTKEYS_ALT_MOD);
			}
			if((modifiers & CUSTKEY_SHIFT_MASK) != 0)
			{
				sprintf(outStr,HOTKEYS_SHIFT_MOD);
				outStr += strlen(HOTKEYS_SHIFT_MOD);
			}
		}

		// and append the translated non-modifier key
		TranslateKey(wParam,outStr);
	}
}

static bool keyPressLock = false;

static LRESULT CALLBACK HotInputCustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// retrieve the custom structure POINTER for THIS window
    InputCust *icp = GetInputCustom(hwnd);
	HWND pappy = (HWND__ *)GetWindowLongPtr(hwnd,GWLP_HWNDPARENT);
	funky= hwnd;

	static HWND selectedItem = NULL;

	char temp[100];
	COLORREF col;
    switch(msg)
    {

	case WM_GETDLGCODE:
		return DLGC_WANTARROWS|DLGC_WANTALLKEYS|DLGC_WANTCHARS;
		break;


    case WM_NCCREATE:

        // Allocate a new CustCtrl structure for this window.
        icp = (InputCust *) malloc( sizeof(InputCust) );

        // Failed to allocate, stop window creation.
        if(icp == NULL) return FALSE;

        // Initialize the CustCtrl structure.
        icp->hwnd      = hwnd;
        icp->crForeGnd = GetSysColor(COLOR_WINDOWTEXT);
        icp->crBackGnd = GetSysColor(COLOR_WINDOW);
        icp->hFont     = (HFONT__ *) GetStockObject(DEFAULT_GUI_FONT);

        // Assign the window text specified in the call to CreateWindow.
        SetWindowText(hwnd, ((CREATESTRUCT *)lParam)->lpszName);

        // Attach custom structure to this window.
        SetInputCustom(hwnd, icp);

		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);

		keyPressLock = false;

		selectedItem = NULL;

		SetTimer(hwnd,747,125,NULL);

        // Continue with window creation.
        return TRUE;

    // Clean up when the window is destroyed.
    case WM_NCDESTROY:
        free(icp);
        break;
	case WM_PAINT:
		return InputCustom_OnPaint(icp,wParam,lParam);
		break;
	case WM_ERASEBKGND:
		return 1;
/*
	case WM_KEYUP:
		{
			int count = 0;
			for(int i=0;i<256;i++)
				if(GetAsyncKeyState(i))
					count++;

			if(count < 2)
			{
				int p = count;
			}
			if(count < 1)
			{
				int p = count;
			}

			TranslateKey(wParam,temp);
			col = CheckButtonKey(wParam);

			icp->crForeGnd = ((~col) & 0x00ffffff);
			icp->crBackGnd = col;
			SetWindowText(hwnd,temp);
			InvalidateRect(icp->hwnd, NULL, FALSE);
			UpdateWindow(icp->hwnd);
			SendMessage(pappy,WM_USER+43,wParam,(LPARAM)hwnd);
		}
		break;
*/
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:

		{
			int count = 0;
			for(int i=2;i<256;i++)
			{
				if(i >= VK_LSHIFT && i <= VK_RMENU)
					continue;
				if(GetAsyncKeyState(i))
					count++;
			}

			if(count <= 1)
			{
				keyPressLock = false;
			}
		}

		// no break

	case WM_USER+45:
		// assign a hotkey:
		{
			// don't assign pure modifiers on key-down (they're assigned on key-up)
			if(wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_CONTROL)
				break;

			int modifiers = 0;
			if(GetAsyncKeyState(VK_MENU))
				modifiers |= CUSTKEY_ALT_MASK;
			if(GetAsyncKeyState(VK_CONTROL))
				modifiers |= CUSTKEY_CTRL_MASK;
			if(GetAsyncKeyState(VK_SHIFT))
				modifiers |= CUSTKEY_SHIFT_MASK;

			TranslateKeyWithModifiers(wParam, modifiers, temp);

			col = CheckHotKey(wParam,modifiers);
///			if(col == RGB(255,0,0)) // un-redify
///				col = RGB(255,255,255);

			icp->crForeGnd = ((~col) & 0x00ffffff);
			icp->crBackGnd = col;
			SetWindowText(hwnd,_tFromChar(temp));
			InvalidateRect(icp->hwnd, NULL, FALSE);
			UpdateWindow(icp->hwnd);
			SendMessage(pappy,WM_USER+43,wParam,(LPARAM)hwnd);

			keyPressLock = true;

		}
		break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
		if(!keyPressLock)
		{
			int count = 0;
			for(int i=2;i<256;i++)
			{
				if(i >= VK_LSHIFT && i <= VK_RMENU)
					continue;
				if(GetAsyncKeyState(i))
					count++;
			}

			if(count <= 1)
			{
				if(wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_CONTROL)
				{
					if(wParam == VK_SHIFT)
						sprintf(temp, "Shift");
					if(wParam == VK_MENU)
						sprintf(temp, "Alt");
					if(wParam == VK_CONTROL)
						sprintf(temp, "Control");

					col = CheckHotKey(wParam,0);

					icp->crForeGnd = ((~col) & 0x00ffffff);
					icp->crBackGnd = col;
					SetWindowText(hwnd,_tFromChar(temp));
					InvalidateRect(icp->hwnd, NULL, FALSE);
					UpdateWindow(icp->hwnd);
					SendMessage(pappy,WM_USER+43,wParam,(LPARAM)hwnd);
				}
			}
		}
		break;
	case WM_USER+44:

		// set a hotkey field:
		{
		int modifiers = lParam;

		TranslateKeyWithModifiers(wParam, modifiers, temp);

		if(IsWindowEnabled(hwnd))
		{
			col = CheckHotKey(wParam,modifiers);
///			if(col == RGB(255,0,0)) // un-redify
///				col = RGB(255,255,255);
		}
		else
		{
			col = RGB( 192,192,192);
		}
		icp->crForeGnd = ((~col) & 0x00ffffff);
		icp->crBackGnd = col;
		SetWindowText(hwnd,_tFromChar(temp));
		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);
		}
		break;

	case WM_SETFOCUS:
	{
		selectedItem = hwnd;
		col = RGB( 0,255,0);
		icp->crForeGnd = ((~col) & 0x00ffffff);
		icp->crBackGnd = col;
		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);
//		tid = wParam;

		break;
	}
	case WM_KILLFOCUS:
	{
		selectedItem = NULL;
		SendMessage(pappy,WM_USER+46,wParam,(LPARAM)hwnd); // refresh fields on deselect
		break;
	}

	case WM_TIMER:
		if(hwnd == selectedItem)
		{
			FunkyJoyStickTimer();
		}
		SetTimer(hwnd,747,125,NULL);
		break;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		break;
	case WM_ENABLE:
		COLORREF col;
		if(wParam)
		{
			col = RGB( 255,255,255);
			icp->crForeGnd = ((~col) & 0x00ffffff);
			icp->crBackGnd = col;
		}
		else
		{
			col = RGB( 192,192,192);
			icp->crForeGnd = ((~col) & 0x00ffffff);
			icp->crBackGnd = col;
		}
		InvalidateRect(icp->hwnd, NULL, FALSE);
		UpdateWindow(icp->hwnd);
		return true;
    default:
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

