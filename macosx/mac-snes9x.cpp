/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

/***********************************************************************************
  SNES9X for Mac OS (c) Copyright John Stiles

  Snes9x for Mac OS X

  (c) Copyright 2001 - 2011  zones
  (c) Copyright 2002 - 2005  107
  (c) Copyright 2002         PB1400c
  (c) Copyright 2004         Alexander and Sander
  (c) Copyright 2004 - 2005  Steven Seeger
  (c) Copyright 2005         Ryan Vogt
 ***********************************************************************************/


#include "snes9x.h"
#include "memmap.h"
#include "apu.h"
#include "snapshot.h"
#include "cheats.h"
#include "movie.h"
#include "display.h"

#include <wchar.h>

#include "mac-prefix.h"
#include "mac-audio.h"
#include "mac-cart.h"
#include "mac-cheatfinder.h"
#include "mac-dialog.h"
#include "mac-file.h"
#include "mac-multicart.h"
#include "mac-os.h"
#include "mac-quicktime.h"
#include "mac-screenshot.h"
#include "mac-stringtools.h"
#include "mac-snes9x.h"

extern wchar_t  macRecordWChar[MOVIE_MAX_METADATA];


void SNES9X_Go (void)
{
	if (cartOpen)
		running = true;
}

bool8 SNES9X_OpenCart (FSRef *inRef)
{
	OSStatus	err;
	FSRef		cartRef;
	char		filename[PATH_MAX + 1];

	DeinitGameWindow();

	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	ResetCheatFinder();

	if (!inRef)
	{
		if (!NavOpenROMImage(&cartRef))
		{
			cartOpen = false;
			return (false);
		}
	}
	else
		cartRef = *inRef;

	spcFileCount = pngFileCount = 0;

	CheckSaveFolder(&cartRef);

	Settings.ForceLoROM          = (romDetect        == kLoROMForce       );
	Settings.ForceHiROM          = (romDetect        == kHiROMForce       );
	Settings.ForceHeader         = (headerDetect     == kHeaderForce      );
	Settings.ForceNoHeader       = (headerDetect     == kNoHeaderForce    );
	Settings.ForceInterleaved    = (interleaveDetect == kInterleaveForce  );
	Settings.ForceInterleaved2   = (interleaveDetect == kInterleave2Force );
	Settings.ForceInterleaveGD24 = (interleaveDetect == kInterleaveGD24   );
	Settings.ForceNotInterleaved = (interleaveDetect == kNoInterleaveForce);
	Settings.ForcePAL            = (videoDetect      == kPALForce         );
	Settings.ForceNTSC           = (videoDetect      == kNTSCForce        );

	GFX.InfoString = NULL;
	GFX.InfoStringTimeout = 0;

	S9xResetSaveTimer(true);

	err = FSRefMakePath(&cartRef, (unsigned char *) filename, PATH_MAX);

	SNES9X_InitSound();

	if (Memory.LoadROM(filename))
	{
		cartOpen = true;

		SNES9X_LoadSRAM();

		ChangeTypeAndCreator(filename, 'CART', '~9X~');

		AddRecentItem(&cartRef);
		BuildRecentMenu();

		ApplyNSRTHeaderControllers();

		for (int a = 0; a < MAC_MAX_PLAYERS; a++)
			for (int b = 0; b < 12; b++)
				autofireRec[a].nextTime[b] = 0;

		return (true);
	}
	else
	{
		cartOpen = false;
		return (false);
	}
}

bool8 SNES9X_OpenMultiCart (void)
{
	Boolean	r;
	char	cart[2][PATH_MAX + 1];

	DeinitGameWindow();

	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	ResetCheatFinder();

	if (!MultiCartDialog())
	{
		cartOpen = false;
		return (false);
	}

	spcFileCount = pngFileCount = 0;

	Settings.ForceHeader   = (headerDetect     == kHeaderForce  );
	Settings.ForceNoHeader = (headerDetect     == kNoHeaderForce);
	Settings.ForcePAL      = (videoDetect      == kPALForce     );
	Settings.ForceNTSC     = (videoDetect      == kNTSCForce    );

	GFX.InfoString = NULL;
	GFX.InfoStringTimeout = 0;

	S9xResetSaveTimer(true);

	for (int i = 0; i < 2; i++)
	{
		cart[i][0] = 0;
		if (multiCartPath[i])
			r = CFStringGetCString(multiCartPath[i], cart[i], PATH_MAX, kCFStringEncodingUTF8);
	}

	SNES9X_InitSound();

	if (Memory.LoadMultiCart(cart[0], cart[1]))
	{
		cartOpen = true;

		SNES9X_LoadSRAM();

		for (int i = 0; i < 2; i++)
		{
			if (cart[i][0])
				ChangeTypeAndCreator(cart[i], 'CART', '~9X~');
		}

		ApplyNSRTHeaderControllers();

		for (int a = 0; a < MAC_MAX_PLAYERS; a++)
			for (int b = 0; b < 12; b++)
				autofireRec[a].nextTime[b] = 0;

		return (true);
	}
	else
	{
		cartOpen = false;
		return (false);
	}
}

void SNES9X_LoadSRAM (void)
{
	if (cartOpen)
		Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR));
}

void SNES9X_SaveSRAM (void)
{
	const char	*sramFilename;

	if (cartOpen)
	{
		sramFilename = S9xGetFilename(".srm", SRAM_DIR);
		Memory.SaveSRAM(sramFilename);
		ChangeTypeAndCreator(sramFilename, 'SRAM', '~9X~');
	}
}

void SNES9X_Reset (void)
{
	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xReset();
		SNES9X_LoadSRAM();
	}
}

void SNES9X_SoftReset (void)
{
	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xSoftReset();
		SNES9X_LoadSRAM();
	}
}

bool8 SNES9X_Freeze (void)
{
	OSStatus	err;
	FSRef		ref;
	int			which;
    const char	*filename;

	if (cartOpen)
	{
		MacStopSound();

		which = PromptFreezeDefrost(true);

		if (which >= 0)
		{
			filename = S9xGetFreezeFilename(which);

			err = FSPathMakeRef((unsigned char *) filename, &ref, NULL);
			if (!err)
				FSDeleteObject(&ref);

			S9xFreezeGame(filename);
			ChangeTypeAndCreator(filename, 'SAVE', '~9X~');
			err = FSPathMakeRef((unsigned char *) filename, &ref, NULL);
			WriteThumbnailToResourceFork(&ref, 128, 120);

			SNES9X_Go();

			MacStartSound();
			return (true);
		}
		else
		{
			MacStartSound();
			return (false);
		}
	}
	else
		return (false);
}

bool8 SNES9X_Defrost (void)
{
	int			which;
	const char	*filename;

	if (cartOpen)
	{
		MacStopSound();

		which = PromptFreezeDefrost(false);

		if (which >= 0)
		{
			filename = S9xGetFreezeFilename(which);
			S9xUnfreezeGame(filename);

			SNES9X_Go();

			MacStartSound();
			return (true);
		}
		else
		{
			MacStartSound();
			return (false);
		}
	}
	else
		return (false);
}

bool8 SNES9X_FreezeTo (void)
{
	OSStatus	err;
	FSRef		ref;
    char		filename[PATH_MAX + 1];

	if (cartOpen)
	{
		if (NavFreezeTo(filename))
		{
			err = FSPathMakeRef((unsigned char *) filename, &ref, NULL);
			if (!err)
				FSDeleteObject(&ref);

			S9xFreezeGame(filename);
			ChangeTypeAndCreator(filename, 'SAVE', '~9X~');
			err = FSPathMakeRef((unsigned char *) filename, &ref, NULL);
			WriteThumbnailToResourceFork(&ref, 128, 120);

			return (true);
		}
	}

	return (false);
}

bool8 SNES9X_DefrostFrom (void)
{
	char	filename[PATH_MAX + 1];

	if (cartOpen)
	{
		if (NavDefrostFrom(filename))
		{
			S9xUnfreezeGame(filename);

			SNES9X_Go();

			return (true);
		}
	}

	return (false);
}

bool8 SNES9X_RecordMovie (void)
{
	OSStatus	err;
	FSRef		ref;
    char		filename[PATH_MAX + 1];

	if (cartOpen)
	{
		if (NavRecordMovieTo(filename))
		{
			err = FSPathMakeRef((unsigned char *) filename, &ref, NULL);
			if (!err)
				FSDeleteObject(&ref);

			int		r;
			uint8   opt = 0, mask = 0;

			for (int i = 0; i <= 4; i++)
				if (macRecordFlag & (1 << i))
					mask |= (1 << i);

			if (macRecordFlag & (1 << 5))
				opt |= MOVIE_OPT_FROM_RESET;
			else
				opt |= MOVIE_OPT_FROM_SNAPSHOT;

			r = S9xMovieCreate(filename, mask, opt, macRecordWChar, wcslen(macRecordWChar));
			if (r == SUCCESS)
			{
				ChangeTypeAndCreator(filename, 'SMOV', '~9X~');

				if ((macRecordFlag & (1 << 5)) == 0)
				{
					err = FSPathMakeRef((unsigned char *) filename, &ref, NULL);
					WriteThumbnailToResourceFork(&ref, 128, 120);
				}

				SNES9X_Go();

				return (true);
			}
		}
	}

	return (false);
}

bool8 SNES9X_PlayMovie (void)
{
	char	filename[PATH_MAX + 1];

	if (cartOpen)
	{
		if (NavPlayMovieFrom(filename))
		{
			int r;

			r = S9xMovieOpen(filename, macPlayFlag & 1);
			if (r == SUCCESS)
			{
				SNES9X_Go();

				return (true);
			}
		}
	}

	return (false);
}

bool8 SNES9X_QTMovieRecord (void)
{
    char	filename[PATH_MAX + 1];

	if (cartOpen)
	{
		if (NavQTMovieRecordTo(filename))
		{
			MacQTStartRecording(filename);
			macQTRecord = true;
			SNES9X_Go();

			return (true);
		}
	}

	macQTRecord = false;

	return (false);
}

void SNES9X_Quit (void)
{
	DeinitGameWindow();

	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	finished = true;
}

void SNES9X_InitSound (void)
{
	S9xInitSound(0);
}
