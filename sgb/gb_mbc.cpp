/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Memory Bank Controllers. MBC1, MBC3, MBC5 cover ~95% of commercial
// GB carts; MBC2, HuC1, HuC3, MMM01, and Sachen MMC1 are also
// handled, as are MBC6 (dual half-bank windows + flash) and MBC7
// (accelerometer + 93LC56 EEPROM).
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
//
//   HuC1: (MBC1-like, with an infrared link)
//     0x0000-0x1FFF  RAM/IR select — value 0x0E routes 0xA000-0xBFFF to the
//                    IR register; any other value routes it to cart RAM.
//                    HuC1 RAM has no separate enable (always live).
//     0x2000-0x3FFF  ROM bank (low 6 bits)
//     0x4000-0x5FFF  RAM bank (low 2 bits)
//
//   HuC3: (MBC-like, with an RTC command interface + IR)
//     0x0000-0x1FFF  $A000-$BFFF window mode (low nibble): $0/$A = RAM (ro/rw),
//                    $B = RTC command, $C = response, $D = semaphore, $E = IR
//     0x2000-0x3FFF  ROM bank (7 bits, no 0->1 remap)
//     0x4000-0x5FFF  RAM bank (low 2 bits)
//
//   TAMA5: (Bandai Tamagotchi — a nibble register port, not the usual banking)
//     0xA001  register select — which TAMA5 register the next 0xA000 access hits
//     0xA000  nibble data port. Registers: 0/1 = ROM bank lo/hi, 4/5 = write
//             data lo/hi, 6/7 = address hi/lo (writing reg 7 commits), 0xA =
//             ready handshake (reads 0xF1), 0xC/0xD = read result lo/hi. The
//             5-bit address + a 3-bit mode pick a 32-byte EEPROM cell or a
//             TAMA6 RTC register / command.

#include "gb_mbc.h"
#include "gb_cart.h"

#include <ctime>
#include <cstring>

static bool (*g_gb_camera_cb)(unsigned char *, int, int) = nullptr;

void S9xGBSetCameraCallback(bool (*cb)(unsigned char *, int, int))
{
	g_gb_camera_cb = cb;
}

int g_cam_countdown = 0;

namespace SGB {

// TAMA5 reuses existing MbcState fields (it never coexists with the mappers
// that own them), keeping MbcState's size — and the savestate layout — fixed:
//   rom_bank          = ROM bank (BANK_LO | BANK_HI << 4)
//   rtc_select        = selected TAMA5 register (the 0xA001 latch)
//   sachen_outer_bank = packed write byte (WRITE_HI << 4 | WRITE_LO)
//   sachen_outer_mask = packed address byte (ADDR_HI << 4 | ADDR_LO);
//                       low 5 bits = RAM/command address, top 3 bits = op mode
//   rtc_regs[0..4] + rtc_latched[0..1] = TAMA6 BCD timer page, one nibble each
namespace {

inline void Tama5SetRtcNib(MbcState &s, uint8_t idx, uint8_t nib)
{
	nib &= 0x0F;
	const uint8_t b = static_cast<uint8_t>(idx >> 1);
	uint8_t &cell = (b < 5) ? s.rtc_regs[b] : s.rtc_latched[b - 5];
	cell = (idx & 1) ? static_cast<uint8_t>((cell & 0x0F) | (nib << 4))
	                 : static_cast<uint8_t>((cell & 0xF0) | nib);
}

inline uint8_t Tama5RtcNib(const MbcState &s, uint8_t idx)
{
	const uint8_t b = static_cast<uint8_t>(idx >> 1);
	const uint8_t cell = (b < 5) ? s.rtc_regs[b] : s.rtc_latched[b - 5];
	return (idx & 1) ? static_cast<uint8_t>(cell >> 4)
	                 : static_cast<uint8_t>(cell & 0x0F);
}

inline void Tama5SeedRtc(MbcState &s)
{
	std::time_t t = std::time(nullptr);
	const std::tm *lt = std::localtime(&t);
	if (!lt) return;
	auto put2 = [&](uint8_t lo, int v) {
		Tama5SetRtcNib(s, lo, static_cast<uint8_t>(v % 10));
		Tama5SetRtcNib(s, static_cast<uint8_t>(lo + 1), static_cast<uint8_t>((v / 10) % 10));
	};
	put2(0x0, lt->tm_sec);
	put2(0x2, lt->tm_min);
	put2(0x4, lt->tm_hour);
	Tama5SetRtcNib(s, 0x6, static_cast<uint8_t>(lt->tm_wday));
	put2(0x7, lt->tm_mday);
	put2(0x9, lt->tm_mon + 1);
	put2(0xB, lt->tm_year % 100);
}

} // anonymous

void MbcReset(MbcState &s)
{
	s.rom_bank   = 1;
	s.ram_bank   = 0;
	// HuC1 RAM is always live — its $0000-$1FFF register only routes the
	// $A000-$BFFF window between RAM and the IR register — so default it
	// enabled. (Cart type is assigned before MbcReset runs.)
	s.ram_enable = (s.type == MbcType::HuC1);
	s.mbc1_mode  = false;
	s.rtc_latch  = false;
	s.rtc_select = 0;
	for (int i = 0; i < 5; ++i) { s.rtc_regs[i] = 0; s.rtc_latched[i] = 0; }
	s.sachen_outer_bank = 0;
	s.sachen_outer_mask = 0;
	s.sachen_locked     = true;
	s.sachen_unlock_ctr = 0;

	s.mmm01_locked            = false;
	s.mmm01_mbc1_mode         = false;
	s.mmm01_mbc1_mode_disable = false;
	s.mmm01_multiplex_mode    = false;
	s.mmm01_rom_bank_low      = 0;
	s.mmm01_rom_bank_mid      = 0;
	s.mmm01_rom_bank_high     = 0;
	s.mmm01_ram_bank_low      = 0;
	s.mmm01_ram_bank_high     = 0;
	s.mmm01_rom_bank_mask     = 0;
	s.mmm01_ram_bank_mask     = 0xFF;
	s.mmm01_just_locked       = false;

	if (s.type == MbcType::M161) s.rom_bank = 0;   // power-on page 0 = menu
	if (s.type == MbcType::TAMA5) Tama5SeedRtc(s);
	if (s.type == MbcType::MBC7)
	{
		s.rtc_regs[1] = 0xFF;
		s.rtc_regs[2] = 0xFF;
		s.rtc_select  = 0x01;
	}
	if (s.type == MbcType::MBC6)
	{
		s.rom_bank           = 2;
		s.mmm01_rom_bank_low = 3;
		s.ram_bank           = 0;
		s.mmm01_rom_bank_mid = 1;
	}
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
// a >= 1MB cart can expose banks 0x20/0x40/0x60. On a multicart the 2-bit
// BANK2 register drives one bit lower (A18-A19), so it shifts by 4 and the
// game slot's bank 0 (the menu / sub-game header) appears here.
inline uint32_t Mbc1Bank0(const MbcState &s, bool multicart)
{
	if (!s.mbc1_mode) return 0;
	return (s.ram_bank & 0x03) << (multicart ? 4 : 5);
}

inline uint32_t Mbc1BankN(const MbcState &s, bool multicart)
{
	uint32_t lo = s.rom_bank & 0x1F;
	if (lo == 0) lo = 1;
	// Multicart wiring leaves BANK1 only 4 effective bits and shifts BANK2
	// down to bits 4-5, so each BANK2 value selects a 256 KiB game slot.
	if (multicart)
		return (lo & 0x0F) | ((s.ram_bank & 0x03) << 4);
	return lo | ((s.ram_bank & 0x03) << 5);
}

inline uint32_t Mbc1RamBank(const MbcState &s, bool multicart)
{
	// Multicarts carry no cart RAM — BANK2 is the game selector, not a RAM bank.
	if (multicart) return 0;
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

// Sachen MMC1 header bit-permutation: while locked, reads in 0x0100-0x01FF pass
// through the cart's A0/A6 and A1/A4 swaps so the boot ROM's logo check sees the
// real Nintendo logo at 0x0104-0x0133. Whether the lock survives the boot hand-
// off is per-cart — see Cart::sachen_runs_raw and gb_memory.cpp 0xFF50.
inline uint16_t SachenLockedHeaderXform(uint16_t addr)
{
	if ((addr & 0xFF00u) != 0x0100u) return addr;
	return static_cast<uint16_t>((addr & ~0x53u)
	                           | ((addr >> 6) & 0x01u)
	                           | ((addr >> 3) & 0x02u)
	                           | ((addr << 3) & 0x10u)
	                           | ((addr << 6) & 0x40u));
}

// MMM01 effective bank for the $0000-$3FFF region. While unlocked the
// mapper forces all outer ROM lines to 1 so the last 32 KiB of ROM
// (the menu) is exposed. Once locked, the menu has populated the base
// (rom_bank_mid/high) and mask (rom_bank_mask) registers; the masked
// bits of rom_bank_low get overlaid with the base so the sub-game can
// only switch within its allotted window.
inline uint32_t Mmm01Rom0Bank(const MbcState &s, size_t rom_size)
{
	if (!s.mmm01_locked)
	{
		const uint32_t banks = static_cast<uint32_t>(rom_size / 0x4000u);
		return banks >= 2 ? banks - 2u : 0u;
	}
	const uint8_t lowmask = static_cast<uint8_t>(s.mmm01_rom_bank_mask << 1);
	const uint8_t mid = s.mmm01_multiplex_mode && s.mmm01_mbc1_mode
	                  ? 0u
	                  : (s.mmm01_multiplex_mode ? s.mmm01_ram_bank_low : s.mmm01_rom_bank_mid);
	return static_cast<uint32_t>(
	          (s.mmm01_rom_bank_low & lowmask)
	        | (static_cast<uint32_t>(mid) << 5)
	        | (static_cast<uint32_t>(s.mmm01_rom_bank_high) << 7));
}

inline uint32_t Mmm01RomBank(const MbcState &s, size_t rom_size)
{
	if (!s.mmm01_locked)
	{
		const uint32_t banks = static_cast<uint32_t>(rom_size / 0x4000u);
		return banks >= 1 ? banks - 1u : 1u;
	}
	const uint8_t mid = s.mmm01_multiplex_mode ? s.mmm01_ram_bank_low : s.mmm01_rom_bank_mid;
	uint32_t bank = static_cast<uint32_t>(s.mmm01_rom_bank_low)
	              | (static_cast<uint32_t>(mid) << 5)
	              | (static_cast<uint32_t>(s.mmm01_rom_bank_high) << 7);
	if (bank == Mmm01Rom0Bank(s, rom_size)) ++bank;
	return bank;
}

inline uint32_t Mmm01RamBank(const MbcState &s)
{
	if (!s.mmm01_locked) return 0;
	if (s.mmm01_multiplex_mode)
		return static_cast<uint32_t>(s.mmm01_rom_bank_mid
		                           | (static_cast<uint32_t>(s.mmm01_ram_bank_high) << 2));
	return static_cast<uint32_t>(s.mmm01_ram_bank_low
	                           | (static_cast<uint32_t>(s.mmm01_ram_bank_high) << 2));
}

// ---- HuC3 ----------------------------------------------------------------
// HuC3 adds an RTC reached through a small command interface, plus IR. It
// never coexists with MBC3, so it borrows the (otherwise MBC3-only) RTC
// fields as its own state — keeping MbcState's size, and therefore the
// savestate layout, unchanged:
//   rtc_select     = $A000-$BFFF window mode (low nibble of $0000-$1FFF write)
//   rtc_regs[0]    = RTC register-map access pointer
//   rtc_regs[1]    = last extended-command argument ($2 = boot status request)
//   rtc_regs[2]    = response nibble (result of the last read command)
//   rtc_regs[3..4] = minutes (12-bit), rtc_latched[0..1] = days (16-bit)
inline uint16_t Huc3Minutes(const MbcState &s) { return static_cast<uint16_t>(s.rtc_regs[3] | (s.rtc_regs[4] << 8)); }
inline uint16_t Huc3Days(const MbcState &s)    { return static_cast<uint16_t>(s.rtc_latched[0] | (s.rtc_latched[1] << 8)); }
inline void Huc3SetMinutes(MbcState &s, uint16_t v) { s.rtc_regs[3] = v & 0xFF; s.rtc_regs[4] = (v >> 8) & 0x0F; }
inline void Huc3SetDays(MbcState &s, uint16_t v)    { s.rtc_latched[0] = v & 0xFF; s.rtc_latched[1] = (v >> 8) & 0xFF; }

inline uint8_t Huc3ReadReg(const MbcState &s, uint8_t idx)
{
	if (idx < 3) return (Huc3Minutes(s) >> (idx * 4)) & 0x0F;
	if (idx < 7) return (Huc3Days(s) >> ((idx - 3) * 4)) & 0x0F;
	return 0;   // alarm/tone registers ($58+) not modeled
}

inline void Huc3WriteReg(MbcState &s, uint8_t idx, uint8_t nib)
{
	nib &= 0x0F;
	if (idx < 3)
	{
		const uint16_t m = Huc3Minutes(s);
		Huc3SetMinutes(s, static_cast<uint16_t>((m & ~(0x0Fu << (idx * 4))) | (nib << (idx * 4))));
	}
	else if (idx < 7)
	{
		const uint16_t d = Huc3Days(s);
		Huc3SetDays(s, static_cast<uint16_t>((d & ~(0x0Fu << ((idx - 3) * 4))) | (nib << ((idx - 3) * 4))));
	}
	// idx >= 7 (alarm/tone) intentionally dropped
}

// Execute one command written to the $B window. Per SameBoy, HuC3 runs the
// command immediately on write; the $D semaphore then always reads ready.
inline void Huc3Command(MbcState &s, uint8_t value)
{
	uint8_t &idx = s.rtc_regs[0];
	const uint8_t arg = value & 0x0F;
	switch (value >> 4)
	{
		case 1: s.rtc_regs[2] = Huc3ReadReg(s, idx); ++idx;             break;  // read + advance
		case 2: Huc3WriteReg(s, idx, arg);                              break;  // write
		case 3: Huc3WriteReg(s, idx, arg); ++idx;                       break;  // write + advance
		case 4: idx = static_cast<uint8_t>((idx & 0xF0) | arg);         break;  // pointer low nibble
		case 5: idx = static_cast<uint8_t>((idx & 0x0F) | (arg << 4));  break;  // pointer high nibble
		case 6: s.rtc_regs[1] = arg;                                    break;  // extended command
		default:                                                        break;
	}
}

// TAMA5: a write to register 7 (ADDR_LO) commits the operation selected by the
// top 3 bits of the packed address byte — RAM write, RTC command, or RTC-page
// write. RAM reads (mode 1) are serviced lazily in MbcRead.
inline void Tama5Trigger(Cart &c)
{
	MbcState &s = c.mbc;
	const uint8_t addr5 = static_cast<uint8_t>(s.sachen_outer_mask & 0x1F);
	const uint8_t mode  = static_cast<uint8_t>(s.sachen_outer_mask >> 5);
	const uint8_t data  = s.sachen_outer_bank;
	switch (mode)
	{
		case 0x0:
			WriteSram(c, addr5, data);
			break;
		case 0x2:
			switch (addr5)
			{
				case 0x0: s.rtc_latch = false; break;
				case 0x1: s.rtc_latch = true;  break;
				case 0x4: Tama5SetRtcNib(s, 0x2, data & 0x0F); Tama5SetRtcNib(s, 0x3, static_cast<uint8_t>(data >> 4)); break;
				case 0x5: Tama5SetRtcNib(s, 0x4, data & 0x0F); Tama5SetRtcNib(s, 0x5, static_cast<uint8_t>(data >> 4)); break;
				default: break;
			}
			break;
		case 0x4:
			if ((s.sachen_outer_mask & 0x0F) == 0x0)
			{
				const uint8_t idx = static_cast<uint8_t>(data & 0x0F);
				if (idx < 0x0D) Tama5SetRtcNib(s, idx, static_cast<uint8_t>(data >> 4));
			}
			break;
		default:
			break;
	}
}

inline uint16_t Mbc7EepromWord(const std::vector<uint8_t> &sram, uint8_t word)
{
	const uint32_t off = static_cast<uint32_t>(word & 0x7F) * 2u;
	return static_cast<uint16_t>(ReadSram(sram, off) | (ReadSram(sram, off + 1) << 8));
}

inline void Mbc7EepromSetWord(Cart &c, uint8_t word, uint16_t value)
{
	const uint32_t off = static_cast<uint32_t>(word & 0x7F) * 2u;
	WriteSram(c, off,     static_cast<uint8_t>(value));
	WriteSram(c, off + 1, static_cast<uint8_t>(value >> 8));
}

inline void Mbc7EepromFill(Cart &c, uint16_t value)
{
	for (uint8_t w = 0; w < 0x80; ++w) Mbc7EepromSetWord(c, w, value);
}

void Mbc7EepromClock(Cart &c, uint8_t value)
{
	MbcState &s = c.mbc;
	const bool cs       = (value & 0x80) != 0;
	const bool di       = (value & 0x02) != 0;
	const bool clk      = (value & 0x40) != 0;
	const bool clk_prev = (s.rtc_select & 0x40) != 0;
	bool do_bit = (s.rtc_select & 0x01) != 0;

	if (cs && clk && !clk_prev)
	{
		uint16_t read_bits = static_cast<uint16_t>(s.rtc_regs[1] | (s.rtc_regs[2] << 8));
		uint16_t command   = static_cast<uint16_t>(s.rtc_regs[3] | (s.rtc_regs[4] << 8));
		uint8_t  args_left = s.rtc_regs[0];
		bool     we        = s.mmm01_mbc1_mode;

		do_bit    = (read_bits >> 15) & 1;
		read_bits = static_cast<uint16_t>((read_bits << 1) | 1);

		if (args_left == 0)
		{
			command = static_cast<uint16_t>(((command << 1) | (di ? 1 : 0)) & 0x7FF);
			if (command & 0x400)
			{
				const uint8_t word = static_cast<uint8_t>(command & 0x7F);
				switch ((command >> 6) & 0xF)
				{
					case 0x8: case 0x9: case 0xA: case 0xB:
						read_bits = Mbc7EepromWord(c.sram, word);
						command = 0;
						break;
					case 0x3: we = true;  command = 0; break;
					case 0x0: we = false; command = 0; break;
					case 0x4: case 0x5: case 0x6: case 0x7:
						if (we) Mbc7EepromSetWord(c, word, 0);
						args_left = 16;
						break;
					case 0xC: case 0xD: case 0xE: case 0xF:
						if (we) { Mbc7EepromSetWord(c, word, 0xFFFF); read_bits = 0x3FFF; }
						command = 0;
						break;
					case 0x2:
						if (we) { Mbc7EepromFill(c, 0xFFFF); read_bits = 0xFF; }
						command = 0;
						break;
					case 0x1:
						if (we) Mbc7EepromFill(c, 0);
						args_left = 16;
						break;
				}
			}
		}
		else
		{
			--args_left;
			do_bit = true;
			if (di && we)
			{
				const uint16_t bit = static_cast<uint16_t>(1u << args_left);
				if (command & 0x100)
				{
					const uint8_t w = static_cast<uint8_t>(command & 0x7F);
					Mbc7EepromSetWord(c, w, static_cast<uint16_t>(Mbc7EepromWord(c.sram, w) | bit));
				}
				else
				{
					for (uint8_t w = 0; w < 0x80; ++w)
						Mbc7EepromSetWord(c, w, static_cast<uint16_t>(Mbc7EepromWord(c.sram, w) | bit));
				}
			}
			if (args_left == 0) { command = 0; read_bits = 0x3FFF; }
		}

		s.rtc_regs[0] = args_left;
		s.rtc_regs[1] = static_cast<uint8_t>(read_bits);
		s.rtc_regs[2] = static_cast<uint8_t>(read_bits >> 8);
		s.rtc_regs[3] = static_cast<uint8_t>(command);
		s.rtc_regs[4] = static_cast<uint8_t>(command >> 8);
		s.mmm01_mbc1_mode = we;
	}

	s.rtc_select = static_cast<uint8_t>((do_bit ? 0x01 : 0) | (di ? 0x02 : 0) | (clk ? 0x40 : 0) | (cs ? 0x80 : 0));
}

uint8_t Mbc6Read(const MbcState &s, const std::vector<uint8_t> &rom, const std::vector<uint8_t> &sram, uint16_t addr)
{
	if (addr < 0x4000)
		return static_cast<uint8_t>(ReadRom(rom, addr));
	if (addr < 0x6000)
		return s.mbc1_mode ? 0xFF : static_cast<uint8_t>(ReadRom(rom, (s.rom_bank * 0x2000u) + (addr - 0x4000u)));
	if (addr < 0x8000)
		return s.mmm01_mbc1_mode ? 0xFF : static_cast<uint8_t>(ReadRom(rom, (s.mmm01_rom_bank_low * 0x2000u) + (addr - 0x6000u)));
	if (addr >= 0xA000 && addr < 0xB000)
		return (s.ram_enable && !sram.empty()) ? ReadSram(sram, (s.ram_bank * 0x1000u) + (addr - 0xA000u)) : 0xFF;
	if (addr >= 0xB000 && addr < 0xC000)
		return (s.ram_enable && !sram.empty()) ? ReadSram(sram, (s.mmm01_rom_bank_mid * 0x1000u) + (addr - 0xB000u)) : 0xFF;
	return 0xFF;
}

} // anonymous

uint8_t MbcRead(MbcState &s, const std::vector<uint8_t> &rom, const std::vector<uint8_t> &sram, uint16_t addr, bool mbc1_multicart)
{
	if (s.type == MbcType::MBC6)
		return Mbc6Read(s, rom, sram, addr);
	if (s.type == MbcType::M161 && addr < 0x8000)
	{
		// One 32 KiB page covers the whole $0000-$7FFF window.
		const uint32_t page = s.rom_bank & 0x07;
		return static_cast<uint8_t>(ReadRom(rom, (page << 15) | (addr & 0x7FFF)));
	}
	if (addr < 0x4000)
	{
		// Bank 0 region — mostly direct, except for MBC1 mode 1 quirk,
		// Sachen MMC1 outer-bank/header xform, and MMM01 menu mapping.
		uint32_t bank = 0;
		uint16_t eff_addr = addr;
		if (s.type == MbcType::MBC1)
		{
			bank = Mbc1Bank0(s, mbc1_multicart);
		}
		else if (s.type == MbcType::SachenMMC1)
		{
			bank = SachenBank0(s);
			if (s.sachen_locked) eff_addr = SachenLockedHeaderXform(addr);
		}
		else if (s.type == MbcType::MMM01)
		{
			bank = Mmm01Rom0Bank(s, rom.size());
		}
		else if (s.type == MbcType::MBC3)
		{
			bank = s.sachen_outer_bank;   // Duz multicart base bank; 0 for normal MBC3
		}
		return static_cast<uint8_t>(ReadRom(rom, (bank * 0x4000u) + eff_addr));
	}
	if (addr < 0x8000)
	{
		uint32_t bank = 1;
		switch (s.type)
		{
			case MbcType::MBC1: bank = Mbc1BankN(s, mbc1_multicart); break;
			case MbcType::MBC3: bank = (s.rom_bank ? s.rom_bank : 1) + s.sachen_outer_bank; break;
			case MbcType::MBC5: bank = s.rom_bank; break;
			case MbcType::MBC7: bank = s.rom_bank; break;
			case MbcType::HuC1: bank = s.rom_bank ? s.rom_bank : 1; break;
			case MbcType::HuC3: bank = s.rom_bank; break;
			case MbcType::TAMA5: bank = s.rom_bank; break;
			case MbcType::Camera: bank = s.rom_bank ? s.rom_bank : 1; break;
			case MbcType::MBC2: bank = (s.rom_bank & 0x0F) ? (s.rom_bank & 0x0F) : 1; break;
			case MbcType::SachenMMC1: bank = SachenBankN(s); break;
			case MbcType::MMM01:      bank = Mmm01RomBank(s, rom.size()); break;
			default:            bank = 1; break;
		}
		return static_cast<uint8_t>(ReadRom(rom, (bank * 0x4000u) + (addr - 0x4000u)));
	}
	if (addr >= 0xA000 && addr < 0xC000)
	{
		// HuC1 routes this window between cart RAM and the IR register.
		// ram_enable doubles as the RAM/IR select (true = RAM, false = IR).
		// With no link partner the IR sensor idles at $C0 (no light).
		if (s.type == MbcType::HuC1)
		{
			if (!s.ram_enable) return 0xC0;
			return ReadSram(sram, ((s.ram_bank & 0x03) * 0x2000u) + (addr - 0xA000u));
		}

		// HuC3 multiplexes RAM, the RTC command/response, a ready semaphore
		// and IR onto this window, selected by the mode set at $0000-$1FFF.
		if (s.type == MbcType::HuC3)
		{
			switch (s.rtc_select)
			{
				case 0x0:        // RAM, read-only
				case 0xA:        // RAM, read/write
					return ReadSram(sram, ((s.ram_bank & 0x03) * 0x2000u) + (addr - 0xA000u));
				case 0xC:        // command response — $62 status must read back 1
					return (s.rtc_regs[1] == 0x02) ? 0x01 : s.rtc_regs[2];
				case 0xD: return 0x01;   // semaphore: always ready
				case 0xE: return 0x00;   // IR: no link partner
				default:  return 0xFF;   // open bus
			}
		}

		if (s.type == MbcType::TAMA5)
		{
			if (addr & 1) return 0xFF;
			const uint8_t addr5 = static_cast<uint8_t>(s.sachen_outer_mask & 0x1F);
			const uint8_t mode  = static_cast<uint8_t>(s.sachen_outer_mask >> 5);
			if (s.rtc_select == 0x0A) return 0xF1;
			if (s.rtc_select == 0x0C || s.rtc_select == 0x0D)
			{
				uint8_t value = 0x0F;
				if (mode == 0x1)
					value = ReadSram(sram, addr5);
				else if (mode == 0x2)
					value = (addr5 == 0x6) ? static_cast<uint8_t>((Tama5RtcNib(s, 0x3) << 4) | Tama5RtcNib(s, 0x2))
					      : (addr5 == 0x7) ? static_cast<uint8_t>((Tama5RtcNib(s, 0x5) << 4) | Tama5RtcNib(s, 0x4))
					      :                  addr5;
				else if (mode == 0x4)
				{
					const uint8_t idx = static_cast<uint8_t>(s.sachen_outer_bank & 0x0F);
					value = (idx < 0x0D) ? Tama5RtcNib(s, idx) : 0;
				}
				if (s.rtc_select == 0x0D) value = static_cast<uint8_t>(value >> 4);
				return static_cast<uint8_t>(value | 0xF0);
			}
			return 0xF1;
		}

		if (s.type == MbcType::MBC7)
		{
			if (!s.ram_enable || !s.mbc1_mode || addr >= 0xB000) return 0xFF;
			switch ((addr >> 4) & 0xF)
			{
				case 2: return static_cast<uint8_t>(s.rtc_latch ? 0xD0 : 0x00);
				case 3: return static_cast<uint8_t>(s.rtc_latch ? 0x81 : 0x80);
				case 4: return static_cast<uint8_t>(s.rtc_latch ? 0xD0 : 0x00);
				case 5: return static_cast<uint8_t>(s.rtc_latch ? 0x81 : 0x80);
				case 6: return 0x00;
				case 8: return s.rtc_select;
				default: return 0xFF;
			}
		}

		if (s.type == MbcType::Camera)
		{
			if (s.mbc1_mode)
			{
				if ((addr & 0x7F) == 0) return (g_cam_countdown > 0) ? 0x01 : 0x00;
				return 0x00;
			}
			if (g_cam_countdown > 0) return 0x00;
			if (!s.ram_enable) return 0xFF;
			return ReadSram(sram, ((s.ram_bank & 0x0F) * 0x2000u) + (addr - 0xA000u));
		}

		if (!s.ram_enable && s.type != MbcType::MBC5) return 0xFF;

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
			case MbcType::MBC1: bank = Mbc1RamBank(s, mbc1_multicart); break;
			case MbcType::MBC3: bank = s.ram_bank & 0x07; break;
			case MbcType::MBC5: bank = s.ram_bank & 0x0F; break;
			case MbcType::MMM01: bank = Mmm01RamBank(s) & 0x0F; break;
			default:            bank = 0;               break;
		}
		return ReadSram(sram, (bank * 0x2000u) + (addr - 0xA000u));
	}
	return 0xFF;
}

static void GbCameraCapture(Cart &c)
{
	const uint8_t *reg = c.camera_regs;
	const int expo = (reg[2] << 8) | reg[3];
	g_cam_countdown = 129792 + expo * 64;
	const int W = 128, H = 112;
	unsigned char src[W * H];
	if (!(g_gb_camera_cb && g_gb_camera_cb(src, W, H)))
		std::memset(src, 0x80, sizeof(src));

	for (int ty = 0; ty < H / 8; ++ty)
	for (int tx = 0; tx < W / 8; ++tx)
	{
		const uint32_t tile_off = 0x100u + static_cast<uint32_t>(ty * (W / 8) + tx) * 16u;
		for (int row = 0; row < 8; ++row)
		{
			uint8_t lo = 0, hi = 0;
			const int py = ty * 8 + row;
			for (int col = 0; col < 8; ++col)
			{
				const int px = tx * 8 + col;
				int v = src[py * W + px] * expo / 0x1000;
				if (v < 0) v = 0; else if (v > 255) v = 255;
				const int base = ((px & 3) + (py & 3) * 4) * 3 + 6;
				int shade;
				if      (v < reg[base + 0]) shade = 3;
				else if (v < reg[base + 1]) shade = 2;
				else if (v < reg[base + 2]) shade = 1;
				else                        shade = 0;
				lo |= static_cast<uint8_t>((shade & 1) << (7 - col));
				hi |= static_cast<uint8_t>(((shade >> 1) & 1) << (7 - col));
			}
			WriteSram(c, tile_off + row * 2 + 0, lo);
			WriteSram(c, tile_off + row * 2 + 1, hi);
		}
	}
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
			WriteSram(c, (Mbc1RamBank(s, c.mbc1_multicart) * 0x2000u) + (addr - 0xA000u), value);
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
			if (c.duz_multicart && value == 0xC0) { s.sachen_outer_mask = 1; break; }   // unlock Duz register port
			if (c.duz_multicart) s.sachen_outer_mask = 0;
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
			if (c.duz_multicart && s.sachen_outer_mask)
			{
				// Register port: $A000 latches the index, $A100 writes it. Only $A3 (ROM base, in 32 KiB pages) affects banking.
				if (addr & 0x0100) { if (s.sachen_unlock_ctr == 0xA3) s.sachen_outer_bank = static_cast<uint8_t>(value << 1); }
				else                 s.sachen_unlock_ctr = value;
				break;
			}
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

	case MbcType::MMM01:
		// 4 register ranges, all writable from unlocked menu mode; once
		// the menu writes bit 6 of $0000-$1FFF the mapper locks down and
		// only the MBC1-compatible fields (rom_bank_low / ram_bank_low /
		// mbc1_mode + ram_enable) remain writable.
		if (addr < 0x2000)
		{
			s.ram_enable = ((value & 0x0F) == 0x0A);
			if (!s.mmm01_locked)
			{
				s.mmm01_ram_bank_mask = static_cast<uint8_t>(value >> 4);
				if (value & 0x40)
				{
					s.mmm01_locked      = true;
					s.mmm01_just_locked = true;
				}
			}
		}
		else if (addr < 0x4000)
		{
			if (!s.mmm01_locked)
			{
				s.mmm01_rom_bank_mid = static_cast<uint8_t>(value >> 5);
			}
			const uint8_t lowmask = static_cast<uint8_t>(s.mmm01_rom_bank_mask << 1);
			s.mmm01_rom_bank_low = static_cast<uint8_t>(
			    (s.mmm01_rom_bank_low & lowmask)
			  | (value & static_cast<uint8_t>(~lowmask)));
		}
		else if (addr < 0x6000)
		{
			s.mmm01_ram_bank_low = static_cast<uint8_t>(
			    value | static_cast<uint8_t>(~s.mmm01_ram_bank_mask));
			if (!s.mmm01_locked)
			{
				s.mmm01_ram_bank_high     = static_cast<uint8_t>((value >> 2) & 0x03);
				s.mmm01_rom_bank_high     = static_cast<uint8_t>((value >> 4) & 0x03);
				s.mmm01_mbc1_mode_disable = (value & 0x40) != 0;
			}
		}
		else if (addr < 0x8000)
		{
			if (!s.mmm01_mbc1_mode_disable)
			{
				s.mmm01_mbc1_mode = (value & 0x01) != 0;
			}
			if (!s.mmm01_locked)
			{
				s.mmm01_rom_bank_mask  = static_cast<uint8_t>((value >> 2) & 0x0F);
				s.mmm01_multiplex_mode = (value & 0x40) != 0;
			}
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (!s.ram_enable) break;
			WriteSram(c, ((Mmm01RamBank(s) & 0x0F) * 0x2000u) + (addr - 0xA000u), value);
		}
		break;

	case MbcType::HuC1:
		if (addr < 0x2000)
		{
			// Value $0E selects the IR register at $A000-$BFFF; any other
			// value selects cart RAM. HuC1 RAM has no separate enable, so
			// the select line is modeled directly in ram_enable.
			s.ram_enable = ((value & 0x0F) != 0x0E);
		}
		else if (addr < 0x4000)
		{
			s.rom_bank = value & 0x3F;
		}
		else if (addr < 0x6000)
		{
			s.ram_bank = value & 0x03;
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			// RAM mode → cart RAM; IR mode → IR LED, which we discard.
			if (!s.ram_enable) break;
			WriteSram(c, ((s.ram_bank & 0x03) * 0x2000u) + (addr - 0xA000u), value);
		}
		break;

	case MbcType::HuC3:
		if (addr < 0x2000)
		{
			// $A000-$BFFF window mode; RAM is read/write only in mode $A.
			s.rtc_select = value & 0x0F;
			s.ram_enable = (s.rtc_select == 0x0A);
		}
		else if (addr < 0x4000)
		{
			s.rom_bank = value & 0x7F;   // 7-bit, MBC5-style (no 0->1 remap)
		}
		else if (addr < 0x6000)
		{
			s.ram_bank = value & 0x03;
		}
		else if (addr < 0x8000)
		{
			// $6000-$7FFF: no effect on HuC3.
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			switch (s.rtc_select)
			{
				case 0xA: WriteSram(c, ((s.ram_bank & 0x03) * 0x2000u) + (addr - 0xA000u), value); break;
				case 0xB: Huc3Command(s, value); break;  // RTC command (executes immediately)
				default:  break;                          // RO RAM / semaphore / IR / undefined
			}
		}
		break;

	case MbcType::TAMA5:
		if (addr >= 0xA000 && addr < 0xC000)
		{
			if (addr & 1)
			{
				s.rtc_select = static_cast<uint8_t>(value & 0x0F);
			}
			else
			{
				const uint8_t v = static_cast<uint8_t>(value & 0x0F);
				switch (s.rtc_select)
				{
					case 0x0: s.rom_bank = (s.rom_bank & 0xF0u) | v; break;
					case 0x1: s.rom_bank = (s.rom_bank & 0x0Fu) | (static_cast<uint32_t>(v) << 4); break;
					case 0x4: s.sachen_outer_bank = static_cast<uint8_t>((s.sachen_outer_bank & 0xF0) | v); break;
					case 0x5: s.sachen_outer_bank = static_cast<uint8_t>((s.sachen_outer_bank & 0x0F) | (v << 4)); break;
					case 0x6: s.sachen_outer_mask = static_cast<uint8_t>((s.sachen_outer_mask & 0x0F) | (v << 4)); break;
					case 0x7:
						s.sachen_outer_mask = static_cast<uint8_t>((s.sachen_outer_mask & 0xF0) | v);
						Tama5Trigger(c);
						break;
					default: break;
				}
			}
		}
		break;

	case MbcType::M161:
		// Write-once page register: the first write to $0000-$7FFF latches the
		// selected 32 KiB page; later writes (e.g. a sub-game's own $2000 bank
		// poke) are ignored. mbc1_mode is reused as the "latched" flag.
		if (addr < 0x8000 && !s.mbc1_mode)
		{
			s.rom_bank  = value & 0x07;
			s.mbc1_mode = true;
		}
		break;

	case MbcType::Camera:
		if (addr < 0x2000)
		{
			s.ram_enable = ((value & 0x0F) == 0x0A);
		}
		else if (addr < 0x4000)
		{
			s.rom_bank = value & 0x3F;
		}
		else if (addr < 0x6000)
		{
			s.mbc1_mode = (value & 0x10) != 0;
			if (!s.mbc1_mode) s.ram_bank = value & 0x0F;
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (s.mbc1_mode)
			{
				const uint16_t r = static_cast<uint16_t>((addr - 0xA000u) & 0x7F);
				if (r < 0x36)
				{
					c.camera_regs[r] = value;
					if (r == 0 && (value & 0x01)) GbCameraCapture(c);
				}
				break;
			}
			if (!s.ram_enable) break;
			WriteSram(c, ((s.ram_bank & 0x0F) * 0x2000u) + (addr - 0xA000u), value);
		}
		break;

	case MbcType::MBC6:
		if (addr < 0x0400)
		{
			s.ram_enable = (value == 0x0A);
		}
		else if (addr < 0x0800)
		{
			s.ram_bank = value;
		}
		else if (addr < 0x0C00)
		{
			s.mmm01_rom_bank_mid = value;
		}
		else if (addr >= 0x2000 && addr < 0x2800)
		{
			s.rom_bank = value;
		}
		else if (addr >= 0x2800 && addr < 0x3000)
		{
			s.mbc1_mode = (value & 0x08) != 0;
		}
		else if (addr >= 0x3000 && addr < 0x3800)
		{
			s.mmm01_rom_bank_low = value;
		}
		else if (addr >= 0x3800 && addr < 0x4000)
		{
			s.mmm01_mbc1_mode = (value & 0x08) != 0;
		}
		else if (addr >= 0xA000 && addr < 0xB000)
		{
			if (s.ram_enable && !c.sram.empty())
				WriteSram(c, (s.ram_bank * 0x1000u) + (addr - 0xA000u), value);
		}
		else if (addr >= 0xB000 && addr < 0xC000)
		{
			if (s.ram_enable && !c.sram.empty())
				WriteSram(c, (s.mmm01_rom_bank_mid * 0x1000u) + (addr - 0xB000u), value);
		}
		break;

	case MbcType::MBC7:
		if (addr < 0x2000)
		{
			s.ram_enable = (value == 0x0A);
		}
		else if (addr < 0x4000)
		{
			s.rom_bank = value;
		}
		else if (addr < 0x6000)
		{
			s.mbc1_mode = (value == 0x40);
		}
		else if (addr >= 0xA000 && addr < 0xC000)
		{
			if (!s.ram_enable || !s.mbc1_mode || addr >= 0xB000) break;
			switch ((addr >> 4) & 0xF)
			{
				case 0: if (value == 0x55) s.rtc_latch = false; break;
				case 1: if (value == 0xAA) s.rtc_latch = true;  break;
				case 8: Mbc7EepromClock(c, value); break;
				default: break;
			}
		}
		break;

	default:
		break;
	}
}

void MbcNotifyHighWrite(MbcState &s, uint16_t /*addr*/, uint8_t value)
{
	if (s.type != MbcType::SachenMMC1) return;
	if (!s.sachen_locked) return;
	// Per Tauwasser's RE the unlock sequence is specifically value $31
	// written to addresses with A15 set. Filtering on $31 keeps the SGB
	// boot ROM's VRAM clear (writes garbage A) and packet-protocol
	// $FF00 traffic (writes $00/$20/$30) from prematurely unlocking.
	if (value != 0x31) return;
	if (s.sachen_unlock_ctr < 0x30)
	{
		if (++s.sachen_unlock_ctr >= 0x30) s.sachen_locked = false;
	}
}

} // namespace SGB
