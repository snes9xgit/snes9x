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

  MSU-1 code
  (c) Copyright 2016         qwertymodo


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

#include "snes9x.h"
#include "display.h"
#include "msu1.h"
#include "apu/bapu/dsp/blargg_endian.h"
#include <fstream>
#include <sys/stat.h>

std::ifstream dataFile, audioFile;
uint32 audioLoopPos;
uint32 partial_samples;

// Sample buffer
int16 *bufPos, *bufBegin, *bufEnd;

bool AudioOpen()
{
	MSU1.MSU1_STATUS |= AudioError;

	if (audioFile.is_open())
		audioFile.close();

	char ext[_MAX_EXT];
	snprintf(ext, _MAX_EXT, "-%d.pcm", MSU1.MSU1_CURRENT_TRACK);

	audioFile.clear();
	audioFile.open(S9xGetFilename(ext, ROMFILENAME_DIR), std::ios::in | std::ios::binary);
	if (audioFile.good())
	{
		if (audioFile.get() != 'M')
			return false;
		if (audioFile.get() != 'S')
			return false;
		if (audioFile.get() != 'U')
			return false;
		if (audioFile.get() != '1')
			return false;

		audioFile.read((char *)&audioLoopPos, 4);
		audioLoopPos = GET_LE32(&audioLoopPos);
		audioLoopPos <<= 2;
		audioLoopPos += 8;

		MSU1.MSU1_STATUS &= ~AudioError;
		return true;
	}

	return false;
}

bool DataOpen()
{
	if (dataFile.is_open())
		dataFile.close();

	dataFile.clear();
	dataFile.open(S9xGetFilename(".msu", ROMFILENAME_DIR), std::ios::in | std::ios::binary);
	return dataFile.is_open();
}

void S9xResetMSU(void)
{
	MSU1.MSU1_STATUS		= 0;
	MSU1.MSU1_DATA_SEEK		= 0;
	MSU1.MSU1_DATA_POS		= 0;
	MSU1.MSU1_TRACK_SEEK	= 0;
	MSU1.MSU1_CURRENT_TRACK = 0;
	MSU1.MSU1_RESUME_TRACK	= 0;
	MSU1.MSU1_VOLUME		= 0;
	MSU1.MSU1_CONTROL		= 0;
	MSU1.MSU1_AUDIO_POS		= 0;
	MSU1.MSU1_RESUME_POS	= 0;


	bufPos				= 0;
	bufBegin			= 0;
	bufEnd				= 0;

	partial_samples = 0;

	if (dataFile.is_open())
		dataFile.close();

	if (audioFile.is_open())
		audioFile.close();

	Settings.MSU1 = S9xMSU1ROMExists();
}

void S9xMSU1Init(void)
{
	S9xResetMSU();
	DataOpen();
}

bool S9xMSU1ROMExists(void)
{
	struct stat buf;
	return (stat(S9xGetFilename(".msu", ROMFILENAME_DIR), &buf) == 0);
}

void S9xMSU1Generate(int sample_count)
{
	partial_samples += 44100 * sample_count;

	while (((uintptr_t)bufPos < (uintptr_t)bufEnd) && (MSU1.MSU1_STATUS & AudioPlaying) && partial_samples > 32040)
	{
		if (audioFile.is_open())
		{
			int16 sample;
			if (audioFile.read((char *)&sample, 2).good())
			{
				sample = (int16)((double)(int16)GET_LE16(&sample) * (double)MSU1.MSU1_VOLUME / 255.0);

				*(bufPos++) = sample;
				MSU1.MSU1_AUDIO_POS += 2;
				partial_samples -= 32040;
			}
			else
			if (audioFile.eof())
			{
				sample = (int16)((double)(int16)GET_LE16(&sample) * (double)MSU1.MSU1_VOLUME / 255.0);

				*(bufPos++) = sample;
				MSU1.MSU1_AUDIO_POS += 2;
				partial_samples -= 32040;

				if (MSU1.MSU1_STATUS & AudioRepeating)
				{
					audioFile.clear();
					MSU1.MSU1_AUDIO_POS = audioLoopPos;
					audioFile.seekg(MSU1.MSU1_AUDIO_POS);
				}
				else
				{
					MSU1.MSU1_STATUS &= ~(AudioPlaying | AudioRepeating);
					audioFile.clear();
					audioFile.seekg(8);
					return;
				}
			}
			else
			{
				MSU1.MSU1_STATUS &= ~(AudioPlaying | AudioRepeating);
				return;
			}
		}
		else
		{
			MSU1.MSU1_STATUS &= ~(AudioPlaying | AudioRepeating);
			return;
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
		MSU1.MSU1_DATA_POS++;
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

	return 0;
}


void S9xMSU1WritePort(int port, uint8 byte)
{
	switch (port)
	{
	case 0:
		MSU1.MSU1_DATA_SEEK &= 0xFFFFFF00;
		MSU1.MSU1_DATA_SEEK |= byte << 0;
		break;
	case 1:
		MSU1.MSU1_DATA_SEEK &= 0xFFFF00FF;
		MSU1.MSU1_DATA_SEEK |= byte << 8;
		break;
	case 2:
		MSU1.MSU1_DATA_SEEK &= 0xFF00FFFF;
		MSU1.MSU1_DATA_SEEK |= byte << 16;
		break;
	case 3:
		MSU1.MSU1_DATA_SEEK &= 0x00FFFFFF;
		MSU1.MSU1_DATA_SEEK |= byte << 24;
		MSU1.MSU1_DATA_POS = MSU1.MSU1_DATA_SEEK;
		if(dataFile.good())
			dataFile.seekg(MSU1.MSU1_DATA_POS);
		break;
	case 4:
		MSU1.MSU1_TRACK_SEEK &= 0xFF00;
		MSU1.MSU1_TRACK_SEEK |= byte;
		break;
	case 5:
		MSU1.MSU1_TRACK_SEEK &= 0x00FF;
		MSU1.MSU1_TRACK_SEEK |= (byte << 8);
		MSU1.MSU1_CURRENT_TRACK = MSU1.MSU1_TRACK_SEEK;

		MSU1.MSU1_STATUS &= ~AudioPlaying;
		MSU1.MSU1_STATUS &= ~AudioRepeating;

		if (AudioOpen())
		{
			if (MSU1.MSU1_CURRENT_TRACK == MSU1.MSU1_RESUME_TRACK)
			{
				MSU1.MSU1_AUDIO_POS = MSU1.MSU1_RESUME_POS;
				MSU1.MSU1_RESUME_POS = 0;
				MSU1.MSU1_RESUME_TRACK = ~0;
			}
			else
			{
				MSU1.MSU1_AUDIO_POS = 8;
			}

			audioFile.seekg(MSU1.MSU1_AUDIO_POS);
		}
		break;
	case 6:
		MSU1.MSU1_VOLUME = byte;
		break;
	case 7:
		if (MSU1.MSU1_STATUS & (AudioBusy | AudioError))
			break;

		MSU1.MSU1_STATUS = (MSU1.MSU1_STATUS & ~0x30) | ((byte & 0x03) << 4);

		if ((byte & (Play | Resume)) == Resume)
		{
			MSU1.MSU1_RESUME_TRACK = MSU1.MSU1_CURRENT_TRACK;
			MSU1.MSU1_RESUME_POS = MSU1.MSU1_AUDIO_POS;
		}
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

void S9xMSU1PostLoadState(void)
{
	if (DataOpen())
	{
		dataFile.seekg(MSU1.MSU1_DATA_POS);
	}

	if (MSU1.MSU1_STATUS & AudioPlaying)
	{
		if (AudioOpen())
		{
			audioFile.seekg(4);
			audioFile.read((char *)&audioLoopPos, 4);
			audioLoopPos = GET_LE32(&audioLoopPos);
			audioLoopPos <<= 2;
			audioLoopPos += 8;

			audioFile.seekg(MSU1.MSU1_AUDIO_POS);
		}
		else
		{
			MSU1.MSU1_STATUS &= ~(AudioPlaying | AudioRepeating);
			MSU1.MSU1_STATUS |= AudioError;
		}
	}

	bufPos = 0;
	bufBegin = 0;
	bufEnd = 0;

	partial_samples = 0;
}
