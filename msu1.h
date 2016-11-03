#ifndef _MSU1_H_
#define _MSU1_H_
#include "snes9x.h"

struct SMSU1
{
	uint8	MSU1_STATUS;
	uint32	MSU1_SEEK;
	uint16	MSU1_TRACK;
	uint8	MSU1_VOLUME;
	uint8	MSU1_CONTROL;
};

enum SMSU1_FLAG : uint8 {
	Revision		= 0x02,	//max: 0x07
	AudioResume		= 0x04,
	AudioError		= 0x08,
	AudioPlaying	= 0x10,
	AudioRepeating	= 0x20,
	AudioBusy		= 0x40,
	DataBusy		= 0x80,
};

extern struct SMSU1	MSU1;

void S9xMSU1Init(void);
void S9xMSU1Execute(void);
uint8 S9xMSU1ReadPort(int port);
void S9xMSU1WritePort(int port, uint8 byte);
uint16 S9xMSU1Samples(void);
void S9xMSU1SetOutput(int16 *out, int size);

#endif
