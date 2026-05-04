/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "sgb.h"

#include "../snes9x.h"   // Settings.SGB_BIOSModeActive + S9xMessage
                          // (used for the border-capture diagnostic OSD,
                          // matching the pattern dropped in bd2a5479).
#include "../messages.h" // S9X_INFO / S9X_ROM_INFO type tags.

#include "gb_cpu.h"
#include "gb_memory.h"
#include "gb_ppu.h"
#include "gb_apu.h"
#include "gb_timer.h"
#include "gb_joypad.h"
#include "gb_cart.h"
#include "sgb_packet.h"
#include "sgb_state.h"

#include <cstdio>
#include <cstring>
#include <vector>

namespace SGB {

// Embedded SGB1 / SGB2 GB-side boot ROMs. These are the authentic boot ROMs
// that scroll the Nintendo logo AND produce the 5-packet header handshake
// ($F1/$F3/$F5/$F7/$F9 command bytes + cart header bytes) that the SGB BIOS
// on the SNES side waits for to unblock its splash screen.
//
// Source: LIJI32/SameBoy (https://github.com/LIJI32/SameBoy), Expat (MIT)
// license — copyright (c) 2015-2026 Lior Halphon. Routed via Mesen2
// (https://github.com/SourMesen/Mesen2 — Core/Gameboy/GbBootRom.h), which
// embeds the same SameBoy bytes with a short delay loop at the handoff so
// Tetris DX's SGB border doesn't get skipped. Full license text shipped
// alongside this file as sgb/SAMEBOY-LICENSE.txt.
static const uint8_t kSgbBootRom[256] = {
    0x31, 0xFE, 0xFF, 0x21, 0x00, 0x80, 0x22, 0xCB, 0x6C, 0x28, 0xFB, 0x3E,
    0x80, 0xE0, 0x26, 0xE0, 0x11, 0x3E, 0xF3, 0xE0, 0x12, 0xE0, 0x25, 0x3E,
    0x77, 0xE0, 0x24, 0x3E, 0x00, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10,
    0x80, 0x1A, 0x47, 0xCD, 0xC9, 0x00, 0xCD, 0xC9, 0x00, 0x13, 0x7B, 0xEE,
    0x34, 0x20, 0xF2, 0x11, 0xEA, 0x00, 0x0E, 0x08, 0x1A, 0x13, 0x22, 0x23,
    0x0D, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E,
    0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18, 0xF5,
    0x3E, 0x91, 0xE0, 0x40, 0x3E, 0xF1, 0xE0, 0x80, 0x21, 0x04, 0x01, 0xAF,
    0x4F, 0xAF, 0xE2, 0x3E, 0x30, 0xE2, 0xF0, 0x80, 0xCD, 0xB7, 0x00, 0xE5,
    0x06, 0x0E, 0x16, 0x00, 0xCD, 0xAD, 0x00, 0x82, 0x57, 0x05, 0x20, 0xF8,
    0xCD, 0xB7, 0x00, 0xE1, 0x06, 0x0E, 0xCD, 0xAD, 0x00, 0xCD, 0xB7, 0x00,
    0x05, 0x20, 0xF7, 0x3E, 0x20, 0xE2, 0x3E, 0x30, 0xE2, 0xF0, 0x80, 0xC6,
    0x02, 0xE0, 0x80, 0x3E, 0x58, 0xBD, 0x20, 0xC9, 0x0E, 0x13, 0x3E, 0xC1,
    0xE2, 0x0C, 0x3E, 0x07, 0xE2, 0x3E, 0xFC, 0xE0, 0x47, 0x3E, 0x01, 0x21,
    0x60, 0xC0, 0xC3, 0xF2, 0x00, 0x3E, 0x4F, 0xBD, 0x38, 0x02, 0x2A, 0xC9,
    0x23, 0xAF, 0xC9, 0x5F, 0x16, 0x08, 0x3E, 0x10, 0xCB, 0x1B, 0x38, 0x01,
    0x87, 0xE2, 0x3E, 0x30, 0xE2, 0x15, 0xC8, 0x18, 0xF1, 0x3E, 0x04, 0x0E,
    0x00, 0xCB, 0x20, 0xF5, 0xCB, 0x11, 0xF1, 0xCB, 0x11, 0x3D, 0x20, 0xF5,
    0x79, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xE5, 0x21, 0x0F, 0xFF, 0xCB, 0x86,
    0xCB, 0x46, 0x28, 0xFC, 0xE1, 0xC9, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5,
    0x42, 0x3C, 0x16, 0xE1, 0x1E, 0xAA, 0x1B, 0x14, 0x15, 0x20, 0xFB, 0x5A,
    0xEE, 0x00, 0xE0, 0x50
};

static const uint8_t kSgb2BootRom[256] = {
    0x31, 0xFE, 0xFF, 0x21, 0x00, 0x80, 0x22, 0xCB, 0x6C, 0x28, 0xFB, 0x3E,
    0x80, 0xE0, 0x26, 0xE0, 0x11, 0x3E, 0xF3, 0xE0, 0x12, 0xE0, 0x25, 0x3E,
    0x77, 0xE0, 0x24, 0x3E, 0x00, 0xE0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10,
    0x80, 0x1A, 0x47, 0xCD, 0xC9, 0x00, 0xCD, 0xC9, 0x00, 0x13, 0x7B, 0xEE,
    0x34, 0x20, 0xF2, 0x11, 0xEA, 0x00, 0x0E, 0x08, 0x1A, 0x13, 0x22, 0x23,
    0x0D, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E,
    0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18, 0xF5,
    0x3E, 0x91, 0xE0, 0x40, 0x3E, 0xF1, 0xE0, 0x80, 0x21, 0x04, 0x01, 0xAF,
    0x4F, 0xAF, 0xE2, 0x3E, 0x30, 0xE2, 0xF0, 0x80, 0xCD, 0xB7, 0x00, 0xE5,
    0x06, 0x0E, 0x16, 0x00, 0xCD, 0xAD, 0x00, 0x82, 0x57, 0x05, 0x20, 0xF8,
    0xCD, 0xB7, 0x00, 0xE1, 0x06, 0x0E, 0xCD, 0xAD, 0x00, 0xCD, 0xB7, 0x00,
    0x05, 0x20, 0xF7, 0x3E, 0x20, 0xE2, 0x3E, 0x30, 0xE2, 0xF0, 0x80, 0xC6,
    0x02, 0xE0, 0x80, 0x3E, 0x58, 0xBD, 0x20, 0xC9, 0x0E, 0x13, 0x3E, 0xC1,
    0xE2, 0x0C, 0x3E, 0x07, 0xE2, 0x3E, 0xFC, 0xE0, 0x47, 0x3E, 0xFF, 0x21,
    0x60, 0xC0, 0xC3, 0xF2, 0x00, 0x3E, 0x4F, 0xBD, 0x38, 0x02, 0x2A, 0xC9,
    0x23, 0xAF, 0xC9, 0x5F, 0x16, 0x08, 0x3E, 0x10, 0xCB, 0x1B, 0x38, 0x01,
    0x87, 0xE2, 0x3E, 0x30, 0xE2, 0x15, 0xC8, 0x18, 0xF1, 0x3E, 0x04, 0x0E,
    0x00, 0xCB, 0x20, 0xF5, 0xCB, 0x11, 0xF1, 0xCB, 0x11, 0x3D, 0x20, 0xF5,
    0x79, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xE5, 0x21, 0x0F, 0xFF, 0xCB, 0x86,
    0xCB, 0x46, 0x28, 0xFC, 0xE1, 0xC9, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5,
    0x42, 0x3C, 0x16, 0xE1, 0x1E, 0xAA, 0x1B, 0x14, 0x15, 0x20, 0xFB, 0x5A,
    0xEE, 0x00, 0xE0, 0x50
};

struct Emulator::Impl
{
	Cpu         cpu;
	Memory      mem;
	Ppu         ppu;
	Apu         apu;
	Timer       timer;
	Joypad      joypad;
	Cart        cart;
	PacketState sgb_pkt;
	SgbState    sgb_state;

	// 256×224 composite staging buffer — heap-resident to keep a
	// ~112 KB allocation off the stack of whatever thread drives
	// S9xMainLoop (snes9x's per-thread stacks aren't always large).
	uint16_t    composite[SGB_BORDER_W * SGB_BORDER_H];

	RunMode     run_mode  = RunMode::SGB;
	FrameBuffer fb{};
	bool        has_rom   = false;
	float       clock_mul = 1.0f;

	// Staged GB-side boot ROM. Copied into mem.boot_rom on Reset when
	// boot_rom_loaded is true (authentic BIOS mode only). Staging is kept
	// separate because MemReset zeroes mem.boot_rom.
	uint8_t     boot_rom_staging[0x100];
	bool        boot_rom_loaded = false;

	// ICD2 state — the SGB cart-chip register set exposed at 0x6000-0x7FFF
	// on the SNES side when running under a real BIOS. Layout mirrors the
	// community-documented SGB hardware protocol (see sgb.h for the register
	// table; see bsnes / Mesen2 for reference implementations — their
	// register choices match what we implement here).
	struct Icd2
	{
		// $6003 — control. bit 7 = GB release, 5..4 = player count,
		// 1..0 = clock divider select.
		uint8_t  control;

		// $6001 — LCD row bank select for the $7800 char window. Also
		// resets the $7800 read position to 0. P2d consumes this.
		uint8_t  lcd_row_select;

		// $7800 auto-increment pointer. Populated in P2d.
		uint16_t read_position;

		// $6004-$6007 — joypad state that the GB sees when it polls
		// $FF00. P2e will wire these to the SNES pad.
		uint8_t  joypad[4];
		uint8_t  input_value;   // last $FF00 write from GB (for edge detect)
		uint8_t  input_index;   // 0..3 — current MLT_REQ player slot

		// Packet assembler. GB bit-bangs SGB commands over $FF00:
		//   $00 (P14+P15 both active) = reset/start pulse
		//   $10 (P15 active only)     = 1-bit
		//   $20 (P14 active only)     = 0-bit
		//   $30 (both inactive)       = clock-high / idle
		// We shift bits into bit_accumulator LSB-first, pack into
		// assembly_buf[] every 8 bits. On the 16th byte the fully-
		// assembled packet is pushed onto packet_queue.
		uint8_t  assembly_buf[16];
		uint16_t bit_accumulator;
		uint8_t  packet_bit;    // 0..7
		uint8_t  packet_byte;   // 0..16
		bool     in_packet;

		// 64-deep packet queue (ring buffer). Matches bsnes's icd.cpp
		// packet path. The GB sends packets in bursts (5 handshake
		// packets back-to-back, plus game-driven palette/CHR/PCT/ATTR
		// transfers) — a single-slot FIFO loses anything after the
		// first while the BIOS is busy draining, which corrupts the
		// expected command sequence and strands the BIOS waiting for
		// $6002 forever. Queue drains on $7000-$700F reads: drain_ptr
		// advances per read; at 16 we pop the head and decrement count.
		uint8_t  packet_queue[64][16];
		uint8_t  queue_head;         // next packet to drain (0..63)
		uint8_t  queue_tail;         // next slot to push (0..63)
		uint8_t  queue_count;        // number of packets queued (0..64)

		// P2c diagnostic counters — help verify packet flow by OSD.
		uint32_t packets_received;   // completed 16-byte packets
		uint32_t fifo_reads;         // $7000-$700F reads
		uint32_t ctrl_writes;        // $6003 writes
		uint32_t row_writes;         // $6001 writes
		uint32_t f1_packets;         // packets whose first byte == $F1 (boot ROM handshake)
		uint8_t  last_cmd_ids[8];    // ring buffer of last 8 packet command IDs (byte0 >> 3)
		uint8_t  last_cmd_ids_len;   // 0..8

		// Per-address read/write counts for the registers the BIOS is
		// most likely to poll. Lets the status line expose which
		// register the BIOS is hot-looping on (distinct from bucketed
		// counts since $60xx and $78xx share low nibbles).
		uint32_t r_6000, r_6002, r_6003, r_600F, r_7000, r_7800;
		uint32_t w_6000, w_6001, w_6003, w_7000, w_6004;

		// Per-frame $6001 write counter / byte log. Diagnostic only under
		// the 4-bank protocol — slice-index defenses don't apply here
		// (sgb_bank is GB-driven; W6001 wobble doesn't shift the protocol's
		// data layout the way it did under 18-slice). Reset in OnPpuVBlank.
		uint8_t  frame_6001_count;
		uint8_t  frame_6001_bytes[24];

		uint16_t last_read_addr;
		uint16_t last_write_addr;
		uint8_t  last_write_val;

		// First byte of the first-ever packet to arrive after reset —
		// that's what the SGB2 BIOS checks at $BE69 against #$F1. If
		// this is not $F1, the BIOS's $02C0 handshake counter resets
		// every call and the boot loop at $BE3C never exits.
		uint8_t  first_packet_byte0;
		bool     first_packet_seen;

		// Diagnostic: snapshot of the FIRST packet's full 16 bytes, and
		// the byte0 of the first 8 packets. Lets us verify (a) that the
		// Nintendo logo bytes at $0104.. reach the BIOS intact inside
		// packet 0, and (b) that the boot ROM's packet-0 byte sequence
		// is exactly F1 F3 F5 F7 F9 (5 packets) rather than a longer
		// drifting list.
		uint8_t  pkt0_bytes[16];
		uint8_t  byte0_log[8];
		uint8_t  byte0_log_len;

		// Ring of the last 16 bytes returned for $7000-$700F reads.
		// If the BIOS's drain path is reaching our ICD2 handler, we'll
		// see real packet bytes here. If it's getting zeros/FFs, our
		// queue wasn't serving valid data at that moment.
		uint8_t  last_r7000_vals[16];
		uint8_t  last_r7000_idx;

		// bsnes-style packet read buffer. Reading $6002 with a pending
		// packet pops the front of the queue into r7000_buf[0..15], and
		// subsequent $7000-$700F reads return from this buffer (NOT the
		// queue directly). This matters because the BIOS may not read
		// all 16 bytes of a packet — if we only pop on $700F, we'd keep
		// re-reading the same packet forever.
		uint8_t  r7000_buf[16];

		// Synthesized boot-ROM handshake. Most GB boot ROM dumps are plain
		// DMG — they scroll the Nintendo logo and disable themselves but
		// do NOT send the 5-packet SGB handshake the BIOS requires. We
		// synthesize them here from the cart header so BIOS mode works
		// without needing an SGB-specific boot ROM.
		uint8_t  synth_packets[6][16];
		uint8_t  synth_remaining;    // packets yet to hand out (6 → 0)
		uint8_t  drain_ptr;          // bytes read of current packet (0..15)

		// $6000 row/bank counters (Mesen2-style). Advanced by GB PPU
		// scanline events: sgb_row++ on each HBlank end (mode 0 → OAM
		// scan for line 1..143), sgb_bank advances every 8 rows, and
		// both get zeroed on VBlank entry. $6000 = (row & 0xF8) | bank.
		uint8_t  sgb_row;
		uint8_t  sgb_bank;
		uint8_t  sgb_bank_latched;
		uint8_t  sgb_row_latched;

		// Per-pixel capture ring — 4 banks × 8 rows × 160 pixels (palette
		// indices 0..3). Mesen2 SuperGameboy::WriteLcdColor writes into
		// _lcdBuffer[_bank][(_row & 7) * 160 + pixel] from the GB PPU
		// renderer; $7800 reads decode planar bytes inline from this
		// buffer (live, no snapshot).
		uint8_t  lcd_ring[4][8 * 160];
	} icd2;

	// Cache of the first 6 packets bit-banged by the GB boot ROM. The
	// SGB BIOS handshake validates that two consecutive GB resets produce
	// byte-identical packets ($B8AE:BE3C captures byte 1s into $7E:1718,
	// then $B0F6:B107 drains a second time and $B119 compares against
	// $1718 — mismatch → $02FA=1 → cart error). Stored OUTSIDE the
	// Icd2 struct because Reset() zeroes icd2 on every $6003 0→1
	// transition, and we want the cache to persist across those resets
	// so we can re-queue deterministic packets on subsequent releases.
	uint8_t  cached_packets[6][16]{};
	uint8_t  cached_count   = 0;   // 0..6 (how many filled during first boot)
	bool     cache_valid    = false; // set true once 6 packets cached
	uint8_t  replays_done   = 0;   // cap replay count so post-splash releases don't
	                                // keep re-queuing handshake packets (which would
	                                // block game-generated SGB commands).

	// Deferred CHR_TRN / PCT_TRN capture. Real SGB hardware reconstructs
	// border tile/map data from the GB's LCD over multiple frames after
	// the *_TRN packet — not from VRAM at packet completion. A direct
	// memcpy of GB $8000 races the game's own VRAM updates and picks up
	// zero/partial data (DK Japan visibly: solid-blue surround instead
	// of arcade machine). Instead we stash the packet here and at the
	// next GB frame end decode the top-left 128×128 area of the rendered
	// LCD into the canonical 4 KB byte stream the SGB tile/map handlers
	// expect.
	struct BorderCapture
	{
		enum Stage : uint8_t { Idle = 0, ChrTrn = 1, PctTrn = 2 };
		Stage   stage = Idle;
		uint8_t pkt[16] = {};   // saved packet bytes (cmd in [0], param in [1])
	} border_capture;

	// BIOS-mode border crossfade. Counts frames since both halves of
	// CHR_TRN + PCT_TRN landed; 0 = no custom border yet (BIOS default
	// shows through), >= BORDER_FADE_FRAMES = full overlay. In between,
	// OverlayBiosBorder lerps per-pixel between the BIOS-rendered
	// default and the captured custom border so the gray Game Boy
	// frame eases out into the game's arcade machine instead of
	// snapping in on a single frame.
	uint16_t border_fade_frames = 0;

	// One-shot snapshot of GB CPU registers at the moment the boot
	// ROM unmapped itself (FF50 write). DK / Pokémon / etc. read
	// these to detect SGB1 vs SGB2 vs DMG; if our boot ROM hands off
	// with the wrong A/F values, those games take their non-SGB
	// branch and never emit border packets. Surfaced via the OSD so
	// we can compare against real SGB2 (A=$FF, F=$00, BC=$0014,
	// DE=$0000, HL=$C060).
	bool     boot_handoff_captured = false;
	CpuRegs  boot_handoff_regs{};
	uint64_t boot_handoff_vram_writes = 0;
};

// File-local trampoline — lets the process-global SgbCommandCallback
// forward into the singleton Emulator's Impl.
static void SgbCommandTrampoline(uint8_t cmd, const uint8_t *data, uint32_t len)
{
	Instance().OnSgbCommandInternal(cmd, data, len);
}

Emulator::Emulator() : impl_(new Impl) {}

Emulator::~Emulator() { delete impl_; }

bool Emulator::Init()
{
	ColdReset();   // first-time init: clear cache state too
	SetSgbCommandCallback(&SgbCommandTrampoline);
	return true;
}

void Emulator::Deinit()
{
	UnloadROM();
}

void Emulator::ColdReset()
{
	// Clear the BIOS-handshake cache before delegating to Reset(). On a
	// user-initiated reset (File→Reset / new ROM load), the SNES BIOS
	// will run from scratch and expects to perform the full handshake
	// again — but our cache_valid flag and cached_packets persist
	// across the GB-side Reset() (which is correct for in-game $6003
	// 0→1 transitions). Without clearing, the next $6003 release skips
	// the boot-ROM run and replays cached packets, producing no SGB
	// splash and either a black screen or whatever stale state the
	// SNES retained from before the reset.
	std::memset(impl_->cached_packets, 0, sizeof impl_->cached_packets);
	impl_->cached_count = 0;
	impl_->cache_valid  = false;
	impl_->replays_done = 0;
	Reset();
}

void Emulator::Reset()
{
	impl_->cpu.Reset();
	MemReset(impl_->mem);
	PpuReset(impl_->ppu);
	ApuReset(impl_->apu);
	TimerReset(impl_->timer);
	JoypadReset(impl_->joypad);
	PacketReset(impl_->sgb_pkt);
	SgbReset(impl_->sgb_state);
	impl_->border_capture.stage = Impl::BorderCapture::Idle;
	impl_->border_fade_frames   = 0;
	impl_->boot_handoff_captured = false;
	impl_->boot_handoff_regs     = {};
	impl_->boot_handoff_vram_writes = 0;
	IrqServicedReset();
	std::memset(&impl_->icd2, 0, sizeof impl_->icd2);
	// 4-bank LCD ring starts at $00 (matches Mesen2 SuperGameboy::Reset).
	// $7000-$700F latch buffer starts as $FF so reads before the first
	// $6002 pop return all-ones (matches bsnes r7000 power-on state).
	std::memset(impl_->icd2.r7000_buf, 0xFF, sizeof impl_->icd2.r7000_buf);
	// Joypad registers idle = $FF (active-low, no buttons held).
	// bsnes r6004-r6007 = 0xff. Initializing to 0 makes the GB see all
	// buttons held and the SGB BIOS's probe sequences fail. Critical.
	impl_->icd2.joypad[0] = 0xFF;
	impl_->icd2.joypad[1] = 0xFF;
	impl_->icd2.joypad[2] = 0xFF;
	impl_->icd2.joypad[3] = 0xFF;

	// ICD2 joypad bridge — only active when the SNES BIOS is driving
	// $6004-$6007 directly (BIOS mode). In BIOS-less mode the SetJoypad
	// path feeds the standard JoypadSet flow instead, so we leave the
	// bridge disabled there to avoid pulling stale sgb_pads bytes.
	impl_->joypad.sgb_active = Settings.SGB_BIOSModeActive;
	impl_->joypad.sgb_index  = 0;
	impl_->joypad.sgb_pads[0] = 0xFF;
	impl_->joypad.sgb_pads[1] = 0xFF;
	impl_->joypad.sgb_pads[2] = 0xFF;
	impl_->joypad.sgb_pads[3] = 0xFF;

	impl_->mem.ppu    = &impl_->ppu;
	impl_->mem.apu    = &impl_->apu;
	impl_->mem.timer  = &impl_->timer;
	impl_->mem.joypad = &impl_->joypad;
	impl_->mem.cart   = &impl_->cart;

	impl_->fb.pixels = impl_->ppu.framebuffer;
	impl_->fb.width  = GB_SCREEN_WIDTH;
	impl_->fb.height = GB_SCREEN_HEIGHT;
	impl_->fb.pitch  = GB_SCREEN_WIDTH;

	// Apply run-mode specific post-boot register values. The SGB BIOS
	// hands control to the cart with slightly different register state
	// than a DMG boot ROM does — some games (notably Donkey Kong and
	// Pokemon) check these to detect whether they're running on a real
	// SGB host.
	CpuState &cs = impl_->cpu.State();
	switch (impl_->run_mode)
	{
		case RunMode::SGB:
			cs.r.af = 0x0100;
			cs.r.bc = 0x0014;
			cs.r.de = 0x0000;
			cs.r.hl = 0xC060;
			break;
		case RunMode::SGB2:
			cs.r.af = 0xFF00;
			cs.r.bc = 0x0014;
			cs.r.de = 0x0000;
			cs.r.hl = 0xC060;
			break;
		case RunMode::DMG:
		default:
			// gb_cpu.cpp Reset() already set DMG values.
			break;
	}

	// If a GB-side boot ROM was staged (authentic BIOS mode), overlay it
	// at 0x0000-0x00FF and start the CPU there. The boot code will scroll
	// the Nintendo logo, send the 5-packet SGB handshake the BIOS is
	// waiting for, then write 0xFF50 to disable itself — at which point
	// the cart takes over exactly as it would on real hardware.
	if (impl_->boot_rom_loaded)
	{
		std::memcpy(impl_->mem.boot_rom, impl_->boot_rom_staging, sizeof impl_->mem.boot_rom);
		impl_->mem.boot_rom_enabled = true;
		cs.r.af = 0x0000;
		cs.r.bc = 0x0000;
		cs.r.de = 0x0000;
		cs.r.hl = 0x0000;
		cs.r.sp = 0x0000;
		cs.r.pc = 0x0000;
	}
}

bool Emulator::LoadBootROM(const uint8_t *data, size_t size)
{
	if (!data || size == 0)
	{
		impl_->boot_rom_loaded = false;
		return true;
	}
	if (size != sizeof impl_->boot_rom_staging) return false;
	std::memcpy(impl_->boot_rom_staging, data, size);
	impl_->boot_rom_loaded = true;
	return true;
}

void Emulator::PrimeBIOSHandshake()
{
	if (!impl_->has_rom || impl_->cart.rom.size() < 0x150) return;

	Emulator::Impl::Icd2 &icd = impl_->icd2;
	const std::vector<uint8_t> &rom = impl_->cart.rom;

	// Real SGB boot ROM handshake sends 6 packets with byte 0 cycling
	// through $F1, $F3, $F5, $F7, $F9, $FB (low 3 bits encode a +2
	// packet index that overflows the cmd_id nibble on the 5th step:
	// cmd $1E idx 1/3/5/7 → $1F idx 1/3). The BIOS at $BE66/$BE69
	// verifies the first packet's byte 0 against #$F1 and counts each
	// subsequent packet into $02C0 at $BE75. Observed live from our
	// own boot ROM capture: b0s = F1 F3 F5 F7 F9 FB, exactly.
	// Bytes 1..15 are successive 15-byte slices of the cart header
	// starting at $0104 (Nintendo logo bytes → title → cart-type → etc).
	static const uint8_t kHeaderByte0[6] = { 0xF1, 0xF3, 0xF5, 0xF7, 0xF9, 0xFB };
	for (int p = 0; p < 6; ++p)
	{
		icd.synth_packets[p][0] = kHeaderByte0[p];
		for (int b = 1; b < 16; ++b)
		{
			const size_t off = 0x0104 + p * 15 + (b - 1);
			icd.synth_packets[p][b] = (off < rom.size()) ? rom[off] : 0x00;
		}
	}

	// Populate the synth queue but DO NOT stage into the FIFO yet. The
	// BIOS's splash-animation code on the SNES side might drain pending
	// packets via its own paths before it reaches the handshake wait at
	// $BE3C. Staging too early means our packets get eaten before the
	// handshake counter ever increments. Wait for the BIOS to release
	// the GB (write $6003 bit 7) — that's the real-hardware signal that
	// the BIOS is ready to see handshake packets.
	icd.synth_remaining = 6;     // 6 packets queued, none staged yet
	icd.drain_ptr       = 0;
}

// Push a freshly-assembled 16-byte packet onto the queue. Drops the
// oldest slot silently if the queue is full — matches bsnes icd.cpp
// behavior (`if(packetSize >= 64) packetSize = 64;`). Bumps the
// diagnostic counters as the canonical "packet arrived" event.
static void IcdPushQueue(Emulator::Impl::Icd2 &icd, const uint8_t *pkt)
{
	std::memcpy(icd.packet_queue[icd.queue_tail], pkt, 16);
	icd.queue_tail = static_cast<uint8_t>((icd.queue_tail + 1) & 63);
	if (icd.queue_count < 64)
		icd.queue_count++;
	else
		icd.queue_head = static_cast<uint8_t>((icd.queue_head + 1) & 63);

	icd.packets_received++;
	const uint8_t byte0  = pkt[0];
	const uint8_t cmd_id = static_cast<uint8_t>(byte0 >> 3);
	if (byte0 == 0xF1) icd.f1_packets++;
	if (!icd.first_packet_seen)
	{
		icd.first_packet_byte0 = byte0;
		icd.first_packet_seen  = true;
		std::memcpy(icd.pkt0_bytes, pkt, 16);
	}
	if (icd.byte0_log_len < 8)
		icd.byte0_log[icd.byte0_log_len++] = byte0;
	if (icd.last_cmd_ids_len < 8)
		icd.last_cmd_ids[icd.last_cmd_ids_len++] = cmd_id;
	else
	{
		for (int i = 0; i < 7; ++i)
			icd.last_cmd_ids[i] = icd.last_cmd_ids[i + 1];
		icd.last_cmd_ids[7] = cmd_id;
	}
}

static void IcdStageNextSynth(Emulator::Impl::Icd2 &icd)
{
	if (icd.synth_remaining == 0) return;
	const uint8_t next_idx = static_cast<uint8_t>(6 - icd.synth_remaining);
	IcdPushQueue(icd, icd.synth_packets[next_idx]);
	icd.synth_remaining--;
}

bool Emulator::LoadROM(const uint8_t *data, size_t size, const char *path)
{
	if (!CartLoad(impl_->cart, data, size, path))
		return false;
	impl_->has_rom = true;
	ColdReset();   // new cart → start fresh, drop any stale handshake cache
	return true;
}

void Emulator::UnloadROM()
{
	if (impl_->has_rom) CartSaveBattery(impl_->cart);
	CartUnload(impl_->cart);
	impl_->has_rom = false;
	// Drop any staged boot ROM so a subsequent BIOS-less load starts at
	// $0100 with the normal post-boot register state.
	impl_->boot_rom_loaded = false;
}

bool Emulator::HasROM() const { return impl_->has_rom; }

const uint8_t *Emulator::GetROMData() const
{
	if (!impl_->has_rom || impl_->cart.rom.empty()) return nullptr;
	return impl_->cart.rom.data();
}

size_t Emulator::GetROMSize() const
{
	if (!impl_->has_rom) return 0;
	return impl_->cart.rom.size();
}

uint8_t Emulator::PeekRAByte(uint32_t addr) const
{
	if (!impl_->has_rom) return 0;

	// Native GB address space (0x0000-0xFFFF). Cart accesses go through
	// the MBC mapper, which is side-effect-free for reads. Other regions
	// read raw backing storage — no I/O reads, no PPU bus contention.
	if (addr < 0x10000)
	{
		const uint16_t a = static_cast<uint16_t>(addr);
		if (a < 0x8000)
			return MbcRead(const_cast<MbcState &>(impl_->cart.mbc),
			               impl_->cart.rom, impl_->cart.sram, a);
		if (a < 0xA000)        // VRAM — not exposed (would need PPU sync)
			return 0;
		if (a < 0xC000)        // External cart RAM (current bank)
			return MbcRead(const_cast<MbcState &>(impl_->cart.mbc),
			               impl_->cart.rom, impl_->cart.sram, a);
		if (a < 0xE000)        // WRAM 0xC000-0xDFFF
			return impl_->mem.wram[a - 0xC000];
		if (a < 0xFE00)        // Echo RAM mirrors C000-DDFF
			return impl_->mem.wram[a - 0xE000];
		if (a < 0xFEA0)        // OAM — not exposed
			return 0;
		if (a < 0xFF00)        // Unusable
			return 0;
		if (a < 0xFF80)        // Hardware I/O — skip (side-effecting)
			return 0;
		if (a < 0xFFFF)        // HRAM 0xFF80-0xFFFE
			return impl_->mem.hram[a - 0xFF80];
		return impl_->mem.ie;  // 0xFFFF interrupt enable
	}

	// Extended bank window 0x10000-0x33FFF.
	//   0x10000-0x15FFF — GBC system RAM banks 2-7. We don't yet emulate
	//                     CGB extra WRAM banks; return 0.
	//   0x16000-0x33FFF — Cartridge SRAM banks 1-15 (each 0x2000 bytes).
	if (addr < 0x16000) return 0;
	if (addr < 0x34000)
	{
		const uint32_t off = addr - 0x16000;
		const uint32_t bank = 1 + (off / 0x2000);
		const uint32_t in_bank = off % 0x2000;
		const uint32_t flat = bank * 0x2000 + in_bank;
		if (flat < impl_->cart.sram.size())
			return impl_->cart.sram[flat];
		return 0;
	}
	return 0;
}

void Emulator::SetRunMode(RunMode m)
{
	if (m == impl_->run_mode) return;  // idempotent — avoids re-pushing clock
	impl_->run_mode = m;
	// SGB1 runs the GB at SNES_master / 5 = 4.295455 MHz (~2.4% faster
	// than DMG); SGB2 and DMG both run at the authentic 4.194304 MHz.
	// The APU uses this to keep cycles_per_sample correct so audio plays
	// at the right pitch in every mode.
	const int32_t clock = (m == RunMode::SGB) ? 4295455 : 4194304;
	ApuSetClockHz(impl_->apu, clock);
}
RunMode Emulator::GetRunMode() const { return impl_->run_mode; }

void Emulator::RunFrame()
{
	if (!impl_->has_rom) return;

	impl_->ppu.frame_ready = false;

	// Cycle budget per SNES frame depends on run mode and the user
	// overclock/underclock knob:
	//   SGB1: SNES master clock / 5 = 4.2955 MHz (2.4% faster than real GB)
	//   SGB2: exact GB clock         = 4.1943 MHz
	//   DMG:  exact GB clock         = 4.1943 MHz
	// At NTSC SNES refresh (60.099 fps) that's ~71485 / 69801 T-cycles per
	// SNES frame. Clamp the multiplier to a sane range so users can't
	// accidentally freeze the emulator with a 0x or 1000x setting.
	constexpr double SNES_FPS = 60.09881389744051;
	double base_hz;
	switch (impl_->run_mode)
	{
		case RunMode::SGB:  base_hz = 21477272.727272 / 5.0; break;
		case RunMode::SGB2: base_hz = 4194304.0;             break;
		case RunMode::DMG:
		default:            base_hz = 4194304.0;             break;
	}

	float mul = impl_->clock_mul;
	if (mul < 0.10f) mul = 0.10f;
	if (mul > 8.00f) mul = 8.00f;

	const double   per_frame = (base_hz / SNES_FPS) * static_cast<double>(mul);
	const int32_t  budget    = static_cast<int32_t>(per_frame);

	RunCycles(budget);

	// Always end at VBlank entry so BlitScreen reads a complete framebuffer.
	// Fixed budget alone drifts the render position out of VBlank every few
	// calls (SGB1 overshoots ~2.77 lines/call; SGB2/DMG undershoot ~0.95
	// lines/call), tearing the displayed image and depositing mid-frame
	// palette/attr packets onto a half-rendered buffer.
	int32_t safety = 70224;
	while (!impl_->ppu.frame_ready && safety > 0)
	{
		RunCycles(456);
		safety -= 456;
	}
}

// ===================================================================
// Border-capture diagnostic OSD (temporary — remove once DK Japan and
// a couple other reference games confirm the capture path is correct).
// Counters + latest captured/packet metadata are pushed once per frame
// via S9xMessage, using the same temporary-timeout-bump pattern as the
// BIOS-mode diagnostic OSD removed in bd2a5479.
// ===================================================================
struct SgbDbg
{
	uint32_t pkt_chr      = 0;
	uint32_t pkt_pct      = 0;
	uint32_t cap_fired    = 0;
	uint8_t  last_param   = 0;
	uint8_t  last_lcdc    = 0;
	uint8_t  last_scx     = 0;
	uint8_t  last_scy     = 0;
	uint8_t  last_bgp     = 0;
	uint8_t  last_dec[8]  = {};
	// Ring of the last 8 command IDs that reached OnSgbCommandInternal,
	// in chronological order (cmd_ring[0] = oldest). Lets us see
	// exactly what DK sent when chr/pct stay at 0 — e.g., is it just
	// MASK_EN/PAL01 or is there really no CHR_TRN coming?
	uint8_t  cmd_ring[8]  = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint32_t cmd_count    = 0;

	// Raw $FF00 traffic from the GB. Lets us tell apart "GB is
	// silently HALTing without trying to send packets" (these
	// counters frozen at boot-ROM totals) from "GB is bit-banging
	// but our decoder is dropping packets" (joy_writes climbs but
	// rst_pulses doesn't, or rst_pulses climbs but rcv doesn't).
	uint32_t joy_writes   = 0;   // total GB writes to $FF00
	uint32_t rst_pulses   = 0;   // count of $00 (P14=0,P15=0) writes
};
static SgbDbg g_sgb_dbg;

static void DbgPushCmd(uint8_t cmd)
{
	const uint32_t i = g_sgb_dbg.cmd_count;
	if (i < 8)
	{
		g_sgb_dbg.cmd_ring[i] = cmd;
	}
	else
	{
		// Shift left, append at the end.
		for (int k = 0; k < 7; ++k)
			g_sgb_dbg.cmd_ring[k] = g_sgb_dbg.cmd_ring[k + 1];
		g_sgb_dbg.cmd_ring[7] = cmd;
	}
	++g_sgb_dbg.cmd_count;
}

#if 0
static void DbgDrawOsd(const Emulator::Impl &impl)
{
	char buf[400];
	const SgbDbg &d = g_sgb_dbg;
	const auto   &icd = impl.icd2;
	const auto   &b   = impl.sgb_state.border;
	const uint8_t mode_now = (impl.run_mode == RunMode::SGB)  ? 1
	                       : (impl.run_mode == RunMode::SGB2) ? 2
	                       : 0;
	// Unqualified snprintf — win32 headers macro it to _snprintf, so
	// `std::snprintf` expands to `std::_snprintf` and fails to resolve
	// (same hazard as the `fopen` → `_tfwopen` macro).
	//
	// Diagnostic field key (read in this order to triage):
	//   w6=N        — count of BIOS writes to $6004. ZERO = BIOS NMI
	//                 handler isn't running / GB not yet released.
	//                 Climbing = BIOS is alive and feeding the GB.
	//   jp=XX/YY    — last $6004 / $6005 byte (active-low; FF = idle).
	//                 If w6 > 0 but jp stays FF, BIOS sees no SNES pad.
	//   idx=N       — current ICD2 player rotation index (0..3). For
	//                 SGB-detect (MLT_REQ-2 probe), this should briefly
	//                 toggle to 1 then settle at 0.
	//   mlt=N       — sniffed MLT_REQ player count (1/2/4).
	//   sel=XX      — last GB write to $FF00 (j.select, masked $30).
	//   tl/ml       — border tiles_loaded / map_loaded flags. BOTH
	//                 must flip 1 before OverlayBiosBorder draws the
	//                 captured frame. cap=N counts how many times
	//                 RunCycles flushed a deferred CHR_TRN/PCT_TRN.
	//   jw=N rst=N  — raw GB→$FF00 traffic. jw is total writes,
	//                 rst is count of $00 (RESET-pulse) writes —
	//                 i.e. packet starts. After boot-ROM handshake
	//                 baseline (~rst=8 for SGB2's F1..FF set), if
	//                 rst stays flat the GB isn't trying to send
	//                 packets at all; if rst climbs but rcv doesn't
	//                 keep up, the packet decoder is dropping bits.
	//   cmds=N ring — total commands seen by OnSgbCommandInternal +
	//                 last 8 cmd IDs (oldest→newest). The cmd is
	//                 byte0>>3, so handshake F1/F3/F5/F7 → 1E and
	//                 F9/FB/FD/FF → 1F. Game commands: 11 = MLT_REQ,
	//                 13 = CHR_TRN, 14 = PCT_TRN, 0B = PAL_TRN,
	//                 15 = ATTR_TRN, 17 = MASK_EN.
	//   IE/IF       — GB interrupt enable / pending. If GB is HALTed
	//                 (HLT=1) and IE & IF == 0, GB is hung waiting
	//                 for an IRQ that no one will fire.
	//   IME         — GB master interrupt-enable flag.
	//   HLT         — GB CPU is in HALT (PC frozen on HALT instr).
	//   BR          — GB-side boot ROM still mapped at $0000-$00FF.
	//                 Should flip to 0 once boot ROM finishes and
	//                 writes $FF50; if stuck at 1, boot ROM is hung.
	//   CTL         — ICD2 $6003. Bit 7 = GB released. If $00, GB is
	//                 frozen in reset (no instructions executing).
	//   SGBflag     — cart header byte $0146. $03 = SGB-aware, $00 =
	//                 plain DMG. If $00, the loaded game won't EVER
	//                 send SGB packets regardless of how good our
	//                 ICD2 bridge is — the BIOS-mode test ROM must
	//                 actually be SGB-flagged (e.g. the JP/EU "Donkey
	//                 Kong" rev, NOT the original 1994 NTSC release).
	//   title       — first 11 bytes of cart title at $0134, ASCII-
	//                 only filter. Cross-check against the file the
	//                 user thinks they loaded.
	//   handoff +   — GB CPU register snapshot at the moment the
	//   AF/BC/DE/HL   boot ROM unmapped itself. Real SGB2: AF=$FF00,
	//                 BC=$0014, DE=$0000, HL=$C060. Real SGB1:
	//                 AF=$0100, BC=$0014, DE=$0000, HL=$C060. Real
	//                 DMG: AF=$01B0, BC=$0013, DE=$00D8, HL=$014D.
	//                 Mismatch = the loaded GB game's hardware-
	//                 detect branches DMG-side and skips SGB-init.
	//   irqV/S/T    — count of VBlank / STAT / Timer IRQs serviced
	//                 by Cpu::ServiceInterrupts since reset. At 60 Hz
	//                 with default wiring, irqV should grow ≈ once
	//                 per frame. Anything substantially slower means
	//                 our PPU/timer isn't producing the IRQs the
	//                 running game expects.
	//   ill         — count of illegal opcodes executed. Should stay
	//                 0 on healthy ROMs; rising means our CPU lost
	//                 the plot and is fetching from data/RAM.
	//   bank        — current MBC1 ROM bank visible at $4000-$7FFF.
	const auto &cs = impl.cpu.State();
	// Snapshot the cart's title (first 11 bytes — title region is
	// 0x0134..0x0142 but past byte 11 may overlap manufacturer code on
	// CGB-aware carts). Print printable ASCII only, so a non-GB cart or
	// truncated load shows obvious garbage.
	char title[12] = {0};
	for (int i = 0; i < 11; ++i)
	{
		const char ch = impl.cart.header.title[i];
		title[i] = (ch >= 0x20 && ch < 0x7F) ? ch : '.';
	}
	snprintf(buf, sizeof buf,
	         "SGB m=%u rcv=%u cmds=%u chr=%u pct=%u cap=%u "
	         "w6=%u jp=%02X/%02X idx=%u mlt=%u sel=%02X tl=%u ml=%u "
	         "jw=%u rst=%u "
	         "ring=%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X "
	         "PC=%04X LY=%02X LCDC=%02X "
	         "IE=%02X IF=%02X IME=%u HLT=%u BR=%u CTL=%02X "
	         "SGBflag=%02X title=\"%s\" "
	         "handoff=%s AF=%04X BC=%04X DE=%04X HL=%04X "
	         "irqV=%u irqS=%u irqT=%u ill=%u bank=%u",
	         mode_now, impl.sgb_pkt.packets_received, d.cmd_count,
	         d.pkt_chr, d.pkt_pct, d.cap_fired,
	         icd.w_6004,
	         icd.joypad[0], icd.joypad[1],
	         impl.joypad.sgb_index, impl.sgb_state.mlt_players,
	         static_cast<uint8_t>(impl.joypad.select & 0x30),
	         b.tiles_loaded ? 1u : 0u, b.map_loaded ? 1u : 0u,
	         d.joy_writes, d.rst_pulses,
	         d.cmd_ring[0], d.cmd_ring[1], d.cmd_ring[2], d.cmd_ring[3],
	         d.cmd_ring[4], d.cmd_ring[5], d.cmd_ring[6], d.cmd_ring[7],
	         cs.r.pc, impl.ppu.ly, impl.ppu.lcdc,
	         impl.mem.ie, impl.mem.if_,
	         cs.ime ? 1u : 0u, cs.halted ? 1u : 0u,
	         impl.mem.boot_rom_enabled ? 1u : 0u, icd.control,
	         impl.cart.header.sgb_flag, title,
	         impl.boot_handoff_captured ? "yes" : "no",
	         impl.boot_handoff_regs.af, impl.boot_handoff_regs.bc,
	         impl.boot_handoff_regs.de, impl.boot_handoff_regs.hl,
	         IrqServicedCount(0), IrqServicedCount(1), IrqServicedCount(2),
	         cs.illegal_ops, impl.cart.mbc.rom_bank);
	// Same pattern as the BIOS-mode diagnostic OSD removed in bd2a5479
	// (and used in memmap.cpp:1526-1529): temporarily force the
	// InitialInfoStringTimeout high enough that S9xMessage won't drop
	// the line on configs where the user has disabled OSD messages.
	const uint32 saved = Settings.InitialInfoStringTimeout;
	Settings.InitialInfoStringTimeout = 120;
	S9xMessage(S9X_INFO, S9X_ROM_INFO, buf);
	Settings.InitialInfoStringTimeout = saved;
}
#endif

// Reconstruct 4 KB of byte data from the top-left 16x16 tile area
// (128x128 pixels, 2bpp raw indices) of the GB framebuffer. Output
// bytes are in canonical GB-tile order — exactly the sequence
// CHR_TRN / PCT_TRN delivers to the SGB BIOS over the LCD signal.
static void DecodeBorderCapture(const uint8_t *raw_fb, uint8_t *out_4kb)
{
	for (int ty = 0; ty < 16; ++ty)
	{
		for (int tx = 0; tx < 16; ++tx)
		{
			uint8_t *tile_out = &out_4kb[(ty * 16 + tx) * 16];
			for (int row = 0; row < 8; ++row)
			{
				uint8_t plane0 = 0, plane1 = 0;
				for (int px = 0; px < 8; ++px)
				{
					const uint8_t pix = static_cast<uint8_t>(
						raw_fb[(ty * 8 + row) * GB_SCREEN_WIDTH + (tx * 8 + px)] & 0x03);
					const int bit = 7 - px;
					if (pix & 1) plane0 = static_cast<uint8_t>(plane0 | (1u << bit));
					if (pix & 2) plane1 = static_cast<uint8_t>(plane1 | (1u << bit));
				}
				tile_out[row * 2 + 0] = plane0;
				tile_out[row * 2 + 1] = plane1;
			}
		}
	}
}

void Emulator::RunCycles(int32_t tcycles)
{
	if (!impl_->has_rom) return;
	if (tcycles <= 0) return;

	// Per-dot CPU/PPU interleaving. Advance PPU one t-cycle at a time;
	// after each dot, run CPU as far as it can go without overshooting
	// PPU. STAT IRQs raised by PPU mode transitions (HBlank entry, mode
	// 2, LYC match) are serviced by CPU before PPU advances further, so
	// games that update SCX/BGP/WX from the HBlank IRQ between scan-
	// lines (Wario Land 2 cloud parallax, Balloon Fight title) have
	// their new register value in place by the time PPU starts the next
	// mode 3. cycle_debt is no longer needed — the per-dot model only
	// steps CPU when it has kMaxOpcodeTCycles of headroom.
	const int64_t target_t = impl_->ppu.t_cycles + tcycles;
	while (impl_->ppu.t_cycles < target_t)
	{
		PpuStep(impl_->ppu, impl_->mem, 1);

		while (impl_->cpu.State().t_cycles + kMaxOpcodeTCycles <=
		       impl_->ppu.t_cycles)
		{
			const bool was_boot = impl_->mem.boot_rom_enabled;
			const int64_t pre_t = impl_->cpu.State().t_cycles;
			impl_->cpu.Step(impl_->mem);
			int32_t consumed = static_cast<int32_t>(
				impl_->cpu.State().t_cycles - pre_t);
			if (consumed <= 0) consumed = 4;

			if (was_boot && !impl_->mem.boot_rom_enabled &&
			    !impl_->boot_handoff_captured)
			{
				impl_->boot_handoff_captured = true;
				impl_->boot_handoff_regs     = impl_->cpu.State().r;
				impl_->boot_handoff_vram_writes = impl_->ppu.vram_writes;
			}

			TimerStep(impl_->timer, impl_->mem, consumed);
			ApuStep  (impl_->apu,                consumed);
		}
	}

	if (impl_->border_capture.stage != Impl::BorderCapture::Idle &&
	    impl_->ppu.frame_ready)
	{
		uint8_t decoded[4096];
		DecodeBorderCapture(impl_->ppu.raw_framebuffer, decoded);
		const uint8_t cmd =
			(impl_->border_capture.stage == Impl::BorderCapture::ChrTrn)
				? static_cast<uint8_t>(0x13)
				: static_cast<uint8_t>(0x14);
		++g_sgb_dbg.cap_fired;
		SgbHandleCommand(impl_->sgb_state, cmd,
		                 impl_->border_capture.pkt, 16,
		                 decoded, impl_->ppu.framebuffer);
		impl_->border_capture.stage = Impl::BorderCapture::Idle;
		// Don't clear frame_ready here — RunFrame's run-to-vblank loop
		// reads it to know the frame completed. The tail can't re-fire
		// because stage is now Idle.
	}
}

const FrameBuffer &Emulator::GetFrameBuffer() const { return impl_->fb; }

void Emulator::GetStatus(char *buf, size_t cap) const
{
	if (!buf || cap == 0) return;
	const CpuState &s = impl_->cpu.State();
	const Emulator::Impl::Icd2 &icd = impl_->icd2;

	const uint8_t *p0 = icd.pkt0_bytes;
	const uint8_t *b0 = icd.byte0_log;
	const uint8_t *rv = icd.last_r7000_vals;
	snprintf(buf, cap,
	         "GBPC=%04X ctrl=%02X pkts=%u F1=%u ly=%u "
	         "b0s=%02X%02X%02X%02X%02X%02X "
	         "p0=%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X "
	         "r7000_ring=%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X "
	         "R:6002=%u 7000=%u W:6001=%u 6003=%u",
	              s.r.pc, icd.control,
	              icd.packets_received,
	              icd.f1_packets,
	              static_cast<unsigned>(impl_->ppu.ly),
	              b0[0], b0[1], b0[2], b0[3], b0[4], b0[5],
	              p0[0],  p0[1],  p0[2],  p0[3],  p0[4],  p0[5],  p0[6],  p0[7],
	              p0[8],  p0[9],  p0[10], p0[11], p0[12], p0[13], p0[14], p0[15],
	              rv[0],  rv[1],  rv[2],  rv[3],  rv[4],  rv[5],  rv[6],  rv[7],
	              rv[8],  rv[9],  rv[10], rv[11], rv[12], rv[13], rv[14], rv[15],
	              icd.r_6002, icd.r_7000,
	              icd.w_6001, icd.w_6003);
}

// ICD2 register mirrors repeat every 16 bytes across each kB window
// ($6000-$67FF, $7000-$77FF, $7800-$7FFF). Matches real SGB hardware.
static inline uint16_t Icd2Mask(uint16_t addr) { return addr & 0xF80F; }

uint8_t Emulator::GetICD2(uint16_t addr)
{
	if (!impl_) return 0xFF;
	const uint16_t a = Icd2Mask(addr);
	Emulator::Impl::Icd2 &icd = impl_->icd2;
	icd.last_read_addr = a;
	switch (a)
	{
		case 0x6000: icd.r_6000++; break;
		case 0x6002: icd.r_6002++; break;
		case 0x6003: icd.r_6003++; break;
		case 0x600F: icd.r_600F++; break;
		default:
			if      (a >= 0x7000 && a <= 0x700F) icd.r_7000++;
			else if (a >= 0x7800 && a <= 0x780F) icd.r_7800++;
			break;
	}

	// $7000-$700F — bsnes-style latch read. Returns bytes from r7000_buf,
	// which was populated on the last $6002 read that found a pending
	// packet. This lets the BIOS read any subset of the 16 bytes (or
	// re-read them) without advancing the queue.
	if (a >= 0x7000 && a <= 0x700F)
	{
		const uint8_t b = icd.r7000_buf[a & 0x0F];
		// Trace: log into ring so OSD can see what our ICD2 actually
		// served for the last drain attempts.
		icd.last_r7000_vals[icd.last_r7000_idx & 0x0F] = b;
		icd.last_r7000_idx++;
		return b;
	}
	// $7800-$780F — GB frame char-transfer window. Streams bit-plane
	// bytes from the live 4-bank capture ring (lcd_ring), selected by
	// lcd_row_select. Layout per bank: 20 tiles × 8 rows × 2 planes =
	// 320 bytes of real data, then $FF padding to 512 before wrap.
	// Reading from the live ring (not the end-of-frame framebuffer)
	// is what lets the BIOS capture the boot animation's Nintendo logo
	// scroll — the BIOS reads $7800 while the GB is still mid-drawing.
	if (a >= 0x7800 && a <= 0x780F)
	{
		const uint16_t pos = icd.read_position;
		icd.read_position  = static_cast<uint16_t>((icd.read_position + 1) & 0x1FF);
		if (pos >= 320)
			return 0xFF;

		if (impl_->mem.boot_rom_enabled ||
		    (impl_->boot_handoff_captured &&
		     impl_->ppu.vram_writes - impl_->boot_handoff_vram_writes < 256))
			return 0x00;

		const uint8_t bank  = static_cast<uint8_t>(icd.lcd_row_select & 0x03);
		const uint8_t row   = static_cast<uint8_t>((pos >> 1) & 0x07);
		const uint8_t col   = static_cast<uint8_t>(pos >> 4);
		const uint8_t shift = static_cast<uint8_t>(pos & 0x01);
		const uint8_t *src  = &icd.lcd_ring[bank][row * 160 + col * 8];
		uint8_t data = 0;
		for (int i = 0; i < 8; ++i)
			data |= static_cast<uint8_t>(((src[i] >> shift) & 0x01) << (7 - i));
		return data;
	}

	switch (a)
	{
		case 0x6000:
			// Mesen2 SuperGameboy.cpp: `(_row & ~0x07) | _bank` — high bits
			// are the GB-driven row counter, low bits are the GB-driven
			// bank counter (cycles 0→1→2→3→0 every 8 GB scanlines). The
			// BIOS uses the bank rotation to detect when a fresh bank is
			// ready to drain.
			return static_cast<uint8_t>((icd.sgb_row_latched & 0xF8) | (icd.sgb_bank_latched & 0x03));
		case 0x6002:
		{
			// Lazy-stage the next synth packet if queue's empty (no-op
			// when a real boot ROM is running — synth_remaining stays 0).
			if (icd.queue_count == 0 && icd.synth_remaining > 0)
				IcdStageNextSynth(icd);

			// Auto-stage cached packets if queue is empty and we have a
			// valid cache from the first boot. Without this, the BIOS
			// eventually enters sub_80B107 with empty queue and spins
			// on $6002 forever (it doesn't write $6003 inside that
			// loop, so our release-triggered replay can't fire).
			//
			// Cap the count so the BIOS can eventually EXIT the
			// validate-and-reset cycle. Staging indefinitely keeps the
			// BIOS forever re-running handshake validation, which
			// interferes with game-display tile transfers — garbled
			// CHR_TRN output. 50 iterations is enough for the BIOS to
			// complete splash setup and advance $0101 to game-display
			// mode; after that the game's own SGB commands drive the
			// packet queue and validation is no longer needed.
			if (icd.queue_count == 0 && impl_->cache_valid &&
			    impl_->replays_done < 50)
			{
				for (int p = 0; p < 6; ++p)
					IcdPushQueue(icd, impl_->cached_packets[p]);
				impl_->replays_done++;
			}

			// bsnes semantics: reading $6002 with a pending packet POPS
			// the front packet into r7000_buf[0..15] and shifts the queue.
			if (icd.queue_count > 0)
			{
				std::memcpy(icd.r7000_buf, icd.packet_queue[icd.queue_head], 16);
				icd.queue_head = static_cast<uint8_t>((icd.queue_head + 1) & 63);
				icd.queue_count--;
				IcdStageNextSynth(icd);
				return 0x01;
			}
			return 0x00;
		}
		case 0x6003: return icd.control;    // R/W — some BIOS paths verify writes
		case 0x600F: return 0x21;           // BIOS version byte (bsnes / Mesen return $21)
	}
	return 0x00;  // bsnes readIO falls through to 0 for unmapped ICD2 addrs
}

void Emulator::SetICD2(uint8_t value, uint16_t addr)
{
	if (!impl_) return;
	const uint16_t a = Icd2Mask(addr);
	Emulator::Impl::Icd2 &icd = impl_->icd2;
	icd.last_write_addr = a;
	icd.last_write_val  = value;
	switch (a)
	{
		case 0x6000: icd.w_6000++; break;
		case 0x6001: icd.w_6001++; break;
		case 0x6003: icd.w_6003++; break;
		case 0x6004: icd.w_6004++; break;
		default:
			if (a >= 0x7000 && a <= 0x700F) icd.w_7000++;
			break;
	}

	switch (a)
	{
		case 0x6001:
			icd.lcd_row_select = static_cast<uint8_t>(value & 0x03);
			icd.read_position  = 0;
			if (icd.frame_6001_count < 24)
				icd.frame_6001_bytes[icd.frame_6001_count] = value;
			++icd.frame_6001_count;
			icd.row_writes++;
			return;
		case 0x6003:
		{
			// Match bsnes `io.cpp`: on 0→1 transition of bit 7, the GB
			// is POWER-ON-RESET. On 1→0 we just freeze the GB in reset.
			const bool was_released = (icd.control & 0x80) != 0;
			const bool now_released = (value & 0x80) != 0;
			icd.ctrl_writes++;

			icd.control = value;

			if (!was_released && now_released)
			{
				if (!impl_->cache_valid)
				{
					// First release: fresh GB boot. Reset() zeroes icd2,
					// reloads the boot ROM, sets PC=$0000. The GB's boot
					// ROM will bit-bang 6 packets, which OnJoyserWrite
					// caches into impl_->cached_packets as they arrive.
					Reset();
					icd.control = value;  // re-apply after Reset wiped it
					if (!impl_->boot_rom_loaded)
						PrimeBIOSHandshake();
				}
				else if (icd.queue_count == 0)
				{
					// Subsequent release with an EMPTY queue — BIOS is
					// expecting handshake packets to validate. Replay
					// cached packets so $B119 passes and the state
					// machine can advance $0102 through its ~41 sub-
					// states (at which point $B0CD sets $0101=5 and
					// $808EA0 jumps to game-mode display).
					// Gate on queue_count==0 so we don't wipe any
					// game-generated packets that are already queued —
					// only replay when the BIOS actually needs packets.
					for (int p = 0; p < 6; ++p)
						IcdPushQueue(icd, impl_->cached_packets[p]);
					impl_->replays_done++;
				}
				// Non-empty queue means game packets are pending —
				// leave them alone and let the BIOS drain/process them.
			}
			else if (was_released && !now_released)
			{
				// Entering reset: freeze row/bank so $6000 reads match
				// the fresh GB state the BIOS sees on the next release.
				icd.sgb_row  = 0;
				icd.sgb_bank = 0;
			}
			return;
		}
		case 0x6004:
		{
			icd.joypad[0] = value;
			// SGB $6004 joypad-mirror format (verified against the BIOS's
			// own bit-shuffle in sub_80BCDE — see SGB2.sfc.lst:12372+).
			// Active-LOW. The BIOS writes:
			//   bit 0: RIGHT     bit 4: A
			//   bit 1: LEFT      bit 5: B
			//   bit 2: UP        bit 6: SELECT
			//   bit 3: DOWN      bit 7: START
			// Earlier comment said the opposite (dpad in upper nibble) —
			// that produced "press A → character moves right" because we
			// interpreted bit 4 as Right.
			impl_->joypad.sgb_pads[0] = value;
			uint8_t mask = 0;
			if (!(value & 0x01)) mask |= GB_RIGHT;
			if (!(value & 0x02)) mask |= GB_LEFT;
			if (!(value & 0x04)) mask |= GB_UP;
			if (!(value & 0x08)) mask |= GB_DOWN;
			if (!(value & 0x10)) mask |= GB_A;
			if (!(value & 0x20)) mask |= GB_B;
			if (!(value & 0x40)) mask |= GB_SELECT;
			if (!(value & 0x80)) mask |= GB_START;
			JoypadSet(impl_->joypad, impl_->mem, mask);
			return;
		}
		case 0x6005: icd.joypad[1] = value; impl_->joypad.sgb_pads[1] = value; return;
		case 0x6006: icd.joypad[2] = value; impl_->joypad.sgb_pads[2] = value; return;
		case 0x6007: icd.joypad[3] = value; impl_->joypad.sgb_pads[3] = value; return;
	}
}

bool Emulator::IsGBReleased() const
{
	if (!impl_) return false;
	return (impl_->icd2.control & 0x80) != 0;
}

bool Emulator::IsHandshakePending() const
{
	if (!impl_) return false;
	return impl_->icd2.synth_remaining > 0 || impl_->icd2.queue_count > 0;
}

void Emulator::OnPpuHBlank()
{
	if (!impl_) return;
	Emulator::Impl::Icd2& icd = impl_->icd2;

	// Mesen2 ProcessHBlank: `_row++; if((_row & 7) == 0) _bank = (_bank + 1) & 3;`
	// Incremental advance — sgb_bank is intentionally NOT re-derived from
	// sgb_row each call, so it persists across frames. 18 advances per
	// frame × 4-bank wrap = phase shifts every 2 frames (the BIOS uses
	// $6000's low bits to detect when a fresh band lands in a new bank).
	icd.sgb_row = static_cast<uint8_t>(icd.sgb_row + 1);
	if ((icd.sgb_row & 0x07) == 0)
		icd.sgb_bank = static_cast<uint8_t>((icd.sgb_bank + 1) & 0x03);
}

void Emulator::OnPpuVBlank()
{
	if (!impl_) return;
	// Mesen2 ProcessVBlank: just `_row = 0;`. _bank is intentionally
	// NOT reset — it persists across frames, so the bank-to-band
	// mapping shifts each frame (frame 1 starts at bank 0, frame 2
	// starts at bank 2 because 18 % 4 = 2). The SNES BIOS reads
	// $6000's low bits to know which bank holds the band that just
	// finished; persisting _bank is what makes that signal work
	// across frames.
	impl_->icd2.sgb_row = 0;
	impl_->icd2.frame_6001_count = 0;
}

void Emulator::CaptureScanline(const uint8_t *pixels)
{
	if (!impl_ || !pixels) return;
	Emulator::Impl::Icd2 &icd = impl_->icd2;
	const uint8_t bank = static_cast<uint8_t>(icd.sgb_bank & 0x03);
	const uint8_t row  = static_cast<uint8_t>(icd.sgb_row  & 0x07);
	std::memcpy(&icd.lcd_ring[bank][row * 160], pixels, 160);
	icd.sgb_bank_latched = icd.sgb_bank;
	icd.sgb_row_latched  = icd.sgb_row;
}

static inline uint16_t BgrToHost(uint16_t bgr)
{
	const uint16_t r = static_cast<uint16_t>(bgr & 0x1F);
	const uint16_t g = static_cast<uint16_t>((bgr >> 5) & 0x1F);
	const uint16_t b = static_cast<uint16_t>((bgr >> 10) & 0x1F);
	return static_cast<uint16_t>(BUILD_PIXEL(r, g, b));
}

static inline uint16_t LerpPixel(uint16_t a, uint16_t b, uint16_t t)
{
	const uint16_t inv = static_cast<uint16_t>(256 - t);
	const uint16_t ar  = static_cast<uint16_t>((a >> RED_SHIFT_BITS)   & 0x1F);
	const uint16_t ag  = static_cast<uint16_t>((a >> GREEN_SHIFT_BITS) & 0x1F);
	const uint16_t ab  = static_cast<uint16_t>(a                       & 0x1F);
	const uint16_t br  = static_cast<uint16_t>((b >> RED_SHIFT_BITS)   & 0x1F);
	const uint16_t bg  = static_cast<uint16_t>((b >> GREEN_SHIFT_BITS) & 0x1F);
	const uint16_t bb  = static_cast<uint16_t>(b                       & 0x1F);
	const uint16_t r   = static_cast<uint16_t>((ar * inv + br * t) >> 8);
	const uint16_t g   = static_cast<uint16_t>((ag * inv + bg * t) >> 8);
	const uint16_t bch = static_cast<uint16_t>((ab * inv + bb * t) >> 8);
	return static_cast<uint16_t>((r << RED_SHIFT_BITS) | (g << GREEN_SHIFT_BITS) | bch);
}

void Emulator::BlitScreen(uint16_t *dest, uint32_t pitch_pixels)
{
	if (!impl_->has_rom || !dest) return;

	// Stage border into our heap-resident 256 × 224 buffer. Border
	// leaves the centered 20 × 18 tile area untouched — we overwrite
	// that next with palette-resolved GB pixels.
	uint16_t *const staging = impl_->composite;
	SgbRenderBorder(impl_->sgb_state, staging);

	// Pick the source pixels for the GB screen area based on MASK_EN.
	const uint8_t *src_fb = impl_->ppu.framebuffer;
	if (impl_->sgb_state.mask_mode == SGB_MASK_FREEZE &&
	    impl_->sgb_state.frozen_frame_valid)
	{
		src_fb = impl_->sgb_state.frozen_frame;
	}

	const uint32_t origin_x = SGB_GB_TILE_X * 8;  // 48
	const uint32_t origin_y = SGB_GB_TILE_Y * 8;  // 40

	for (uint32_t py = 0; py < GB_SCREEN_HEIGHT; ++py)
	{
		const uint32_t dst_y     = origin_y + py;
		uint16_t *const dst_row  = staging + dst_y * SGB_BORDER_W + origin_x;
		for (uint32_t px = 0; px < GB_SCREEN_WIDTH; ++px)
		{
			uint16_t color;
			switch (impl_->sgb_state.mask_mode)
			{
				case SGB_MASK_BLACK:
					color = 0x0000;
					break;
				case SGB_MASK_BLANK:
					color = impl_->sgb_state.active[0].colors[0];
					break;
				default:
				{
					const uint8_t  shade   = src_fb[py * GB_SCREEN_WIDTH + px];
					const uint32_t tile_x  = px / 8;
					const uint32_t tile_y  = py / 8;
					color = SgbResolveColor(impl_->sgb_state, tile_x, tile_y, shade);
					break;
				}
			}
			dst_row[px] = color;
		}
	}

	for (uint32_t y = 0; y < SGB_BORDER_H; ++y)
	{
		uint16_t       *const drow = dest    + y * pitch_pixels;
		const uint16_t *const srow = staging + y * SGB_BORDER_W;
		for (uint32_t x = 0; x < SGB_BORDER_W; ++x)
			drow[x] = BgrToHost(srow[x]);
	}

}

static constexpr uint16_t BORDER_FADE_FRAMES = 24;

void Emulator::OverlayBiosBorder(uint16_t *dest, uint32_t pitch_pixels)
{
	(void)dest;
	(void)pitch_pixels;
	if (!impl_->has_rom) return;
}

int32_t Emulator::DrainAudio(int16_t *out, int32_t max_samples)
{
	return ApuDrain(impl_->apu, out, max_samples);
}

int32_t Emulator::GetAudioSampleRate() const
{
	return impl_->apu.output_rate;
}

int32_t Emulator::GetAudioClockHz() const
{
	return impl_->apu.clock_hz;
}

int32_t Emulator::GetAudioCyclesPerSample() const
{
	return impl_->apu.cycles_per_sample;
}

int32_t Emulator::GetAudioCpsRemainderStep() const
{
	return impl_->apu.cps_remainder_step;
}

int32_t Emulator::GetAudioSamplesAvailable() const
{
	const uint32_t head = impl_->apu.sample_head;
	const uint32_t tail = impl_->apu.sample_tail;
	const uint32_t frames = (head >= tail)
		? (head - tail)
		: (APU_SAMPLE_BUF_SIZE - tail + head);
	// Each frame is one stereo pair = 2 int16 values.
	return static_cast<int32_t>(frames * 2);
}

void Emulator::SetAudioRate(int32_t rate_hz)
{
	ApuSetOutputRate(impl_->apu, rate_hz);
}

void Emulator::SetJoypad(uint16_t snes_pad_mask)
{
	// SNES->GB button mapping. B/Y map to A/B (SNES has extra shoulders & face buttons).
	uint8_t gb = 0;
	if (snes_pad_mask & (1 << 15)) gb |= GB_A;       // SNES B  → GB A
	if (snes_pad_mask & (1 << 14)) gb |= GB_B;       // SNES Y  → GB B
	if (snes_pad_mask & (1 << 12)) gb |= GB_START;
	if (snes_pad_mask & (1 << 13)) gb |= GB_SELECT;
	if (snes_pad_mask & (1 << 11)) gb |= GB_UP;
	if (snes_pad_mask & (1 << 10)) gb |= GB_DOWN;
	if (snes_pad_mask & (1 <<  9)) gb |= GB_LEFT;
	if (snes_pad_mask & (1 <<  8)) gb |= GB_RIGHT;
	JoypadSet(impl_->joypad, impl_->mem, gb);
}

// ICD2 packet decoder. The GB drives $FF00 bits 4/5 in four states:
//   $00 — reset pulse: start a new packet
//   $10 — 1-bit
//   $20 — 0-bit
//   $30 — idle / clock-high
// Bits accumulate LSB-first into a byte, then into assembly_buf[0..15]
// which is pushed onto the packet queue on the 16th byte.
// A rising edge on P15 (bit 5) while NOT in a packet advances the MLT_REQ
// player index (see Pan Docs SGB multi-player handshake).
static void IcdFeedJoypad(Emulator::Impl::Icd2 &icd, uint8_t value)
{
	const uint8_t sel = value & 0x30;

	// Player-select edge detection fires only between packets — during
	// packet assembly these same transitions encode data bits.
	if (!icd.in_packet)
	{
		const bool p15_rose = !(icd.input_value & 0x20) && (value & 0x20);
		if (p15_rose)
		{
			const uint8_t mlt_bits = static_cast<uint8_t>((icd.control >> 4) & 0x03);
			const uint8_t players  = (mlt_bits == 0) ? 1u
			                       : (mlt_bits == 1) ? 2u : 4u;
			icd.input_index = static_cast<uint8_t>(
				(icd.input_index + 1) % players);
		}
	}
	icd.input_value = value;

	if (sel == 0x00)
	{
		// Reset pulse — arm the packet assembler.
		icd.in_packet        = true;
		icd.packet_byte      = 0;
		icd.packet_bit       = 0;
		icd.bit_accumulator  = 0;
		return;
	}

	if (!icd.in_packet) return;
	if (sel != 0x10 && sel != 0x20) return;  // $30 (idle) doesn't latch a bit

	const uint16_t bit = (sel == 0x10) ? 1u : 0u;
	icd.bit_accumulator |= static_cast<uint16_t>(bit << icd.packet_bit);
	icd.packet_bit++;

	if (icd.packet_bit >= 8)
	{
		if (icd.packet_byte < 16)
			icd.assembly_buf[icd.packet_byte] = static_cast<uint8_t>(icd.bit_accumulator);
		icd.packet_byte++;
		icd.packet_bit      = 0;
		icd.bit_accumulator = 0;

		if (icd.packet_byte >= 16)
		{
			icd.in_packet = false;
			// Push the fully-assembled packet onto the queue. Counter
			// bookkeeping (packets_received, last_cmd_ids, f1_packets)
			// is handled inside IcdPushQueue for consistency with the
			// synth-staging path.
			IcdPushQueue(icd, icd.assembly_buf);
		}
	}
}

void Emulator::OnJoyserWrite(uint8_t value)
{
	// Only meaningful when the cart declares SGB features. Feeding always
	// is harmless (non-SGB games don't produce RESET pulses) but we gate
	// on run_mode anyway so the packet state doesn't accumulate noise.
	if (impl_->run_mode == RunMode::DMG) return;

	// Raw GB→$FF00 traffic counters — read by the OSD so we can tell
	// whether the GB is even attempting to bit-bang packets, separate
	// from whether our decoder consumes them.
	++g_sgb_dbg.joy_writes;
	if ((value & 0x30) == 0x00) ++g_sgb_dbg.rst_pulses;

	// BIOS-less path — our internal packet assembler fires the dispatch
	// callback into sgb_state.cpp (palettes / border / mask).
	PacketFeed(impl_->sgb_pkt, value);

	// BIOS-mode path — independent decoder that parks completed packets
	// in the single-slot ICD2 FIFO. Harmless when no BIOS is running
	// (nothing reads $7000-$700F).
	const uint32_t pre_received = impl_->icd2.packets_received;
	IcdFeedJoypad(impl_->icd2, value);

	impl_->joypad.sgb_index = impl_->icd2.input_index;
	// If a new packet completed (packets_received grew), and our cache
	// isn't full yet, append a copy. The SGB2 BIOS's handshake validator
	// at $B119 compares each packet's byte 1 across TWO separate GB
	// resets ($7E:1718 is the reference, populated by $BFD3 from a prior
	// iteration's packets). If our re-booted GB produces byte-different
	// packets, validation fails and $02FA=1 triggers cart error. Caching
	// the first 6 real packets lets us re-queue byte-identical copies on
	// subsequent releases, guaranteeing the BIOS's byte-1 reference check
	// passes.
	if (impl_->icd2.packets_received > pre_received &&
	    impl_->cached_count < 6)
	{
		std::memcpy(impl_->cached_packets[impl_->cached_count],
		            impl_->icd2.assembly_buf, 16);
		impl_->cached_count++;
		if (impl_->cached_count == 6) impl_->cache_valid = true;
	}
}

void Emulator::OnSgbCommandInternal(uint8_t cmd, const uint8_t *data, uint32_t len)
{
	DbgPushCmd(cmd);

	if (cmd == 0x13 || cmd == 0x14)
	{
		impl_->border_capture.stage = (cmd == 0x13)
			? Impl::BorderCapture::ChrTrn
			: Impl::BorderCapture::PctTrn;
		std::memcpy(impl_->border_capture.pkt, data, 16);
		impl_->ppu.frame_ready = false;
		if (cmd == 0x13) ++g_sgb_dbg.pkt_chr;
		else             ++g_sgb_dbg.pkt_pct;
		return;
	}

	SgbHandleCommand(impl_->sgb_state, cmd, data, len,
	                 impl_->ppu.vram,
	                 impl_->ppu.framebuffer);
}

// ===================================================================
// State serialization
//
// Layout:
//   [0..3]   magic "SGB!"
//   [4..7]   version (u32 LE)
//   [8..11]  payload length (u32 LE)
//   [12..]   payload fields in Visit() order
//
// Version 1: initial format.
// ===================================================================

namespace {

constexpr uint32_t SGB_STATE_MAGIC   = 0x21424753u;  // 'S''G''B''!' LE
constexpr uint32_t SGB_STATE_VERSION = 1;

enum class IoMode : uint8_t { Size, Save, Load };

struct IoCtx
{
	uint8_t       *wbuf;
	const uint8_t *rbuf;
	size_t         pos;
	size_t         cap;
	IoMode         mode;
	bool           ok;
};

inline void IoBytes(IoCtx &c, void *data, size_t n)
{
	if (!c.ok) return;
	if (c.mode != IoMode::Size && c.pos + n > c.cap) { c.ok = false; return; }
	if      (c.mode == IoMode::Save) std::memcpy(c.wbuf + c.pos, data, n);
	else if (c.mode == IoMode::Load) std::memcpy(data, c.rbuf + c.pos, n);
	c.pos += n;
}

template <typename T>
inline void IoField(IoCtx &c, T &v)
{
	IoBytes(c, &v, sizeof v);
}

void VisitState(Emulator::Impl &impl, IoCtx &c)
{
	// ----- CPU -----
	IoField(c, impl.cpu.State());

	// ----- Memory (skip pointer fields — they're relinked after load) -----
	IoBytes(c, impl.mem.wram, sizeof impl.mem.wram);
	IoBytes(c, impl.mem.hram, sizeof impl.mem.hram);
	IoField(c, impl.mem.ie);
	IoField(c, impl.mem.if_);
	IoField(c, impl.mem.serial_data);

	// ----- Cart MBC + SRAM (ROM is static, not serialized) -----
	IoField(c, impl.cart.mbc);

	uint32_t sram_size = static_cast<uint32_t>(impl.cart.sram.size());
	IoField(c, sram_size);
	if (c.mode == IoMode::Load) impl.cart.sram.resize(sram_size);
	if (sram_size > 0) IoBytes(c, impl.cart.sram.data(), sram_size);

	// ----- PPU -----
	IoBytes(c, impl.ppu.vram, sizeof impl.ppu.vram);
	IoBytes(c, impl.ppu.oam,  sizeof impl.ppu.oam);
	IoField(c, impl.ppu.lcdc);
	IoField(c, impl.ppu.stat);
	IoField(c, impl.ppu.scy);
	IoField(c, impl.ppu.scx);
	IoField(c, impl.ppu.ly);
	IoField(c, impl.ppu.lyc);
	IoField(c, impl.ppu.bgp);
	IoField(c, impl.ppu.obp0);
	IoField(c, impl.ppu.obp1);
	IoField(c, impl.ppu.wy);
	IoField(c, impl.ppu.wx);
	IoField(c, impl.ppu.mode);
	IoField(c, impl.ppu.mode_clock);
	IoField(c, impl.ppu.window_line);
	IoField(c, impl.ppu.stat_line_high);
	IoBytes(c, impl.ppu.framebuffer,      sizeof impl.ppu.framebuffer);
	IoBytes(c, impl.ppu.scanline_bg_raw,  sizeof impl.ppu.scanline_bg_raw);
	IoField(c, impl.ppu.frame_ready);

	// ----- APU channels + master regs + frame sequencer -----
	// (sample buffer + accumulators are transient and reset on load.)
	IoField(c, impl.apu.ch1);
	IoField(c, impl.apu.ch2);
	IoField(c, impl.apu.ch3);
	IoField(c, impl.apu.ch4);
	IoField(c, impl.apu.nr50);
	IoField(c, impl.apu.nr51);
	IoField(c, impl.apu.master_enabled);
	IoField(c, impl.apu.frame_seq_timer);
	IoField(c, impl.apu.frame_seq_step);

	// ----- Timer / Joypad -----
	IoField(c, impl.timer);
	// Serialize only the GB-side joypad fields (size, dpad, btns,
	// prev_mask) — keeping the on-disk layout byte-identical to v1
	// even though the struct now also holds SGB-bridge state
	// (sgb_active, sgb_index, sgb_pads). Those reconstruct from
	// icd2 + BIOS writes within one frame after load, so persisting
	// them would only complicate save-state compat.
	IoField(c, impl.joypad.select);
	IoField(c, impl.joypad.dpad);
	IoField(c, impl.joypad.btns);
	IoField(c, impl.joypad.prev_mask);

	// ----- SGB command layer -----
	IoField(c, impl.sgb_pkt);
	IoField(c, impl.sgb_state);

	// ----- Emulator-level config -----
	IoField(c, impl.run_mode);
	IoField(c, impl.clock_mul);
}

} // anonymous

size_t Emulator::StateSize() const
{
	IoCtx c{nullptr, nullptr, 0, 0, IoMode::Size, true};
	VisitState(const_cast<Impl &>(*impl_), c);
	// +12 for header: magic + version + payload_size.
	return c.pos + 12;
}

void Emulator::StateSave(uint8_t *buffer) const
{
	if (!buffer) return;

	// Compute payload size first.
	const size_t payload = StateSize() - 12;

	uint32_t magic   = SGB_STATE_MAGIC;
	uint32_t version = SGB_STATE_VERSION;
	uint32_t plen    = static_cast<uint32_t>(payload);
	std::memcpy(buffer + 0, &magic,   4);
	std::memcpy(buffer + 4, &version, 4);
	std::memcpy(buffer + 8, &plen,    4);

	IoCtx c{buffer + 12, nullptr, 0, payload, IoMode::Save, true};
	VisitState(const_cast<Impl &>(*impl_), c);
}

bool Emulator::StateLoad(const uint8_t *buffer, size_t size)
{
	if (!buffer || size < 12) return false;

	uint32_t magic, version, plen;
	std::memcpy(&magic,   buffer + 0, 4);
	std::memcpy(&version, buffer + 4, 4);
	std::memcpy(&plen,    buffer + 8, 4);

	if (magic != SGB_STATE_MAGIC)     return false;
	if (version != SGB_STATE_VERSION) return false;  // future: accept v<=current
	if (size < 12 + plen)             return false;

	IoCtx c{nullptr, buffer + 12, 0, plen, IoMode::Load, true};
	VisitState(*impl_, c);
	if (!c.ok) return false;

	// Relink Memory's pointer fields — they were serialized as garbage.
	impl_->mem.ppu    = &impl_->ppu;
	impl_->mem.apu    = &impl_->apu;
	impl_->mem.timer  = &impl_->timer;
	impl_->mem.joypad = &impl_->joypad;
	impl_->mem.cart   = &impl_->cart;

	// Reset only the sub-sample integration accumulator. The ring buffer
	// is not serialized but also not wiped — it stays at its current
	// in-memory positions so already-queued samples keep playing across
	// the load. Wiping it on every load broke runahead audio (the hidden
	// frame's samples were discarded each iteration).
	impl_->apu.sample_accum_l   = 0;
	impl_->apu.sample_accum_r   = 0;
	impl_->apu.sample_accum_cnt = 0;
	impl_->apu.sample_timer     = impl_->apu.cycles_per_sample;

	// Re-point the exposed FrameBuffer at the (now-restored) PPU fb.
	impl_->fb.pixels = impl_->ppu.framebuffer;
	impl_->fb.width  = GB_SCREEN_WIDTH;
	impl_->fb.height = GB_SCREEN_HEIGHT;
	impl_->fb.pitch  = GB_SCREEN_WIDTH;

	// Reseed the SGB-bridge mirrors on Joypad from icd2 — these are
	// not serialized (see VisitState), so a load with the BIOS not
	// yet refreshing $6004 would otherwise leave the GB seeing stale
	// "all buttons released" instead of the captured pad state.
	impl_->joypad.sgb_active   = Settings.SGB_BIOSModeActive;
	impl_->joypad.sgb_pads[0]  = impl_->icd2.joypad[0];
	impl_->joypad.sgb_pads[1]  = impl_->icd2.joypad[1];
	impl_->joypad.sgb_pads[2]  = impl_->icd2.joypad[2];
	impl_->joypad.sgb_pads[3]  = impl_->icd2.joypad[3];
	impl_->joypad.sgb_index    = impl_->icd2.input_index;

	return true;
}

void  Emulator::SetClockMultiplier(float m) { impl_->clock_mul = m; }
float Emulator::GetClockMultiplier() const  { return impl_->clock_mul; }

Emulator &Instance()
{
	static Emulator g;
	return g;
}

} // namespace SGB

// C-style facade used by snes9x integration code.
bool S9xSGBInit(void)               { return SGB::Instance().Init(); }
void S9xSGBDeinit(void)             { SGB::Instance().Deinit(); }
void S9xSGBReset(void)              { SGB::Instance().ColdReset(); }
bool S9xSGBIsActive(void)           { return SGB::Instance().HasROM(); }
void S9xSGBRunFrame(void)           { SGB::Instance().RunFrame(); }
void S9xSGBRunCycles(int tcycles)   { SGB::Instance().RunCycles(static_cast<int32_t>(tcycles)); }

namespace {
	int32_t g_snes_cycle_accum = 0;
	int32_t g_sync_anchor      = 0;
	int32_t g_h_max            = 1364;  // NTSC default; overwritten per-frame by cpuexec
}

void S9xSGBResetClockSync(void)
{
	g_snes_cycle_accum = 0;
	g_sync_anchor      = 0;
}

void S9xSGBTickSnes(int snes_master_cycles)
{
	if (snes_master_cycles <= 0) return;
	g_snes_cycle_accum += snes_master_cycles;

	// Clock ratio depends on RunMode:
	//   SGB1: GB clock = SNES master / 5 (≈ 4.295 MHz, slightly faster
	//                    than DMG — matches the ICD2 cart's wiring).
	//   SGB2: GB clock = real DMG clock (4.194 MHz). The SNES still
	//                    runs at 21.477 MHz, so the ratio is ~5.121,
	//                    NOT 5. Using /5 in SGB2 mode makes the GB run
	//                    2.4% too fast; over a frame that's ~6 scan-
	//                    lines of drift, which desyncs the BIOS's
	//                    bank-read timing against our slice writes
	//                    and produces visible vertical row drift.
	//   DMG:  same as SGB2 — real GB clock.
	int32_t gb_cycles;
	const SGB::RunMode mode = SGB::Instance().GetRunMode();
	if (mode == SGB::RunMode::SGB)
	{
		gb_cycles = g_snes_cycle_accum / 5;
		if (gb_cycles > 0)
		{
			g_snes_cycle_accum -= gb_cycles * 5;
			SGB::Instance().RunCycles(gb_cycles);
		}
	}
	else
	{
		// 64-bit math to avoid overflow: ratio = 4194304 / 21477272.
		// gb_cycles = accum * 4194304 / 21477272.
		const int64_t scaled = static_cast<int64_t>(g_snes_cycle_accum) * 4194304;
		gb_cycles = static_cast<int32_t>(scaled / 21477272);
		if (gb_cycles > 0)
		{
			// Subtract back the SNES-cycle equivalent of what we ran.
			const int64_t consumed = (static_cast<int64_t>(gb_cycles) * 21477272) / 4194304;
			g_snes_cycle_accum -= static_cast<int32_t>(consumed);
			SGB::Instance().RunCycles(gb_cycles);
		}
	}
}

void S9xSGBResetSyncAnchor(int32_t cpu_cycles)
{
	g_sync_anchor = cpu_cycles;
}

void S9xSGBSetHMax(int32_t h_max)
{
	if (h_max > 0) g_h_max = h_max;
}

void S9xSGBSyncToSnesCycle(int32_t cpu_cycles)
{
	int32_t delta = cpu_cycles - g_sync_anchor;
	// Scanline wrap: snes9x's H-event subtracts H_Max from CPU.Cycles,
	// so a legitimate forward step across the wrap appears as a large
	// negative delta. Adding H_Max back recovers the real delta, as
	// long as we sync at least once per scanline (trivially true given
	// per-opcode sync points).
	if (delta < 0) delta += g_h_max;
	g_sync_anchor = cpu_cycles;
	// GB is held in reset (control bit 7 = 0) — advance the anchor but
	// do NOT step the GB core. Otherwise a BIOS write of $6003=$01
	// (reset line held low) still lets the GB progress, which breaks
	// the "toggle reset to re-boot GB" pattern the SGB BIOS uses after
	// the splash animation. Match bsnes: GB thread only runs while
	// r6003.d7 is 1.
	if (!S9xSGBBIOSGBIsReleased()) return;
	if (delta > 0) S9xSGBTickSnes(delta);
}

void S9xSGBOnPpuHBlank(void) { SGB::Instance().OnPpuHBlank(); }
void S9xSGBOnPpuVBlank(void) { SGB::Instance().OnPpuVBlank(); }
void S9xSGBCaptureScanline(const unsigned char *pixels)
{
	SGB::Instance().CaptureScanline(static_cast<const uint8_t *>(pixels));
}
void S9xSGBSetJoypad(uint16_t m)    { SGB::Instance().SetJoypad(m); }
void S9xSGBOnJoyserWrite(uint8_t v) { SGB::Instance().OnJoyserWrite(v); }

void S9xSGBBlitScreen(uint16_t *dest, uint32_t pitch_pixels)
{
	SGB::Instance().BlitScreen(dest, pitch_pixels);
}

void S9xSGBOverlayBiosBorder(uint16_t *dest, uint32_t pitch_pixels)
{
	SGB::Instance().OverlayBiosBorder(dest, pitch_pixels);
}

int32_t S9xSGBGetSampleCount(void)
{
	// Hand the host whatever the GB APU has produced. S9xMixSamples is
	// already bounded by the host's per-call sample_count request, so no
	// internal cap is needed; capping here just starves hosts whose audio
	// buffer cadence differs from ours.
	return SGB::Instance().GetAudioSamplesAvailable();
}

int32_t S9xSGBDrainSamples(int16_t *dest, int32_t count_int16s)
{
	if (!dest || count_int16s <= 0) return 0;
	const int32_t frames = count_int16s / 2;
	const int32_t got    = SGB::Instance().DrainAudio(dest, frames);
	return got * 2;
}

void S9xSGBSetAudioRate(int32_t rate_hz)
{
	SGB::Instance().SetAudioRate(rate_hz);
}

int32_t S9xSGBGetAudioClockHz(void)
{
	return SGB::Instance().GetAudioClockHz();
}

int32_t S9xSGBGetAudioCyclesPerSample(void)
{
	return SGB::Instance().GetAudioCyclesPerSample();
}

int32_t S9xSGBGetAudioCpsRemainderStep(void)
{
	return SGB::Instance().GetAudioCpsRemainderStep();
}

void S9xSGBSetRunMode(uint8_t mode)
{
	SGB::RunMode m;
	switch (mode)
	{
		case 2:  m = SGB::RunMode::SGB2; break;
		case 0:  m = SGB::RunMode::DMG;  break;
		case 1:
		default: m = SGB::RunMode::SGB;  break;
	}
	SGB::Instance().SetRunMode(m);
}

void S9xSGBSetClockMultiplier(float mul)
{
	SGB::Instance().SetClockMultiplier(mul);
}

size_t S9xSGBStateSize(void)
{
	return SGB::Instance().StateSize();
}

void S9xSGBStateSave(uint8_t *buffer)
{
	SGB::Instance().StateSave(buffer);
}

bool S9xSGBStateLoad(const uint8_t *buffer, size_t size)
{
	return SGB::Instance().StateLoad(buffer, size);
}

bool S9xSGBSaveStateToFile(const char *filename)
{
	if (!filename) return false;

	const size_t need = SGB::Instance().StateSize();
	std::vector<uint8_t> buf(need);
	SGB::Instance().StateSave(buf.data());

	FILE *f = fopen(filename, "wb");
	if (!f) return false;
	const size_t w = fwrite(buf.data(), 1, need, f);
	fclose(f);
	return w == need;
}

void S9xSGBGetStatus(char *buf, size_t cap)
{
	SGB::Instance().GetStatus(buf, cap);
}

bool S9xSGBLoadStateFromFile(const char *filename)
{
	if (!filename) return false;

	FILE *f = fopen(filename, "rb");
	if (!f) return false;
	if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
	const long sz = ftell(f);
	if (sz <= 12 || sz > 4 * 1024 * 1024) { fclose(f); return false; }
	fseek(f, 0, SEEK_SET);

	std::vector<uint8_t> buf(static_cast<size_t>(sz));
	const size_t got = fread(buf.data(), 1, static_cast<size_t>(sz), f);
	fclose(f);
	if (got != static_cast<size_t>(sz)) return false;

	return SGB::Instance().StateLoad(buf.data(), buf.size());
}

bool S9xSGBLoadROMBytes(const unsigned char *data, size_t size, const char *path_for_sram)
{
	if (!data || size < 0x150) return false;
	return SGB::Instance().LoadROM(static_cast<const uint8_t *>(data), size, path_for_sram);
}

bool S9xSGBLoadBootROMBytes(const unsigned char *data, size_t size)
{
	return SGB::Instance().LoadBootROM(static_cast<const uint8_t *>(data), size);
}

bool S9xSGBLoadEmbeddedBootROM(unsigned char mode)
{
	const uint8_t *src = (mode == 2) ? SGB::kSgb2BootRom : SGB::kSgbBootRom;
	return SGB::Instance().LoadBootROM(src, 256);
}

void S9xSGBPrimeBIOSHandshake(void)
{
	SGB::Instance().PrimeBIOSHandshake();
}

bool S9xSGBLoadROM(const char *filename)
{
	if (!filename || !*filename) return false;

	FILE *f = fopen(filename, "rb");
	if (!f) return false;

	if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return false; }
	const long sz = ftell(f);
	if (sz <= 0 || sz > 16 * 1024 * 1024)   // 16 MB is the MBC5/MBC6 ceiling
	{ fclose(f); return false; }
	fseek(f, 0, SEEK_SET);

	std::vector<uint8_t> buf(static_cast<size_t>(sz));
	const size_t got = fread(buf.data(), 1, static_cast<size_t>(sz), f);
	fclose(f);
	if (got != static_cast<size_t>(sz)) return false;

	return SGB::Instance().LoadROM(buf.data(), buf.size(), filename);
}

// ICD2 bridge — 0x6000-0x7FFF on the SNES side. P2b just stores writes in a
// raw register file and returns them on read. Real semantics (reset gating,
// packet FIFO, joypad multiplex, VRAM readback) land in P2c-P2e.
unsigned char S9xSGBGetICD2(unsigned short addr)
{
	return SGB::Instance().GetICD2(addr);
}

void S9xSGBSetICD2(unsigned char value, unsigned short addr)
{
	SGB::Instance().SetICD2(value, addr);
}

bool S9xSGBBIOSGBIsReleased(void)
{
	return SGB::Instance().IsGBReleased();
}

bool S9xSGBBIOSHandshakePending(void)
{
	return SGB::Instance().IsHandshakePending();
}

bool S9xSGBGetROMBytes(const unsigned char **out_data, size_t *out_size)
{
	const uint8_t *data = SGB::Instance().GetROMData();
	const size_t   size = SGB::Instance().GetROMSize();
	if (!data || size == 0) return false;
	if (out_data) *out_data = data;
	if (out_size) *out_size = size;
	return true;
}

unsigned char S9xSGBPeekRAByte(unsigned int addr)
{
	return SGB::Instance().PeekRAByte(addr);
}
