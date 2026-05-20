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
	None   = 0,
	MBC1   = 1,
	MBC2   = 2,
	MBC3   = 3,
	MBC5   = 5,
	MBC6   = 6,  // not yet implemented
	MBC7   = 7,  // not yet implemented
	HuC1   = 8,  // not yet implemented
	HuC3   = 9,  // not yet implemented
	MMM01  = 10  // not yet implemented
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
};

struct Cart;

void MbcReset(MbcState &s);
uint8_t MbcRead(MbcState &s, const std::vector<uint8_t> &rom, const std::vector<uint8_t> &sram, uint16_t addr);
void    MbcWrite(Cart &c, uint16_t addr, uint8_t value);

} // namespace SGB

#endif
