/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// Hitachi HD64180 (Z180-compatible) CPU core for the Super Famicom Box
// supervisor board. Full documented Z80 instruction set plus the Z180
// extensions (MLT, TST, TSTIO, IN0, OUT0, OTIM/OTDM(+R), SLP); every
// undocumented encoding raises the Z180 TRAP (ITC bit7, restart at 0000h)
// exactly like the real chip — the SFC-Box KROM relies on that. On-chip
// peripherals: MMU (CBR/BBR/CBAR), the two PRT reload timers, CSI/O
// (transmit only — the box uses it to feed the MB90082 OSD chip), ASCI
// status stubs, and the INT0/INT1/INT2 + internal interrupt controller.
// See docs/sfcbox.md for the board-level picture.

#ifndef _HD64180_H_
#define _HD64180_H_

#include "port.h"

struct SHD64180Callbacks
{
	// Physical-address (19-bit + carry room) memory access; the MMU
	// translation has already happened when these are called.
	uint8	(*MemRead) (uint32 addr);
	void	(*MemWrite) (uint32 addr, uint8 byte);
	// External I/O (anything not claimed by the on-chip 00h-3Fh block).
	// The SFC-Box decodes only the low address byte; the full 16-bit
	// port is passed so the board can mask as it likes.
	uint8	(*IORead) (uint16 port);
	void	(*IOWrite) (uint16 port, uint8 byte);
	// A CSI/O transmit completed (chip-select routing is the board's
	// problem — on the SFC-Box port [81h].W bit7 gates the OSD).
	void	(*CSIOTransmit) (uint8 byte);
};

struct SHD64180
{
	// Register file
	uint8	A, F, B, C, D, E, H, L;
	uint8	A2, F2, B2, C2, D2, E2, H2, L2;	// shadow set
	uint16	IX, IY, SP, PC;
	uint8	I, R;
	uint8	IFF1, IFF2, IM;
	uint8	Halted;			// HALT or SLP: waiting for an interrupt

	// External interrupt request lines (level-triggered, active low on
	// silicon; here 1 = asserted).
	uint8	INT0Line, INT1Line, INT2Line;
	uint8	NMILine, NMILatch;

	// On-chip I/O registers (indexed by port 00h-3Fh where stateful)
	uint8	CNTLA0, CNTLA1, CNTLB0, CNTLB1, STAT0, STAT1;
	uint8	TDR0, TDR1, RDR0, RDR1;
	uint8	CNTR, TRDR;		// CSI/O
	uint16	TMDR0, RLDR0, TMDR1, RLDR1;
	uint8	TCR;
	uint8	FRC;
	uint8	DCNTL, IL, ITC;
	uint8	RCR, ICR;
	uint8	CBR, BBR, CBAR;	// MMU

	int32	CSIOCountdown;	// PHI cycles until the pending TX finishes (0 = idle)
	int32	PRTPrescale;	// PHI/20 divider shared by both timers
	int32	FRCPrescale;	// PHI/10 divider for the free-running counter
	uint8	TIF0Read, TIF1Read;	// TCR-then-TMDR TIF clearing sequence

	int32	Cycles;			// consumed within the current Execute() slice
	uint32	TotalCycles;	// lifetime, for the board's debug traces
};

extern struct SHD64180				HD64180;
extern struct SHD64180Callbacks		HD64180CB;

void	HD64180_Reset (void);
// Run at least `cycles` PHI clocks (finishes the last instruction);
// returns the number actually consumed.
int32	HD64180_Execute (int32 cycles);
void	HD64180_SetINT0 (bool8 asserted);
void	HD64180_SetINT1 (bool8 asserted);
void	HD64180_SetINT2 (bool8 asserted);
// 19-bit physical address for a CPU-visible (16-bit) address — exposed for
// the board's debug tracing.
uint32	HD64180_TranslateAddr (uint16 logical);

#endif
