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


#include "port.h"
#include "cheats.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "mac-prefix.h"
#include "mac-dialog.h"
#include "mac-os.h"
#include "mac-stringtools.h"
#include "mac-cheat.h"

#define	kDataBrowser	'BRSR'
#define	kCmCheckBox		'CHK_'
#define	kCmAddress		'ADDR'
#define	kCmValue		'VALU'
#define	kCmDescription	'DESC'
#define	kNewButton		'NEW_'
#define	kDelButton		'DEL_'
#define	kAllButton		'ALL_'

extern SCheatData	Cheat;

bool S9xGameGenieToRaw(const std::string &code, uint32 &address, uint8 &byte);
bool S9xProActionReplayToRaw(const std::string &code, uint32 &address, uint8 &byte);

@implementation S9xCheatItem

- (void)setAddress:(uint32)address value:(uint8)value cheatDescription:(const char *)cheatDescription
{
	bool enabled = self.enabled.boolValue;

	char code[256];
	sprintf(code, "%x=%x", address, value);
	S9xModifyCheatGroup(self.cheatID, cheatDescription, code);

	if (enabled)
	{
		S9xEnableCheatGroup(self.cheatID);
	}
}

@dynamic address;
- (NSNumber *)address
{
	return @(Cheat.group[self.cheatID].cheat[0].address);
}

- (void)setAddress:(NSNumber *)address
{
	[self setAddress:address.unsignedIntValue value:Cheat.group[self.cheatID].cheat[0].byte cheatDescription:self.cheatDescription.UTF8String];
}

@dynamic value;
- (NSNumber *)value
{
	return @(Cheat.group[self.cheatID].cheat[0].byte);
}

- (void)setValue:(NSNumber *)value
{
	[self setAddress:Cheat.group[self.cheatID].cheat[0].address value:value.unsignedCharValue cheatDescription:self.cheatDescription.UTF8String];
}

@dynamic enabled;
- (NSNumber *)enabled
{
	return @(Cheat.group[self.cheatID].enabled);
}

- (void)setEnabled:(NSNumber *)enabled
{
	if (enabled.boolValue)
	{
		S9xEnableCheatGroup(self.cheatID);
	}
	else
	{
		S9xDisableCheatGroup(self.cheatID);
	}
}

@dynamic cheatDescription;
- (NSString *)cheatDescription
{
	return [NSString stringWithUTF8String:Cheat.group[self.cheatID].name.c_str()];
}

- (void)setCheatDescription:(NSString *)cheatDescription
{
	[self setAddress:Cheat.group[self.cheatID].cheat[0].address value:Cheat.group[self.cheatID].cheat[0].byte cheatDescription:cheatDescription.UTF8String];
}

- (void)delete
{
	S9xDeleteCheatGroup(self.cheatID);
}

@end

extern "C"
{

bool CheatValuesFromCode(NSString *code, uint32 *address, uint8 *value)
{
	const char *text = code.UTF8String;

	if (!S9xGameGenieToRaw (text, *address, *value))
	{
		return true;
	}

	if (!S9xProActionReplayToRaw (text, *address, *value))
	{
		return true;
	}

	else if (sscanf (text, "%x = %x", (unsigned int *)address, (unsigned int *)value) == 2)
	{
		return true;
	}

	else if (sscanf (text, "%x / %x", (unsigned int *)address, (unsigned int *)value) == 2)
	{
		return true;
	}

	return false;
}

void CreateCheatFromAddress(NSNumber *address, NSNumber *value, NSString *cheatDescription)
{
	char code[256];
	sprintf(code, "%x=%x", address.unsignedIntValue, value.unsignedCharValue);
	S9xAddCheatGroup(cheatDescription.UTF8String, code);
	S9xEnableCheatGroup(Cheat.group.size() - 1);
}

void CreateCheatFromCode(NSString *code, NSString *cheatDescription)
{
	S9xAddCheatGroup(cheatDescription.UTF8String, code.UTF8String);
	S9xEnableCheatGroup(Cheat.group.size() - 1);
}

NSArray<S9xCheatItem *> *GetAllCheats(void)
{
	NSMutableArray *cheats = [NSMutableArray new];
	uint32 cheatID = 0;

	for (auto it : Cheat.group)
	{
		S9xCheatItem *cheat    = [S9xCheatItem new];
		cheat.cheatID		   = cheatID++;

		[cheats addObject:cheat];
	}

	return cheats;
}
}
