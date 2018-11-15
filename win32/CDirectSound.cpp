/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

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

void CDirectSound::SetVolume(double volume)
{
	if (!initDone)
		return;

	// convert percentage to hundredths of dB
	LONG dbVolume = (LONG)(10 * log10(volume) * 2 * 100);

	lpDSB->SetVolume(dbVolume);
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

