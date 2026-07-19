/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// HD64180 (Z180) core for the Super Famicom Box supervisor board.
// See hd64180.h for scope and docs/sfcbox.md for the board context.
//
// Dispatch is a plain switch over the primary opcode with an index-mode
// parameter for the DD/FD prefixes. The HD64180 traps every undocumented
// encoding (unlike NMOS Z80s which execute them), so the prefix decoder
// only whitelists the documented (IX/IY) forms and routes everything else
// to Trap(). Cycle counts are accumulated at bus-access granularity
// (fetch 4 / memory 3 / io 4 + per-class internal cycles) — close enough
// for the KROM's software-handshaked serial links, not cycle-exact.

#include <string.h>
#include "port.h"
#include "hd64180.h"

struct SHD64180			HD64180;
struct SHD64180Callbacks	HD64180CB;

#define z	HD64180

// Flag bits
#define FL_C	0x01
#define FL_N	0x02
#define FL_P	0x04	// parity/overflow
#define FL_X	0x08
#define FL_H	0x10
#define FL_Y	0x20
#define FL_Z	0x40
#define FL_S	0x80

// ITC (INT/TRAP control, on-chip reg 34h)
#define ITC_ITE0	0x01
#define ITC_ITE1	0x02
#define ITC_ITE2	0x04
#define ITC_UFO		0x40
#define ITC_TRAP	0x80

// TCR (timer control, on-chip reg 10h)
#define TCR_TDE0	0x01
#define TCR_TDE1	0x02
#define TCR_TIE0	0x10
#define TCR_TIE1	0x20
#define TCR_TIF0	0x40
#define TCR_TIF1	0x80

// CNTR (CSI/O control, on-chip reg 0Ah)
#define CNTR_SS		0x07
#define CNTR_TE		0x10
#define CNTR_RE		0x20
#define CNTR_EIE	0x40
#define CNTR_EF		0x80

static uint8	ParityTable[256];
static bool8	TablesBuilt = FALSE;

static void BuildTables (void)
{
	for (int i = 0; i < 256; i++)
	{
		int bits = 0;
		for (int b = 0; b < 8; b++)
			bits += (i >> b) & 1;
		ParityTable[i] = (bits & 1) ? 0 : FL_P;
	}

	TablesBuilt = TRUE;
}

// ---------------------------------------------------------------------------
// MMU + bus access

uint32 HD64180_TranslateAddr (uint16 logical)
{
	uint32	page = logical >> 12;

	if (page >= (uint32) (z.CBAR >> 4))
		return ((logical + ((uint32) z.CBR << 12)) & 0xfffff);
	if (page >= (uint32) (z.CBAR & 0x0f))
		return ((logical + ((uint32) z.BBR << 12)) & 0xfffff);
	return (logical);
}

static inline uint8 RdMem (uint16 addr)
{
	z.Cycles += 3;
	return (HD64180CB.MemRead(HD64180_TranslateAddr(addr)));
}

static inline void WrMem (uint16 addr, uint8 byte)
{
	z.Cycles += 3;
	HD64180CB.MemWrite(HD64180_TranslateAddr(addr), byte);
}

static inline uint8 Fetch (void)
{
	z.Cycles += 3;
	return (HD64180CB.MemRead(HD64180_TranslateAddr(z.PC++)));
}

static inline uint8 FetchM1 (void)
{
	z.Cycles += 4;
	z.R = (z.R & 0x80) | ((z.R + 1) & 0x7f);
	return (HD64180CB.MemRead(HD64180_TranslateAddr(z.PC++)));
}

static inline uint16 Fetch16 (void)
{
	uint16	lo = Fetch();
	return (lo | ((uint16) Fetch() << 8));
}

static inline uint16 RdMem16 (uint16 addr)
{
	uint16	lo = RdMem(addr);
	return (lo | ((uint16) RdMem(addr + 1) << 8));
}

static inline void WrMem16 (uint16 addr, uint16 word)
{
	WrMem(addr, (uint8) word);
	WrMem(addr + 1, (uint8) (word >> 8));
}

static inline void Push16 (uint16 word)
{
	z.SP -= 2;
	WrMem(z.SP, (uint8) word);
	WrMem((uint16) (z.SP + 1), (uint8) (word >> 8));
}

static inline uint16 Pop16 (void)
{
	uint16	word = RdMem16(z.SP);
	z.SP += 2;
	return (word);
}

// Register-pair helpers
static inline uint16 GetBC (void)	{ return (((uint16) z.B << 8) | z.C); }
static inline uint16 GetDE (void)	{ return (((uint16) z.D << 8) | z.E); }
static inline uint16 GetHL (void)	{ return (((uint16) z.H << 8) | z.L); }
static inline uint16 GetAF (void)	{ return (((uint16) z.A << 8) | z.F); }
static inline void SetBC (uint16 v)	{ z.B = v >> 8; z.C = (uint8) v; }
static inline void SetDE (uint16 v)	{ z.D = v >> 8; z.E = (uint8) v; }
static inline void SetHL (uint16 v)	{ z.H = v >> 8; z.L = (uint8) v; }
static inline void SetAF (uint16 v)	{ z.A = v >> 8; z.F = (uint8) v; }

// ixmode: 0 = HL, 1 = IX, 2 = IY (for the DD/FD prefixed forms)
static inline uint16 GetIdx (int ixmode)
{
	return (ixmode == 1 ? z.IX : ixmode == 2 ? z.IY : GetHL());
}

static inline void SetIdx (int ixmode, uint16 v)
{
	if (ixmode == 1)		z.IX = v;
	else if (ixmode == 2)	z.IY = v;
	else					SetHL(v);
}

// Effective address of the (HL) / (IX+d) / (IY+d) memory operand. In index
// mode this fetches the displacement byte (and costs the extra internal
// cycles the Z180 spends on the add).
static inline uint16 EffAddr (int ixmode)
{
	if (!ixmode)
		return (GetHL());

	int8	d = (int8) Fetch();
	z.Cycles += 5;
	return ((uint16) (GetIdx(ixmode) + d));
}

// ---------------------------------------------------------------------------
// On-chip I/O (00h-3Fh at the reset ICR base; the SFC-Box never moves it)

static uint8 OnChipRead (uint8 port)
{
	switch (port)
	{
		case 0x00:	return (z.CNTLA0);
		case 0x01:	return (z.CNTLA1);
		case 0x02:	return (z.CNTLB0);
		case 0x03:	return (z.CNTLB1);
		// ASCI stubs: TDRE (bit1) always ready, RDRF (bit7) never set —
		// the KROM's ASCI use is a dead demo-recording feature.
		case 0x04:	return ((z.STAT0 & ~0x80) | 0x02);
		case 0x05:	return ((z.STAT1 & ~0x80) | 0x02);
		case 0x06:	return (z.TDR0);
		case 0x07:	return (z.TDR1);
		case 0x08:	return (z.RDR0);
		case 0x09:	return (z.RDR1);
		case 0x0a:	return (z.CNTR);
		case 0x0b:	return (z.TRDR);

		case 0x0c:	// TMDR0L: reading latches the current TIF0-clear step
			if (z.TIF0Read)
			{
				z.TCR &= ~TCR_TIF0;
				z.TIF0Read = FALSE;
			}
			return ((uint8) z.TMDR0);
		case 0x0d:
			if (z.TIF0Read)
			{
				z.TCR &= ~TCR_TIF0;
				z.TIF0Read = FALSE;
			}
			return ((uint8) (z.TMDR0 >> 8));
		case 0x0e:	return ((uint8) z.RLDR0);
		case 0x0f:	return ((uint8) (z.RLDR0 >> 8));

		case 0x10:	// TCR: arms the TIF-clear sequences
			z.TIF0Read = (z.TCR & TCR_TIF0) ? TRUE : FALSE;
			z.TIF1Read = (z.TCR & TCR_TIF1) ? TRUE : FALSE;
			return (z.TCR);

		case 0x14:
			if (z.TIF1Read)
			{
				z.TCR &= ~TCR_TIF1;
				z.TIF1Read = FALSE;
			}
			return ((uint8) z.TMDR1);
		case 0x15:
			if (z.TIF1Read)
			{
				z.TCR &= ~TCR_TIF1;
				z.TIF1Read = FALSE;
			}
			return ((uint8) (z.TMDR1 >> 8));
		case 0x16:	return ((uint8) z.RLDR1);
		case 0x17:	return ((uint8) (z.RLDR1 >> 8));

		case 0x18:	return (z.FRC);
		case 0x32:	return (z.DCNTL);
		case 0x33:	return (z.IL);
		case 0x34:	return (z.ITC);
		case 0x36:	return (z.RCR);
		case 0x38:	return (z.CBR);
		case 0x39:	return (z.BBR);
		case 0x3a:	return (z.CBAR);
		case 0x3f:	return (z.ICR);

		default:	return (0xff);
	}
}

static void OnChipWrite (uint8 port, uint8 byte)
{
	switch (port)
	{
		case 0x00:	z.CNTLA0 = byte;	break;
		case 0x01:	z.CNTLA1 = byte;	break;
		case 0x02:	z.CNTLB0 = byte;	break;
		case 0x03:	z.CNTLB1 = byte;	break;
		case 0x04:	z.STAT0 = (z.STAT0 & ~0x09) | (byte & 0x09);	break;
		case 0x05:	z.STAT1 = (z.STAT1 & ~0x09) | (byte & 0x09);	break;
		case 0x06:	z.TDR0 = byte;		break;	// swallowed (recorder is unused)
		case 0x07:	z.TDR1 = byte;		break;

		case 0x0a:
			// CSI/O control. Setting TE starts an 8-bit transmit clocked
			// at PHI / (20 << SS); TE drops and EF raises when it lands.
			// Writing EF=0 clears the end flag.
			z.CNTR = (z.CNTR & CNTR_EF) | (byte & ~CNTR_EF);
			if (!(byte & CNTR_EF))
				z.CNTR &= ~CNTR_EF;
			if ((byte & CNTR_TE) && z.CSIOCountdown <= 0)
				z.CSIOCountdown = 8 * (20 << (z.CNTR & CNTR_SS));
			break;

		case 0x0b:	z.TRDR = byte;		break;

		case 0x0c:	z.TMDR0 = (z.TMDR0 & 0xff00) | byte;					break;
		case 0x0d:	z.TMDR0 = (z.TMDR0 & 0x00ff) | ((uint16) byte << 8);	break;
		case 0x0e:	z.RLDR0 = (z.RLDR0 & 0xff00) | byte;					break;
		case 0x0f:	z.RLDR0 = (z.RLDR0 & 0x00ff) | ((uint16) byte << 8);	break;

		case 0x10:
			// TIF bits are read-only from the bus side.
			z.TCR = (z.TCR & (TCR_TIF0 | TCR_TIF1)) | (byte & 0x3f);
			break;

		case 0x14:	z.TMDR1 = (z.TMDR1 & 0xff00) | byte;					break;
		case 0x15:	z.TMDR1 = (z.TMDR1 & 0x00ff) | ((uint16) byte << 8);	break;
		case 0x16:	z.RLDR1 = (z.RLDR1 & 0xff00) | byte;					break;
		case 0x17:	z.RLDR1 = (z.RLDR1 & 0x00ff) | ((uint16) byte << 8);	break;

		case 0x32:	z.DCNTL = byte;	break;
		case 0x33:	z.IL = byte & 0xe0;	break;

		case 0x34:
			// TRAP is cleared by writing 0, never set; UFO is read-only.
			z.ITC = (z.ITC & ITC_UFO) | (z.ITC & ITC_TRAP & byte) | (byte & 0x07);
			break;

		case 0x36:	z.RCR = byte;	break;
		case 0x38:	z.CBR = byte;	break;
		case 0x39:	z.BBR = byte;	break;
		case 0x3a:	z.CBAR = byte;	break;
		case 0x3f:	z.ICR = byte & 0xe0;	break;

		default:	break;
	}
}

static inline uint8 RdIO (uint16 port)
{
	z.Cycles += 4;
	if ((port & 0xffc0) == 0)
		return (OnChipRead((uint8) port));
	return (HD64180CB.IORead(port));
}

static inline void WrIO (uint16 port, uint8 byte)
{
	z.Cycles += 4;
	if ((port & 0xffc0) == 0)
	{
		OnChipWrite((uint8) port, byte);
		return;
	}
	HD64180CB.IOWrite(port, byte);
}

// ---------------------------------------------------------------------------
// ALU helpers

static inline void SetSZP (uint8 v)
{
	z.F = (z.F & FL_C) | (v & (FL_S | FL_X | FL_Y)) | (v ? 0 : FL_Z) | ParityTable[v];
}

static inline uint8 Inc8 (uint8 v)
{
	uint8	r = v + 1;
	z.F = (z.F & FL_C) | (r & (FL_S | FL_X | FL_Y)) | (r ? 0 : FL_Z) |
		  ((r & 0x0f) == 0 ? FL_H : 0) | (r == 0x80 ? FL_P : 0);
	return (r);
}

static inline uint8 Dec8 (uint8 v)
{
	uint8	r = v - 1;
	z.F = (z.F & FL_C) | FL_N | (r & (FL_S | FL_X | FL_Y)) | (r ? 0 : FL_Z) |
		  ((r & 0x0f) == 0x0f ? FL_H : 0) | (r == 0x7f ? FL_P : 0);
	return (r);
}

static inline void Add8 (uint8 v, uint8 carry)
{
	uint16	r = z.A + v + carry;
	uint8	res = (uint8) r;
	z.F = (res & (FL_S | FL_X | FL_Y)) | (res ? 0 : FL_Z) |
		  (((z.A ^ v ^ res) & 0x10) ? FL_H : 0) |
		  ((~(z.A ^ v) & (z.A ^ res) & 0x80) ? FL_P : 0) |
		  ((r & 0x100) ? FL_C : 0);
	z.A = res;
}

static inline void Sub8 (uint8 v, uint8 carry, bool8 store)
{
	uint16	r = z.A - v - carry;
	uint8	res = (uint8) r;
	z.F = FL_N | (res & (FL_S | FL_X | FL_Y)) | (res ? 0 : FL_Z) |
		  (((z.A ^ v ^ res) & 0x10) ? FL_H : 0) |
		  (((z.A ^ v) & (z.A ^ res) & 0x80) ? FL_P : 0) |
		  ((r & 0x100) ? FL_C : 0);
	if (store)
		z.A = res;
}

static inline void And8 (uint8 v)	{ z.A &= v; SetSZP(z.A); z.F = (z.F & ~(FL_N | FL_C)) | FL_H; }
static inline void Or8 (uint8 v)	{ z.A |= v; SetSZP(z.A); z.F &= ~(FL_N | FL_C | FL_H); }
static inline void Xor8 (uint8 v)	{ z.A ^= v; SetSZP(z.A); z.F &= ~(FL_N | FL_C | FL_H); }

static inline uint16 Add16 (uint16 a, uint16 b)
{
	uint32	r = (uint32) a + b;
	z.F = (z.F & (FL_S | FL_Z | FL_P)) |
		  (((a ^ b ^ r) & 0x1000) ? FL_H : 0) |
		  ((r & 0x10000) ? FL_C : 0) |
		  (((uint8) (r >> 8)) & (FL_X | FL_Y));
	z.Cycles += 7;
	return ((uint16) r);
}

static inline void Adc16 (uint16 v)
{
	uint16	hl = GetHL();
	uint32	r = (uint32) hl + v + (z.F & FL_C);
	uint16	res = (uint16) r;
	z.F = ((res & 0x8000) ? FL_S : 0) | (res ? 0 : FL_Z) |
		  (((hl ^ v ^ res) & 0x1000) ? FL_H : 0) |
		  ((~(hl ^ v) & (hl ^ res) & 0x8000) ? FL_P : 0) |
		  ((r & 0x10000) ? FL_C : 0) |
		  (((uint8) (res >> 8)) & (FL_X | FL_Y));
	SetHL(res);
	z.Cycles += 7;
}

static inline void Sbc16 (uint16 v)
{
	uint16	hl = GetHL();
	uint32	r = (uint32) hl - v - (z.F & FL_C);
	uint16	res = (uint16) r;
	z.F = FL_N | ((res & 0x8000) ? FL_S : 0) | (res ? 0 : FL_Z) |
		  (((hl ^ v ^ res) & 0x1000) ? FL_H : 0) |
		  (((hl ^ v) & (hl ^ res) & 0x8000) ? FL_P : 0) |
		  ((r & 0x10000) ? FL_C : 0) |
		  (((uint8) (res >> 8)) & (FL_X | FL_Y));
	SetHL(res);
	z.Cycles += 7;
}

// Rotate/shift cores (CB block + RLCA family)
static inline uint8 Rlc (uint8 v)	{ uint8 c = v >> 7; v = (v << 1) | c;            SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }
static inline uint8 Rrc (uint8 v)	{ uint8 c = v & 1;  v = (v >> 1) | (c << 7);     SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }
static inline uint8 Rl (uint8 v)	{ uint8 c = v >> 7; v = (v << 1) | (z.F & FL_C); SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }
static inline uint8 Rr (uint8 v)	{ uint8 c = v & 1;  v = (v >> 1) | ((z.F & FL_C) << 7); SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }
static inline uint8 Sla (uint8 v)	{ uint8 c = v >> 7; v <<= 1;                     SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }
static inline uint8 Sra (uint8 v)	{ uint8 c = v & 1;  v = (v >> 1) | (v & 0x80);   SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }
static inline uint8 Srl (uint8 v)	{ uint8 c = v & 1;  v >>= 1;                     SetSZP(v); z.F = (z.F & ~(FL_H | FL_N | FL_C)) | (c ? FL_C : 0); return (v); }

// TST: AND without storing the result (Z180). H is set, C cleared.
static inline void Tst8 (uint8 v)
{
	uint8	r = z.A & v;
	z.F = (r & (FL_S | FL_X | FL_Y)) | (r ? 0 : FL_Z) | ParityTable[r] | FL_H;
}

static inline void Daa (void)
{
	uint8	a = z.A;
	uint8	adjust = 0;
	uint8	carry = z.F & FL_C;

	if ((z.F & FL_H) || (a & 0x0f) > 9)
		adjust |= 0x06;
	if (carry || a > 0x99)
	{
		adjust |= 0x60;
		carry = FL_C;
	}

	uint8	r = (z.F & FL_N) ? a - adjust : a + adjust;
	z.F = (z.F & FL_N) | carry |
		  (((a ^ r) & 0x10) ? FL_H : 0) |
		  (r & (FL_S | FL_X | FL_Y)) | (r ? 0 : FL_Z) | ParityTable[r];
	z.A = r;
}

// ---------------------------------------------------------------------------
// TRAP (undocumented opcode): set ITC.TRAP/UFO, stack the offending PC,
// restart at 0000h. `opstart` is the address of the first opcode byte;
// `third` is set when the offence was in the third byte (after a prefix).

static void Trap (uint16 opstart, bool8 third)
{
	z.ITC |= ITC_TRAP;
	if (third)
		z.ITC |= ITC_UFO;
	else
		z.ITC &= ~ITC_UFO;

	Push16(opstart);
	z.PC = 0;
	z.Halted = FALSE;
	z.Cycles += 11;
}

// ---------------------------------------------------------------------------
// Register operand decode for the main block: 0..7 = B C D E H L (mem) A.
// Reads/writes of index 6 are the caller's business (memory operand).

static inline uint8 GetReg (int r)
{
	switch (r)
	{
		case 0: return (z.B);
		case 1: return (z.C);
		case 2: return (z.D);
		case 3: return (z.E);
		case 4: return (z.H);
		case 5: return (z.L);
		case 7: return (z.A);
	}
	return (0);
}

static inline void SetReg (int r, uint8 v)
{
	switch (r)
	{
		case 0: z.B = v;	break;
		case 1: z.C = v;	break;
		case 2: z.D = v;	break;
		case 3: z.E = v;	break;
		case 4: z.H = v;	break;
		case 5: z.L = v;	break;
		case 7: z.A = v;	break;
	}
}

static inline bool8 CondMet (int cc)
{
	switch (cc)
	{
		case 0: return (!(z.F & FL_Z));	// NZ
		case 1: return ((z.F & FL_Z) != 0);	// Z
		case 2: return (!(z.F & FL_C));	// NC
		case 3: return ((z.F & FL_C) != 0);	// C
		case 4: return (!(z.F & FL_P));	// PO
		case 5: return ((z.F & FL_P) != 0);	// PE
		case 6: return (!(z.F & FL_S));	// P
		case 7: return ((z.F & FL_S) != 0);	// M
	}
	return (FALSE);
}

// ---------------------------------------------------------------------------
// CB block. In index mode the displacement is fetched BEFORE the sub-opcode
// (DD CB d op) and only the (HL)-column forms are documented on the Z180.

static void ExecCB (int ixmode, uint16 opstart)
{
	uint16	ea = 0;
	uint8	op;

	if (ixmode)
	{
		int8	d = (int8) Fetch();
		ea = (uint16) (GetIdx(ixmode) + d);
		op = Fetch();	// not an M1 cycle on DDCB
		z.Cycles += 2;

		if ((op & 0x07) != 6 || (op & 0xf8) == 0x30)
		{
			Trap(opstart, TRUE);
			return;
		}
	}
	else
	{
		op = FetchM1();

		if ((op & 0xf8) == 0x30)	// SLL: executes on NMOS Z80s, traps here
		{
			Trap(opstart, FALSE);
			return;
		}
	}

	int		r = op & 0x07;
	int		bit = (op >> 3) & 0x07;
	bool8	mem = (r == 6);
	uint8	v = mem ? RdMem(ixmode ? ea : GetHL()) : GetReg(r);

	switch (op >> 6)
	{
		case 0:	// rotates/shifts
			switch (bit)
			{
				case 0: v = Rlc(v);	break;
				case 1: v = Rrc(v);	break;
				case 2: v = Rl(v);	break;
				case 3: v = Rr(v);	break;
				case 4: v = Sla(v);	break;
				case 5: v = Sra(v);	break;
				case 7: v = Srl(v);	break;
			}
			break;

		case 1:	// BIT b,r
		{
			uint8	t = v & (1 << bit);
			z.F = (z.F & FL_C) | FL_H | (t ? 0 : (FL_Z | FL_P)) | (t & FL_S) |
				  (v & (FL_X | FL_Y));
			return;
		}

		case 2:	v &= ~(1 << bit);	break;	// RES
		case 3:	v |= (1 << bit);	break;	// SET
	}

	if (mem)
		WrMem(ixmode ? ea : GetHL(), v);
	else
		SetReg(r, v);
}

// ---------------------------------------------------------------------------
// ED block: standard Z80 entries plus the Z180 extensions; anything else
// traps (with UFO flagging the third byte).

static void ExecED (uint16 opstart)
{
	uint8	op = FetchM1();

	// Z180: IN0 g,(m) / OUT0 (m),g / TST g — low quadrant
	if (op < 0x40)
	{
		int	r = (op >> 3) & 0x07;

		switch (op & 0x07)
		{
			case 0:	// IN0 r,(m)  (r==6: flags only)
			{
				uint8	v = RdIO(Fetch());
				if (r != 6)
					SetReg(r, v);
				SetSZP(v);
				z.F &= ~(FL_H | FL_N);
				return;
			}

			case 1:	// OUT0 (m),r
				if (r == 6)
					break;	// ED 31: undefined
				WrIO(Fetch(), GetReg(r));
				return;

			case 4:	// TST r / TST (HL)
				Tst8(r == 6 ? RdMem(GetHL()) : GetReg(r));
				return;

			default:
				break;
		}

		Trap(opstart, TRUE);
		return;
	}

	if (op >= 0x80)
	{
		switch (op)
		{
			case 0x83:	// OTIM: out (C),(HL); HL++, C++, B--
			case 0x8b:	// OTDM: out (C),(HL); HL--, C--, B--
			case 0x93:	// OTIMR
			case 0x9b:	// OTDMR
			{
				bool8	up = !(op & 0x08);
				bool8	rep = (op & 0x10) != 0;

				WrIO(z.C, RdMem(GetHL()));
				SetHL(GetHL() + (up ? 1 : -1));
				z.C += up ? 1 : -1;
				z.B--;
				z.F = (z.B ? 0 : FL_Z) | FL_N;
				if (rep && z.B)
				{
					z.PC -= 2;
					z.Cycles += 2;
				}
				return;
			}

			// Block transfer / compare / io (A0-BB, documented subset)
			case 0xa0: case 0xa8: case 0xb0: case 0xb8:	// LDI/LDD/LDIR/LDDR
			{
				bool8	up = !(op & 0x08);
				bool8	rep = (op & 0x10) != 0;

				uint8	v = RdMem(GetHL());
				WrMem(GetDE(), v);
				SetHL(GetHL() + (up ? 1 : -1));
				SetDE(GetDE() + (up ? 1 : -1));
				SetBC(GetBC() - 1);
				z.F = (z.F & (FL_S | FL_Z | FL_C)) | (GetBC() ? FL_P : 0);
				z.Cycles += 2;
				if (rep && GetBC())
				{
					z.PC -= 2;
					z.Cycles += 5;
				}
				return;
			}

			case 0xa1: case 0xa9: case 0xb1: case 0xb9:	// CPI/CPD/CPIR/CPDR
			{
				bool8	up = !(op & 0x08);
				bool8	rep = (op & 0x10) != 0;

				uint8	v = RdMem(GetHL());
				uint8	r = z.A - v;
				SetHL(GetHL() + (up ? 1 : -1));
				SetBC(GetBC() - 1);
				z.F = (z.F & FL_C) | FL_N | (r & FL_S) | (r ? 0 : FL_Z) |
					  (((z.A ^ v ^ r) & 0x10) ? FL_H : 0) | (GetBC() ? FL_P : 0);
				z.Cycles += 5;
				if (rep && GetBC() && r)
				{
					z.PC -= 2;
					z.Cycles += 5;
				}
				return;
			}

			case 0xa2: case 0xaa: case 0xb2: case 0xba:	// INI/IND/INIR/INDR
			{
				bool8	up = !(op & 0x08);
				bool8	rep = (op & 0x10) != 0;

				WrMem(GetHL(), RdIO(GetBC()));
				SetHL(GetHL() + (up ? 1 : -1));
				z.B--;
				z.F = (z.B ? 0 : FL_Z) | FL_N;
				if (rep && z.B)
				{
					z.PC -= 2;
					z.Cycles += 5;
				}
				return;
			}

			case 0xa3: case 0xab: case 0xb3: case 0xbb:	// OUTI/OUTD/OTIR/OTDR
			{
				bool8	up = !(op & 0x08);
				bool8	rep = (op & 0x10) != 0;

				uint8	v = RdMem(GetHL());
				z.B--;
				WrIO(GetBC(), v);
				SetHL(GetHL() + (up ? 1 : -1));
				z.F = (z.B ? 0 : FL_Z) | FL_N;
				if (rep && z.B)
				{
					z.PC -= 2;
					z.Cycles += 5;
				}
				return;
			}
		}

		Trap(opstart, TRUE);
		return;
	}

	// 40h-7Fh quadrant
	switch (op)
	{
		case 0x40: case 0x48: case 0x50: case 0x58:
		case 0x60: case 0x68: case 0x78:	// IN r,(C)
		{
			uint8	v = RdIO(GetBC());
			SetReg((op >> 3) & 7, v);
			SetSZP(v);
			z.F &= ~(FL_H | FL_N);
			return;
		}

		case 0x41: case 0x49: case 0x51: case 0x59:
		case 0x61: case 0x69: case 0x79:	// OUT (C),r
			WrIO(GetBC(), GetReg((op >> 3) & 7));
			return;

		case 0x42:	Sbc16(GetBC());	return;
		case 0x52:	Sbc16(GetDE());	return;
		case 0x62:	Sbc16(GetHL());	return;
		case 0x72:	Sbc16(z.SP);	return;
		case 0x4a:	Adc16(GetBC());	return;
		case 0x5a:	Adc16(GetDE());	return;
		case 0x6a:	Adc16(GetHL());	return;
		case 0x7a:	Adc16(z.SP);	return;

		case 0x43:	WrMem16(Fetch16(), GetBC());	return;
		case 0x53:	WrMem16(Fetch16(), GetDE());	return;
		case 0x63:	WrMem16(Fetch16(), GetHL());	return;
		case 0x73:	WrMem16(Fetch16(), z.SP);		return;
		case 0x4b:	SetBC(RdMem16(Fetch16()));		return;
		case 0x5b:	SetDE(RdMem16(Fetch16()));		return;
		case 0x6b:	SetHL(RdMem16(Fetch16()));		return;
		case 0x7b:	z.SP = RdMem16(Fetch16());		return;

		case 0x44:	// NEG
		{
			uint8	a = z.A;
			z.A = 0;
			Sub8(a, 0, TRUE);
			return;
		}

		case 0x45:	// RETN
			z.IFF1 = z.IFF2;
			z.PC = Pop16();
			return;

		case 0x4d:	// RETI
			z.IFF1 = z.IFF2;
			z.PC = Pop16();
			return;

		case 0x46:	z.IM = 0;	return;
		case 0x56:	z.IM = 1;	return;
		case 0x5e:	z.IM = 2;	return;

		case 0x47:	z.I = z.A;	z.Cycles += 1;	return;
		case 0x4f:	z.R = z.A;	z.Cycles += 1;	return;

		case 0x57:	// LD A,I
			z.A = z.I;
			z.F = (z.F & FL_C) | (z.A & (FL_S | FL_X | FL_Y)) | (z.A ? 0 : FL_Z) |
				  (z.IFF2 ? FL_P : 0);
			z.Cycles += 1;
			return;

		case 0x5f:	// LD A,R
			z.A = z.R;
			z.F = (z.F & FL_C) | (z.A & (FL_S | FL_X | FL_Y)) | (z.A ? 0 : FL_Z) |
				  (z.IFF2 ? FL_P : 0);
			z.Cycles += 1;
			return;

		case 0x67:	// RRD
		{
			uint8	m = RdMem(GetHL());
			WrMem(GetHL(), (uint8) ((m >> 4) | (z.A << 4)));
			z.A = (z.A & 0xf0) | (m & 0x0f);
			SetSZP(z.A);
			z.F &= ~(FL_H | FL_N);
			z.Cycles += 4;
			return;
		}

		case 0x6f:	// RLD
		{
			uint8	m = RdMem(GetHL());
			WrMem(GetHL(), (uint8) ((m << 4) | (z.A & 0x0f)));
			z.A = (z.A & 0xf0) | (m >> 4);
			SetSZP(z.A);
			z.F &= ~(FL_H | FL_N);
			z.Cycles += 4;
			return;
		}

		// Z180 extensions in this quadrant
		case 0x4c:	SetBC((uint16) z.B * z.C);	z.Cycles += 13;	return;	// MLT BC
		case 0x5c:	SetDE((uint16) z.D * z.E);	z.Cycles += 13;	return;	// MLT DE
		case 0x6c:	SetHL((uint16) z.H * z.L);	z.Cycles += 13;	return;	// MLT HL
		case 0x7c:	// MLT SP
			z.SP = (uint16) (z.SP >> 8) * (uint8) z.SP;
			z.Cycles += 13;
			return;

		case 0x64:	Tst8(Fetch());	return;			// TST m

		case 0x74:	// TSTIO m: flags from (io C) AND m
		{
			uint8	v = RdIO(z.C) & Fetch();
			z.F = (v & (FL_S | FL_X | FL_Y)) | (v ? 0 : FL_Z) | ParityTable[v] | FL_H;
			return;
		}

		case 0x76:	// SLP — PC stays past the opcode so the wake-up
					// interrupt's RETI resumes at the next instruction
			z.Halted = TRUE;
			return;
	}

	Trap(opstart, TRUE);
}

// ---------------------------------------------------------------------------
// Main opcode block, parameterized by index mode. In index mode only the
// documented IX/IY forms may execute; the rest trap (the HD64180 refuses
// the "prefix ignored" and IXH/IXL encodings NMOS Z80s tolerate).

// Opcodes legal after DD/FD: those with a (HL) memory operand and those
// operating on HL as a 16-bit register.
static const uint8 IdxLegal[256] =
{
//	x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xa xb xc xd xe xf
	0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0,	// 0x: ADD IX,BC
	0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0,	// 1x: ADD IX,DE
	0, 1, 1, 1, 0, 0, 0, 0,  0, 1, 1, 1, 0, 0, 0, 0,	// 2x: LD IX,nn/(nn), INC/DEC IX, ADD IX,IX
	0, 0, 0, 0, 1, 1, 1, 0,  0, 1, 0, 0, 0, 0, 0, 0,	// 3x: INC/DEC/LD (IX+d), ADD IX,SP
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// 4x: LD r,(IX+d)
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// 5x
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// 6x
	1, 1, 1, 1, 1, 1, 0, 1,  0, 0, 0, 0, 0, 0, 1, 0,	// 7x: LD (IX+d),r / LD A,(IX+d)
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// 8x: ADD/ADC A,(IX+d)
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// 9x: SUB/SBC (IX+d)
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// ax: AND/XOR (IX+d)
	0, 0, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 1, 0,	// bx: OR/CP (IX+d)
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 1, 0, 0, 0, 0,	// cx: DDCB prefix
	0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,	// dx
	0, 1, 0, 1, 0, 1, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0,	// ex: POP/EX (SP)/PUSH/JP (IX)
	0, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0, 0, 0, 0, 0,	// fx: LD SP,IX
};

static void ExecOp (uint8 op, int ixmode, uint16 opstart)
{
	if (ixmode && !IdxLegal[op])
	{
		Trap(opstart, TRUE);
		return;
	}

	switch (op)
	{
		case 0x00:	return;	// NOP

		case 0x01:	SetBC(Fetch16());	return;
		case 0x11:	SetDE(Fetch16());	return;
		case 0x21:	SetIdx(ixmode, Fetch16());	return;
		case 0x31:	z.SP = Fetch16();	return;

		case 0x02:	WrMem(GetBC(), z.A);	return;
		case 0x12:	WrMem(GetDE(), z.A);	return;
		case 0x0a:	z.A = RdMem(GetBC());	return;
		case 0x1a:	z.A = RdMem(GetDE());	return;

		case 0x22:	WrMem16(Fetch16(), GetIdx(ixmode));	return;
		case 0x2a:	SetIdx(ixmode, RdMem16(Fetch16()));	return;
		case 0x32:	WrMem(Fetch16(), z.A);	return;
		case 0x3a:	z.A = RdMem(Fetch16());	return;

		case 0x03:	SetBC(GetBC() + 1);	z.Cycles += 2;	return;
		case 0x13:	SetDE(GetDE() + 1);	z.Cycles += 2;	return;
		case 0x23:	SetIdx(ixmode, GetIdx(ixmode) + 1);	z.Cycles += 2;	return;
		case 0x33:	z.SP++;	z.Cycles += 2;	return;
		case 0x0b:	SetBC(GetBC() - 1);	z.Cycles += 2;	return;
		case 0x1b:	SetDE(GetDE() - 1);	z.Cycles += 2;	return;
		case 0x2b:	SetIdx(ixmode, GetIdx(ixmode) - 1);	z.Cycles += 2;	return;
		case 0x3b:	z.SP--;	z.Cycles += 2;	return;

		case 0x04: case 0x0c: case 0x14: case 0x1c:
		case 0x24: case 0x2c: case 0x3c:	// INC r
			SetReg(op >> 3, Inc8(GetReg(op >> 3)));
			return;

		case 0x34:	// INC (HL)/(IX+d)
		{
			uint16	ea = EffAddr(ixmode);
			WrMem(ea, Inc8(RdMem(ea)));
			return;
		}

		case 0x05: case 0x0d: case 0x15: case 0x1d:
		case 0x25: case 0x2d: case 0x3d:	// DEC r
			SetReg(op >> 3, Dec8(GetReg(op >> 3)));
			return;

		case 0x35:	// DEC (HL)/(IX+d)
		{
			uint16	ea = EffAddr(ixmode);
			WrMem(ea, Dec8(RdMem(ea)));
			return;
		}

		case 0x06: case 0x0e: case 0x16: case 0x1e:
		case 0x26: case 0x2e: case 0x3e:	// LD r,n
			SetReg(op >> 3, Fetch());
			return;

		case 0x36:	// LD (HL)/(IX+d),n — displacement precedes the literal
		{
			uint16	ea = EffAddr(ixmode);
			WrMem(ea, Fetch());
			return;
		}

		case 0x07:	// RLCA
		{
			uint8	c = z.A >> 7;
			z.A = (z.A << 1) | c;
			z.F = (z.F & (FL_S | FL_Z | FL_P)) | (z.A & (FL_X | FL_Y)) | (c ? FL_C : 0);
			return;
		}

		case 0x0f:	// RRCA
		{
			uint8	c = z.A & 1;
			z.A = (z.A >> 1) | (c << 7);
			z.F = (z.F & (FL_S | FL_Z | FL_P)) | (z.A & (FL_X | FL_Y)) | (c ? FL_C : 0);
			return;
		}

		case 0x17:	// RLA
		{
			uint8	c = z.A >> 7;
			z.A = (z.A << 1) | (z.F & FL_C);
			z.F = (z.F & (FL_S | FL_Z | FL_P)) | (z.A & (FL_X | FL_Y)) | (c ? FL_C : 0);
			return;
		}

		case 0x1f:	// RRA
		{
			uint8	c = z.A & 1;
			z.A = (z.A >> 1) | ((z.F & FL_C) << 7);
			z.F = (z.F & (FL_S | FL_Z | FL_P)) | (z.A & (FL_X | FL_Y)) | (c ? FL_C : 0);
			return;
		}

		case 0x27:	Daa();	return;
		case 0x2f:	// CPL
			z.A = ~z.A;
			z.F = (z.F & (FL_S | FL_Z | FL_P | FL_C)) | FL_H | FL_N | (z.A & (FL_X | FL_Y));
			return;
		case 0x37:	// SCF
			z.F = (z.F & (FL_S | FL_Z | FL_P)) | FL_C | (z.A & (FL_X | FL_Y));
			return;
		case 0x3f:	// CCF
			z.F = ((z.F & (FL_S | FL_Z | FL_P | FL_C)) ^ FL_C) |
				  ((z.F & FL_C) ? FL_H : 0) | (z.A & (FL_X | FL_Y));
			return;

		case 0x08:	// EX AF,AF'
		{
			uint8	t;
			t = z.A; z.A = z.A2; z.A2 = t;
			t = z.F; z.F = z.F2; z.F2 = t;
			return;
		}

		case 0x09:	SetIdx(ixmode, Add16(GetIdx(ixmode), GetBC()));	return;
		case 0x19:	SetIdx(ixmode, Add16(GetIdx(ixmode), GetDE()));	return;
		case 0x29:	SetIdx(ixmode, Add16(GetIdx(ixmode), GetIdx(ixmode)));	return;
		case 0x39:	SetIdx(ixmode, Add16(GetIdx(ixmode), z.SP));	return;

		case 0x10:	// DJNZ
		{
			int8	d = (int8) Fetch();
			z.Cycles += 1;
			if (--z.B)
			{
				z.PC += d;
				z.Cycles += 4;
			}
			return;
		}

		case 0x18:	// JR
		{
			int8	d = (int8) Fetch();
			z.PC += d;
			z.Cycles += 4;
			return;
		}

		case 0x20: case 0x28: case 0x30: case 0x38:	// JR cc
		{
			int8	d = (int8) Fetch();
			if (CondMet((op >> 3) & 3))
			{
				z.PC += d;
				z.Cycles += 4;
			}
			return;
		}

		case 0x76:	// HALT (PC already past the opcode; see SLP note)
			z.Halted = TRUE;
			return;

		default:
			break;
	}

	// LD r,r' block (40h-7Fh, 76h handled above)
	if (op >= 0x40 && op <= 0x7f)
	{
		int	dst = (op >> 3) & 7;
		int	src = op & 7;

		if (src == 6)
			SetReg(dst, RdMem(EffAddr(ixmode)));
		else if (dst == 6)
			WrMem(EffAddr(ixmode), GetReg(src));
		else
			SetReg(dst, GetReg(src));
		return;
	}

	// ALU op block (80h-BFh)
	if (op >= 0x80 && op <= 0xbf)
	{
		int		src = op & 7;
		uint8	v = (src == 6) ? RdMem(EffAddr(ixmode)) : GetReg(src);

		switch ((op >> 3) & 7)
		{
			case 0:	Add8(v, 0);	break;
			case 1:	Add8(v, z.F & FL_C);	break;
			case 2:	Sub8(v, 0, TRUE);	break;
			case 3:	Sub8(v, z.F & FL_C, TRUE);	break;
			case 4:	And8(v);	break;
			case 5:	Xor8(v);	break;
			case 6:	Or8(v);		break;
			case 7:	Sub8(v, 0, FALSE);	break;	// CP
		}
		return;
	}

	// C0h-FFh
	switch (op)
	{
		case 0xc0: case 0xc8: case 0xd0: case 0xd8:
		case 0xe0: case 0xe8: case 0xf0: case 0xf8:	// RET cc
			z.Cycles += 1;
			if (CondMet((op >> 3) & 7))
				z.PC = Pop16();
			return;

		case 0xc9:	z.PC = Pop16();	return;

		case 0xc1:	SetBC(Pop16());	return;
		case 0xd1:	SetDE(Pop16());	return;
		case 0xe1:	SetIdx(ixmode, Pop16());	return;
		case 0xf1:	SetAF(Pop16());	return;

		case 0xc5:	z.Cycles += 1;	Push16(GetBC());	return;
		case 0xd5:	z.Cycles += 1;	Push16(GetDE());	return;
		case 0xe5:	z.Cycles += 1;	Push16(GetIdx(ixmode));	return;
		case 0xf5:	z.Cycles += 1;	Push16(GetAF());	return;

		case 0xc2: case 0xca: case 0xd2: case 0xda:
		case 0xe2: case 0xea: case 0xf2: case 0xfa:	// JP cc,nn
		{
			uint16	target = Fetch16();
			if (CondMet((op >> 3) & 7))
				z.PC = target;
			return;
		}

		case 0xc3:	z.PC = Fetch16();	return;
		case 0xe9:	z.PC = GetIdx(ixmode);	return;	// JP (HL)/(IX)

		case 0xc4: case 0xcc: case 0xd4: case 0xdc:
		case 0xe4: case 0xec: case 0xf4: case 0xfc:	// CALL cc,nn
		{
			uint16	target = Fetch16();
			if (CondMet((op >> 3) & 7))
			{
				z.Cycles += 1;
				Push16(z.PC);
				z.PC = target;
			}
			return;
		}

		case 0xcd:	// CALL nn
		{
			uint16	target = Fetch16();
			z.Cycles += 1;
			Push16(z.PC);
			z.PC = target;
			return;
		}

		case 0xc7: case 0xcf: case 0xd7: case 0xdf:
		case 0xe7: case 0xef: case 0xf7: case 0xff:	// RST p
			z.Cycles += 1;
			Push16(z.PC);
			z.PC = op & 0x38;
			return;

		case 0xc6:	Add8(Fetch(), 0);	return;
		case 0xce:	Add8(Fetch(), z.F & FL_C);	return;
		case 0xd6:	Sub8(Fetch(), 0, TRUE);	return;
		case 0xde:	Sub8(Fetch(), z.F & FL_C, TRUE);	return;
		case 0xe6:	And8(Fetch());	return;
		case 0xee:	Xor8(Fetch());	return;
		case 0xf6:	Or8(Fetch());	return;
		case 0xfe:	Sub8(Fetch(), 0, FALSE);	return;

		case 0xd3:	WrIO(((uint16) z.A << 8) | Fetch(), z.A);	return;	// OUT (n),A
		case 0xdb:	z.A = RdIO(((uint16) z.A << 8) | Fetch());	return;	// IN A,(n)

		case 0xd9:	// EXX
		{
			uint8	t;
			t = z.B; z.B = z.B2; z.B2 = t;
			t = z.C; z.C = z.C2; z.C2 = t;
			t = z.D; z.D = z.D2; z.D2 = t;
			t = z.E; z.E = z.E2; z.E2 = t;
			t = z.H; z.H = z.H2; z.H2 = t;
			t = z.L; z.L = z.L2; z.L2 = t;
			return;
		}

		case 0xe3:	// EX (SP),HL/IX
		{
			uint16	t = RdMem16(z.SP);
			WrMem16(z.SP, GetIdx(ixmode));
			SetIdx(ixmode, t);
			z.Cycles += 3;
			return;
		}

		case 0xeb:	// EX DE,HL (never prefixed — IdxLegal rejects that)
		{
			uint16	t = GetDE();
			SetDE(GetHL());
			SetHL(t);
			return;
		}

		case 0xf3:	z.IFF1 = z.IFF2 = 0;	return;	// DI
		case 0xfb:	// EI (takes effect after the following instruction —
					// approximated by re-enabling immediately; the KROM
					// idles in halt loops where the distinction is moot)
			z.IFF1 = z.IFF2 = 1;
			return;

		case 0xf9:	z.SP = GetIdx(ixmode);	z.Cycles += 2;	return;

		case 0xcb:	ExecCB(ixmode, opstart);	return;
	}

	// Unreachable: all 256 encodings are handled above or in the blocks.
	Trap(opstart, FALSE);
}

// ---------------------------------------------------------------------------
// Interrupts

static void TakeVectoredInt (uint8 offset)
{
	uint16	vaddr = ((uint16) z.I << 8) | (z.IL & 0xe0) | offset;
	z.IFF1 = z.IFF2 = 0;
	z.Halted = FALSE;
	Push16(z.PC);
	z.PC = RdMem16(vaddr);
	z.Cycles += 12;
}

static bool8 CheckInterrupts (void)
{
	if (z.NMILatch)
	{
		z.NMILatch = FALSE;
		z.IFF2 = z.IFF1;
		z.IFF1 = 0;
		z.Halted = FALSE;
		Push16(z.PC);
		z.PC = 0x0066;
		z.Cycles += 11;
		return (TRUE);
	}

	if (!z.IFF1)
		return (FALSE);

	// INT0: external, mode 0/1/2. With nothing driving the bus during the
	// acknowledge the vector byte floats to FFh (IM0 => RST 38h, IM2 =>
	// table entry xxFFh).
	if (z.INT0Line && (z.ITC & ITC_ITE0))
	{
		z.IFF1 = z.IFF2 = 0;
		z.Halted = FALSE;
		if (z.IM == 2)
		{
			Push16(z.PC);
			z.PC = RdMem16(((uint16) z.I << 8) | 0xff);
			z.Cycles += 19;
		}
		else
		{
			Push16(z.PC);
			z.PC = 0x0038;
			z.Cycles += 13;
		}
		return (TRUE);
	}

	if (z.INT1Line && (z.ITC & ITC_ITE1))
	{
		TakeVectoredInt(0x00);
		return (TRUE);
	}

	if (z.INT2Line && (z.ITC & ITC_ITE2))
	{
		TakeVectoredInt(0x02);
		return (TRUE);
	}

	if ((z.TCR & TCR_TIE0) && (z.TCR & TCR_TIF0))
	{
		TakeVectoredInt(0x04);
		return (TRUE);
	}

	if ((z.TCR & TCR_TIE1) && (z.TCR & TCR_TIF1))
	{
		TakeVectoredInt(0x06);
		return (TRUE);
	}

	if ((z.CNTR & CNTR_EIE) && (z.CNTR & CNTR_EF))
	{
		TakeVectoredInt(0x0c);
		return (TRUE);
	}

	return (FALSE);
}

// ---------------------------------------------------------------------------
// On-chip peripheral clocking

static void AdvancePeripherals (int32 cycles)
{
	// PRT timers: divide-by-20 prescaler shared by both channels.
	z.PRTPrescale += cycles;
	while (z.PRTPrescale >= 20)
	{
		z.PRTPrescale -= 20;

		if (z.TCR & TCR_TDE0)
		{
			if (z.TMDR0 == 0)
			{
				z.TMDR0 = z.RLDR0;
				z.TCR |= TCR_TIF0;
			}
			else
				z.TMDR0--;
		}

		if (z.TCR & TCR_TDE1)
		{
			if (z.TMDR1 == 0)
			{
				z.TMDR1 = z.RLDR1;
				z.TCR |= TCR_TIF1;
			}
			else
				z.TMDR1--;
		}
	}

	// Free-running counter: PHI/10.
	z.FRCPrescale += cycles;
	while (z.FRCPrescale >= 10)
	{
		z.FRCPrescale -= 10;
		z.FRC--;
	}

	// CSI/O transmit engine.
	if (z.CSIOCountdown > 0)
	{
		z.CSIOCountdown -= cycles;
		if (z.CSIOCountdown <= 0)
		{
			z.CSIOCountdown = 0;
			z.CNTR = (z.CNTR & ~CNTR_TE) | CNTR_EF;
			if (HD64180CB.CSIOTransmit)
				HD64180CB.CSIOTransmit(z.TRDR);
		}
	}
}

// ---------------------------------------------------------------------------
// Public interface

void HD64180_Reset (void)
{
	if (!TablesBuilt)
		BuildTables();

	memset(&z, 0, sizeof(z));

	z.SP = 0xffff;
	z.F = 0xff;
	// On-chip register reset values (HD64180 datasheet)
	z.CNTLA0 = z.CNTLA1 = 0x10;
	z.CNTLB0 = z.CNTLB1 = 0x07;
	z.STAT0 = 0x00;
	z.STAT1 = 0x02;
	z.CNTR = 0x07;
	z.TMDR0 = z.TMDR1 = 0xffff;
	z.RLDR0 = z.RLDR1 = 0xffff;
	z.FRC = 0xff;
	z.DCNTL = 0xf0;
	z.ITC = 0x01;	// only INT0 enabled out of reset
	z.RCR = 0xfc;
	z.ICR = 0x1f & 0xe0;
	z.CBAR = 0xf0;
}

int32 HD64180_Execute (int32 cycles)
{
	z.Cycles = 0;

	while (z.Cycles < cycles)
	{
		int32	before = z.Cycles;

		if (CheckInterrupts())
		{
			AdvancePeripherals(z.Cycles - before);
			continue;
		}

		if (z.Halted)
		{
			// Idle in small quanta so a peripheral interrupt raised while
			// we sleep can wake us within the same slice.
			int32	idle = cycles - z.Cycles;
			if (idle > 20)
				idle = 20;
			z.Cycles += idle;
			AdvancePeripherals(idle);
			continue;
		}

		uint16	opstart = z.PC;
		uint8	op = FetchM1();

		switch (op)
		{
			case 0xdd:
			case 0xfd:
			{
				uint8	sub = FetchM1();
				// Chained/foreign prefixes are undocumented here.
				if (sub == 0xdd || sub == 0xfd || sub == 0xed)
					Trap(opstart, TRUE);
				else
					ExecOp(sub, (op == 0xdd) ? 1 : 2, opstart);
				break;
			}

			case 0xed:
				ExecED(opstart);
				break;

			default:
				ExecOp(op, 0, opstart);
				break;
		}

		AdvancePeripherals(z.Cycles - before);
	}

	z.TotalCycles += (uint32) z.Cycles;
	return (z.Cycles);
}

void HD64180_SetINT0 (bool8 asserted)	{ z.INT0Line = asserted; }
void HD64180_SetINT1 (bool8 asserted)	{ z.INT1Line = asserted; }
void HD64180_SetINT2 (bool8 asserted)	{ z.INT2Line = asserted; }
