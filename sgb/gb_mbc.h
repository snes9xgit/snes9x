/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SGB_GB_MBC_H_
#define _SGB_GB_MBC_H_

#include <cstdint>
#include <vector>

namespace SGB {

enum class MbcType : uint8_t
{
	None       = 0,
	MBC1       = 1,
	MBC2       = 2,
	MBC3       = 3,
	MBC5       = 5,
	MBC6       = 6,  // not yet implemented
	MBC7       = 7,  // not yet implemented
	HuC1       = 8,  // MBC1-like, with infrared (RAM<->IR via $0000-$1FFF)
	HuC3       = 9,  // MBC-like, with RTC command interface + IR
	MMM01      = 10, // Nintendo multicart meta-mapper (Mani / Taito 4-in-1)
	SachenMMC1 = 11, // Sachen 4B-series unlicensed multicarts
	TAMA5      = 12, // Bandai Tamagotchi — register port + EEPROM + RTC
	M161       = 13  // write-once 32 KiB-page multicart (Mani 4-in-1 "Tetris Set")
};

struct MbcState
{
	MbcType  type       = MbcType::None;
	uint32_t rom_bank   = 1;
	uint32_t ram_bank   = 0;
	bool     ram_enable = false;
	bool     mbc1_mode  = false;    // 0 = ROM banking, 1 = RAM banking

	// MBC3 RTC — 5 8-bit counters + latch gate.
	uint8_t  rtc_regs[5]    = {0};
	uint8_t  rtc_latched[5] = {0};
	bool     rtc_latch      = false;
	uint8_t  rtc_select     = 0;

	// Sachen MMC1: outer-bank/mask + lock with simple unlock counter.
	// While locked, ROM reads in 0x0100-0x01FF go through an A0/A6 and
	// A1/A4 bit-permutation so the bootstrap sees the real Nintendo logo
	// at 0x0104-0x0133. The cart unlocks once the game has issued ~0x30
	// writes to addresses >= 0x8000 (VRAM/WRAM/SRAM/HRAM/IO).
	uint8_t  sachen_outer_bank  = 0;
	uint8_t  sachen_outer_mask  = 0;
	bool     sachen_locked      = true;
	uint8_t  sachen_unlock_ctr  = 0;

	// MMM01: Nintendo multicart meta-mapper. After power-on the last
	// 32 KiB of ROM (the menu) is mapped at $0000-$7FFF; the menu
	// configures rom/ram base+mask registers, then writes bit 6 of
	// $0000-$1FFF to lock into "game mode" where the selected sub-game
	// runs under MBC1-style banking inside its allotted ROM window.
	// Fields mirror SameBoy's mmm01 struct for direct spec parity.
	bool     mmm01_locked            = false;
	bool     mmm01_mbc1_mode         = false;  // bit 0 of $6000-$7FFF
	bool     mmm01_mbc1_mode_disable = false;  // bit 6 of $4000-$5FFF
	bool     mmm01_multiplex_mode    = false;  // bit 6 of $6000-$7FFF
	uint8_t  mmm01_rom_bank_low      = 0;      // MBC1-style low ROM bank
	uint8_t  mmm01_rom_bank_mid      = 0;      // upper 2 bits of $2000-$3FFF
	uint8_t  mmm01_rom_bank_high     = 0;      // upper outer ROM bank bits
	uint8_t  mmm01_ram_bank_low      = 0;
	uint8_t  mmm01_ram_bank_high     = 0;
	uint8_t  mmm01_rom_bank_mask     = 0;      // bits [5:2] of $6000-$7FFF
	uint8_t  mmm01_ram_bank_mask     = 0xFF;   // bits [5:4] of $0000-$1FFF

	// One-shot event flag: rises when the menu writes the lock bit. The
	// SGB layer polls and clears it, then injects PAL01/PAL23 packets
	// that force all 4 SGB system palettes to grayscale — without this
	// the sub-game inherits whatever palette the SGB BIOS assigned to
	// the menu's title during its one-shot handshake, which usually
	// looks badly inverted.
	bool     mmm01_just_locked       = false;
};

struct Cart;

void MbcReset(MbcState &s);
uint8_t MbcRead(MbcState &s, const std::vector<uint8_t> &rom, const std::vector<uint8_t> &sram, uint16_t addr);
void    MbcWrite(Cart &c, uint16_t addr, uint8_t value);

// Notify the mapper of a CPU write to addr >= 0x8000 (VRAM/WRAM/SRAM/HRAM/IO).
// Used by Sachen MMC1 to advance its unlock counter — writes there are
// otherwise invisible to the MBC. Value-sensitive (Sachen counts $31
// writes per Tauwasser's RE) so the byte must be passed.
void    MbcNotifyHighWrite(MbcState &s, uint16_t addr, uint8_t value);

} // namespace SGB

#endif
