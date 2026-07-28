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
	// MBC1 multicart wiring (MBC1M): BANK2 selects a 256 KiB game slot. Static
	// per-ROM, re-derived at load — kept out of MbcState so it never touches
	// the blob-serialized savestate layout.
	bool                  mbc1_multicart = false;
	// Duz "2-in-1" MBC3 multicart: $C0 to $0000 unlocks an $A000/$A100 register port whose reg $A3<<1 is the ROM base bank added to every access.
	bool                  duz_multicart  = false;
	// "23 in 1" MBC5 multicart: $5001/$5002 latch a base bank and a bank mask,
	// both in 2-bank units (register value << 1).
	bool                  mbc5_multicart = false;
	bool                  sachen_runs_raw = false;
	// Sachen carts that keep the boot logo in the RA7-high half of the
	// permuted $0100-$01FF window (the combo/31-in-1 series).
	bool                  sachen_logo_high = false;
	uint8_t               camera_regs[0x36] = {0};
};

bool CartLoad(Cart &c, const uint8_t *data, size_t size, const char *path);
void CartUnload(Cart &c);

// Persist battery-backed SRAM (and RTC if present) to ".sav" alongside the ROM.
bool CartSaveBattery(const Cart &c);
bool CartLoadBattery(Cart &c);

bool CartSaveBatteryToPath(const Cart &c, const char *path);
bool CartLoadBatteryFromPath(Cart &c, const char *path);

// While the boot ROM is mapped, a Sachen cart's mapper feeds the boot the
// Nintendo logo at 0x0104-0x0133 (logo-less single-game carts store none, and
// even the 4B multicarts only descramble to it) so the DMG/SGB boot logo check
// passes — exactly as the hardware does. Returns true and the canonical byte
// for a SachenMMC1 cart in that range; callers gate this on boot_rom_enabled so
// the running game still reads its real (address-swapped) header afterwards.
bool SachenBootLogoByte(const Cart &c, uint16_t addr, uint8_t &out);

} // namespace SGB

void S9xGBSetCameraCallback(bool (*cb)(unsigned char *dst, int width, int height));

#endif
