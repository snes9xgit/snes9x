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
#include "sfcbox.h"
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

	memset(Memory.RAM, SNESGameFixes.RAMInitialValue, sizeof(Memory.RAM));
	memset(Memory.VRAM, 0x00, sizeof(Memory.VRAM));
	memset(Memory.FillRAM, 0, 0x8000);

	S9xResetBSX();
	if (PF94.active)
	{
		S9xPF94LoadGames();
		S9xPF94Reset();
	}
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

	// SGB / GB / GBC: cold-reset the GB core on a hard reset. In BIOS mode
	// (SGB_BIOSModeActive) this clears the handshake cache so the next
	// $6003 release re-runs the boot ROM and replays the splash. In
	// BIOS-less mode (SuperGameBoy) the SNES side is being reset around
	// us — leaving the GB CPU/PPU/APU in mid-game state would make them
	// run against a freshly-reset SNES with mismatched expectations,
	// which crashes the emulator. Resetting the GB core too restarts
	// the game cleanly.
	if (Settings.SuperGameBoy || Settings.SGB_BIOSModeActive)
		S9xSGBReset();

	// SFC-Box: a hard reset power-cycles the supervisor board too; the
	// freshly-reset KROM immediately holds the SNES until it's ready.
	if (Settings.SFCBox)
		S9xSFCBoxPowerOn();

	S9xInitCheatData();
}

static uint8  *sgb_softreset_state       = NULL;
static uint32  sgb_softreset_state_size  = 0;
static bool8   sgb_softreset_state_valid = FALSE;

void S9xSGBCaptureSoftResetCheckpoint (void)
{
	if (!Settings.SGB_BIOSModeActive)
		return;
	uint32 sz = S9xFreezeSize();
	if (sz == 0)
		return;
	if (sgb_softreset_state_size != sz)
	{
		delete[] sgb_softreset_state;
		sgb_softreset_state      = new uint8[sz];
		sgb_softreset_state_size = sz;
	}
	sgb_softreset_state_valid = S9xFreezeGameMem(sgb_softreset_state, sgb_softreset_state_size);
}

void S9xSGBInvalidateSoftResetCheckpoint (void)
{
	sgb_softreset_state_valid = FALSE;
}

static bool8 S9xSGBRestoreSoftResetCheckpoint (void)
{
	if (!sgb_softreset_state_valid)
		return FALSE;

	size_t  sram_size = S9xSGBGetSRAMSize();
	uint8  *sram_copy = NULL;
	if (sram_size > 0)
	{
		uint8 *live = S9xSGBGetSRAM();
		if (live)
		{
			sram_copy = new uint8[sram_size];
			memcpy(sram_copy, live, sram_size);
		}
	}

	int result = S9xUnfreezeGameMem(sgb_softreset_state, sgb_softreset_state_size);

	if (sram_copy)
	{
		uint8 *live = S9xSGBGetSRAM();
		if (result == SUCCESS && live && S9xSGBGetSRAMSize() == sram_size)
			memcpy(live, sram_copy, sram_size);
		delete[] sram_copy;
	}

	return (result == SUCCESS) ? TRUE : FALSE;
}

void S9xSoftReset (void)
{
	// SGB BIOS mode: warm-restart only from the settled checkpoint;
	// anything earlier must power-cycle or the BIOS wedges mid-splash.
	if (Settings.SGB_BIOSModeActive)
	{
		if (S9xSGBRestoreSoftResetCheckpoint())
		{
			S9xInitCheatData();
			return;
		}
		S9xReset();
		return;
	}
	if (Settings.SuperGameBoy)
	{
		if (!S9xSGBSoftReset())
		{
			S9xReset();
			return;
		}
		S9xInitCheatData();
		return;
	}

	// SFC-Box: a soft reset restarts the CURRENT GAME — the L+R+Select+
	// Start joypad-combo semantics, where the KROM just pulses the SNES
	// reset line with the mapping intact. The supervisor board keeps
	// running untouched, so the plain SNES-side reset below is exactly
	// right. (A full box restart — the front-panel button — is the hard
	// reset, S9xReset.)

	S9xResetSaveTimer(FALSE);

	memset(Memory.FillRAM, 0, 0x8000);

	if (Settings.BS)
		S9xResetBSX();

	if (PF94.active)
		S9xPF94Reset();

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

	S9xInitCheatData();
}
