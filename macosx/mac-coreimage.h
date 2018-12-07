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


#ifndef _mac_coreimage_h_
#define _mac_coreimage_h_

void InitCoreImage (void);
void DeinitCoreImage (void);
void InitCoreImageFilter (void);
void DeinitCoreImageFilter (void);
void ConfigureCoreImageFilter (void);
void InitCoreImageContext (CGLContextObj, CGLPixelFormatObj);
void DeinitCoreImageContext (void);
void DrawWithCoreImageFilter (CGRect, CGImageRef);

#endif
