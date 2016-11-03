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
	while (((uintptr_t)bufPos < (uintptr_t)bufEnd) && (MSU1.MSU1_STATUS & AudioPlaying))
	{
		if (audioFile.is_open())
		{
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

			int16 sample = 0;
			audioFile.get(((char *)&sample), 2);
			
			sample = (double)sample * (double)MSU1.MSU1_VOLUME / 255.0;

			*(bufPos++) = sample;

			audioPos += 2;
		}
		else
		{
			MSU1.MSU1_STATUS &= ~(AudioPlaying | AudioRepeating);
			audioFile.seekg(8);
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
