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

  (c) Copyright 2009 - 2017  BearOso,
                             OV2

  (c) Copyright 2017         qwertymodo

  (c) Copyright 2011 - 2017  Hans-Kristian Arntzen,
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
  (c) Copyright 2004 - 2017  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2017  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones

  Libretro port
  (c) Copyright 2011 - 2017  Hans-Kristian Arntzen,
                             Daniel De Matteis
                             (Under no circumstances will commercial rights be given)


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
#include "memmap.h"
#include "cheats.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

uint8 S9xGetByteFree (uint32);
void S9xSetByteFree (uint8, uint32);


uint8 S9xGetByteFree (uint32 address)
{
	int32	Cycles = CPU.Cycles;
    int32   NextEvent = CPU.NextEvent;
	uint8	byte;

    CPU.NextEvent = 0x7FFFFFFF;
	byte = S9xGetByteQuiet(address);
    CPU.NextEvent = NextEvent;
	CPU.Cycles = Cycles;

	return (byte);
}

void S9xSetByteFree (uint8 byte, uint32 address)
{
	int32	Cycles = CPU.Cycles;
    int32  NextEvent = CPU.NextEvent;

    CPU.NextEvent = 0x7FFFFFFF;
	S9xSetByteQuiet(byte, address);
    CPU.NextEvent = NextEvent;
	CPU.Cycles = Cycles;
}

void S9xInitWatchedAddress (void)
{
	for (unsigned int i = 0; i < sizeof(watches) / sizeof(watches[0]); i++)
		watches[i].on = false;

}

void S9xInitCheatData (void)
{
	Cheat.RAM = Memory.RAM;
	Cheat.SRAM = Memory.SRAM;
	Cheat.FillRAM = Memory.FillRAM;
}

bool S9xIsValidCheatCode(const char *codestring)
{
	std::string codes(codestring);

	if (codes.empty() || codes[codes.length() - 1] == '+')
		return false; // std::getline cannot handle this case

	std::istringstream iss(codes);
	std::string code;
	while(std::getline(iss, code, '+'))
	{
		uint32 address;
		uint8 byte;
		if (S9xProActionReplayToRaw(code.c_str(), address, byte) != NULL &&
			S9xGameGenieToRaw(code.c_str(), address, byte) != NULL)
		{
			uint8 bytes[3];
			uint8 num_bytes;

			if (S9xGoldFingerToRaw(code.c_str(), address, num_bytes, bytes) != NULL)
				return false; // invalid format
		}
	}
	return true;
}

static inline bool S9xCompileCheat(struct SCheatItem& cheat, bool8 save_current_value)
{
	std::string codes(cheat.code);
	std::vector<SCheat> cheatVector;

	if (codes.empty() || codes[codes.length() - 1] == '+')
		return false; // std::getline cannot handle this case

	std::istringstream iss(codes);
	std::string code;
	while(std::getline(iss, code, '+'))
	{
		SCheat c;
		if (S9xProActionReplayToRaw(code.c_str(), c.address, c.byte) == NULL ||
			S9xGameGenieToRaw(code.c_str(), c.address, c.byte) == NULL)
		{
			c.saved = save_current_value;
			if (c.saved) {
				c.saved_byte = S9xGetByteFree(c.address);
			}
			cheatVector.push_back(c);
		}
		else
		{
			uint32 address;
			uint8 bytes[3];
			uint8 num_bytes;

			if (S9xGoldFingerToRaw(code.c_str(), address, num_bytes, bytes) == NULL)
			{
				for (int i = 0; i < num_bytes; i++)
				{
					c.address = address + i;
					c.byte = bytes[i];
					c.saved = save_current_value;
					if (c.saved) {
						c.saved_byte = S9xGetByteFree(c.address);
					}
					cheatVector.push_back(c);
				}
			}
			else
				return false; // invalid format
		}
	}
	cheat.c.clear();
	copy(cheatVector.begin(), cheatVector.end(), std::back_inserter(cheat.c));
	return true;
}

void S9xAddCheat (bool8 enable, bool8 save_current_value, const char *code, const char *name)
{
	struct SCheatItem cheat;
	cheat.code = code;
	cheat.name = name;
	cheat.enabled = enable;
	if (S9xCompileCheat(cheat, save_current_value))
		Cheat.c.push_back(cheat);
}

void S9xAddCheat (bool8 enable, bool8 save_current_value, uint32 address, uint8 byte, const char *name)
{
	if (address >= 0x000000 && address <= 0xFFFFFF) {
		char code[10];
		sprintf(code, "%06X:%02X", address, byte);
		S9xAddCheat(enable, save_current_value, code, name);
	}
}

void S9xDeleteCheat (uint32 which1)
{
	std::vector<SCheatItem>::iterator iter = Cheat.c.begin();
	std::advance(iter, which1);
	if (which1 < Cheat.c.size())
	{
		if (Cheat.c[which1].enabled)
			S9xRemoveCheat(which1);
		Cheat.c.erase(iter);
	}
}

void S9xDeleteCheats (void)
{
	S9xRemoveCheats();
	Cheat.c.clear();
}

void S9xRemoveCheat (uint32 which1)
{
	std::vector<SCheat>::iterator iter = Cheat.c[which1].c.begin();
	while (iter != Cheat.c[which1].c.end())
	{
		SCheat& c = *iter;
		if (c.saved)
		{
			uint32	address = c.address;

			int		block = (address & 0xffffff) >> MEMMAP_SHIFT;
			uint8	*ptr = Memory.Map[block];

			if (ptr >= (uint8 *) CMemory::MAP_LAST)
				*(ptr + (address & 0xffff)) = c.saved_byte;
			else
				S9xSetByteFree(c.saved_byte, address);
		}
		iter++;
	}
}

void S9xRemoveCheats (void)
{
	for (uint32 i = 0; i < Cheat.c.size(); i++)
		if (Cheat.c[i].enabled)
			S9xRemoveCheat(i);
}

void S9xEnableCheat (uint32 which1)
{
	if (which1 < Cheat.c.size() && !Cheat.c[which1].enabled)
	{
		Cheat.c[which1].enabled = TRUE;
		S9xApplyCheat(which1);
	}
}

void S9xDisableCheat (uint32 which1)
{
	if (which1 < Cheat.c.size() && Cheat.c[which1].enabled)
	{
		S9xRemoveCheat(which1);
		Cheat.c[which1].enabled = FALSE;
	}
}

void S9xApplyCheat (uint32 which1, bool8 force_save_current_value)
{
	std::vector<SCheat>::iterator iter = Cheat.c[which1].c.begin();
	while (iter != Cheat.c[which1].c.end())
	{
		SCheat& c = *iter;
		uint32	address = c.address;

		if (force_save_current_value)
		{
			c.saved_byte = S9xGetByteFree(address);
			c.saved = TRUE;
		}

		int		block = (address & 0xffffff) >> MEMMAP_SHIFT;
		uint8	*ptr = Memory.Map[block];

		if (ptr >= (uint8 *) CMemory::MAP_LAST)
			*(ptr + (address & 0xffff)) = c.byte;
		else
			S9xSetByteFree(c.byte, address);

		iter++;
	}
}

void S9xApplyCheats (bool8 force_save_current_value)
{
	if (Settings.ApplyCheats)
	{
		for (uint32 i = 0; i < Cheat.c.size(); i++)
			if (Cheat.c[i].enabled)
				S9xApplyCheat(i, force_save_current_value);
	}
}

std::string& string_replace (std::string& str, const std::string& from, const std::string& to)
{
	std::string::size_type pos = 0;
	while (pos = str.find(from, pos), pos != std::string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
	return str;
}

std::string csv_escape(const std::string &token, bool quoteAlways = false)
{
	std::string res(token);
	bool needsQuote = false;
	if (res.find("\"", 0) != std::string::npos) {
		needsQuote = true;
		string_replace(res, "\"", "\"\"");
	}
	if (res.find(",", 0) != std::string::npos ||
		res.find("\n", 0) != std::string::npos) {
		needsQuote = true;
	}
	if (needsQuote || quoteAlways) {
		res.insert(0, "\"");
		res.append("\"");
	}
	return res;
}


std::string csv_unescape(const std::string &token)
{
	std::string res(token);
	if (res.length() >= 2 && res[0] == '\"' && res[res.length() - 1] == '\"') {
		res.erase(res.length() - 1, 1);
		res.erase(0, 1);
	}
	string_replace(res, "\"\"", "\"");
	return res;
}

std::vector<std::string> csv_split(const std::string &str, bool *valid = NULL)
{
	std::vector<std::string> res;

	if (valid != NULL) {
		*valid = true;
	}

	std::string::size_type start = 0;
	std::string::size_type current = 0;
	std::string::size_type found;
	bool insideQuote = false;
	while ((found = str.find_first_of(",\"", current)) != std::string::npos) {
		char c = str[found];
		if (c == '\"') {
			if (insideQuote) {
				if (str[found + 1] == '\"') {
					found++; // skip the next quote
				}
				else {
					if (valid != NULL && str[found + 1] != ',' && str[found + 1] != '\0' && str[found + 1] != '\r' && str[found + 1] != '\n') {
						*valid = false;
					}
					insideQuote = false;
				}
			}
			else {
				if (valid != NULL && found > 0 && str[found - 1] != ',' && str[found - 1] != '\r' && str[found - 1] != '\n') {
					*valid = false;
				}
				insideQuote = true;
			}
		}
		else if (c == ',') {
			if (!insideQuote) {
				res.push_back(std::string(str, start, found - start));
				start = found + 1;
			}
		}
		current = found + 1;
	}
	res.push_back(std::string(str, start, str.size() - start));

	if (valid != NULL && insideQuote) {
		*valid = false;
	}

	for (int i = 0; i < res.size(); i++) {
		res[i] = csv_unescape(res[i]);
	}
	return res;
}

std::string csv_join(std::vector<std::string> &v, bool quoteAlways = false)
{
	std::string res;

	std::vector<std::string>::iterator iter = v.begin();
	bool first = true;
	while(iter != v.end())
	{
		if (first) {
			first = false;
		}
		else {
			res.append(",");
		}

		res.append(csv_escape(*iter, quoteAlways));
		iter++;
	}
	return res;
}

bool8 IsOldCheatFile (const char *filename)
{
	FILE	*fs;

	fs = fopen(filename, "rb");
	if (!fs)
		return (FALSE);

	int c = fgetc(fs);
	fclose(fs);

	return ((c & ~(4 | 8)) == 0);
}

void trimLastNewline(char *str)
{
	size_t len = strlen(str);
	while (len > 0)
	{
		const char c = str[len - 1];
		if (c != '\n' && c != '\r')
		{
			break;
		}
		len--;
		str[len] = '\0';
	}
}

bool8 S9xLoadCheatFile (const char *filename)
{
	FILE	*fs;
	char	data[1024];

	if (IsOldCheatFile(filename))
		return S9xLoadCheatFileOld(filename);

	Cheat.c.clear();

	fs = fopen(filename, "r");
	if (!fs)
		return (FALSE);

	while (fgets(data, 1024, fs) != NULL)
	{
		trimLastNewline(data);

		std::vector<std::string> v = csv_split(std::string(data));
		if (v.size() != 3) {
			fclose(fs);
			return (FALSE);
		}

		struct SCheatItem c;
		if (v[0] == "enabled") {
			c.enabled = TRUE;
		}
		else if (v[0] == "disabled") {
			c.enabled = FALSE;
		}
		else {
			fclose(fs);
			return (FALSE);
		}
		c.code = v[1];
		c.name = v[2];

		if (!S9xCompileCheat(c, FALSE))
		{
			fclose(fs);
			return (FALSE);
		}
		Cheat.c.push_back(c);
	}

	fclose(fs);

	return (TRUE);
}

bool8 S9xLoadCheatFileOld (const char *filename)
{
	FILE	*fs;
	uint8	data[28];

	Cheat.c.clear();

	fs = fopen(filename, "rb");
	if (!fs)
		return (FALSE);

	while (fread((void *) data, 1, 28, fs) == 28)
	{
		struct SCheatItem c;
		uint32 address;
		uint8 byte;
		uint8 saved_byte;
		bool8 saved;
		c.enabled = (data[0] & 4) == 0;
		byte = data[1];
		address = data[2] | (data[3] << 8) |  (data[4] << 16);
		saved_byte = data[5];
		saved = (data[0] & 8) != 0;

		char code[10];
		char name[22];
		sprintf(code, "%06X%02X", address, byte);
		memmove(name, &data[8], 20);
		name[20] = '\0';
		c.code = code;
		c.name = name;

		if (!S9xCompileCheat(c, FALSE))
		{
			fclose(fs);
			return (FALSE);
		}

		std::vector<SCheat>::iterator iter = c.c.begin();
		while (iter != c.c.end())
		{
			iter->saved_byte = saved_byte;
			iter->saved = saved;
			iter++;
		}

		Cheat.c.push_back(c);
	}

	fclose(fs);

	S9xSaveCheatFile(filename);
	return (TRUE);
}

bool8 S9xSaveCheatFile (const char *filename)
{
	if (Cheat.c.size() == 0)
	{
		remove(filename);
		return (TRUE);
	}

	FILE	*fs;

	fs = fopen(filename, "w");
	if (!fs)
		return (FALSE);

	for (uint32 i = 0; i < Cheat.c.size(); i++)
	{
		struct SCheatItem& c = Cheat.c[i];
		std::vector<std::string> v;
		v.push_back(c.enabled ? "enabled" : "disabled");
		v.push_back(c.code);
		v.push_back(c.name);
		if (fprintf(fs, "%s\n", csv_join(v, true).c_str()) < 0)
		{
			fclose(fs);
			return (FALSE);
		}
	}

	return (fclose(fs) == 0);
}
