/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef CXHAUDIO2_H
#define CXAUDIO2_H
#include "XAudio2.h"
#include "../snes9x.h"
#include <windows.h>
#include "IS9xSoundOutput.h"

class CXAudio2 : public IXAudio2VoiceCallback, public IS9xSoundOutput
{
private:
	IXAudio2SourceVoice *pSourceVoice;
	IXAudio2 *pXAudio2;
	IXAudio2MasteringVoice* pMasterVoice;

	bool initDone;							// has init been called successfully?

	volatile LONG bufferCount;				// currently submitted XAudio2 buffers

	UINT32 sum_bufferSize;					// the size of soundBuffer
	UINT32 singleBufferSamples;				// samples in one block
	UINT32 singleBufferBytes;				// bytes in one block
	UINT32 blockCount;						// soundBuffer is divided into blockCount blocks
											// currently set to 8
	UINT32 writeOffset;						// offset into the buffer for the next block
	UINT32 partialOffset;					// offset into non-complete block
	uint8 *soundBuffer;						// the buffer itself

	bool InitVoices(void);
	void DeInitVoices(void);

	void PushBuffer(UINT32 AudioBytes,BYTE *pAudioData,void *pContext);	
	void BeginPlayback(void);
	void StopPlayback(void);
	void ProcessSound(void);
	bool InitXAudio2(void);
	void DeInitXAudio2(void);
    int GetAvailableBytes();

public:
	CXAudio2(void);
	~CXAudio2(void);
		
	// inherited from IXAudio2VoiceCallback - we only use OnBufferEnd
	STDMETHODIMP_(void) OnBufferEnd(void *pBufferContext);
	STDMETHODIMP_(void) OnBufferStart(void *pBufferContext){}
	STDMETHODIMP_(void) OnLoopEnd(void *pBufferContext){}
	STDMETHODIMP_(void) OnStreamEnd() {}
	STDMETHODIMP_(void) OnVoiceError(void *pBufferContext, HRESULT Error) {}
	STDMETHODIMP_(void) OnVoiceProcessingPassEnd() {}
	STDMETHODIMP_(void) OnVoiceProcessingPassStart(UINT32 BytesRequired) {}


	// Inherited from IS9xSoundOutput
	bool InitSoundOutput(void) { return InitXAudio2(); }
	void DeInitSoundOutput(void) { DeInitXAudio2(); }
	bool SetupSound(void);
	void SetVolume(double volume);
	std::vector<std::wstring> GetDeviceList();
	int FindDeviceIndex(TCHAR *audio_device);
};

#endif
