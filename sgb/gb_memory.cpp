/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "gb_memory.h"
#include "gb_cart.h"
#include "gb_cpu.h"
#include "gb_ppu.h"
#include "gb_apu.h"
#include "gb_timer.h"
#include "gb_joypad.h"
#include "gb_mbc.h"
#include "sgb.h"

#include <cstring>

namespace SGB {

namespace {
SerialByteCallback g_serial_cb = nullptr;
uint8_t            g_dma_last  = 0xFF;  // 0xFF46 last written byte — register reads echo this.
bool               g_dma_vram_bypass = false;
}

inline bool VramBlocked(const Memory &m)
{
	return !g_dma_vram_bypass && m.ppu &&
	       m.ppu->mode == PpuMode::Transfer && (m.ppu->lcdc & 0x80);
}

inline bool CramBlocked(const Memory &m)
{
	return m.ppu && m.ppu->cgb &&
	       m.ppu->mode == PpuMode::Transfer && (m.ppu->lcdc & 0x80) &&
	       m.ppu->mode_clock > GB_MODE3_SETUP_DOTS + m.ppu->mode3_sprite_stall;
}

void SetSerialCallback(SerialByteCallback cb) { g_serial_cb = cb; }

void MemReset(Memory &m, bool cgb)
{
	std::memset(m.wram, 0, sizeof m.wram);
	std::memset(m.hram, 0, sizeof m.hram);
	m.ie             = 0;
	m.if_            = 0xE1;   // bits 5-7 always set, VBlank latent
	m.serial_data    = 0;
	m.serial_control = 0;
	// Boot ROM staging — zeroed on reset. S9xSGBLoadBootROM fills these
	// in from the user-provided sgb.boot.rom / sgb2.boot.rom before the
	// GB CPU starts. boot_rom_enabled stays false until LoadBootROM sets it.
	std::memset(m.boot_rom, 0, sizeof m.boot_rom);
	m.boot_rom_enabled = false;
	g_dma_last         = cgb ? 0x00 : 0xFF;

	m.svbk         = 1;
	m.key1_armed   = false;
	m.double_speed = false;
	m.hdma1 = m.hdma2 = m.hdma3 = m.hdma4 = 0;
	m.hdma5        = 0xFF;
	m.hdma_src = m.hdma_dst = m.hdma_len = 0;
	m.hdma_active  = false;
	m.hdma_hblank_latch = false;
}

static uint8_t ReadIO(Memory &m, uint16_t addr);
static void    WriteIO(Memory &m, uint16_t addr, uint8_t value);
static void    DoOamDma(Memory &m, uint8_t value);
static void    HdmaTrigger(Memory &m, uint8_t value);

namespace {
inline uint32_t VramBankBase(const Memory &m)
{
	return (m.ppu && (m.ppu->vbk & 1)) ? 0x2000u : 0u;
}

inline uint32_t WramBankBase(const Memory &m)
{
	if (m.ppu && m.ppu->cgb)
	{
		const uint8_t b = m.svbk & 0x07;
		return static_cast<uint32_t>(b ? b : 1) * 0x1000u;
	}
	return 0x1000u;
}

inline void CgbWritePalette(uint8_t *pal, uint8_t &idx, uint8_t value, bool store)
{
	if (store) pal[idx & 0x3F] = value;
	if (idx & 0x80) idx = static_cast<uint8_t>((idx & 0x80) | ((idx + 1) & 0x3F));
}
} // namespace

uint8_t MemRead(Memory &m, uint16_t addr)
{
	// Boot ROM overlay — first 256 bytes mirror the DMG/SGB boot ROM
	// while it's still enabled. The boot code writes 0x01 to 0xFF50
	// as its final act, which clears boot_rom_enabled and exposes the
	// cart bytes underneath.
	if (m.boot_rom_enabled && addr < 0x0100)
	{
		return m.boot_rom[addr];
	}
	// While the boot ROM is mapped, a Sachen cart's mapper supplies the Nintendo
	// logo at 0x0104-0x0133 so the DMG/SGB boot logo check passes — what the
	// hardware does (logo-less single-game carts store none). Gated on
	// boot_rom_enabled, so once the boot hands off the running game reads its
	// real address-swapped header and the cart's own self-check still matches.
	if (m.boot_rom_enabled && m.cart)
	{
		uint8_t logo;
		if (SachenBootLogoByte(*m.cart, addr, logo)) return logo;
	}
	if (addr < 0x8000)
	{
		return m.cart ? MbcRead(m.cart->mbc, m.cart->rom, m.cart->sram, addr, m.cart->mbc1_multicart) : 0xFF;
	}
	if (addr < 0xA000)
	{
		if (!m.ppu || VramBlocked(m)) return 0xFF;
		return m.ppu->vram[(addr - 0x8000) + VramBankBase(m)];
	}
	if (addr < 0xC000)
	{
		return m.cart ? MbcRead(m.cart->mbc, m.cart->rom, m.cart->sram, addr, m.cart->mbc1_multicart) : 0xFF;
	}
	if (addr < 0xD000)
	{
		return m.wram[addr - 0xC000];
	}
	if (addr < 0xE000)
	{
		return m.wram[WramBankBase(m) + (addr - 0xD000)];
	}
	if (addr < 0xF000)
	{
		return m.wram[addr - 0xE000];  // Echo of C000-CFFF
	}
	if (addr < 0xFE00)
	{
		return m.wram[WramBankBase(m) + (addr - 0xF000)];  // Echo of D000-DDFF
	}
	if (addr < 0xFEA0)
	{
		return m.ppu ? m.ppu->oam[addr - 0xFE00] : 0xFF;
	}
	if (addr < 0xFF00)
	{
		return 0xFF;  // unusable
	}
	if (addr < 0xFF80)
	{
		return ReadIO(m, addr);
	}
	if (addr < 0xFFFF)
	{
		return m.hram[addr - 0xFF80];
	}
	return m.ie;
}

void MemWrite(Memory &m, uint16_t addr, uint8_t value)
{
	if (addr < 0x8000)
	{
		if (m.cart) MbcWrite(*m.cart, addr, value);
		return;
	}
	if (m.cart && m.cart->mbc.type == MbcType::SachenMMC1)
	{
		MbcNotifyHighWrite(m.cart->mbc, addr, value);
	}
	if (addr < 0xA000)
	{
		if (m.ppu)
		{
			if (VramBlocked(m)) return;
			m.ppu->vram[(addr - 0x8000) + VramBankBase(m)] = value;
			m.ppu->vram_writes++;
		}
		return;
	}
	if (addr < 0xC000)
	{
		if (m.cart) MbcWrite(*m.cart, addr, value);
		return;
	}
	if (addr < 0xD000)
	{
		m.wram[addr - 0xC000] = value;
		return;
	}
	if (addr < 0xE000)
	{
		m.wram[WramBankBase(m) + (addr - 0xD000)] = value;
		return;
	}
	if (addr < 0xF000)
	{
		m.wram[addr - 0xE000] = value;
		return;
	}
	if (addr < 0xFE00)
	{
		m.wram[WramBankBase(m) + (addr - 0xF000)] = value;
		return;
	}
	if (addr < 0xFEA0)
	{
		if (m.ppu) m.ppu->oam[addr - 0xFE00] = value;
		return;
	}
	if (addr < 0xFF00)
	{
		return;  // unusable
	}
	if (addr < 0xFF80)
	{
		WriteIO(m, addr, value);
		return;
	}
	if (addr < 0xFFFF)
	{
		m.hram[addr - 0xFF80] = value;
		return;
	}
	m.ie = value;
}

uint16_t MemRead16(Memory &m, uint16_t addr)
{
	uint16_t lo = MemRead(m, addr);
	uint16_t hi = MemRead(m, static_cast<uint16_t>(addr + 1));
	return static_cast<uint16_t>(lo | (hi << 8));
}

void MemWrite16(Memory &m, uint16_t addr, uint16_t value)
{
	MemWrite(m, addr,                              static_cast<uint8_t>(value & 0xFF));
	MemWrite(m, static_cast<uint16_t>(addr + 1),  static_cast<uint8_t>((value >> 8) & 0xFF));
}

// ---------------------------------------------------------------------------
// I/O register dispatch
// ---------------------------------------------------------------------------

static uint8_t ReadIO(Memory &m, uint16_t addr)
{
	switch (addr)
	{
		case 0xFF00: return m.joypad ? JoypadRead(*m.joypad) : 0xFF;
		case 0xFF01: return m.serial_data;
		case 0xFF02: return static_cast<uint8_t>((m.serial_control & 0x81) | 0x7E);
		case 0xFF04: case 0xFF05: case 0xFF06: case 0xFF07:
			return m.timer ? TimerRead(*m.timer, addr) : 0xFF;
		case 0xFF0F: return static_cast<uint8_t>(m.if_ | 0xE0);
		case 0xFF46: return g_dma_last;
		case 0xFF4D:
			return (m.ppu && m.ppu->cgb)
				? static_cast<uint8_t>((m.double_speed ? 0x80 : 0x00) |
				                       (m.key1_armed   ? 0x01 : 0x00) | 0x7E)
				: 0xFF;
		case 0xFF4F:
			return (m.ppu && m.ppu->cgb) ? static_cast<uint8_t>(m.ppu->vbk | 0xFE) : 0xFF;
		case 0xFF55:
			return (m.ppu && m.ppu->cgb) ? m.hdma5 : 0xFF;
		case 0xFF68:
			return (m.ppu && m.ppu->cgb) ? m.ppu->bcps : 0xFF;
		case 0xFF69:
			return (m.ppu && m.ppu->cgb) ? m.ppu->bg_pal[m.ppu->bcps & 0x3F] : 0xFF;
		case 0xFF6A:
			return (m.ppu && m.ppu->cgb) ? m.ppu->ocps : 0xFF;
		case 0xFF6B:
			return (m.ppu && m.ppu->cgb) ? m.ppu->obj_pal[m.ppu->ocps & 0x3F] : 0xFF;
		case 0xFF70:
			return (m.ppu && m.ppu->cgb) ? static_cast<uint8_t>(m.svbk | 0xF8) : 0xFF;
	}
	if (addr >= 0xFF10 && addr <= 0xFF3F)
	{
		return m.apu ? ApuRead(*m.apu, addr, m.ppu && m.ppu->cgb) : 0xFF;
	}
	if (addr >= 0xFF40 && addr <= 0xFF4B)
	{
		return m.ppu ? PpuReadReg(*m.ppu, addr) : 0xFF;
	}
	return 0xFF;
}

static void WriteIO(Memory &m, uint16_t addr, uint8_t value)
{
	switch (addr)
	{
		case 0xFF00:
			if (m.joypad) JoypadWrite(*m.joypad, value);
			// Feed SGB command-packet sniffer. Benign when SGB mode inactive.
			S9xSGBOnJoyserWrite(value);
			return;
		case 0xFF01:
			m.serial_data = value;
			return;
		case 0xFF02:
			m.serial_control = value;
			// Internal clock (bit 0 = 1) completes instantly with no peer:
			// push the byte to the observer callback and fire the serial IRQ,
			// then clear the start bit. External clock (bit 0 = 0) has no
			// partner clocking bits in, so bit 7 stays set and no IRQ fires —
			// matching real DMG with a disconnected link cable. Games like
			// Tetris Plus rely on this silence to detect "no link partner".
			// SB latches $FF: disconnected MISO floats high, so each clock
			// shifts in a 1. Alleyway's serial-IRQ input loop depends on this.
			if ((value & 0x81) == 0x81)
			{
				if (g_serial_cb) g_serial_cb(m.serial_data);
				m.serial_data    = 0xFF;
				m.if_            = static_cast<uint8_t>(m.if_ | IRQ_SERIAL);
				m.serial_control = static_cast<uint8_t>(value & 0x7F);
			}
			return;
		case 0xFF04: case 0xFF05: case 0xFF06: case 0xFF07:
			if (m.timer) TimerWrite(*m.timer, addr, value);
			return;
		case 0xFF0F:
			m.if_ = static_cast<uint8_t>((value & 0x1F) | 0xE0);
			return;
		case 0xFF46:
			g_dma_last = value;
			DoOamDma(m, value);
			return;
		case 0xFF50:
			// Boot-ROM disable: writing any non-zero value (canonically 0x01)
			// latches off the boot overlay so the cart bytes at 0x0000-0x00FF
			// become visible. Real hardware: bit 0 disables, can't be re-enabled.
			if (value != 0)
			{
				m.boot_rom_enabled = false;
				if (m.cart && m.cart->sachen_runs_raw) m.cart->mbc.sachen_locked = false;
			}
			return;
		case 0xFF4D:
			if (m.ppu && m.ppu->cgb) m.key1_armed = (value & 0x01) != 0;
			return;
		case 0xFF4F:
			if (m.ppu && m.ppu->cgb) m.ppu->vbk = value & 0x01;
			return;
		case 0xFF51: if (m.ppu && m.ppu->cgb) m.hdma1 = value; return;
		case 0xFF52: if (m.ppu && m.ppu->cgb) m.hdma2 = value; return;
		case 0xFF53: if (m.ppu && m.ppu->cgb) m.hdma3 = value; return;
		case 0xFF54: if (m.ppu && m.ppu->cgb) m.hdma4 = value; return;
		case 0xFF55: if (m.ppu && m.ppu->cgb) HdmaTrigger(m, value); return;
		case 0xFF68: if (m.ppu && m.ppu->cgb) m.ppu->bcps = value; return;
		case 0xFF69: if (m.ppu && m.ppu->cgb) CgbWritePalette(m.ppu->bg_pal, m.ppu->bcps, value, !CramBlocked(m)); return;
		case 0xFF6A: if (m.ppu && m.ppu->cgb) m.ppu->ocps = value; return;
		case 0xFF6B: if (m.ppu && m.ppu->cgb) CgbWritePalette(m.ppu->obj_pal, m.ppu->ocps, value, !CramBlocked(m)); return;
		case 0xFF70: if (m.ppu && m.ppu->cgb) m.svbk = value & 0x07; return;
	}
	if (addr >= 0xFF10 && addr <= 0xFF3F)
	{
		if (m.apu) ApuWrite(*m.apu, addr, value, m.ppu && m.ppu->cgb);
		return;
	}
	if (addr >= 0xFF40 && addr <= 0xFF4B)
	{
		if (m.ppu) PpuWriteReg(*m.ppu, m, addr, value);
		return;
	}
	// Remaining I/O addresses (CGB regs, etc.) are ignored.
}

// ---------------------------------------------------------------------------
// OAM DMA — instant copy of 160 bytes from (value << 8) to OAM.
// Real HW takes 160 T-cycles and blocks non-HRAM access during that time.
// P3 may tighten the timing; most games are unaffected by the simplification.
// ---------------------------------------------------------------------------

static void DoOamDma(Memory &m, uint8_t value)
{
	if (!m.ppu) return;
	const uint16_t src = static_cast<uint16_t>(value << 8);
	g_dma_vram_bypass = true;
	for (int i = 0; i < 0xA0; ++i)
	{
		m.ppu->oam[i] = MemRead(m, static_cast<uint16_t>(src + i));
	}
	g_dma_vram_bypass = false;
}

static void DoGdma(Memory &m, uint16_t src, uint16_t dst, uint16_t blocks)
{
	const uint32_t n = static_cast<uint32_t>(blocks) * 0x10u;
	g_dma_vram_bypass = true;
	for (uint32_t i = 0; i < n; ++i)
	{
		const uint8_t b = MemRead(m, static_cast<uint16_t>(src + i));
		MemWrite(m, static_cast<uint16_t>(0x8000 + ((dst + i) & 0x1FFF)), b);
	}
	g_dma_vram_bypass = false;
}

static void HdmaTransferBlock(Memory &m)
{
	g_dma_vram_bypass = true;
	for (uint32_t i = 0; i < 0x10; ++i)
	{
		const uint8_t b = MemRead(m, static_cast<uint16_t>(m.hdma_src + i));
		MemWrite(m, static_cast<uint16_t>(0x8000 + ((m.hdma_dst + i) & 0x1FFF)), b);
	}
	g_dma_vram_bypass = false;
	m.hdma_src = static_cast<uint16_t>(m.hdma_src + 0x10);
	m.hdma_dst = static_cast<uint16_t>(m.hdma_dst + 0x10);
	m.hdma1 = static_cast<uint8_t>(m.hdma_src >> 8);
	m.hdma2 = static_cast<uint8_t>(m.hdma_src & 0xF0);
	m.hdma3 = static_cast<uint8_t>(m.hdma_dst >> 8);
	m.hdma4 = static_cast<uint8_t>(m.hdma_dst & 0xF0);
	--m.hdma_len;
	if (m.hdma_len == 0)
	{
		m.hdma_active = false;
		m.hdma5       = 0xFF;
	}
	else
	{
		m.hdma5 = static_cast<uint8_t>((m.hdma_len - 1) & 0x7F);
	}
}

static void HdmaTrigger(Memory &m, uint8_t value)
{
	const uint16_t src    = static_cast<uint16_t>(((m.hdma1 << 8) | m.hdma2) & 0xFFF0);
	const uint16_t dst    = static_cast<uint16_t>(((m.hdma3 << 8) | m.hdma4) & 0x1FF0);
	const uint16_t blocks = static_cast<uint16_t>((value & 0x7F) + 1);

	if (value & 0x80)
	{
		m.hdma_src    = src;
		m.hdma_dst    = dst;
		m.hdma_len    = blocks;
		m.hdma_active = true;
		m.hdma5       = static_cast<uint8_t>(value & 0x7F);
		if (m.ppu && !(m.ppu->lcdc & 0x80))
		{
			HdmaTransferBlock(m);
		}
		else if (m.ppu && m.ppu->mode == PpuMode::HBlank &&
		         !m.hdma_hblank_latch)
		{
			HdmaTransferBlock(m);
			m.hdma_hblank_latch = true;
		}
	}
	else if (m.hdma_active)
	{
		m.hdma_active = false;
		m.hdma5       = static_cast<uint8_t>(0x80 | ((m.hdma_len - 1) & 0x7F));
	}
	else
	{
		DoGdma(m, src, dst, blocks);
		const uint16_t end_src = static_cast<uint16_t>(src + blocks * 0x10);
		const uint16_t end_dst = static_cast<uint16_t>(dst + blocks * 0x10);
		m.hdma1 = static_cast<uint8_t>(end_src >> 8);
		m.hdma2 = static_cast<uint8_t>(end_src & 0xF0);
		m.hdma3 = static_cast<uint8_t>(end_dst >> 8);
		m.hdma4 = static_cast<uint8_t>(end_dst & 0xF0);
		m.hdma5 = 0xFF;
	}
}

void MemHdmaHBlank(Memory &m)
{
	m.hdma_hblank_latch = false;
	if (!m.hdma_active) return;
	HdmaTransferBlock(m);
	m.hdma_hblank_latch = true;
}

void MemHdmaLcdOff(Memory &m)
{
	if (!m.hdma_active) return;
	HdmaTransferBlock(m);
}

} // namespace SGB
