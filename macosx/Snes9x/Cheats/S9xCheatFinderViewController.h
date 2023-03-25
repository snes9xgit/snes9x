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
  (c) Copyright 2019 - 2023  Michael Donald Buckley
 ***********************************************************************************/

#import <Cocoa/Cocoa.h>

#import <snes9x_framework/snes9x_framework.h>

NS_ASSUME_NONNULL_BEGIN

@protocol S9xCheatFinderDelegate <NSObject>
- (void)addedCheat;
@end

@interface S9xCheatFinderViewController : NSViewController

@property (nonatomic, strong) S9xEngine *engine;
@property (nonatomic, weak) id<S9xCheatFinderDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
