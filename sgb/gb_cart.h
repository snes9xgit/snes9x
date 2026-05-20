/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SGB_GB_CART_H_
#define _SGB_GB_CART_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "gb_mbc.h"

namespace SGB {

// Cart header lives at 0x0100-0x014F on every GB ROM.
struct CartHeader
{
	char     title[17]   = {0};
	uint8_t  cgb_flag    = 0;
	uint8_t  sgb_flag    = 0;
	uint8_t  cart_type   = 0;
	uint32_t rom_size    = 0;  // bytes
	uint32_t ram_size    = 0;
	uint8_t  dest_code   = 0;
	uint8_t  header_sum  = 0;
	uint16_t global_sum  = 0;
	bool     checksum_ok = false;
};

struct Cart
{
	CartHeader            header;
	std::vector<uint8_t>  rom;
	std::vector<uint8_t>  sram;
	MbcState              mbc;
	std::string           path;
	bool                  has_battery = false;
	bool                  has_rtc     = false;
	bool                  has_rumble  = false;
	bool                  sram_dirty  = false;
};

bool CartLoad(Cart &c, const uint8_t *data, size_t size, const char *path);
void CartUnload(Cart &c);

// Persist battery-backed SRAM (and RTC if present) to ".sav" alongside the ROM.
bool CartSaveBattery(const Cart &c);
bool CartLoadBattery(Cart &c);

bool CartSaveBatteryToPath(const Cart &c, const char *path);
bool CartLoadBatteryFromPath(Cart &c, const char *path);

} // namespace SGB

#endif
