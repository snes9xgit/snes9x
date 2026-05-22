/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Memory Bank Controllers. MBC1, MBC3, MBC5 cover ~95% of commercial
// GB carts. MBC2/MBC6/MBC7/HuC1/HuC3/MMM01 are stubbed (treated as
// read-only no-MBC) and will be filled in as needed.
//
// Register meanings per Pan Docs:
//
//   MBC1:
//     0x0000-0x1FFF  RAM enable (write 0x0A low nibble to enable)
//     0x2000-0x3FFF  ROM bank lower 5 bits (0 auto-corrects to 1)
//     0x4000-0x5FFF  RAM bank / ROM bank upper 2 bits (mode-dependent)
//     0x6000-0x7FFF  Mode select — 0=ROM banking, 1=RAM banking
//
//   MBC3:
//     0x0000-0x1FFF  RAM/RTC enable
//     0x2000-0x3FFF  ROM bank (7 bits, 0 auto-corrects to 1)
//     0x4000-0x5FFF  RAM bank (0..7) or RTC register select (0x08..0x0C)
//     0x6000-0x7FFF  RTC latch — 0 then 1 latches the RTC counters
//
//   MBC5:
//     0x0000-0x1FFF  RAM enable
//     0x2000-0x2FFF  ROM bank lower 8 bits (0 stays 0)
//     0x3000-0x3FFF  ROM bank bit 8 (only bit 0 of value matters)
//     0x4000-0x5FFF  RAM bank 0..0x0F (bit 3 of value = rumble on rumble carts)

#include "gb_mbc.h"
#include "gb_cart.h"

namespace SGB {

void MbcReset(MbcState &s)
{
	s.rom_bank   = 1;
	s.ram_bank   = 0;
	s.ram_enable = false;
	s.mbc1_mode  = false;
	s.rtc_latch  = false;
	s.rtc_select = 0;
	for (int i = 0; i < 5; ++i) { s.rtc_regs[i] = 0; s.rtc_latched[i] = 0; }
	s.sachen_outer_bank = 0;
	s.sachen_outer_mask = 0;
	s.sachen_locked     = true;
	s.sachen_unlock_ctr = 0;
}

namespace {

inline uint32_t ReadRom(const std::vector<uint8_t> &rom, uint32_t offset)
{
	if (rom.empty()) return 0xFF;
	return rom[offset % rom.size()];
}

inline uint8_t ReadSram(const std::vector<uint8_t> &sram, uint32_t offset)
{
	if (sram.empty()) return 0xFF;
	return sram[offset % sram.size()];
}

inline void WriteSram(Cart &c, uint32_t offset, uint8_t value)
{
	if (c.sram.empty()) return;
	uint8_t &cell = c.sram[offset % c.sram.size()];
	if (cell != value)
	{
		cell = value;
		c.sram_dirty = true;
	}
}

// Effective 0x0000-0x3FFF bank for MBC1 — normally 0, but mode 1 with
// a >= 1MB cart can expose banks 0x20/0x40/0x60.
inline uint32_t Mbc1Bank0(const MbcState &s)
{
	if (!s.mbc1_mode) return 0;
	return (s.ram_bank & 0x03) << 5;
}

inline uint32_t Mbc1BankN(const MbcState &s)
{
	uint32_t lo = s.rom_bank & 0x1F;
	if (lo == 0) lo = 1;
	uint32_t hi = (s.ram_bank & 0x03) << 5;
	return lo | hi;
}

inline uint32_t Mbc1RamBank(const MbcState &s)
{
	return s.mbc1_mode ? (s.ram_bank & 0x03) : 0;
}

inline uint32_t SachenBank0(const MbcState &s)
{
	return static_cast<uint32_t>(s.sachen_outer_bank & s.sachen_outer_mask);
}

inline uint32_t SachenBankN(const MbcState &s)
{
	const uint32_t outer = s.sachen_outer_bank & s.sachen_outer_mask;
	const uint32_t inner = s.rom_bank & (~static_cast<uint32_t>(s.sachen_outer_mask) & 0xFFu);
	return outer | inner;
}

// Sachen MMC1 header bit-permutation: when locked, reads in 0x0100-0x01FF
// pass through the cart's A0/A6 and A1/A4 swaps so the bootstrap sees the
// real Nintendo logo at 0x0104-0x0133. Outside that 256-byte window the
// CPU sees raw ROM, so the game's actual entry point at 0x0100-0x0103
// (encoded as e.g. `00 21 60 02` → unscrambles to `00 C3 60 6F` = JP $6F60)
// transfers control to game code in a higher bank.
inline uint16_t SachenLockedHeaderXform(uint16_t addr)
{
	if ((addr & 0xFF00u) != 0x0100u) return addr;
	return static_cast<uint16_t>((addr & ~0x53u)
	                           | ((addr >> 6) & 0x01u)
	                           | ((addr >> 3) & 0x02u)
	                           | ((addr << 3) & 0x10u)
	                           | ((addr << 6) & 0x40u));
}

} // anonymous

uint8_t MbcRead(MbcState &s, const std::vector<uint8_t> &rom, const std::vector<uint8_t> &sram, uint16_t addr)
{
	if (addr < 0x4000)
	{
		// Bank 0 region — mostly direct, except for MBC1 mode 1 quirk
		// and Sachen MMC1 outer-bank routing + locked-mode header xform.
		uint32_t bank = 0;
		uint16_t eff_addr = addr;
		if (s.type == MbcType::MBC1)
		{
			bank = Mbc1Bank0(s);
		}
		else if (s.type == MbcType::SachenMMC1)
		{
			bank = SachenBank0(s);
			if (s.sachen_locked) eff_addr = SachenLockedHeaderXform(addr);
		}
		return static_cast<uint8_t>(ReadRom(rom, (bank * 0x4000u) + eff_addr));
	}
	if (addr < 0x8000)
	{
		uint32_t bank = 1;
		switch (s.type)
		{
			case MbcType::MBC1: bank = Mbc1BankN(s); break;
			case MbcType::MBC3: bank = s.rom_bank ? s.rom_bank : 1; break;
			case MbcType::MBC5: bank = s.rom_bank; break;
			case MbcType::MBC2: bank = (s.rom_bank & 0x0F) ? (s.rom_bank & 0x0F) : 1; break;
			case MbcType::SachenMMC1: bank = SachenBankN(s); break;
			default:            bank = 1; break;
		}
		return static_cast<uint8_t>(ReadRom(rom, (bank * 0x4000u) + (addr - 0x4000u)));
	}
	if (addr >= 0xA000 && addr < 0xC000)
	{
		if (!s.ram_enable) return 0xFF;

		// MBC3 RTC select exposes latched RTC values in this window.
		if (s.type == MbcType::MBC3 && s.rtc_select >= 0x08 && s.rtc_select <= 0x0C)
		{
			return s.rtc_latched[s.rtc_select - 0x08];
		}

		// MBC2 has internal 512 x 4-bit RAM — upper nibble reads as 0xF.
		if (s.type == MbcType::MBC2)
		{
			uint32_t off = (addr - 0xA000) & 0x01FF;
			return static_cast<uint8_t>(ReadSram(sram, off) | 0xF0);
		}

		uint32_t bank = 0;
		switch (s.type)
		{
			case MbcType::MBC1: bank = Mbc1RamBank(s); break;
			case MbcType::MBC3: bank = s.ram_bank & 0x07; break;
			case MbcType::MBC5: bank = s.ram_bank & 0x0F; break;
			default:            bank = 0;               break;
		}
		return ReadSram(sram, (bank * 0x2000u) + (addr - 0xA000u));
	}
	return 0xFF;
}

void MbcWrite(Cart &c, uint16_t addr, uint8_t value)
{
	MbcState &s = c.mbc;
	switch (s.type)
	{

	case MbcType::None:
		if (addr >= 0xA000 && addr < 0xC000)
		{
			WriteSram(c, addr - 0xA000, value);
		}
		break;

	case MbcType::MBC1:
		if (addr < 0x2000)
		{
			s.ram_enable = ((value & 0x0F) == 0x0A);
		}
		else if (addr < 0x4000)
		{
			uint32_t v = value & 0x1F;
			if (v == 0) v = 1;
			s.rom_bank = v;
		}
		else if (addr < 0x6000)
		{
			// Stored raw; mode gate applied at read time via Mbc1BankN/Mbc1RamBank.
			s.ram_bank = value & 0x03;
		}
		else if (addr < 0x8000)
		{
			s.mbc1_mode = (value & 0x01) != 0;
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (!s.ram_enable) break;
			WriteSram(c, (Mbc1RamBank(s) * 0x2000u) + (addr - 0xA000u), value);
		}
		break;

	case MbcType::MBC2:
		if (addr < 0x4000)
		{
			// MBC2 consolidates RAM enable and ROM bank select on the same range:
			// bit 8 of the address selects which function.
			if ((addr & 0x0100) == 0)
			{
				s.ram_enable = ((value & 0x0F) == 0x0A);
			}
			else
			{
				uint32_t v = value & 0x0F;
				if (v == 0) v = 1;
				s.rom_bank = v;
			}
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (!s.ram_enable) break;
			// 512 x 4-bit — only low nibble stored.
			WriteSram(c, (addr - 0xA000) & 0x01FF, static_cast<uint8_t>(value & 0x0F));
		}
		break;

	case MbcType::MBC3:
		if (addr < 0x2000)
		{
			s.ram_enable = ((value & 0x0F) == 0x0A);
		}
		else if (addr < 0x4000)
		{
			uint32_t v = value & 0x7F;
			if (v == 0) v = 1;
			s.rom_bank = v;
		}
		else if (addr < 0x6000)
		{
			s.rtc_select = static_cast<uint8_t>(value & 0x0F);
			if (value <= 0x07) s.ram_bank = value & 0x07;
		}
		else if (addr < 0x8000)
		{
			// RTC latch: 0 → 1 transition latches current RTC regs.
			if (!s.rtc_latch && value == 0x01)
			{
				for (int i = 0; i < 5; ++i) s.rtc_latched[i] = s.rtc_regs[i];
			}
			s.rtc_latch = (value == 0x00);
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (!s.ram_enable) break;
			if (s.rtc_select >= 0x08 && s.rtc_select <= 0x0C)
			{
				s.rtc_regs[s.rtc_select - 0x08] = value;
			}
			else
			{
				WriteSram(c, ((s.ram_bank & 0x07) * 0x2000u) + (addr - 0xA000u), value);
			}
		}
		break;

	case MbcType::MBC5:
		if (addr < 0x2000)
		{
			s.ram_enable = ((value & 0x0F) == 0x0A);
		}
		else if (addr < 0x3000)
		{
			s.rom_bank = (s.rom_bank & 0x100) | value;
		}
		else if (addr < 0x4000)
		{
			s.rom_bank = (s.rom_bank & 0x0FF) | (static_cast<uint32_t>(value & 0x01) << 8);
		}
		else if (addr < 0x6000)
		{
			s.ram_bank = value & 0x0F;
			// bit 3 = rumble for rumble carts; ignored here (P7 may wire it).
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (!s.ram_enable) break;
			WriteSram(c, ((s.ram_bank & 0x0F) * 0x2000u) + (addr - 0xA000u), value);
		}
		break;

	case MbcType::SachenMMC1:
		// Outer-bank / inner-bank / mask. The outer registers latch only
		// while the inner-bank holds D5:D4 = 0b11 (per Tauwasser's RE).
		if (addr < 0x2000)
		{
			if ((s.rom_bank & 0x30) == 0x30) s.sachen_outer_bank = value;
		}
		else if (addr < 0x4000)
		{
			s.rom_bank = value ? value : 1u;
		}
		else if (addr < 0x6000)
		{
			if ((s.rom_bank & 0x30) == 0x30) s.sachen_outer_mask = value;
		}
		break;

	default:
		// MBC6, MBC7, HuC1, HuC3, MMM01: treat as read-only no-MBC.
		break;
	}
}

void MbcNotifyHighWrite(MbcState &s, uint16_t /*addr*/)
{
	if (s.type != MbcType::SachenMMC1) return;
	if (!s.sachen_locked) return;
	if (s.sachen_unlock_ctr < 0x30)
	{
		if (++s.sachen_unlock_ctr >= 0x30) s.sachen_locked = false;
	}
}

} // namespace SGB
