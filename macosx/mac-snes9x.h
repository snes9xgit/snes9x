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


#ifndef _mac_snes9x_h_
#define _mac_snes9x_h_

enum
{
	kAutoROMType       = 0,
	kLoROMForce        = 2,
	kHiROMForce        = 3
};

enum
{
	kAutoInterleave    = 0,
	kNoInterleaveForce = 2,
	kInterleaveForce   = 3,
	kInterleave2Force  = 4,
	kInterleaveGD24    = 5
};

enum
{
	kAutoVideo         = 0,
	kPALForce          = 2,
	kNTSCForce         = 3
};

enum
{
	kAutoHeader        = 0,
	kNoHeaderForce     = 2,
	kHeaderForce       = 3
};

void SNES9X_Go (void);
void SNES9X_LoadSRAM (void);
void SNES9X_SaveSRAM (void);
void SNES9X_Reset (void);
void SNES9X_SoftReset (void);
void SNES9X_Quit (void);
void SNES9X_InitSound (void);
bool8 SNES9X_OpenCart (FSRef *);
bool8 SNES9X_OpenMultiCart (void);
bool8 SNES9X_Freeze (void);
bool8 SNES9X_FreezeTo (void);
bool8 SNES9X_Defrost (void);
bool8 SNES9X_DefrostFrom (void);
bool8 SNES9X_RecordMovie (void);
bool8 SNES9X_PlayMovie (void);
bool8 SNES9X_QTMovieRecord (void);

#endif
