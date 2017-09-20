/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2016  BearOso,
                             OV2

  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  S-SMP emulator code used in 1.54+
  (c) Copyright 2016         byuu

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2016  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2016  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones

  Libretro port
  (c) Copyright 2011 - 2016  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/




// CDirectSound.cpp: implementation of the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#include "wsnes9x.h"
#include "../snes9x.h"
#include "../apu/apu.h"
#include "CDirectSound.h"
#include <process.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDirectSound::CDirectSound()
{
    lpDS = NULL;
    lpDSB = NULL;
    lpDSBPrimary = NULL;

    initDone = NULL;
	blockCount = 0;
	blockSize = 0;
	bufferSize = 0;
	blockSamples = 0;
	hTimer = NULL;
}

CDirectSound::~CDirectSound()
{
	DeInitDirectSound();
}

/*  CDirectSound::InitDirectSound
initializes the DirectSound object, the timer queue for the mixing timer, and sets the cooperation level
-----
returns true if successful, false otherwise
*/
bool CDirectSound::InitDirectSound ()
{
	HRESULT dErr;

	if(initDone)
		return true;

    if (!lpDS)
    {
        dErr = DirectSoundCreate (NULL, &lpDS, NULL);
        if (dErr != DS_OK)
        {
            MessageBox (GUI.hWnd, TEXT("\
Unable to initialise DirectSound. You will not be able to hear any\n\
sound effects or music while playing.\n\n\
It is usually caused by not having DirectX installed, another\n\
application that has already opened DirectSound in exclusive\n\
mode or the Windows WAVE device has been opened."),
                        TEXT("Snes9X - Unable to Open DirectSound"),
                        MB_OK | MB_ICONWARNING);
            return (false);
        }
    }
	initDone = true;
	dErr = lpDS->SetCooperativeLevel (GUI.hWnd, DSSCL_PRIORITY | DSSCL_EXCLUSIVE);
    if (!SUCCEEDED(dErr))
    {
		dErr = lpDS->SetCooperativeLevel (GUI.hWnd, DSSCL_PRIORITY);
		if (!SUCCEEDED(dErr))
		{
			if (!SUCCEEDED(lpDS -> SetCooperativeLevel (GUI.hWnd, DSSCL_NORMAL)))
			{
				DeInitDirectSound();
				initDone = false;
			}
			if (initDone)
				MessageBox (GUI.hWnd, TEXT("\
Unable to set DirectSound's  priority cooperative level.\n\
Another application is dicating the sound playback rate,\n\
sample size and mono/stereo setting."),
					TEXT("Snes9X - Unable to Set DirectSound priority"),
							MB_OK | MB_ICONWARNING);
			else
				MessageBox (GUI.hWnd, TEXT("\
Unable to set any DirectSound cooperative level. You will\n\
not be able to hear any sound effects or music while playing.\n\n\
It is usually caused by another application that has already\n\
opened DirectSound in exclusive mode."),
					TEXT("Snes9X - Unable to DirectSound"),
							MB_OK | MB_ICONWARNING);
		}
	}

    return (initDone);
}

/*  CDirectSound::DeInitDirectSound
releases all DirectSound objects and buffers
*/
void CDirectSound::DeInitDirectSound()
{
	initDone = false;

	DeInitSoundBuffer();
	
	if( lpDS != NULL)
    {
        lpDS->SetCooperativeLevel (GUI.hWnd, DSSCL_NORMAL);
        lpDS->Release ();
        lpDS = NULL;
    }
}

/*  CDirectSound::InitSoundBuffer
creates the DirectSound buffers and allocates the temp buffer for SoundSync
-----
returns true if successful, false otherwise
*/
bool CDirectSound::InitSoundBuffer()
{
	DSBUFFERDESC dsbd;
	WAVEFORMATEX wfx,wfx_actual;
	HRESULT dErr;

	blockCount = 4;
	blockTime = GUI.SoundBufferSize / blockCount;

	blockSamples = (Settings.SoundPlaybackRate * blockTime) / 1000;
    blockSamples *= (Settings.Stereo ? 2 : 1);
	blockSize = blockSamples * (Settings.SixteenBitSound ? 2 : 1);
	bufferSize = blockSize * blockCount;

    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = Settings.Stereo ? 2 : 1;
    wfx.nSamplesPerSec = Settings.SoundPlaybackRate;
    wfx.nBlockAlign = (Settings.SixteenBitSound ? 2 : 1) * (Settings.Stereo ? 2 : 1);
    wfx.wBitsPerSample = Settings.SixteenBitSound ? 16 : 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    ZeroMemory (&dsbd, sizeof(DSBUFFERDESC) );
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_STICKYFOCUS;

    dErr = lpDS->CreateSoundBuffer (&dsbd, &lpDSBPrimary, NULL);
    if (dErr != DS_OK)
    {
        lpDSB = NULL;
        return (false);
    }

    lpDSBPrimary->SetFormat (&wfx);
    if (lpDSBPrimary->GetFormat (&wfx_actual, sizeof (wfx_actual), NULL) == DS_OK)
    {
		if(wfx.nSamplesPerSec!=wfx_actual.nSamplesPerSec ||
			wfx_actual.nChannels != wfx.nChannels || wfx.wBitsPerSample != wfx_actual.wBitsPerSample) {
			return false;
		}
    }

    lpDSBPrimary->Play (0, 0, DSBPLAY_LOOPING);

    ZeroMemory (&dsbd, sizeof (dsbd));
    dsbd.dwSize = sizeof( dsbd);
    dsbd.dwFlags = DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME |
               DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STICKYFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
    dsbd.dwBufferBytes = bufferSize;
    dsbd.lpwfxFormat = &wfx;

    if (lpDS->CreateSoundBuffer (&dsbd, &lpDSB, NULL) != DS_OK)
    {
        lpDSBPrimary->Release ();
        lpDSBPrimary = NULL;
        lpDSB->Release();
        lpDSB = NULL;
        return (false);
    }

	return true;
}

/*  CDirectSound::DeInitSoundBuffer
deinitializes the DirectSound/temp buffers and stops the mixing timer
*/
void CDirectSound::DeInitSoundBuffer()
{
	if(hTimer) {
		timeKillEvent(hTimer);
		hTimer = NULL;
	}
	if( lpDSB != NULL)
    {
        lpDSB->Stop ();
        lpDSB->Release();
        lpDSB = NULL;
    }
    if( lpDSBPrimary != NULL)
    {
        lpDSBPrimary->Stop ();
        lpDSBPrimary->Release();
        lpDSBPrimary = NULL;
    }
}

/*  CDirectSound::SetupSound
applies sound setting changes by recreating the buffers and starting a new mixing timer
it fills the buffer before starting playback
-----
returns true if successful, false otherwise
*/
bool CDirectSound::SetupSound()
{
	HRESULT hResult;

	if(!initDone)
		return false;
	
	DeInitSoundBuffer();
	InitSoundBuffer();
	
	BYTE  *B1;
	DWORD S1;
	hResult = lpDSB->Lock (0, 0, (void **)&B1,
                                       &S1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (hResult == DSERR_BUFFERLOST)
    {
        lpDSB->Restore ();
        hResult = lpDSB->Lock (0, 0, (void **)&B1,
                                       &S1, NULL, NULL, DSBLOCK_ENTIREBUFFER);
    }
    if (!SUCCEEDED(hResult))
	{
        hResult = lpDSB -> Unlock (B1, S1, NULL, NULL);
        return true;
	}

	S9xMixSamples(B1,blockSamples * blockCount);
	lpDSB->Unlock(B1,S1,NULL,NULL);

	lpDSB->Play (0, 0, DSBPLAY_LOOPING);

	last_block = blockCount - 1;
	
	hTimer = timeSetEvent (blockTime/2, blockTime/2, SoundTimerCallback, (DWORD_PTR)this, TIME_PERIODIC);
	if(!hTimer) {
		DeInitSoundBuffer();
		return false;
	}

    return (true);
}

/*  CDirectSound::ProcessSound
Finishes core sample creation, syncronizes the buffer access.
*/
void CDirectSound::ProcessSound()
{
	EnterCriticalSection(&GUI.SoundCritSect);

	S9xFinalizeSamples();

	LeaveCriticalSection(&GUI.SoundCritSect);
}

/*  CDirectSound::MixSound
the mixing function called by the mix timer
uses the current play position to decide if a new block can be filled with audio data
synchronizes the core buffer access with a critical section
*/
void CDirectSound::MixSound()
{
    DWORD play_pos = 0, write_pos = 0;
    HRESULT hResult;
	DWORD curr_block;
	
	if(!initDone)
		return;
	
    lpDSB->GetCurrentPosition (&play_pos, NULL);

	curr_block = ((play_pos / blockSize) + blockCount) % blockCount;

    if (curr_block != last_block)
    {
	BYTE  *B1, *B2;
	DWORD S1, S2;

        write_pos = curr_block * blockSize;
        last_block = curr_block;

        hResult = lpDSB->Lock (write_pos, blockSize, (void **)&B1,
                                       &S1, (void **)&B2, &S2, 0);
        if (hResult == DSERR_BUFFERLOST)
        {
            lpDSB->Restore ();
            hResult = lpDSB->Lock (write_pos, blockSize,
                                           (void **)&B1, &S1, (void **)&B2,
                                           &S2, 0);
        }
        if (!SUCCEEDED(hResult))
		{
	        hResult = lpDSB -> Unlock (B1, S1, B2, S2);
            return;
		}

		EnterCriticalSection(&GUI.SoundCritSect);
        if (B1)
		{
			S9xMixSamples(B1,(Settings.SixteenBitSound?S1>>1:S1));
		}
        if (B2)
		{
			S9xMixSamples(B2,(Settings.SixteenBitSound?S2>>1:S2));
		}
		LeaveCriticalSection(&GUI.SoundCritSect);

		SetEvent(GUI.SoundSyncEvent);

        hResult = lpDSB -> Unlock (B1, S1, B2, S2);
        if (!SUCCEEDED(hResult))
            return;
    }
}

/*  CDirectSound::SoundTimerCallback
Timer callback that tries to mix a new block. Called twice each block.
*/

VOID CALLBACK CDirectSound::SoundTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2) {
	CDirectSound *S9xDirectSound = (CDirectSound *)dwUser;
    S9xDirectSound->MixSound();
}

