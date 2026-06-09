/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _SGB_GB_MEMORY_H_
#define _SGB_GB_MEMORY_H_

#include <cstdint>

namespace SGB {

struct Cart;
struct Ppu;
struct Apu;
struct Timer;
struct Joypad;

// GB address space — flat 16-bit. All access goes through this one bus.
//   0x0000-0x3FFF  ROM bank 0
//   0x4000-0x7FFF  ROM bank N (MBC-switched)
//   0x8000-0x9FFF  VRAM
//   0xA000-0xBFFF  External RAM (cart)
//   0xC000-0xDFFF  WRAM
//   0xE000-0xFDFF  Echo RAM (mirror of C000-DDFF)
//   0xFE00-0xFE9F  OAM
//   0xFEA0-0xFEFF  Unusable
//   0xFF00-0xFF7F  I/O
//   0xFF80-0xFFFE  HRAM
//   0xFFFF         IE register
struct Memory
{
	Cart   *cart   = nullptr;
	Ppu    *ppu    = nullptr;
	Apu    *apu    = nullptr;
	Timer  *timer  = nullptr;
	Joypad *joypad = nullptr;

	uint8_t wram[0x8000];
	uint8_t hram[0x7F];
	uint8_t ie;             // 0xFFFF
	uint8_t if_;            // 0xFF0F
	uint8_t serial_data;    // 0xFF01 last written byte
	uint8_t serial_control; // 0xFF02 last written; bit 7 stays set on external-clock waits

	// GB boot ROM (256 bytes). Overlays cart ROM at 0x0000-0x00FF while
	// boot_rom_enabled is true; cleared by the first write to 0xFF50.
	// Populated by S9xSGBLoadBootROM in authentic BIOS mode; untouched in
	// BIOS-less mode (boot_rom_enabled stays false, cart is visible from reset).
	uint8_t boot_rom[0x100];
	bool    boot_rom_enabled;

	uint8_t  svbk = 1;            // 0xFF70
	bool     key1_armed   = false;
	bool     double_speed = false;

	uint8_t  hdma1 = 0, hdma2 = 0, hdma3 = 0, hdma4 = 0;  // 0xFF51-0xFF54
	uint8_t  hdma5 = 0xFF;        // 0xFF55 status
	uint16_t hdma_src = 0, hdma_dst = 0, hdma_len = 0;
	bool     hdma_active = false;
	bool     hdma_hblank_latch = false;
};

uint8_t MemRead(Memory &m, uint16_t addr);
void    MemWrite(Memory &m, uint16_t addr, uint8_t value);

// 16-bit helpers assume little-endian, two sequential 8-bit accesses.
uint16_t MemRead16(Memory &m, uint16_t addr);
void     MemWrite16(Memory &m, uint16_t addr, uint16_t value);

void MemReset(Memory &m);

// Transfer one 0x10-byte CGB HDMA block during HBlank. No-op when inactive.
void MemHdmaHBlank(Memory &m);

// LCD turned off outside HBlank with an HBlank HDMA armed: the off edge
// counts as entering HBlank, so one pending block fires (SameBoy GB_lcd_off).
void MemHdmaLcdOff(Memory &m);

// Callback fires each time the CPU initiates a serial transfer (write
// 0x80/0x81 to 0xFF02). The byte passed is whatever was in 0xFF01 at
// the moment. Used by the test harness to capture Blargg output; P6a
// may hook it too. nullptr disables.
using SerialByteCallback = void (*)(uint8_t byte);
void SetSerialCallback(SerialByteCallback cb);

} // namespace SGB

#endif
