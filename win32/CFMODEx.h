#ifndef CFMODEX_H
#define CFMODEX_H
#include "../../FMODEx/api/inc/fmod.hpp"
#include "IS9xSoundOutput.h"

class CFMODEx: public IS9xSoundOutput
{
	bool initDone;

	FMOD::System *fmodSystem;
	FMOD::Sound *fmodStream;


	int sampleCount;
	int bufferSize;

	bool InitFMODEx();
	void DeInitFMODEx();

	bool InitStream();
	void DeInitStream();

	static FMOD_RESULT F_CALLBACK FMODExStreamCallback(
	  FMOD_SOUND *  sound, 
	  void *  data, 
	  unsigned int  datalen
	);

public:
	CFMODEx(void);
	~CFMODEx(void);

	// Inherited from IS9xSoundOutput
	bool InitSoundOutput(void) { return InitFMODEx(); }
	void DeInitSoundOutput(void) { DeInitFMODEx(); }
	bool SetupSound();
	void ProcessSound();
};

#endif