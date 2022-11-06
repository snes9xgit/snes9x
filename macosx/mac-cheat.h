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
  (c) Copyright 2019 - 2022  Michael Donald Buckley
 ***********************************************************************************/


#ifndef _mac_cheat_h_
#define _mac_cheat_h_

#import <Foundation/Foundation.h>

@interface S9xCheatItem : NSView
@property (nonatomic, assign) uint32	cheatID;
@property (nonatomic, strong) NSNumber	*address;
@property (nonatomic, strong) NSNumber	*value;
@property (nonatomic, strong) NSNumber	*enabled;
@property (nonatomic, strong) NSString	*cheatDescription;

- (void)delete;
@end

#ifdef __cplusplus
extern "C" {
#endif
bool CheatValuesFromCode(NSString *code, uint32 *address, uint8 *value);
void CreateCheatFromAddress(NSNumber *address, NSNumber *value, NSString *cheatDescription);
void CreateCheatFromCode(NSString *code, NSString *cheatDescription);
NSArray<S9xCheatItem *> *GetAllCheats(void);
#ifdef __cplusplus
}
#endif

#endif
