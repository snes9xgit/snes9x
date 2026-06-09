/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gb_cart.h"

#include <cstdio>
#include <cstring>

namespace SGB {

namespace {

// Translate the header byte at 0x0149 into the physical RAM size.
// Values per Pan Docs: 0=none, 1=2KB (rare), 2=8KB, 3=32KB, 4=128KB, 5=64KB.
uint32_t DecodeRamSize(uint8_t code)
{
	switch (code)
	{
		case 0: return 0;
		case 1: return   2 * 1024;
		case 2: return   8 * 1024;
		case 3: return  32 * 1024;
		case 4: return 128 * 1024;
		case 5: return  64 * 1024;
		default: return 0;
	}
}

// Cart-type → MBC + battery/rtc/rumble flags. Returns false for unknown.
bool DecodeCartType(uint8_t code, MbcType &mbc, bool &battery, bool &rtc, bool &rumble)
{
	battery = rtc = rumble = false;
	switch (code)
	{
		case 0x00: mbc = MbcType::None; return true;
		case 0x01: mbc = MbcType::MBC1; return true;
		case 0x02: mbc = MbcType::MBC1; return true;
		case 0x03: mbc = MbcType::MBC1; battery = true; return true;
		case 0x05: mbc = MbcType::MBC2; return true;
		case 0x06: mbc = MbcType::MBC2; battery = true; return true;
		case 0x08: mbc = MbcType::None; return true;           // ROM+RAM
		case 0x09: mbc = MbcType::None; battery = true; return true;
		case 0x0B: mbc = MbcType::MMM01; return true;
		case 0x0C: mbc = MbcType::MMM01; return true;
		case 0x0D: mbc = MbcType::MMM01; battery = true; return true;
		case 0x0F: mbc = MbcType::MBC3; battery = true; rtc = true; return true;
		case 0x10: mbc = MbcType::MBC3; battery = true; rtc = true; return true;
		case 0x11: mbc = MbcType::MBC3; return true;
		case 0x12: mbc = MbcType::MBC3; return true;
		case 0x13: mbc = MbcType::MBC3; battery = true; return true;
		case 0x19: mbc = MbcType::MBC5; return true;
		case 0x1A: mbc = MbcType::MBC5; return true;
		case 0x1B: mbc = MbcType::MBC5; battery = true; return true;
		case 0x1C: mbc = MbcType::MBC5; rumble = true; return true;
		case 0x1D: mbc = MbcType::MBC5; rumble = true; return true;
		case 0x1E: mbc = MbcType::MBC5; rumble = true; battery = true; return true;
		case 0x20: mbc = MbcType::MBC6; battery = true; return true;
		case 0x22: mbc = MbcType::MBC7; rumble = true; battery = true; return true;
		case 0xFD: mbc = MbcType::TAMA5; battery = true; rtc = true; return true;
		case 0xFE: mbc = MbcType::HuC3; battery = true; rtc = true; return true;
		case 0xFF: mbc = MbcType::HuC1; battery = true; return true;
		default:                                 return false;
	}
}

// Sachen unlicensed multicarts (4B-007 etc.) hide the Nintendo logo under
// an address bit-permutation across 0x0104-0x01FF. If the standard logo
// slot doesn't match but the permuted bytes do, this is a Sachen cart and
// we route it through the Sachen mapper regardless of the header byte at
// 0x0147 (which is forged — 4B-007 reports MBC1 + 32KB).
constexpr uint8_t kGbNintendoLogo[48] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

inline uint16_t SachenScrambleAddr(uint16_t addr)
{
	return static_cast<uint16_t>((addr & ~0x53u)
	                           | ((addr >> 6) & 0x01u)
	                           | ((addr >> 3) & 0x02u)
	                           | ((addr << 3) & 0x10u)
	                           | ((addr << 6) & 0x40u));
}

bool LooksLikeSachenScrambledLogo(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x0200) return false;
	for (int i = 0; i < 48; ++i)
	{
		const uint16_t a = SachenScrambleAddr(static_cast<uint16_t>(0x0104 + i));
		if (rom[a] != kGbNintendoLogo[i]) return false;
	}
	return true;
}

bool SachenHeaderRunsRaw(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x0200) return false;
	auto rd = [&](uint16_t a) { return rom[SachenScrambleAddr(a)]; };
	const uint8_t e0 = rd(0x0100), e1 = rd(0x0101), e2 = rd(0x0102), e3 = rd(0x0103);
	uint16_t target;
	if (e0 == 0xC3)                    target = static_cast<uint16_t>(e1 | (e2 << 8));
	else if (e0 == 0x00 && e1 == 0xC3) target = static_cast<uint16_t>(e2 | (e3 << 8));
	else                               return false;
	return target < 0x0200;
}

// Logo-less Sachen single-game carts (e.g. Beast Fighter) carry no Nintendo
// logo anywhere — not even under the address swap — so LooksLikeSachenScrambledLogo
// can't see them and DecodeCartType rejects their forged $0147. Identify them
// structurally instead: every Sachen cart presents a *valid Game Boy header*
// once its A0/A6 + A1/A4 address swap is undone — a NOP;JP (or JP) entry into
// ROM plus a header checksum that matches $014D (the standard $0134-$014C
// algorithm the DMG boot uses). A non-GB ROM has essentially no chance of
// satisfying both the entry shape and the checksum, so no logo bytes are needed
// to recognise the cart.
bool LooksLikeSachenScrambledHeader(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x0200) return false;
	auto rd = [&](uint16_t a) { return rom[SachenScrambleAddr(a)]; };
	uint16_t entry;
	if (rd(0x0100) == 0x00 && rd(0x0101) == 0xC3)
		entry = static_cast<uint16_t>(rd(0x0102) | (rd(0x0103) << 8));
	else if (rd(0x0100) == 0xC3)
		entry = static_cast<uint16_t>(rd(0x0101) | (rd(0x0102) << 8));
	else
		return false;
	if (entry >= 0x8000) return false;
	uint8_t sum = 0;
	for (uint16_t a = 0x0134; a <= 0x014C; ++a) sum = static_cast<uint8_t>(sum - rd(a) - 1);
	return sum == rd(0x014D);
}

// MMM01: the menu lives in the LAST 32 KiB of ROM and carries its own
// valid Nintendo logo at offset 0x0104 within that bank — that's how
// real hardware powers on into the menu (upper ROM address lines are
// forced to 1 until the menu unlocks the mapper). The forged $0147
// byte at file offset 0 lies (Mani carts report MBC1/MBC3), so we
// identify the cart by the trailing-bank logo.
bool LooksLikeMmm01(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x10000) return false;          // need >= 64 KiB
	if (rom.size() % 0x8000)  return false;          // and a 32 KiB-aligned size
	const size_t base = rom.size() - 0x8000 + 0x0104;
	if (base + 48 > rom.size()) return false;
	for (int i = 0; i < 48; ++i)
	{
		if (rom[base + i] != kGbNintendoLogo[i]) return false;
	}
	return true;
}

// M161 (Mani 4-in-1 "Tetris Set"): a write-once register selects one of up to
// eight raw 32 KiB pages into $0000-$7FFF — the menu is page 0, the sub-games
// follow. The $0147 byte is forged (this cart claims MBC3+TIMER+BATTERY), so
// identify it structurally: a second Nintendo logo at the first sub-game page
// ($8000) that no single-game cart carries.
bool LooksLikeM161(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x10000) return false;          // need >= menu + one game
	if (rom.size() % 0x8000)  return false;          // 32 KiB-page aligned
	auto logo_at = [&](size_t base) {
		if (base + 0x0104 + 48 > rom.size()) return false;
		for (int i = 0; i < 48; ++i)
			if (rom[base + 0x0104 + i] != kGbNintendoLogo[i]) return false;
		return true;
	};
	if (rom[0x8148] != 0x00) return false;           // page 1 must be a 32 KiB game, not a banked 2-in-1 header
	return logo_at(0x0000) && logo_at(0x8000);
}

// MBC1 multicart (MBC1M): a "menu + games" compilation where the 2-bit BANK2
// register drives ROM A18-A19 (instead of A19-A20), so it selects a 256 KiB
// game slot and BANK1 keeps only its low 4 bits. The header still reports plain
// MBC1, so identify it structurally: a second Nintendo logo at the start of the
// bank-0x10 slot ($40000) that a single-game cart never carries.
bool LooksLikeMbc1Multicart(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x44000) return false;          // need at least slot 1's header
	for (int i = 0; i < 48; ++i)
		if (rom[0x40104 + i] != kGbNintendoLogo[i]) return false;
	return true;
}

// Duz "2-in-1" multicart (e.g. Pokemon Red/Blue): a multi-MB ROM that reports
// plain MBC3 but carries a boot menu in banks 0-1 and each sub-game starting at
// a 32 KiB page boundary, each with its own Nintendo logo. The menu programs the
// ROM base via a register port in the SRAM window. Identify it by the second
// logo at $8000 (which a single game never carries) over a banked sub-game
// header — the same marker M161 rejects, but here the cart is genuinely MBC3.
bool LooksLikeDuzMulticart(const std::vector<uint8_t> &rom)
{
	if (rom.size() < 0x80000) return false;
	if (rom[0x8148] == 0x00)  return false;          // $8000 is a real banked game, not a 32 KiB M161 page
	for (int i = 0; i < 48; ++i)
	{
		if (rom[0x0104 + i] != kGbNintendoLogo[i]) return false;
		if (rom[0x8104 + i] != kGbNintendoLogo[i]) return false;
	}
	return true;
}

std::string MakeSavPath(const std::string &rom_path)
{
	if (rom_path.empty()) return {};
	std::string p = rom_path;
	const size_t dot   = p.find_last_of('.');
	const size_t slash = p.find_last_of("/\\");
	// Only strip extension if the dot is part of the filename, not a directory name.
	if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
	{
		p.resize(dot);
	}
	p += ".sav";
	return p;
}

} // anonymous

bool SachenBootLogoByte(const Cart &c, uint16_t addr, uint8_t &out)
{
	if (c.mbc.type != MbcType::SachenMMC1) return false;
	if (addr < 0x0104u || addr > 0x0133u) return false;
	out = kGbNintendoLogo[addr - 0x0104u];
	return true;
}

bool CartLoad(Cart &c, const uint8_t *data, size_t size, const char *path)
{
	// Minimum viable ROM must at least cover the header + one full bank.
	if (size < 0x0150 || data == nullptr) return false;

	c.rom.assign(data, data + size);

	// ----- Header parse -----
	CartHeader &h = c.header;

	// Title is 16 bytes starting at 0x0134. On CGB carts the last 4 are
	// repurposed for manufacturer code + CGB flag; the CGB flag byte lives
	// at 0x0143 either way so reading it separately is correct for both.
	std::memcpy(h.title, &c.rom[0x0134], 16);
	h.title[16] = 0;
	// Strip non-printable tail (padding is usually 0x00 or 0xFF).
	for (int i = 15; i >= 0; --i)
	{
		uint8_t b = static_cast<uint8_t>(h.title[i]);
		if (b == 0 || b < 0x20 || b >= 0x7F) h.title[i] = 0;
		else                                  break;
	}

	h.cgb_flag  = c.rom[0x0143];
	h.sgb_flag  = c.rom[0x0146];
	h.cart_type = c.rom[0x0147];

	uint8_t rom_code = c.rom[0x0148];
	h.rom_size  = (rom_code <= 8)
	              ? static_cast<uint32_t>(32u * 1024u * (1u << rom_code))
	              : 0;

	uint8_t ram_code = c.rom[0x0149];
	h.ram_size  = DecodeRamSize(ram_code);

	h.dest_code  = c.rom[0x014A];
	h.header_sum = c.rom[0x014D];
	h.global_sum = static_cast<uint16_t>(
		(static_cast<uint16_t>(c.rom[0x014E]) << 8) | c.rom[0x014F]);

	// Header checksum: x = 0; for i in 0x0134..0x014C: x = x - rom[i] - 1;
	{
		uint8_t sum = 0;
		for (int i = 0x0134; i <= 0x014C; ++i)
			sum = static_cast<uint8_t>(sum - c.rom[i] - 1);
		h.checksum_ok = (sum == h.header_sum);
	}

	// ----- MBC selection -----
	// Unlicensed/multicart detection takes priority — both Sachen and
	// Mani-style MMM01 carts forge their $0147 byte (Sachen claims MBC1,
	// Mani claims MBC1/MBC3), so we identify them by structural markers
	// instead. Sachen: bit-permuted logo at $0104-$01FF. MMM01: an
	// additional Nintendo logo at the last 32 KiB of ROM (the boot menu).
	if (LooksLikeSachenScrambledLogo(c.rom))
	{
		c.mbc.type      = MbcType::SachenMMC1;
		c.has_battery   = false;
		c.has_rtc       = false;
		c.has_rumble    = false;
		c.sachen_runs_raw = SachenHeaderRunsRaw(c.rom);
	}
	else if (LooksLikeMmm01(c.rom))
	{
		c.mbc.type     = MbcType::MMM01;
		// The "real" header for an MMM01 cart lives in the menu bank at
		// the end of ROM, not at file offset 0 (which holds a sub-game
		// header that may be partially forged). Re-read the cart_type /
		// rom_size / ram_size bytes from there so RAM allocation and
		// downstream display reflect what real hardware sees first.
		const size_t menu_base = c.rom.size() - 0x8000;
		h.cart_type = c.rom[menu_base + 0x0147];
		const uint8_t menu_rom_code = c.rom[menu_base + 0x0148];
		h.rom_size = (menu_rom_code <= 8)
		           ? static_cast<uint32_t>(32u * 1024u * (1u << menu_rom_code))
		           : static_cast<uint32_t>(c.rom.size());
		h.ram_size = DecodeRamSize(c.rom[menu_base + 0x0149]);
		// Honor battery from the menu-bank header (cart-type $0D is
		// MMM01+RAM+BATTERY); Mani forgeries report MBC1/MBC3 there
		// so fall back to false unless we see the real MMM01 type.
		c.has_battery = (h.cart_type == 0x0D);
		c.has_rtc     = false;
		c.has_rumble  = false;
	}
	else if (LooksLikeM161(c.rom))
	{
		c.mbc.type    = MbcType::M161;
		c.has_battery = false;
		c.has_rtc     = false;
		c.has_rumble  = false;
	}
	else if (DecodeCartType(h.cart_type, c.mbc.type, c.has_battery, c.has_rtc, c.has_rumble))
	{
		// Recognized standard cart type — fields set by DecodeCartType.
	}
	else if (LooksLikeSachenScrambledHeader(c.rom))
	{
		// Logo-less Sachen single-game cart: forged $0147 and no stored logo,
		// but a valid GB header hides under the Sachen address swap. The
		// scrambled header is the cart's real loader, so the xform stays on.
		c.mbc.type        = MbcType::SachenMMC1;
		c.has_battery     = false;
		c.has_rtc         = false;
		c.has_rumble      = false;
		c.sachen_runs_raw = false;
	}
	else
	{
		c.rom.clear();
		return false;
	}
	c.mbc1_multicart = (c.mbc.type == MbcType::MBC1) && LooksLikeMbc1Multicart(c.rom);
	c.duz_multicart  = (c.mbc.type == MbcType::MBC3) && LooksLikeDuzMulticart(c.rom);
	MbcReset(c.mbc);

	// ----- Cart RAM allocation -----
	// Initialize fresh SRAM to $FF. Real hardware SRAM cells with no
	// battery (or first-ever boot with a fresh battery) read as $FF —
	// not zero — and several games use that as the "no save / new game"
	// sentinel. Initial D Gaiden is one: with a zero-init SRAM the game
	// thinks a save exists and walks an in-game-data code path that reads
	// strings out of cart RAM that the new-game flow would have populated,
	// so the dialog box renders empty. Match BGB/mGBA/SameBoy here.
	if (c.mbc.type == MbcType::MBC2)
	{
		// MBC2 has 512 x 4-bit internal RAM regardless of the header RAM
		// size. Real MBC2 internal RAM also defaults to $FF — but only the
		// low nibble is wired, so we'll see $F upper-bits at read time.
		c.sram.assign(512, 0xFF);
	}
	else if (c.mbc.type == MbcType::TAMA5)
	{
		// TAMA5 carts report no RAM in the header ($0149 = 0) but carry a
		// 32-byte EEPROM addressed by the mapper's 5-bit register interface.
		c.sram.assign(0x20, 0xFF);
	}
	else if (c.mbc.type == MbcType::MBC7)
	{
		c.sram.assign(0x100, 0xFF);
	}
	else if (h.ram_size > 0)
	{
		c.sram.assign(h.ram_size, 0xFF);
	}

	c.path = path ? path : "";

	// Pull battery state off disk if available.
	if (c.has_battery) CartLoadBattery(c);

	return true;
}

void CartUnload(Cart &c)
{
	c.rom.clear();
	c.sram.clear();
	c.header      = {};
	c.path.clear();
	c.has_battery    = false;
	c.has_rtc        = false;
	c.has_rumble     = false;
	c.sram_dirty     = false;
	c.mbc1_multicart = false;
	c.duz_multicart  = false;
	MbcReset(c.mbc);
	c.mbc.type    = MbcType::None;
}

bool CartSaveBatteryToPath(const Cart &c, const char *path)
{
	if (!c.has_battery || c.sram.empty() || !path || !*path) return false;

	FILE *f = fopen(path, "wb");
	if (!f) return false;

	const size_t written = fwrite(c.sram.data(), 1, c.sram.size(), f);
	fclose(f);
	return written == c.sram.size();
}

bool CartLoadBatteryFromPath(Cart &c, const char *path)
{
	if (!c.has_battery || c.sram.empty() || !path || !*path) return false;

	FILE *f = fopen(path, "rb");
	if (!f) return false;

	// Cap at the allocated SRAM; ignore any trailing RTC bytes for now
	// (proper RTC persistence lands with the P2 follow-up).
	const size_t want = c.sram.size();
	const size_t got  = fread(c.sram.data(), 1, want, f);
	fclose(f);
	return got == want;
}

bool CartSaveBattery(const Cart &c)
{
	if (c.path.empty()) return false;
	const std::string sav = MakeSavPath(c.path);
	return CartSaveBatteryToPath(c, sav.c_str());
}

bool CartLoadBattery(Cart &c)
{
	if (c.path.empty()) return false;
	const std::string sav = MakeSavPath(c.path);
	return CartLoadBatteryFromPath(c, sav.c_str());
}

} // namespace SGB
