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


#ifndef _mac_cart_h_
#define _mac_cart_h_

bool8 NavOpenROMImage (FSRef *);
bool8 NavBeginOpenROMImageSheet (WindowRef, CFStringRef);
bool8 NavEndOpenROMImageSheet (FSRef *);
bool8 NavBeginChooseFolderSheet (WindowRef);
bool8 NavEndChooseFolderSheet (FSRef *);
bool8 NavFreezeTo (char *);
bool8 NavDefrostFrom (char *);
bool8 NavRecordMovieTo (char *);
bool8 NavPlayMovieFrom (char *);
bool8 NavQTMovieRecordTo (char *);

#endif
