#ifdef FMODEX_SUPPORT
#ifndef FMOD_SUPPORT
#include "CFMODEx.h"
#include "wsnes9x.h"
#include "../snes9x.h"
#include "../apu/apu.h"

CFMODEx::CFMODEx(void)
{
	initDone = false;
	fmodStream = NULL;
	
		fmodSystem = NULL;
}

CFMODEx::~CFMODEx(void)
{
	DeInitFMODEx();
	if(fmodSystem)
		fmodSystem->release();
}

bool CFMODEx::InitStream()
{
	FMOD_CREATESOUNDEXINFO createSoundExInfo={0};
	createSoundExInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	createSoundExInfo.defaultfrequency = Settings.SoundPlaybackRate;
	createSoundExInfo.numchannels = (Settings.Stereo?2:1);
	createSoundExInfo.format = (Settings.SixteenBitSound?FMOD_SOUND_FORMAT_PCM16:FMOD_SOUND_FORMAT_PCM8);
	createSoundExInfo.pcmreadcallback = FMODExStreamCallback;
	createSoundExInfo.suggestedsoundtype = FMOD_SOUND_TYPE_USER;
	//fmodSystem->getDSPBufferSize(&temp,NULL);
	// 768 was the bufferSize in FMOD
	sampleCount = (Settings.SoundPlaybackRate * GUI.SoundBufferSize/2 ) / 1000;
    if (Settings.Stereo)
		sampleCount *= 2;
	bufferSize = sampleCount * (Settings.SixteenBitSound?2:1);
	createSoundExInfo.length = bufferSize;
	createSoundExInfo.decodebuffersize = bufferSize/4;

	FMOD_RESULT fr = fmodSystem->createStream(NULL,FMOD_OPENUSER | FMOD_LOOP_NORMAL | FMOD_OPENRAW,&createSoundExInfo,&fmodStream);

	if(!(fr==FMOD_OK)) {
		return false;
	}

	fr = fmodSystem->playSound(FMOD_CHANNEL_FREE,fmodStream,0,NULL);
	
	return true;
}

void CFMODEx::DeInitStream()
{
	if (fmodStream)
    {
		fmodStream->release();
		fmodStream = NULL;
    }
}

bool CFMODEx::InitFMODEx()
{
	if(initDone)
		return true;

	FMOD_RESULT fr;

	if(!(FMOD::System_Create(&fmodSystem)==FMOD_OK))
		return false;

	switch (GUI.SoundDriver)
    {
        default:
        case WIN_FMODEX_DEFAULT_DRIVER:
			fr = fmodSystem->setOutput(FMOD_OUTPUTTYPE_AUTODETECT);
            break;
        case WIN_FMODEX_ASIO_DRIVER:
            fr = fmodSystem->setOutput(FMOD_OUTPUTTYPE_ASIO);
            break;
		case WIN_FMODEX_OPENAL_DRIVER:
            fr = fmodSystem->setOutput(FMOD_OUTPUTTYPE_OPENAL);
            break;
    }

    fr = fmodSystem->init(2,FMOD_INIT_NORMAL,0);

	if(fr!=FMOD_OK)
		return false;

	initDone = true;
	return true;
}

void CFMODEx::DeInitFMODEx()
{
	initDone = false;
	DeInitStream();
	if(fmodSystem) {
		fmodSystem->release();
		fmodSystem = NULL;
	}
}

bool CFMODEx::SetupSound()
{
	if(!initDone)
		return false;

	DeInitStream();
	return InitStream();
}

void CFMODEx::ProcessSound()
{
	EnterCriticalSection(&GUI.SoundCritSect);

	S9xFinalizeSamples();

	LeaveCriticalSection(&GUI.SoundCritSect);
}

FMOD_RESULT F_CALLBACK CFMODEx::FMODExStreamCallback(
	  FMOD_SOUND *  sound, 
	  void *  data, 
	  unsigned int  datalen
	)
{
    int sample_count = datalen;

	sample_count >>= (Settings.SixteenBitSound?1:0);

	EnterCriticalSection(&GUI.SoundCritSect);

	S9xMixSamples((unsigned char *) data, sample_count);

	LeaveCriticalSection(&GUI.SoundCritSect);

    return FMOD_OK;
}
#endif
#endif