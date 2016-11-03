#include "snes9x.h"
#include "display.h"
#include "msu1.h"
#include <fstream>

std::ifstream dataFile, audioFile;
uint32 dataPos, audioPos, audioResumePos, audioLoopPos;
uint16 audioTrack, audioResumeTrack;
char fName[64];

// Sample buffer
int16 *bufPos, *bufBegin, *bufEnd;

void S9xMSU1Init(void)
{
	MSU1.MSU1_STATUS	= 0;
	MSU1.MSU1_SEEK		= 0;
	MSU1.MSU1_TRACK		= 0;
	MSU1.MSU1_VOLUME	= 0;
	MSU1.MSU1_CONTROL	= 0;

	dataPos				= 0;
	audioPos			= 0;
	audioResumePos		= 0;

	audioTrack			= 0;
	audioResumeTrack	= 0;

	bufPos				= 0;
	bufBegin			= 0;
	bufEnd				= 0;

	if (dataFile.is_open())
		dataFile.close();

	if (audioFile.is_open())
		audioFile.close();

	dataFile.open(S9xGetFilename(".msu", ROMFILENAME_DIR), std::ios::in | std::ios::binary);
}

void S9xMSU1Execute(void)
{
	static long long hitcount = 0;
	//return; // Dummy out for now because it's broken
	while ((bufPos < bufEnd) && (MSU1.MSU1_STATUS & AudioPlaying))
	{
		hitcount++;
		if (audioFile.good())
		{
			audioPos += 2;
			int16 sample = 0;
			((uint8 *)&sample)[0] = audioFile.get();
			((uint8 *)&sample)[1] = audioFile.get();
			
			sample = (double)sample * (double)MSU1.MSU1_VOLUME / 255.0;

			*bufPos = ((uint8 *)&sample)[0];
			*(bufPos + 1) = ((uint8 *)&sample)[1];

			bufPos += 2;

			if (audioFile.eof())
			{
				if (MSU1.MSU1_STATUS & AudioRepeating)
				{
					audioPos = audioLoopPos;
					audioFile.seekg(audioPos);
				}
				else
				{
					MSU1.MSU1_STATUS &= ~(AudioPlaying | AudioRepeating);
					audioFile.seekg(8);
					return;
				}
			}
		}
		else
		{
			MSU1.MSU1_STATUS &= ~AudioPlaying;
		}
	}
}


uint8 S9xMSU1ReadPort(int port)
{
	switch (port)
	{
	case 0:
		return MSU1.MSU1_STATUS;
	case 1:
		if (MSU1.MSU1_STATUS & DataBusy)
			return 0;
		if (dataFile.fail() || dataFile.bad() || dataFile.eof())
			return 0;
		return dataFile.get();
	case 2:
		return 'S';
	case 3:
		return '-';
	case 4:
		return 'M';
	case 5:
		return 'S';
	case 6:
		return 'U';
	case 7:
		return '1';
	}
}


void S9xMSU1WritePort(int port, uint8 byte)
{
	switch (port)
	{
	case 0:
		((uint8 *)(&MSU1.MSU1_SEEK))[0] = byte;
		break;
	case 1:
		((uint8 *)(&MSU1.MSU1_SEEK))[1] = byte;
		break;
	case 2:
		((uint8 *)(&MSU1.MSU1_SEEK))[2] = byte;
		break;
	case 3:
		((uint8 *)(&MSU1.MSU1_SEEK))[3] = byte;
		dataPos = MSU1.MSU1_SEEK;
		if(dataFile.good())
			dataFile.seekg(dataPos);
		break;
	case 4:
		((uint8 *)(&MSU1.MSU1_TRACK))[0] = byte;
		break;
	case 5:
		((uint8 *)(&MSU1.MSU1_TRACK))[1] = byte;
		audioTrack = MSU1.MSU1_TRACK;
		if (audioFile.is_open())
			audioFile.close();
		// This is an ugly hack... need to see if there's a better way to get the base name without extension
		sprintf(fName, "%s", S9xGetFilename(".msu", ROMFILENAME_DIR));
		fName[strlen(fName) - 4] = '\0';
		sprintf(fName, "%s-%d.pcm", fName, audioTrack);

		audioFile.open(fName);
		if (audioFile.is_open() && audioFile.good())
		{
			MSU1.MSU1_STATUS |= AudioError;

			if (audioFile.get() != 'M')
				break;
			if (audioFile.get() != 'S')
				break;
			if (audioFile.get() != 'U')
				break;
			if (audioFile.get() != '1')
				break;

			((uint8 *)(&audioLoopPos))[0] = audioFile.get();
			((uint8 *)(&audioLoopPos))[1] = audioFile.get();
			((uint8 *)(&audioLoopPos))[2] = audioFile.get();
			((uint8 *)(&audioLoopPos))[3] = audioFile.get();
			audioLoopPos += 8;

			MSU1.MSU1_STATUS &= ~AudioPlaying;
			MSU1.MSU1_STATUS &= ~AudioRepeating;
			if (audioTrack == audioResumeTrack)
			{
				audioPos = audioResumePos;
				audioResumeTrack = 0xFFFF;
				audioResumePos = 0;
			}
			else
				audioPos = 8;

			audioFile.seekg(audioPos);

			MSU1.MSU1_STATUS &= ~AudioError;
		}
		else
			MSU1.MSU1_STATUS |= AudioError;
		break;
	case 6:
		MSU1.MSU1_VOLUME = byte;
		break;
	case 7:
		if (MSU1.MSU1_STATUS & (AudioBusy | AudioError))
			break;

		MSU1.MSU1_STATUS = (MSU1.MSU1_STATUS & ~0x30) | ((byte & 0x03) << 4);
		if ((byte & 0x05) == 0x05)
			audioResumeTrack = audioTrack;
		break;
	}
}

uint16 S9xMSU1Samples(void)
{
	return bufPos - bufBegin;
}

void S9xMSU1SetOutput(int16 * out, int size)
{
	bufPos = bufBegin = out;
	bufEnd = out + size;
}
