/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Super Famicom Box supervisor board (see sfcbox.h / docs/sfcbox.md).
// Register behavior follows fullsnes "SNES Hotel Boxes and Arcade Machines".

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "snes9x.h"
#include "memmap.h"
#include "display.h"
#include "movie.h"
#include "hd64180.h"
#include "sfcbox.h"

struct SSFCBox	SFCBox;

// SFCBOX_TRACE=1: log the KROM<->SNES dialogue (mapping/reset/serial-link
// writes and a periodic Z180 PC sample) to stdout for boot debugging.
static int TraceEnabled (void)
{
	static int	cached = -1;
	if (cached < 0)
	{
		const char	*e = getenv("SFCBOX_TRACE");
		cached = (e && *e && *e != '0') ? 1 : 0;
	}
	return (cached);
}

// PHI = 9.216 MHz / 2
#define SFCBOX_PHI			4608000
// Coin switch stays closed 44-80ms; use ~60ms of PHI clocks.
#define SFCBOX_COIN_CYCLES	276480
// External watchdog, reloaded by [81h].W bit6 transitions. Real timeout is
// ~1.5s (0xABA timer0 steps at 1772 Hz); run generous so emulation-timing
// hiccups don't reboot the board under us.
#define SFCBOX_WATCHDOG_CYCLES	(SFCBOX_PHI * 2)

// ---------------------------------------------------------------------------
// S-3520CF RTC

static void RTCLoadHostTime (void)
{
	time_t		t = time(NULL);
	struct tm	*lt = localtime(&t);

	if (!lt)
		return;

	SFCBox.RTC.Sec     = lt->tm_sec > 59 ? 59 : lt->tm_sec;
	SFCBox.RTC.Min     = lt->tm_min;
	SFCBox.RTC.Hour    = lt->tm_hour;
	SFCBox.RTC.Day     = lt->tm_mday;
	SFCBox.RTC.Month   = lt->tm_mon + 1;
	SFCBox.RTC.Year    = lt->tm_year % 100;
	SFCBox.RTC.Weekday = lt->tm_wday;
}

static void RTCTickSecond (void)
{
	struct SSFCBoxRTC	*r = &SFCBox.RTC;

	if (++r->Sec < 60)
		return;
	r->Sec = 0;
	if (++r->Min < 60)
		return;
	r->Min = 0;
	if (++r->Hour < 24)
		return;
	r->Hour = 0;
	r->Weekday = (r->Weekday + 1) % 7;

	static const int	mdays[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	int					days = mdays[r->Month >= 1 && r->Month <= 12 ? r->Month : 1];
	if (r->Month == 2 && (r->Year % 4) == 0)
		days = 29;

	if (++r->Day <= days)
		return;
	r->Day = 1;
	if (++r->Month <= 12)
		return;
	r->Month = 1;
	r->Year = (r->Year + 1) % 100;
}

static uint8 RTCReadReg (uint8 index)
{
	struct SSFCBoxRTC	*r = &SFCBox.RTC;

	if (index == 0x0f)
		return (r->Mode & 3);

	if ((r->Mode & 3) >= 2)
		return (index < 15 ? r->SRAMPages[(r->Mode & 3) - 2][index] & 0x0f : 0);
	if ((r->Mode & 3) != 0)
		return (0x0f);

	bool8	h24 = (r->Control & 0x01) != 0;
	int		hour = h24 ? r->Hour : (r->Hour % 12 == 0 ? 12 : r->Hour % 12);

	switch (index)
	{
		case 0x0:	return (r->Sec % 10);
		case 0x1:	return (r->Sec / 10);
		case 0x2:	return (r->Min % 10);
		case 0x3:	return (r->Min / 10);
		case 0x4:	return (hour % 10);
		case 0x5:	return ((hour / 10) | (r->Hour >= 12 ? 0x08 : 0));
		case 0x6:	return (r->Weekday);
		case 0x7:	return (r->Day % 10);
		case 0x8:	return (r->Day / 10);
		case 0x9:	return (r->Month % 10);
		case 0xa:	return (r->Month / 10);
		case 0xb:	return (r->Year % 10);
		case 0xc:	return (r->Year / 10);
		case 0xd:	return (r->Control & 0x0f);
		case 0xe:	return (0x00);	// STA=0 (stable), LOST=0 (battery good)
	}
	return (0x0f);
}

// Time/date registers are increment-on-write digit counters (the KROM sets
// the clock by bumping each digit); wraps stay inside their own field.
static void RTCWriteReg (uint8 index, uint8 value)
{
	struct SSFCBoxRTC	*r = &SFCBox.RTC;

	if (index == 0x0f)
	{
		if (value & 0x08)	// SYSR: full reset
		{
			r->Mode = 0;
			r->Control = 0;
			return;
		}
		r->Mode = value & 3;
		return;
	}

	if ((r->Mode & 3) >= 2)
	{
		if (index < 15)
			r->SRAMPages[(r->Mode & 3) - 2][index] = value & 0x0f;
		return;
	}
	if ((r->Mode & 3) != 0)
		return;

	switch (index)
	{
		case 0x0:	r->Sec = (r->Sec + 1) % 60;		break;
		case 0x1:	r->Sec = (r->Sec + 10) % 60;	break;
		case 0x2:	r->Min = (r->Min + 1) % 60;		break;
		case 0x3:	r->Min = (r->Min + 10) % 60;	break;
		case 0x4:	r->Hour = (r->Hour + 1) % 24;	break;
		case 0x5:	r->Hour = (r->Hour + 10) % 24;	break;
		case 0x6:	r->Weekday = (r->Weekday + 1) % 7;	break;
		case 0x7:	r->Day = (r->Day % 31) + 1;		break;
		case 0x8:	r->Day = ((r->Day - 1 + 10) % 31) + 1;	break;
		case 0x9:	r->Month = (r->Month % 12) + 1;	break;
		case 0xa:	r->Month = ((r->Month - 1 + 10) % 12) + 1;	break;
		case 0xb:	r->Year = (r->Year + 1) % 100;	break;
		case 0xc:	r->Year = (r->Year + 10) % 100;	break;

		case 0xd:
			r->Control = value & 0x0f;
			if (value & 0x04)	// 30ADJ
			{
				if (r->Sec >= 30)
					r->Min = (r->Min + 1) % 60;
				r->Sec = 0;
			}
			if (value & 0x02)	// CNTR
				r->Sec = 0;
			break;
	}
}

static void RTCNibbleIn (int nibble, uint8 value)
{
	struct SSFCBoxRTC	*r = &SFCBox.RTC;

	if (!r->DirRead)
	{
		// Write transfer: index, data, index, data, ...
		if (!(nibble & 1))
			r->Index = value;
		else
			RTCWriteReg(r->Index, value);
	}
	else
	{
		// Read transfer: index nibbles go out on even positions and the
		// register data comes back three nibble-slots later.
		if (!(nibble & 1))
		{
			r->Index = value;
			if (nibble + 3 < (int) sizeof(r->OutNibbles))
				r->OutNibbles[nibble + 3] = RTCReadReg(value);
		}
	}
}

// Port [A0h].W — RTC pins + WRAM save-area unlock
static void RTCPinWrite (uint8 byte)
{
	struct SSFCBoxRTC	*r = &SFCBox.RTC;

	SFCBox.WRAMUnlock = (byte >> 7) & 1;

	uint8	cs  = byte & 1;			// 1 = selected (pin driven low)
	uint8	clk = (byte >> 3) & 1;	// 0 = clock low, 1 = idle high
	uint8	out = (byte >> 2) & 1;

	if (cs && !r->CS)
	{
		// Transfer begins
		r->BitPos = 0;
		r->ShiftIn = 0;
		memset(r->OutNibbles, 0, sizeof(r->OutNibbles));
	}

	r->DirRead = (byte >> 1) & 1;

	if (cs)
	{
		if (!r->CLK && clk)
		{
			// Rising clock edge: sample master's data-out
			r->ShiftIn |= out << (r->BitPos & 3);
			if ((r->BitPos & 3) == 3)
			{
				RTCNibbleIn(r->BitPos >> 2, r->ShiftIn & 0x0f);
				r->ShiftIn = 0;
			}
			r->BitPos++;
		}
		else if (r->CLK && !clk)
		{
			// Falling edge: present the next output bit
			int	nib = (r->BitPos >> 2) & (sizeof(r->OutNibbles) - 1);
			r->DataIn = (r->OutNibbles[nib] >> (r->BitPos & 3)) & 1;
		}
	}

	r->CS = cs;
	r->CLK = clk;
	r->DataOut = out;
}

// ---------------------------------------------------------------------------
// MB90082 OSD

static void OSDAdvanceCursor (void)
{
	struct SSFCBoxOSD	*o = &SFCBox.OSD;

	if (++o->CursorX >= SFCBOX_OSD_W)
	{
		o->CursorX = 0;
		if (++o->CursorY >= SFCBOX_OSD_H)
			o->CursorY = 0;
	}
}

static void OSDCommand (uint8 cmd, uint16 param)
{
	struct SSFCBoxOSD	*o = &SFCBox.OSD;

	if (TraceEnabled() && cmd != 0x2)
		printf("SFC-Box: OSD cmd %X param %03X\n", cmd, param);

	switch (cmd)
	{
		case 0x0:	// Preset VRAM address
			o->FillMode = (param >> 9) & 1;
			o->CursorY = (param >> 5) & 0x0f;
			o->CursorX = param & 0x1f;
			if (o->CursorY >= SFCBOX_OSD_H)	o->CursorY = SFCBOX_OSD_H - 1;
			if (o->CursorX >= SFCBOX_OSD_W)	o->CursorX = SFCBOX_OSD_W - 1;
			break;

		case 0x1:	// Select color
			o->CharColor = (param >> 4) & 7;
			o->BgColor = param & 7;
			break;

		case 0x2:	// Write character
		{
			uint8	ch = param & 0xff;
			uint8	attr = o->CharColor | (o->BgColor << 3) |
						   (((param >> 9) & 1) << 6) | (((param >> 8) & 1) << 7);

			if (o->FillMode)
			{
				for (int y = 0; y < SFCBOX_OSD_H; y++)
					for (int x = 0; x < SFCBOX_OSD_W; x++)
					{
						if (y < o->CursorY || (y == o->CursorY && x < o->CursorX))
							continue;
						o->VRAMChar[y][x] = ch;
						o->VRAMColor[y][x] = attr;
					}
				o->FillMode = 0;
				o->CursorX = 0;
				o->CursorY = 0;
			}
			else
			{
				o->VRAMChar[o->CursorY][o->CursorX] = ch;
				o->VRAMColor[o->CursorY][o->CursorX] = attr;
				OSDAdvanceCursor();
			}
			break;
		}

		case 0x4:	// Screen control 1
			o->DisplayEnable = param & 1;
			o->ColorMode = (param >> 5) & 1;
			o->ExtSync = (param >> 9) & 1;
			break;

		case 0x6:	// Line control
		{
			int	n = param & 0x0f;
			if (n < SFCBOX_OSD_H)
				o->LineCtrl[n] = (uint8) ((((param >> 9) & 1) << 0) |	// BK solid
										  (((param >> 8) & 1) << 1) |	// zoom Y
										  (((param >> 7) & 1) << 2) |	// zoom X
										  (((param >> 6) & 1) << 3));	// BC shown
			break;
		}

		case 0x7:	o->YOfs = param & 0x3f;	break;
		case 0x8:	o->XOfs = param & 0x3f;	break;
		case 0xa:	o->UnderColor = param & 7;	break;

		default:	// sprite/reserved commands: accepted, not rendered
			break;
	}
}

static void OSDReceiveByte (uint8 byte)
{
	struct SSFCBoxOSD	*o = &SFCBox.OSD;

	if (byte & 0x80)
	{
		o->PendingCmd = byte;
		return;
	}
	if (o->PendingCmd)
	{
		uint8	cmd = (o->PendingCmd >> 3) & 0x0f;
		uint16	param = (uint16) ((o->PendingCmd & 7) << 7) | (byte & 0x7f);
		o->PendingCmd = 0;
		OSDCommand(cmd, param);
	}
}

// 3-bit OSD color -> 5-bit RGB components (bit2=R, bit1=G, bit0=B)
static const uint8	osd_r5[8] = { 0, 0, 0, 0, 31, 31, 31, 31 };
static const uint8	osd_g5[8] = { 0, 0, 31, 31, 0, 0, 31, 31 };
static const uint8	osd_b5[8] = { 0, 31, 0, 31, 0, 31, 0, 31 };

// Glyph dot lookup: 256 chars x 18 rows x 2 bytes; byte0 = dots 0-7 (MSB
// leftmost), byte1 bits 3-0 = dots 8-11 (verified against the KROM boot
// text: the right-side third of every glyph lives in the LOW nibble).
static inline int OSDGlyphDot (uint8 ch, int row, int dot)
{
	const uint8	*g = SFCBox.OSD.Font + (uint32) ch * 36 + row * 2;

	if (dot < 8)
		return ((g[0] >> (7 - dot)) & 1);
	return ((g[1] >> (11 - dot)) & 1);
}

void S9xSFCBoxRenderOSD (uint16 *screen, int pitch, int width, int height)
{
	struct SSFCBoxOSD	*o = &SFCBox.OSD;

	if (!SFCBox.Active || !o->FontLoaded)
		return;

	// In internal-sync mode (Screen Control 1 IE=0) the MB90082 generates
	// the whole picture itself and floods the raster with the under color
	// ("shown only in INTERNAL sync mode" — fullsnes); the KROM uses that
	// for the blue supervisor screens NO$SNS shows. In external-sync mode
	// it genlocks to the SNES video and the plane outside the characters
	// stays transparent. snes9x traditionally superimposes in both modes,
	// so the raster can be turned off (win32 Hacks dialog, default on).
	if (Settings.SFCBoxOSDBackdrop && !o->ExtSync)
	{
		uint16	uc = BUILD_PIXEL(osd_r5[o->UnderColor], osd_g5[o->UnderColor], osd_b5[o->UnderColor]);

		for (int py = 0; py < height; py++)
		{
			uint16	*line = screen + py * pitch;
			for (int px = 0; px < width; px++)
				line[px] = uc;
		}
	}

	if (!o->DisplayEnable)	// DC=0: backdrop only, no character plane
		return;

	// A 12-dot OSD cell spans ~8 SNES pixels; 18 OSD lines map 1:1. The
	// full grid is 192x216 SNES pixels, centered in the frame; hires
	// (512-wide) frames double the horizontal scale.
	int	xscale = (width >= 512) ? 2 : 1;
	int	xbase = (width - 192 * xscale) / 2;
	int	ybase = (height > 216) ? (height - 216) / 2 : 0;

	int	y = ybase;

	for (int row = 0; row < SFCBOX_OSD_H && y < height; row++)
	{
		uint8	lc = o->LineCtrl[row];
		int		zoomy = (lc & 2) ? 2 : 1;
		int		zoomx = (lc & 4) ? 2 : 1;
		int		cellw = 8 * zoomx * xscale;
		int		cellh = 18 * zoomy;

		for (int py = 0; py < cellh && y + py < height; py++)
		{
			uint16	*line = screen + (y + py) * pitch;
			int		grow = py / zoomy;

			for (int col = 0; col < SFCBOX_OSD_W; col++)
			{
				uint8	ch = o->VRAMChar[row][col];
				if (ch == 0xff)		// transparent space
					continue;

				uint8	attr = o->VRAMColor[row][col];
				uint8	cc = o->ColorMode ? (attr & 7) : 7;
				uint8	bc = o->ColorMode ? ((attr >> 3) & 7) : 0;
				bool8	solid = ((attr & 0x40) || (lc & 1)) ? TRUE : FALSE;

				for (int px = 0; px < cellw; px++)
				{
					int	x = xbase + col * cellw + px;
					if (x < 0 || x >= width)
						continue;

					// Each output pixel covers 1.5 OSD dots; OR the pair so
					// single-dot strokes (dakuten, シ vs ン) survive the
					// downscale the way they do on an analog display.
					int	gx = px / (xscale * zoomx);
					int	d0 = gx * 3 / 2;
					int	d1 = (gx * 3 + 1) / 2;
					if (d0 > 11)	d0 = 11;
					if (d1 > 11)	d1 = 11;

					if (OSDGlyphDot(ch, grow, d0) || OSDGlyphDot(ch, grow, d1))
						line[x] = BUILD_PIXEL(osd_r5[cc], osd_g5[cc], osd_b5[cc]);
					else if (solid)
						line[x] = BUILD_PIXEL(osd_r5[bc], osd_g5[bc], osd_b5[bc]);
				}
			}
		}

		y += cellh;
		if (zoomy == 2)		// a double-height line consumes the next row slot
			row++;
	}
}

// ---------------------------------------------------------------------------
// Z180 bus callbacks

static uint8 SFCBoxMemRead (uint32 addr)
{
	addr &= 0x7ffff;

	if (addr < 0x20000)
		return (SFCBox.KROM[addr & 0xffff]);
	if (addr < 0x40000)
		return (SFCBox.WRAM[addr & 0x7fff]);
	if (addr < 0x60000)
		return (SFCBox.GROM[0] ? SFCBox.GROM[0][addr & 0x7fff] : 0xff);
	return (SFCBox.GROM[1] ? SFCBox.GROM[1][addr & 0x7fff] : 0xff);
}

static void SFCBoxMemWrite (uint32 addr, uint8 byte)
{
	addr &= 0x7ffff;

	if (addr >= 0x20000 && addr < 0x40000)
	{
		uint32	off = addr & 0x7fff;
		// Low 16K is the battery-backed operator-settings area, guarded
		// by [A0h].W bit7.
		if (off < 0x4000 && !SFCBox.WRAMUnlock)
			return;
		SFCBox.WRAM[off] = byte;
	}
}

static uint8 SFCBoxIORead (uint16 port)
{
	uint8	p = (uint8) port;

	if (p >= 0x88 && p <= 0x9f)
		p = 0x80 | (p & 7);
	else if (p >= 0xa1 && p <= 0xbf)
		p = 0xa0;

	switch (p)
	{
		case 0x80:	// Keyswitch and buttons (0 = selected/pressed)
		{
			uint8	v = 0x3f & ~(1 << (SFCBox.Keyswitch <= 5 ? SFCBox.Keyswitch : 1));
			v |= SFCBox.TVGameButton ? 0 : 0x40;
			v |= SFCBox.ResetButton ? 0 : 0x80;
			return (v);
		}

		case 0x81:	// SNES transfer and misc input
		{
			uint8	v = 0;
			v |= (SFCBox.CoinCycles > 0) ? 0 : 0x01;		// /INT0 (coin)
			v |= ((SFCBox.WRIOOut >> 3) & 1) << 1;			// ACK from SNES
			v |= ((SFCBox.WRIOOut >> 4) & 1) << 2;			// DATA from SNES
			v |= (SFCBox.WRIOOut & 1) << 3;					// WRIO bit0
			v |= SFCBox.Int1Line ? 0 : 0x40;				// /INT1 (joypad)
			v |= SFCBox.VBlankToggle ? 0x80 : 0;
			return (v);
		}

		case 0x83:	// Joypad status
		{
			uint8	v = 0x18;	// bits 3/4 read as 1
			v |= SFCBox.JoyReady & 3;
			// Manual serial data taps (KROM polarity: pressed = 0)
			for (int pad = 0; pad < 2; pad++)
			{
				int		pos = SFCBox.JoyShiftPos[pad];
				uint8	bit = 0;
				if (pos < 8)
					bit = (SFCBox.JoyData[pad * 2] >> (7 - pos)) & 1;
				else if (pos < 16)
					bit = (SFCBox.JoyData[pad * 2 + 1] >> (15 - pos)) & 1;
				v |= bit << (6 - pad);	// bit6 = joy1, bit5 = joy2
			}
			return (v);
		}

		case 0x84:	return (SFCBox.JoyData[0]);
		case 0x85:	return (SFCBox.JoyData[1]);
		case 0x86:	return (SFCBox.JoyData[2]);
		case 0x87:	return (SFCBox.JoyData[3]);

		case 0xa0:	// RTC input; bits 3/6/7 idle high
			return (0xc8 | SFCBox.RTC.DataIn);

		default:
			return (0xff);
	}
}

static void SFCBoxIOWrite (uint16 port, uint8 byte)
{
	uint8	p = (uint8) port;

	if (p >= 0x88 && p <= 0x9f)
		p = 0x80 | (p & 7);
	else if (p >= 0xa1 && p <= 0xbf)
		p = 0xa0;

	switch (p)
	{
		case 0x80:	// Lines to the SNES (WRIO/RDIO bits 2/5/1)
			if (TraceEnabled() && ((SFCBox.Port80W ^ byte) & 0x07))
				printf("[box] 80h=%02X (stat=%d clk=%d data=%d) PC=%04X\n",
					   byte, byte & 1, (byte >> 1) & 1, (byte >> 2) & 1, HD64180.PC);
			SFCBox.Port80W = byte;
			break;

		case 0x81:
		{
			uint8	prev = SFCBox.Port81W;
			SFCBox.Port81W = byte;

			// bit6 = watchdog pulse, bit7 = OSD chip-select: both toggle
			// constantly; only bits 0-5 changes are interesting.
			if (TraceEnabled() && ((prev ^ byte) & 0x3f))
				printf("[box] 81h=%02X (was %02X) PC=%04X reset=%d\n",
					   byte, prev, HD64180.PC, byte & 1);

			if ((prev ^ byte) & 0x40)		// watchdog reload
				SFCBox.WatchdogCycles = 0;

			if (!(byte & 0x08))				// INT1 acknowledge
			{
				SFCBox.Int1Line = 0;
				HD64180_SetINT1(FALSE);
			}

			// SNES reset line (bit0: 0 = hold in reset)
			if ((prev & 1) && !(byte & 1))
				SFCBox.SNESHeld = TRUE;
			else if (!(prev & 1) && (byte & 1) && SFCBox.SNESHeld)
			{
				SFCBox.SNESHeld = FALSE;
				SFCBox.PendingSNESReset = TRUE;
			}
			break;
		}

		case 0x83:
		{
			uint8	prev = SFCBox.JoyCtrl;
			SFCBox.JoyCtrl = byte;

			if (TraceEnabled() && ((prev ^ byte) & 0x18))
				printf("[box] 83h=%02X (manual=%d swap=%d) PC=%04X\n",
					   byte, (byte >> 3) & 1, (byte >> 4) & 1, HD64180.PC);

			if (byte & 0x01)	// strobe: latch pads, rewind shifters
			{
				SFCBox.JoyShiftPos[0] = SFCBox.JoyShiftPos[1] = 0;
				uint16	p1 = MovieGetJoypad(0), p2 = MovieGetJoypad(1);
				SFCBox.JoyData[0] = (uint8) ~(p1 >> 8);
				SFCBox.JoyData[1] = (uint8) ~p1;
				SFCBox.JoyData[2] = (uint8) ~(p2 >> 8);
				SFCBox.JoyData[3] = (uint8) ~p2;
			}
			// Clock pulses (0 = clocking): joy1 bit2, joy2 bit1
			if ((prev & 0x04) && !(byte & 0x04) && SFCBox.JoyShiftPos[0] < 31)
				SFCBox.JoyShiftPos[0]++;
			if ((prev & 0x02) && !(byte & 0x02) && SFCBox.JoyShiftPos[1] < 31)
				SFCBox.JoyShiftPos[1]++;
			break;
		}

		case 0x84:	case 0x85:	case 0x86:	case 0x87:
			if (TraceEnabled())
				printf("[box] %02Xh=%02X (joy inject) PC=%04X\n", p, byte, HD64180.PC);
			SFCBox.JoyInject[p - 0x84] = byte;
			break;

		case 0xa0:
			RTCPinWrite(byte);
			break;

		case 0xc0:
			if (TraceEnabled() && SFCBox.MapReg0 != byte)
				printf("[box] C0h=%02X (socket=%d slot=%d sram=%d dsp=%d hi=%d) PC=%04X\n",
					   byte, byte & 3, (byte >> 2) & 1, (byte >> 3) & 1, (byte >> 5) & 1, byte >> 7, HD64180.PC);
			if (SFCBox.MapReg0 != byte)
			{
				SFCBox.MapReg0 = byte;
				S9xSFCBoxRemap();
			}
			break;

		case 0xc1:
			if (TraceEnabled() && SFCBox.MapReg1 != byte)
				printf("[box] C1h=%02X (mode=%d srambase=%d sramsz=%d) PC=%04X\n",
					   byte, byte & 3, (byte >> 2) & 3, (byte >> 6) & 3, HD64180.PC);
			if (SFCBox.MapReg1 != byte)
			{
				SFCBox.MapReg1 = byte;
				S9xSFCBoxRemap();
			}
			break;

		default:
			break;
	}
}

static void SFCBoxCSIOTransmit (uint8 byte)
{
	if (SFCBox.Port81W & 0x80)	// OSD chip-selected
		OSDReceiveByte(byte);
}

// ---------------------------------------------------------------------------
// SNES-visible hardware

void S9xSFCBoxSetWRIO (uint8 byte)
{
	if (TraceEnabled() && ((SFCBox.WRIOOut ^ byte) & 0x39))
		printf("[snes] WRIO=%02X (rdy=%d ack=%d data=%d)\n",
			   byte, byte & 1, (byte >> 3) & 1, (byte >> 4) & 1);
	SFCBox.WRIOOut = byte;
}

uint8 S9xSFCBoxGetRDIO (void)
{
	// The SNES reads back its own outputs on bits 0/3/4/6/7; the KROM
	// drives bit2 (STAT), bit5 (CLOCK) and bit1 (DATA) via [80h].W.
	uint8	v = SFCBox.WRIOOut & ~0x26;

	if (SFCBox.Port80W & 0x01)	v |= 0x04;
	if (SFCBox.Port80W & 0x02)	v |= 0x20;
	if (SFCBox.Port80W & 0x04)	v |= 0x02;

	if (TraceEnabled())
	{
		static uint8	last = 0xee;
		static uint32	count = 0;
		count++;
		if (v != last)
		{
			printf("[snes] RDIO=%02X (stat=%d clk=%d data=%d) after %u reads\n",
				   v, (v >> 2) & 1, (v >> 5) & 1, (v >> 1) & 1, count);
			last = v;
			count = 0;
		}
	}
	return (v);
}

// The SNES strobed the joypads: raise INT1 toward the supervisor. The
// KROM acknowledges via [81h].W bit3 (which drops the line again).
void S9xSFCBoxJoypadAccessed (void)
{
	if (!SFCBox.Active)
		return;
	SFCBox.Int1Line = 1;
	HD64180_SetINT1(TRUE);
}

// Manual forwarding mode ([83h].W bit3): the KROM's [84h-87h] latches
// replace the real controllers on the SNES side (demo playback, and the
// KROM<->ATROM bulk transfer path). Wire polarity is inverted vs $4218.
bool8 S9xSFCBoxJoypadOverride (int pad, uint16 *value)
{
	if (!SFCBox.Active || !(SFCBox.JoyCtrl & 0x08))
		return (FALSE);

	int	p = pad & 1;
	if (SFCBox.JoyCtrl & 0x10)
		p ^= 1;

	*value = (uint16) ~(((uint16) SFCBox.JoyInject[p * 2] << 8) | SFCBox.JoyInject[p * 2 + 1]);
	return (TRUE);
}

static inline uint32 SFCBoxSRAMOffset (uint32 address)
{
	uint32	off;

	if (SFCBox.SRAMHiROM)
		off = ((address & 0xf0000) >> 3) | ((address & 0x7fff) - 0x6000);
	else
		off = ((address & 0xf0000) >> 1) | (address & 0x7fff);

	return (SFCBox.SRAMWindowBase + (off & SFCBox.SRAMWindowMask));
}

uint8 S9xGetSFCBoxSRAM (uint32 address)
{
	return (*(Memory.SRAM + SFCBoxSRAMOffset(address)));
}

void S9xSetSFCBoxSRAM (uint8 byte, uint32 address)
{
	*(Memory.SRAM + SFCBoxSRAMOffset(address)) = byte;
	CPU.SRAMModified = TRUE;
}

// ---------------------------------------------------------------------------
// Front panel

void S9xSFCBoxInsertCoin (void)
{
	SFCBox.CoinCycles = SFCBOX_COIN_CYCLES;
}

// ---------------------------------------------------------------------------
// NVRAM (KROM work RAM + RTC SRAM pages) — "<rom>.box" sidecar

bool8 S9xSFCBoxLoadNVRAM (void)
{
	std::string	name = S9xGetFilename(".box", SRAM_DIR);
	FILE		*fp = fopen(name.c_str(), "rb");

	if (!fp)
		return (FALSE);

	size_t	ok = fread(SFCBox.WRAM, 1, SFCBOX_WRAM_SIZE, fp);
	ok += fread(SFCBox.RTC.SRAMPages, 1, sizeof(SFCBox.RTC.SRAMPages), fp);
	fclose(fp);
	return (ok == SFCBOX_WRAM_SIZE + sizeof(SFCBox.RTC.SRAMPages));
}

bool8 S9xSFCBoxSaveNVRAM (void)
{
	std::string	name = S9xGetFilename(".box", SRAM_DIR);
	FILE		*fp = fopen(name.c_str(), "wb");

	if (!fp)
		return (FALSE);

	fwrite(SFCBox.WRAM, 1, SFCBOX_WRAM_SIZE, fp);
	fwrite(SFCBox.RTC.SRAMPages, 1, sizeof(SFCBox.RTC.SRAMPages), fp);
	fclose(fp);
	return (TRUE);
}

// ---------------------------------------------------------------------------
// Savestates. The dynamic supervisor state (Z180 core + board latches +
// battery WRAM + RTC + OSD plane) travels as one packed POD mirror inside
// a magic/version/size-guarded blob. Loader-owned data (KROM, GROM
// pointers, socket tables, OSD font) is re-derived at ROM load and is
// deliberately NOT part of the state.

#define SFCBOX_STATE_VERSION	2

struct SSFCBoxSaveState
{
	struct SHD64180		Z180;

	uint8	WRAM[SFCBOX_WRAM_SIZE];
	uint8	Port80W, Port81W, MapReg0, MapReg1;
	uint8	Keyswitch, ResetButton, TVGameButton;
	int32	CoinCycles;
	uint8	WRIOOut, SNESHeld, PendingSNESReset, WRAMUnlock;
	int32	LastVCounter;
	uint8	JoyCtrl, JoyData[4], JoyInject[4], JoyReady;
	int32	JoyShiftPos[2];
	uint8	Int1Line, VBlankToggle;
	int32	WatchdogCycles;
	int64	CycleRemainder;

	struct SSFCBoxRTC	RTC;

	uint8	OSDPendingCmd;
	uint8	OSDVRAMChar[SFCBOX_OSD_H][SFCBOX_OSD_W];
	uint8	OSDVRAMColor[SFCBOX_OSD_H][SFCBOX_OSD_W];
	uint8	OSDCursorX, OSDCursorY, OSDFillMode;
	uint8	OSDCharColor, OSDBgColor;
	uint8	OSDDisplayEnable, OSDColorMode;
	uint8	OSDLineCtrl[SFCBOX_OSD_H];
	uint8	OSDUnderColor, OSDXOfs, OSDYOfs;
	uint8	OSDExtSync;
};

size_t S9xSFCBoxStateSize (void)
{
	return (8 + sizeof(struct SSFCBoxSaveState));
}

void S9xSFCBoxStateSave (uint8 *buf)
{
	memcpy(buf, "BOX!", 4);
	buf[4] = SFCBOX_STATE_VERSION;
	buf[5] = 0;
	buf[6] = (uint8) (sizeof(struct SSFCBoxSaveState) & 0xff);
	buf[7] = (uint8) ((sizeof(struct SSFCBoxSaveState) >> 8) & 0xff);

	struct SSFCBoxSaveState	*s = (struct SSFCBoxSaveState *) (buf + 8);
	memset(s, 0, sizeof(*s));

	s->Z180 = HD64180;

	memcpy(s->WRAM, SFCBox.WRAM, SFCBOX_WRAM_SIZE);
	s->Port80W = SFCBox.Port80W;			s->Port81W = SFCBox.Port81W;
	s->MapReg0 = SFCBox.MapReg0;			s->MapReg1 = SFCBox.MapReg1;
	s->Keyswitch = SFCBox.Keyswitch;
	s->ResetButton = SFCBox.ResetButton;	s->TVGameButton = SFCBox.TVGameButton;
	s->CoinCycles = SFCBox.CoinCycles;
	s->WRIOOut = SFCBox.WRIOOut;
	s->SNESHeld = SFCBox.SNESHeld;			s->PendingSNESReset = SFCBox.PendingSNESReset;
	s->WRAMUnlock = SFCBox.WRAMUnlock;
	s->LastVCounter = SFCBox.LastVCounter;
	s->JoyCtrl = SFCBox.JoyCtrl;
	memcpy(s->JoyData, SFCBox.JoyData, 4);
	memcpy(s->JoyInject, SFCBox.JoyInject, 4);
	s->JoyReady = SFCBox.JoyReady;
	s->JoyShiftPos[0] = SFCBox.JoyShiftPos[0];
	s->JoyShiftPos[1] = SFCBox.JoyShiftPos[1];
	s->Int1Line = SFCBox.Int1Line;			s->VBlankToggle = SFCBox.VBlankToggle;
	s->WatchdogCycles = SFCBox.WatchdogCycles;
	s->CycleRemainder = (int64) SFCBox.CycleRemainder;

	s->RTC = SFCBox.RTC;

	s->OSDPendingCmd = SFCBox.OSD.PendingCmd;
	memcpy(s->OSDVRAMChar, SFCBox.OSD.VRAMChar, sizeof(s->OSDVRAMChar));
	memcpy(s->OSDVRAMColor, SFCBox.OSD.VRAMColor, sizeof(s->OSDVRAMColor));
	s->OSDCursorX = SFCBox.OSD.CursorX;		s->OSDCursorY = SFCBox.OSD.CursorY;
	s->OSDFillMode = SFCBox.OSD.FillMode;
	s->OSDCharColor = SFCBox.OSD.CharColor;	s->OSDBgColor = SFCBox.OSD.BgColor;
	s->OSDDisplayEnable = SFCBox.OSD.DisplayEnable;
	s->OSDColorMode = SFCBox.OSD.ColorMode;
	memcpy(s->OSDLineCtrl, SFCBox.OSD.LineCtrl, sizeof(s->OSDLineCtrl));
	s->OSDUnderColor = SFCBox.OSD.UnderColor;
	s->OSDXOfs = SFCBox.OSD.XOfs;			s->OSDYOfs = SFCBox.OSD.YOfs;
	s->OSDExtSync = SFCBox.OSD.ExtSync;
}

bool8 S9xSFCBoxStateLoad (const uint8 *buf, size_t size)
{
	if (size < 8 + sizeof(struct SSFCBoxSaveState) ||
		memcmp(buf, "BOX!", 4) != 0 ||
		buf[4] != SFCBOX_STATE_VERSION ||
		(buf[6] | (buf[7] << 8)) != (int) sizeof(struct SSFCBoxSaveState))
	{
		printf("SFC-Box: incompatible supervisor state in snapshot; power-cycling the board instead.\n");
		return (FALSE);
	}

	const struct SSFCBoxSaveState	*s = (const struct SSFCBoxSaveState *) (buf + 8);

	HD64180 = s->Z180;

	memcpy(SFCBox.WRAM, s->WRAM, SFCBOX_WRAM_SIZE);
	SFCBox.Port80W = s->Port80W;			SFCBox.Port81W = s->Port81W;
	SFCBox.MapReg0 = s->MapReg0;			SFCBox.MapReg1 = s->MapReg1;
	SFCBox.Keyswitch = s->Keyswitch;
	SFCBox.ResetButton = s->ResetButton;	SFCBox.TVGameButton = s->TVGameButton;
	SFCBox.CoinCycles = s->CoinCycles;
	SFCBox.WRIOOut = s->WRIOOut;
	SFCBox.SNESHeld = s->SNESHeld;			SFCBox.PendingSNESReset = s->PendingSNESReset;
	SFCBox.WRAMUnlock = s->WRAMUnlock;
	SFCBox.LastVCounter = s->LastVCounter;
	SFCBox.JoyCtrl = s->JoyCtrl;
	memcpy(SFCBox.JoyData, s->JoyData, 4);
	memcpy(SFCBox.JoyInject, s->JoyInject, 4);
	SFCBox.JoyReady = s->JoyReady;
	SFCBox.JoyShiftPos[0] = s->JoyShiftPos[0];
	SFCBox.JoyShiftPos[1] = s->JoyShiftPos[1];
	SFCBox.Int1Line = s->Int1Line;			SFCBox.VBlankToggle = s->VBlankToggle;
	SFCBox.WatchdogCycles = s->WatchdogCycles;
	SFCBox.CycleRemainder = s->CycleRemainder;

	SFCBox.RTC = s->RTC;

	SFCBox.OSD.PendingCmd = s->OSDPendingCmd;
	memcpy(SFCBox.OSD.VRAMChar, s->OSDVRAMChar, sizeof(s->OSDVRAMChar));
	memcpy(SFCBox.OSD.VRAMColor, s->OSDVRAMColor, sizeof(s->OSDVRAMColor));
	SFCBox.OSD.CursorX = s->OSDCursorX;		SFCBox.OSD.CursorY = s->OSDCursorY;
	SFCBox.OSD.FillMode = s->OSDFillMode;
	SFCBox.OSD.CharColor = s->OSDCharColor;	SFCBox.OSD.BgColor = s->OSDBgColor;
	SFCBox.OSD.DisplayEnable = s->OSDDisplayEnable;
	SFCBox.OSD.ColorMode = s->OSDColorMode;
	memcpy(SFCBox.OSD.LineCtrl, s->OSDLineCtrl, sizeof(s->OSDLineCtrl));
	SFCBox.OSD.UnderColor = s->OSDUnderColor;
	SFCBox.OSD.XOfs = s->OSDXOfs;			SFCBox.OSD.YOfs = s->OSDYOfs;
	SFCBox.OSD.ExtSync = s->OSDExtSync;

	// The Z180's interrupt lines are level state we just restored; make
	// sure the core's view matches the board's.
	HD64180_SetINT1(SFCBox.Int1Line ? TRUE : FALSE);
	HD64180_SetINT0(SFCBox.CoinCycles > 0);

	return (TRUE);
}

void S9xSFCBoxPostLoadState (void)
{
	// Rebuild the SNES-visible map from the restored mapping registers
	// (stages the GSU view and re-arms/disarms the SuperFX as needed).
	S9xSFCBoxRemap();
}

// ---------------------------------------------------------------------------
// BIOS loading

static bool8 LoadBIOSFile (const char *name, uint8 *dest, uint32 size, uint32 minsize)
{
	// Probe order: the port's configured BIOS folder, then the ROM's own
	// directory, then the conventional BIOS/ folder inside or beside the
	// ROM folder (games in Roms/, BIOS files in a sibling BIOS/), then
	// whatever directory ROMFilename carries (libretro passes only the
	// basename to the loader, so it may have none).
	std::string	romdir = S9xGetDirectory(ROMFILENAME_DIR);
	std::string	dirs[5];
	int			ndirs = 0;

	dirs[ndirs++] = S9xGetDirectory(BIOS_DIR);
	if (!romdir.empty())
	{
		dirs[ndirs++] = romdir;
		dirs[ndirs++] = romdir + SLASH_STR + "BIOS";
		dirs[ndirs++] = romdir + SLASH_STR + ".." + SLASH_STR + "BIOS";
	}

	std::string	fromname = Memory.ROMFilename;
	size_t		slash = fromname.find_last_of("/\\");
	if (slash != std::string::npos)
		dirs[ndirs++] = fromname.substr(0, slash);

	FILE	*fp = NULL;
	for (int i = 0; i < ndirs && !fp; i++)
	{
		std::string	path = dirs[i] + SLASH_STR + name;
		fp = fopen(path.c_str(), "rb");
	}

	if (!fp)
		return (FALSE);

	size_t	got = fread(dest, 1, size, fp);
	fclose(fp);
	return (got >= minsize);
}

bool8 S9xSFCBoxLoadKROM (void)
{
	if (!LoadBIOSFile("KROM1.BIN", SFCBox.KROM, SFCBOX_KROM_SIZE, SFCBOX_KROM_SIZE) &&
		!LoadBIOSFile("KROM.BIN",  SFCBox.KROM, SFCBOX_KROM_SIZE, SFCBOX_KROM_SIZE) &&
		!LoadBIOSFile("krom1.bin", SFCBox.KROM, SFCBOX_KROM_SIZE, SFCBOX_KROM_SIZE))
	{
		printf("SFC-Box: KROM1.BIN not found in the BIOS directory (get it from "
			   "https://archive.org/details/super-famicom-box-bios).\n");
		return (FALSE);
	}

	SFCBox.OSD.FontLoaded =
		LoadBIOSFile("MB90082.BIN", SFCBox.OSD.Font, SFCBOX_FONT_SIZE, SFCBOX_FONT_SIZE);
	if (!SFCBox.OSD.FontLoaded)
		printf("SFC-Box: MB90082.BIN (OSD font) not found; the supervisor overlay will be invisible.\n");

	return (TRUE);
}

// ---------------------------------------------------------------------------
// Power / reset / main-loop pump

void S9xSFCBoxDeactivate (void)
{
	SFCBox.Active = FALSE;
}

void S9xSFCBoxPowerOn (void)
{
	// Preserve the loader-installed bits across the state wipe.
	uint8	krom[SFCBOX_KROM_SIZE];
	struct SSFCBoxOSD	osd = SFCBox.OSD;
	uint8	*grom0 = SFCBox.GROM[0], *grom1 = SFCBox.GROM[1];
	uint32	romoff[2][4], romsize[2][4];
	uint8	chipset[2] = { SFCBox.SlotChipset[0], SFCBox.SlotChipset[1] };
	bool8	present[2] = { SFCBox.SlotPresent[0], SFCBox.SlotPresent[1] };

	memcpy(krom, SFCBox.KROM, SFCBOX_KROM_SIZE);
	memcpy(romoff, SFCBox.RomOffset, sizeof(romoff));
	memcpy(romsize, SFCBox.RomSize, sizeof(romsize));

	memset(&SFCBox, 0, sizeof(SFCBox));

	memcpy(SFCBox.KROM, krom, SFCBOX_KROM_SIZE);
	SFCBox.OSD = osd;
	memset(SFCBox.OSD.VRAMChar, 0xff, sizeof(SFCBox.OSD.VRAMChar));
	SFCBox.GROM[0] = grom0;
	SFCBox.GROM[1] = grom1;
	memcpy(SFCBox.RomOffset, romoff, sizeof(romoff));
	memcpy(SFCBox.RomSize, romsize, sizeof(romsize));
	SFCBox.SlotChipset[0] = chipset[0];
	SFCBox.SlotChipset[1] = chipset[1];
	SFCBox.SlotPresent[0] = present[0];
	SFCBox.SlotPresent[1] = present[1];

	SFCBox.Active = TRUE;
	SFCBox.Keyswitch = 1;		// "ON" (play mode)
	SFCBox.WRIOOut = 0xff;
	SFCBox.SNESHeld = TRUE;		// the KROM releases us when it's ready
	SFCBox.LastVCounter = -1;

	S9xSFCBoxLoadNVRAM();
	RTCLoadHostTime();

	HD64180CB.MemRead = SFCBoxMemRead;
	HD64180CB.MemWrite = SFCBoxMemWrite;
	HD64180CB.IORead = SFCBoxIORead;
	HD64180CB.IOWrite = SFCBoxIOWrite;
	HD64180CB.CSIOTransmit = SFCBoxCSIOTransmit;
	HD64180_Reset();

	S9xSFCBoxRemap();

	printf("SFC-Box: supervisor board powered on (KROM in control).\n");
}

bool8 S9xSFCBoxSNESHeld (void)
{
	return (SFCBox.Active && SFCBox.SNESHeld);
}

bool8 S9xSFCBoxPendingReset (void)
{
	return (SFCBox.Active && SFCBox.PendingSNESReset);
}

void S9xSFCBoxApplySNESReset (void)
{
	SFCBox.PendingSNESReset = FALSE;
	S9xSoftReset();
}

void S9xSFCBoxEndScanline (void)
{
	if (!SFCBox.Active)
		return;

	// Frame-granular housekeeping at wraparound
	if (CPU.V_Counter == 0 && SFCBox.LastVCounter != 0)
	{
		SFCBox.VBlankToggle ^= 1;

		// Auto-mode joypad capture: fresh controller data "arrives" for
		// the KROM once per frame, in the inverted MSB-first layout.
		uint16	p1 = MovieGetJoypad(0), p2 = MovieGetJoypad(1);
		SFCBox.JoyData[0] = (uint8) ~(p1 >> 8);
		SFCBox.JoyData[1] = (uint8) ~p1;
		SFCBox.JoyData[2] = (uint8) ~(p2 >> 8);
		SFCBox.JoyData[3] = (uint8) ~p2;
		SFCBox.JoyReady = 3;

		// RTC seconds
		if (++SFCBox.RTC.FrameAccum >= (Settings.PAL ? 50 : 60))
		{
			SFCBox.RTC.FrameAccum = 0;
			RTCTickSecond();
		}
	}
	else if (CPU.V_Counter == (int32) (Timings.V_Max >> 1) &&
			 SFCBox.LastVCounter != (int32) (Timings.V_Max >> 1))
		SFCBox.VBlankToggle ^= 1;	// [81h].R bit7 flips at ~120 Hz

	SFCBox.LastVCounter = CPU.V_Counter;

	// One Z180 PC sample per second under tracing: enough to spot where
	// the KROM spins without drowning the interesting one-shot events.
	if (TraceEnabled() && CPU.V_Counter == 0 && SFCBox.RTC.FrameAccum == 0)
	{
		printf("[box] Z180 PC=%04X (phys %05X) held=%d 80W=%02X 81W=%02X halted=%d IFF=%d\n",
			   HD64180.PC, HD64180_TranslateAddr(HD64180.PC), SFCBox.SNESHeld,
			   SFCBox.Port80W, SFCBox.Port81W, HD64180.Halted, HD64180.IFF1);
		if (Settings.SuperFX)
			printf("[gsu] SFR=%02X%02X PBR=%02X ROMBR=%02X SCMR=%02X CLSR=%02X\n",
				   Memory.FillRAM[0x3031], Memory.FillRAM[0x3030],
				   Memory.FillRAM[0x3034], Memory.FillRAM[0x3036],
				   Memory.FillRAM[0x303a], Memory.FillRAM[0x3039]);
	}

	// PHI cycles for one SNES scanline (H_Max master clocks)
	const int64	master = Settings.PAL ? 21281370 : 21477272;
	int64		num = (int64) Timings.H_Max * SFCBOX_PHI + SFCBox.CycleRemainder;
	int32		cycles = (int32) (num / master);
	SFCBox.CycleRemainder = num % master;

	HD64180_SetINT0(SFCBox.CoinCycles > 0);

	int32	ran = HD64180_Execute(cycles);

	if (SFCBox.CoinCycles > 0)
		SFCBox.CoinCycles -= ran;

	SFCBox.WatchdogCycles += ran;
	if (SFCBox.WatchdogCycles > SFCBOX_WATCHDOG_CYCLES)
	{
		printf("SFC-Box: watchdog expired, resetting the supervisor board.\n");
		SFCBox.WatchdogCycles = 0;
		SFCBox.SNESHeld = TRUE;
		HD64180_Reset();
	}
}
