/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "dma.h"
#include "apu/apu.h"
#include "fxemu.h"
#include "sdd1.h"
#include "srtc.h"
#include "snapshot.h"
#include "cheats.h"
#include "sgb/sgb.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

static void S9xResetCPU (void);
static void S9xSoftResetCPU (void);


static void S9xResetCPU (void)
{
	S9xSoftResetCPU();
	Registers.SL = 0xff;
	Registers.P.W = 0;
	Registers.A.W = 0;
	Registers.X.W = 0;
	Registers.Y.W = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);
}

static void S9xSoftResetCPU (void)
{
	CPU.Cycles = 182; // Or 188. This is the cycle count just after the jump to the Reset Vector.
	CPU.PrevCycles = CPU.Cycles;
	CPU.V_Counter = 0;
	CPU.Flags = CPU.Flags & (DEBUG_MODE_FLAG | TRACE_FLAG);
	CPU.PCBase = NULL;
	CPU.NMIPending = FALSE;
	CPU.IRQLine = FALSE;
	CPU.IRQTransition = FALSE;
	CPU.IRQExternal = FALSE;
	CPU.MemSpeed = SLOW_ONE_CYCLE;
	CPU.MemSpeedx2 = SLOW_ONE_CYCLE * 2;
	CPU.FastROMSpeed = SLOW_ONE_CYCLE;
	CPU.InDMA = FALSE;
	CPU.InHDMA = FALSE;
	CPU.InDMAorHDMA = FALSE;
	CPU.InWRAMDMAorHDMA = FALSE;
	CPU.HDMARanInDMA = 0;
	CPU.CurrentDMAorHDMAChannel = -1;
	CPU.WhichEvent = HC_RENDER_EVENT;
	CPU.NextEvent  = Timings.RenderPos;
	CPU.WaitingForInterrupt = FALSE;
	CPU.AutoSaveTimer = 0;
	CPU.SRAMModified = FALSE;

	Registers.PBPC = 0;
	Registers.PB = 0;
	Registers.PCw = S9xGetWord(0xfffc);
	OpenBus = Registers.PCh;
	Registers.D.W = 0;
	Registers.DB = 0;
	Registers.SH = 1;
	Registers.SL -= 3;
	Registers.XH = 0;
	Registers.YH = 0;

	ICPU.ShiftedPB = 0;
	ICPU.ShiftedDB = 0;
	SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	ClearFlags(Decimal);

	Timings.H_Max = Timings.H_Max_Master;
	Timings.V_Max = Timings.V_Max_Master;
	Timings.NMITriggerPos = 0xffff;
	Timings.NextIRQTimer = 0x0fffffff;
	Timings.IRQFlagChanging = IRQ_NONE;

	if (Model->_5A22 == 2)
		Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2;
	else
		Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v1;

	S9xSetPCBase(Registers.PBPC);

	ICPU.S9xOpcodes = S9xOpcodesE1;
	ICPU.S9xOpLengths = S9xOpLengthsM1X1;

	S9xUnpackStatus();
}

void S9xReset (void)
{
	// BIOS-less GB/GBC: same crash protection as S9xSoftReset — the
	// SNES is dormant, running the full SNES reset chain triggers a
	// stale-state SPC DSP synthesis pass that crashes. Cold-reset the
	// GB core and bail out.
	if (Settings.SuperGameBoy && !Settings.SGB_BIOSModeActive)
	{
		S9xSGBReset();
		S9xInitCheatData();
		return;
	}

	S9xResetSaveTimer(FALSE);

	memset(Memory.RAM, 0x55, sizeof(Memory.RAM));
	memset(Memory.VRAM, 0x00, sizeof(Memory.VRAM));
	memset(Memory.FillRAM, 0, 0x8000);

	S9xResetBSX();
	S9xResetCPU();
	S9xResetPPU();
	S9xResetDMA();
	S9xResetAPU();
    S9xResetMSU();

	if (Settings.DSP)
		S9xResetDSP();
	if (Settings.SuperFX)
		S9xResetSuperFX();
	if (Settings.SA1)
		S9xSA1Init();
	if (Settings.SDD1)
		S9xResetSDD1();
	if (Settings.SPC7110)
		S9xResetSPC7110();
	if (Settings.C4)
		S9xInitC4();
	if (Settings.OBC1)
		S9xResetOBC1();
	if (Settings.SRTC)
		S9xResetSRTC();
	if (Settings.MSU1)
		S9xMSU1Init();

	// SGB / GB / GBC: cold-reset the GB core on File→Reset. In BIOS mode
	// (SGB_BIOSModeActive) this clears the handshake cache so the next
	// $6003 release re-runs the boot ROM and replays the splash. In
	// BIOS-less mode (SuperGameBoy) the SNES side is being reset around
	// us — leaving the GB CPU/PPU/APU in mid-game state would make them
	// run against a freshly-reset SNES with mismatched expectations,
	// which crashes the emulator. Resetting the GB core too restarts
	// the game cleanly.
	if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
		S9xSGBReset();

	S9xInitCheatData();
}

void S9xSoftReset (void)
{
	// BIOS-less GB/GBC mode — the SNES side is dormant (cpuexec's
	// SuperGameBoy branch bypasses the 65816/SPC entirely). Running
	// the full SNES reset chain crashes: S9xSoftResetCPU's reset-
	// vector read at line 65 (S9xGetWord(0xfffc)) advances CPU.Cycles
	// past NextEvent, triggering S9xDoHEventProcessing → S9xAPUEnd-
	// Scanline → dsp.synchronize() → SPC_DSP::run, and the SPC DSP
	// has been idle since power-on so its synthesis state can't
	// catch up cleanly. Just cold-reset the GB core and return —
	// the user expects File→Reset to restart the GB game, nothing
	// more. (BIOS mode keeps the full path; the SNES is live there.)
	if (Settings.SuperGameBoy && !Settings.SGB_BIOSModeActive)
	{
		S9xSGBReset();
		S9xInitCheatData();
		return;
	}

	S9xResetSaveTimer(FALSE);

	memset(Memory.FillRAM, 0, 0x8000);

	// SGB BIOS mode: clear SNES WRAM/VRAM so the SGB BIOS state machine cold-boots — a soft reset that preserves it leaves a GB game that sends no SGB palette packets stuck on a black screen.
	if (Settings.SGB_BIOSModeActive)
	{
		memset(Memory.RAM,  0x55, sizeof(Memory.RAM));
		memset(Memory.VRAM, 0x00, sizeof(Memory.VRAM));
	}

	if (Settings.BS)
		S9xResetBSX();

	S9xSoftResetCPU();
	S9xSoftResetPPU();
	S9xResetDMA();
	S9xSoftResetAPU();
    S9xResetMSU();

	if (Settings.DSP)
		S9xResetDSP();
	if (Settings.SuperFX)
		S9xResetSuperFX();
	if (Settings.SA1)
		S9xSA1Init();
	if (Settings.SDD1)
		S9xResetSDD1();
	if (Settings.SPC7110)
		S9xResetSPC7110();
	if (Settings.C4)
		S9xInitC4();
	if (Settings.OBC1)
		S9xResetOBC1();
	if (Settings.SRTC)
		S9xResetSRTC();
	if (Settings.MSU1)
		S9xMSU1Init();

	// Same as S9xReset above — File→Reset uses this path on the win32
	// frontend. Cold-reset the GB core for both BIOS and BIOS-less so
	// the GB doesn't keep running against a freshly-reset SNES side.
	if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
		S9xSGBReset();

	S9xInitCheatData();
}
