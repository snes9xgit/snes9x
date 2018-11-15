/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// CDirectSound.h: interface for the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(DIRECTSOUND_H_INCLUDED)
#define DIRECTSOUND_H_INCLUDED

#include <windows.h>
#include "IS9xSoundOutput.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CDirectSound : public IS9xSoundOutput
{
private:
	LPDIRECTSOUND lpDS;
	LPDIRECTSOUNDBUFFER lpDSB;				// the buffer used for mixing
	LPDIRECTSOUNDBUFFER lpDSBPrimary;

	int blockCount;							// number of blocks in the buffer
	int blockSize;							// bytes in one block
	int blockSamples;						// samples in one block
	int bufferSize;							// bytes in the whole buffer
	int blockTime;							// ms in one block

	DWORD last_block;						// the last block that was mixed

	bool initDone;							// has init been called successfully?
	DWORD hTimer;							// mixing timer
	
	bool InitDirectSound ();
	void DeInitDirectSound();

	bool InitSoundBuffer();
	void DeInitSoundBuffer();

	static VOID CALLBACK SoundTimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
	
	void ProcessSound();
	void MixSound();

public:
	CDirectSound();
	~CDirectSound();

	// Inherited from IS9xSoundOutput
	bool InitSoundOutput(void) { return InitDirectSound(); }
	void DeInitSoundOutput(void) { DeInitDirectSound(); }
	bool SetupSound(void);
	void SetVolume(double volume);
	
};

extern CDirectSound DirectSound;

#endif // !defined(DIRECTSOUND_H_INCLUDED)
