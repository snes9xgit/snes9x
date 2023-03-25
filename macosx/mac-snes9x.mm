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
  (c) Copyright 2019         Michael Donald Buckley
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
#include "mac-dialog.h"
#include "mac-file.h"
#include "mac-os.h"
#include "mac-screenshot.h"
#include "mac-stringtools.h"
#include "mac-snes9x.h"

extern wchar_t  macRecordWChar[MOVIE_MAX_METADATA];


void SNES9X_Go (void)
{
	if (cartOpen)
		running = true;
}

bool8 SNES9X_OpenCart (NSURL *inRef)
{
	NSURL		*cartRef;
	const char	*filename = inRef.path.UTF8String;

	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	if (!inRef)
	{
        cartRef = NavOpenROMImage();
		if (!cartRef)
		{
			cartOpen = false;
			return (false);
		}
	}
	else
		cartRef = inRef;

	spcFileCount = pngFileCount = 0;

	CheckSaveFolder(cartRef);

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

	GFX.InfoString = "";
	GFX.InfoStringTimeout = 0;

	S9xResetSaveTimer(true);

	SNES9X_InitSound();

	if (Memory.LoadROM(filename))
	{
		cartOpen = true;

		SNES9X_LoadSRAM();

		ChangeTypeAndCreator(filename, 'CART', '~9X~');

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

bool8 SNES9X_OpenMultiCart (NSURL *cart1FileURL, NSURL *cart2FileURL)
{
	Boolean	r;

	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}

	spcFileCount = pngFileCount = 0;

	Settings.ForceHeader   = (headerDetect     == kHeaderForce  );
	Settings.ForceNoHeader = (headerDetect     == kNoHeaderForce);
	Settings.ForcePAL      = (videoDetect      == kPALForce     );
	Settings.ForceNTSC     = (videoDetect      == kNTSCForce    );

	GFX.InfoString = "";
	GFX.InfoStringTimeout = 0;

	S9xResetSaveTimer(true);

	SNES9X_InitSound();

	if (Memory.LoadMultiCart(cart1FileURL.path.UTF8String, cart2FileURL.path.UTF8String))
	{
		cartOpen = true;

		SNES9X_LoadSRAM();

		ChangeTypeAndCreator(cart1FileURL.path.UTF8String, 'CART', '~9X~');

		if (cart2FileURL != nil)
		{
			ChangeTypeAndCreator(cart2FileURL.path.UTF8String, 'CART', '~9X~');
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
		Memory.LoadSRAM(S9xGetFilename(".srm", SRAM_DIR).c_str());
}

void SNES9X_SaveSRAM (void)
{
	std::string	sramFilename;

	if (cartOpen)
	{
		sramFilename = S9xGetFilename(".srm", SRAM_DIR);
		Memory.SaveSRAM(sramFilename.c_str());
		ChangeTypeAndCreator(sramFilename.c_str(), 'SRAM', '~9X~');
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
	int			which;
    const char	*filename;

	if (cartOpen)
	{
		MacStopSound();

		which = PromptFreezeDefrost(true);

		if (which >= 0)
		{
			filename = S9xGetFreezeFilename(which);

            unlink(filename);

			S9xFreezeGame(filename);
			ChangeTypeAndCreator(filename, 'SAVE', '~9X~');
            WriteThumbnailToExtendedAttribute(filename, 128, 120);

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
	if (cartOpen)
	{
		NSURL *url = NavFreezeTo();
		if (url != nil)
		{
			const char *filename = url.path.UTF8String;
            unlink(filename);

			S9xFreezeGame(filename);
			ChangeTypeAndCreator(filename, 'SAVE', '~9X~');;
            WriteThumbnailToExtendedAttribute(filename, 128, 120);

			return (true);
		}
	}

	return (false);
}

bool8 SNES9X_DefrostFrom (void)
{
	if (cartOpen)
	{
		NSURL *url = NavDefrostFrom();
		if (url != nil)
		{
			const char *filename = url.path.UTF8String;
			S9xUnfreezeGame(filename);

			SNES9X_Go();

			return (true);
		}
	}

	return (false);
}

bool8 SNES9X_RecordMovie (void)
{
	if (cartOpen)
	{
		NSURL *url = NavRecordMovieTo();
		if (url != nil)
		{
			const char *filename = url.path.UTF8String;
            unlink(filename);

			int		r;
			uint8   opt = 0, mask = 0;

			for (int i = 0; i <= 4; i++)
				if (macRecordFlag & (1 << i))
					mask |= (1 << i);

			if (macRecordFlag & (1 << 5))
				opt |= MOVIE_OPT_FROM_RESET;
			else
				opt |= MOVIE_OPT_FROM_SNAPSHOT;

			r = S9xMovieCreate(filename, mask, opt, macRecordWChar, (int)wcslen(macRecordWChar));
			if (r == SUCCESS)
			{
				ChangeTypeAndCreator(filename, 'SMOV', '~9X~');

				if ((macRecordFlag & (1 << 5)) == 0)
				{
                    WriteThumbnailToExtendedAttribute(filename, 128, 120);
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
	if (cartOpen)
	{
		NSURL *url = NavPlayMovieFrom();
		if (url != nil)
		{
			const char *filename = url.path.UTF8String;
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
	if (cartOpen)
	{
		NSURL *url = NavQTMovieRecordTo();
		if (url != nil)
		{
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
	if (cartOpen)
	{
		SNES9X_SaveSRAM();
		S9xResetSaveTimer(false);
		S9xSaveCheatFile(S9xGetFilename(".cht", CHEAT_DIR));
	}
}

void SNES9X_InitSound (void)
{
	S9xInitSound(0);
}
