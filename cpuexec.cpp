/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "snes9x.h"
#include "memmap.h"
#include "cpuops.h"
#include "dma.h"
#include "apu/apu.h"
#include "fxemu.h"
#include "snapshot.h"
#include "movie.h"
#include "ppu.h"
#include "gfx.h"
#include "sgb/sgb.h"
#include "sfcbox.h"
#include "voicekun.h"
#ifdef DEBUGGER
#include "debug.h"
#include "missing.h"
#include "getset.h"
#endif

static inline void S9xReschedule (void);

void S9xMainLoop (void)
{
	// Super Game Boy mode — run the GB core for one frame and return.
	// The 65816 loop below is bypassed entirely; snes9x's frontends
	// call S9xMainLoop once per frame, so this satisfies the contract.
	if (Settings.SuperGameBoy)
	{
		if (CPU.Flags & SCAN_KEYS_FLAG)
		{
			CPU.Flags &= ~SCAN_KEYS_FLAG;
			S9xMovieUpdate();
		}

		IPPU.RenderThisFrame      = !Settings.InRunAhead;
		PPU.ScreenHeight          = SGB_GB_SCREEN_H;
		IPPU.RenderedScreenWidth  = SGB_GB_SCREEN_W;
		IPPU.RenderedScreenHeight = SGB_GB_SCREEN_H;

		// Pipe the SNES controller 0 bitmask into the GB joypad. P6d's
		// MLT_REQ handling reads from mlt_current_player; for now we
		// only wire the first controller.
		S9xSGBSetJoypad(MovieGetJoypad(0));

		// Push timing knobs each frame so UI changes take effect live.
		// Default GBClockMultiplier to 1.0 if it's been left at its
		// zero-initialized value (backwards-compat with older configs).
		const float mul = (Settings.GBClockMultiplier > 0.0f)
		                  ? Settings.GBClockMultiplier : 1.0f;
		S9xSGBSetClockMultiplier(mul);
		S9xSGBSetRunMode(Settings.GameBoyRunMode);
		// Match the GB APU's downsample target to the host playback rate
		// so the samples we drain need no further rate conversion. The
		// setter is idempotent, so calling every frame is cheap and
		// picks up sound-config changes (output device / rate switch).
		S9xSGBSetAudioRate(Settings.SoundPlaybackRate);

		// Hidden runahead frames advance GB state but skip blit/present:
		// double-presenting per displayed frame can clobber video on
		// some host backends.
		if (!Settings.InRunAhead)
			S9xStartScreenRefresh();
		S9xSGBRunFrame();
		if (!Settings.InRunAhead)
		{
			IPPU.RenderedScreenWidth  = SGB_GB_SCREEN_W;
			IPPU.RenderedScreenHeight = SGB_GB_SCREEN_H;
			S9xSGBBlitScreenGB(GFX.Screen, GFX.RealPPL);
			S9xEndScreenRefresh();
		}

		// Drive the host audio callback to drain GB samples into the
		// sound device. In standard SNES mode the SPC scanline path
		// fires this from S9xAPUEndScanline; in BIOS-less SGB mode
		// that path is bypassed so we trigger it ourselves once per
		// frame. The defensive cap in S9xSGBGetSampleCount keeps
		// ProcessSound's wait condition satisfiable so we don't pin
		// at 1 fps the way the prior naive hookup did.
		if (!Settings.InRunAhead)
			S9xLandSamples();

		if (!Settings.InRunAhead)
			S9xSyncSpeed();

		CPU.Flags |= SCAN_KEYS_FLAG;
		return;
	}

	#define CHECK_FOR_IRQ_CHANGE() \
	if (Timings.IRQFlagChanging) \
	{ \
		if (Timings.IRQFlagChanging & IRQ_TRIGGER_NMI) \
		{ \
			CPU.NMIPending = TRUE; \
			Timings.NMITriggerPos = CPU.Cycles + 6; \
		} \
		if (Timings.IRQFlagChanging & IRQ_CLEAR_FLAG) \
			ClearIRQ(); \
		else if (Timings.IRQFlagChanging & IRQ_SET_FLAG) \
			SetIRQ(); \
		Timings.IRQFlagChanging = IRQ_NONE; \
	}

	if (CPU.Flags & SCAN_KEYS_FLAG)
	{
		CPU.Flags &= ~SCAN_KEYS_FLAG;
		S9xMovieUpdate();
	}

	// Reset the SGB sync anchor to the current SNES cycle at loop entry
	// and publish the scanline period for wrap-compensation. getset.h
	// calls S9xSGBSyncToSnesCycle on every ICD2 access; without a fresh
	// anchor the first delta would be huge (or bogus-negative if a
	// scanline wrap just happened) and misattribute past SNES cycles
	// to the GB core.
	if (Settings.SGB_BIOSModeActive)
	{
		S9xSGBSetHMax(Timings.H_Max);
		S9xSGBResetSyncAnchor(CPU.Cycles);
	}

	for (;;)
	{
		if (Settings.SFCBox)
		{
			// The KROM released the reset line: reboot the SNES side at an
			// instruction boundary (the mapping registers already point at
			// whatever the KROM selected).
			if (S9xSFCBoxPendingReset())
				S9xSFCBoxApplySNESReset();

			// Held in reset: skip opcode execution but keep the H/V event
			// machinery running so the Z180, APU and frame pacing advance.
			if (S9xSFCBoxSNESHeld())
			{
				CPU.Cycles = CPU.NextEvent;
				while (CPU.Cycles >= CPU.NextEvent)
					S9xDoHEventProcessing();
				if (CPU.Flags & SCAN_KEYS_FLAG)
					break;
				continue;
			}
		}

		if (CPU.NMIPending)
		{
			#ifdef DEBUGGER
			if (Settings.TraceHCEvent)
			    S9xTraceFormattedMessage ("Comparing %d to %d\n", Timings.NMITriggerPos, CPU.Cycles);
			#endif
			if (Timings.NMITriggerPos <= CPU.Cycles)
			{
				CPU.NMIPending = FALSE;
				Timings.NMITriggerPos = 0xffff;
				if (CPU.WaitingForInterrupt)
				{
					CPU.WaitingForInterrupt = FALSE;
					Registers.PCw++;
					CPU.Cycles += TWO_CYCLES + ONE_DOT_CYCLE / 2;
					while (CPU.Cycles >= CPU.NextEvent)
						S9xDoHEventProcessing();
				}

				CHECK_FOR_IRQ_CHANGE();
				S9xOpcode_NMI();
			}
		}

		if (CPU.Cycles >= Timings.NextIRQTimer)
		{
			#ifdef DEBUGGER
			S9xTraceMessage ("Timer triggered\n");
			#endif

			S9xUpdateIRQPositions(false);
			CPU.IRQLine = TRUE;
		}

		if (CPU.IRQLine || CPU.IRQExternal)
		{
			if (CPU.WaitingForInterrupt)
			{
				CPU.WaitingForInterrupt = FALSE;
				Registers.PCw++;
				CPU.Cycles += TWO_CYCLES + ONE_DOT_CYCLE / 2;
				while (CPU.Cycles >= CPU.NextEvent)
					S9xDoHEventProcessing();
			}

			if (!CheckFlag(IRQ))
			{
				/* The flag pushed onto the stack is the new value */
				CHECK_FOR_IRQ_CHANGE();
				S9xOpcode_IRQ();
			}
		}

		/* Change IRQ flag for instructions that set it only on last cycle */
		CHECK_FOR_IRQ_CHANGE();

	#ifdef DEBUGGER
		if ((CPU.Flags & BREAK_FLAG) && !(CPU.Flags & SINGLE_STEP_FLAG))
		{
			for (int Break = 0; Break != 6; Break++)
			{
				if (S9xBreakpoint[Break].Enabled &&
					S9xBreakpoint[Break].Bank == Registers.PB &&
					S9xBreakpoint[Break].Address == Registers.PCw)
				{
					if (S9xBreakpoint[Break].Enabled == 2)
						S9xBreakpoint[Break].Enabled = TRUE;
					else
						CPU.Flags |= DEBUG_MODE_FLAG;
				}
			}
		}

		if (CPU.Flags & DEBUG_MODE_FLAG)
			break;

		if (CPU.Flags & TRACE_FLAG)
			S9xTrace();

		if (CPU.Flags & SINGLE_STEP_FLAG)
		{
			CPU.Flags &= ~SINGLE_STEP_FLAG;
			CPU.Flags |= DEBUG_MODE_FLAG;
		}
	#endif

		if (CPU.Flags & SCAN_KEYS_FLAG)
		{
			break;
		}

		// Olympic Summer Games (SGB Enhanced) workaround. The SGB BIOS's
		// JUMP packet handler at $00:C72B does SEI before JMP [$00B8] to
		// transfer control to user-uploaded code (Olympic's $7E:081B
		// handler). Per pandocs the JUMP target runs with IRQs disabled
		// intentionally. On real hardware Olympic re-enables IRQ later
		// via its cmd-09 (SOU_TRN) hook at $7E:0900 which calls sub_80C58D
		// (CLI when $02CA != 0). In our emulation the BIOS reaches the
		// V-counter-gated wait at $00:BA6A (LDA $22; BEQ $-04) before
		// the SOU_TRN packets get drained, so I=1 blocks the IRQ that
		// would write $22 and the wait hangs forever. Clearing I here
		// restores the invariant the wait requires; non-Olympic SGB
		// titles never reach $BA6A with I=1 so they are unaffected.
		if (Settings.SGB_BIOSModeActive &&
		    Registers.PB == 0x00 && Registers.PCw == 0xBA6A &&
		    CheckIRQ())
		{
			ClearIRQ();
		}

		// Voicer-kun: the game names a CD track, starts it, or ends the voice.
		if (Settings.VoiceKun)
		{
			if (VoiceKunHook.PlayPC && Registers.PBPC == VoiceKunHook.PlayPC)
			{
				uint32	s = Registers.S.W;
				int	arg = 0;
				if (!VoiceKunHook.ArmPC)
					arg = VoiceKunHook.ArgStackOff
						? (Memory.RAM[(s + VoiceKunHook.ArgStackOff) & 0x1ffff] |
						   (Memory.RAM[(s + VoiceKunHook.ArgStackOff + 1) & 0x1ffff] << 8))
						  + VoiceKunHook.TrackBias
						: (Registers.A.W & 0xff) + VoiceKunHook.TrackBias;
				S9xVoiceKunPlayTrack(arg);
			}
			else
			if (VoiceKunHook.ArmPC && Registers.PBPC == VoiceKunHook.ArmPC)
			{
				uint32	s = Registers.S.W;
				int	arg = Memory.RAM[(s + VoiceKunHook.ArmArgOff) & 0x1ffff] |
				          (Memory.RAM[(s + VoiceKunHook.ArmArgOff + 1) & 0x1ffff] << 8);
				S9xVoiceKunArmTrack(arg + VoiceKunHook.TrackBias);
			}
			else
			if (VoiceKunHook.StopPC && Registers.PBPC == VoiceKunHook.StopPC)
				S9xVoiceKunStop();
			else
			if (VoiceKunHook.IRCmdPC && Registers.PBPC == VoiceKunHook.IRCmdPC)
			{
				uint32	s = Registers.S.W;
				int	cmd = Memory.RAM[(s + VoiceKunHook.IRCmdArgOff) & 0x1ffff] |
				          (Memory.RAM[(s + VoiceKunHook.IRCmdArgOff + 1) & 0x1ffff] << 8);
				S9xVoiceKunDeckCommand(cmd);
			}

			// Voice counter: ticks once per voiced scene whether or not the
			// game prompts for the disc, so it catches scene changes the
			// prompt hooks miss (continuous play).
			int	vaddr = S9xVoiceKunVoiceIdAddr();
			if (vaddr)
				S9xVoiceKunPollVoiceId(Memory.RAM[vaddr & 0x1ffff]);
		}

		uint8				Op;
		struct	SOpcodes	*Opcodes;

		if (CPU.PCBase)
		{
			Op = CPU.PCBase[Registers.PCw];
			CPU.Cycles += CPU.MemSpeed;
			Opcodes = ICPU.S9xOpcodes;

			if (CPU.Cycles > 1000000)
			{
				Settings.StopEmulation = true;
				CPU.Flags |= HALTED_FLAG;
				S9xMessage(S9X_FATAL_ERROR, 0, "CPU is deadlocked");
				return;
			}
		}
		else
		{
			Op = S9xGetByte(Registers.PBPC);
			OpenBus = Op;
			Opcodes = S9xOpcodesSlow;
		}

		if ((Registers.PCw & MEMMAP_MASK) + ICPU.S9xOpLengths[Op] >= MEMMAP_BLOCK_SIZE)
		{
			uint8	*oldPCBase = CPU.PCBase;

			CPU.PCBase = S9xGetBasePointer(ICPU.ShiftedPB + ((uint16) (Registers.PCw + 4)));
			if (oldPCBase != CPU.PCBase || (Registers.PCw & ~MEMMAP_MASK) == (0xffff & ~MEMMAP_MASK))
				Opcodes = S9xOpcodesSlow;
		}

		Registers.PCw++;
		(*Opcodes[Op].S9xOpcode)();

		if (Settings.SA1)
			S9xSA1MainLoop();

		// Per-SNES-opcode GB sync — instruction-level interleaving.
		// Mesen-equivalent granularity: every SNES opcode advances the
		// GB by the corresponding cycle delta, so $6000/$6002/$7800
		// reads from the BIOS see GB state that varies naturally as
		// SNES cycles tick. Without this fine-grained sync the BIOS
		// band counter ($0294) phase-locks to deterministic GB
		// scanlines, $0294 never reaches 18, the swap never fires,
		// the WRAM ping-pong buffers never fill, and BG3 char renders
		// empty. Costs ~50% wall fps in BIOS mode (was the reason
		// it was removed in 0762b725), but it's the only way to
		// match real-hardware-equivalent BIOS state-machine progress.
		// Only gates on BIOS-released so pre-release boot/handshake
		// stays cheap.
		if (Settings.SGB_BIOSModeActive && S9xSGBBIOSGBIsReleased())
			S9xSGBSyncToSnesCycle(CPU.Cycles);
	}

	// P2 — in BIOS mode the GB core is held in reset until the BIOS
	// writes the release bit to the ICD2 reset register ($6003 bit 7).
	// Matches real SGB hardware: the SNES boots first, brings up border
	// + palette, then unblocks the GB CPU. Until then we skip stepping
	// entirely so the SNES loop keeps its 60 fps budget.
	if (Settings.SGB_BIOSModeActive)
	{
		const bool released = S9xSGBBIOSGBIsReleased();

		if (released)
		{
			const float mul = (Settings.GBClockMultiplier > 0.0f)
			                  ? Settings.GBClockMultiplier : 1.0f;
			S9xSGBSetClockMultiplier(mul);
			S9xSGBSetRunMode(Settings.GameBoyRunMode);
			S9xSGBSetAudioRate(Settings.SoundPlaybackRate);
		}

	}

	S9xPackStatus();
}

static inline void S9xReschedule (void)
{
	switch (CPU.WhichEvent)
	{
		case HC_HBLANK_START_EVENT:
			CPU.WhichEvent = HC_HDMA_START_EVENT;
			CPU.NextEvent  = Timings.HDMAStart;
			break;

		case HC_HDMA_START_EVENT:
			CPU.WhichEvent = HC_HCOUNTER_MAX_EVENT;
			CPU.NextEvent  = Timings.H_Max;
			break;

		case HC_HCOUNTER_MAX_EVENT:
			CPU.WhichEvent = HC_HDMA_INIT_EVENT;
			CPU.NextEvent  = Timings.HDMAInit;
			break;

		case HC_HDMA_INIT_EVENT:
			CPU.WhichEvent = HC_RENDER_EVENT;
			CPU.NextEvent  = Timings.RenderPos;
			break;

		case HC_RENDER_EVENT:
			CPU.WhichEvent = HC_WRAM_REFRESH_EVENT;
			CPU.NextEvent  = Timings.WRAMRefreshPos;
			break;

		case HC_WRAM_REFRESH_EVENT:
			CPU.WhichEvent = HC_HBLANK_START_EVENT;
			CPU.NextEvent  = Timings.HBlankStart;
			break;
	}
}

void S9xDoHEventProcessing (void)
{
#ifdef DEBUGGER
	static char	eventname[7][32] =
	{
		"",
		"HC_HBLANK_START_EVENT",
		"HC_HDMA_START_EVENT  ",
		"HC_HCOUNTER_MAX_EVENT",
		"HC_HDMA_INIT_EVENT   ",
		"HC_RENDER_EVENT      ",
		"HC_WRAM_REFRESH_EVENT"
	};
#endif

#ifdef DEBUGGER
	if (Settings.TraceHCEvent)
		S9xTraceFormattedMessage("--- HC event processing  (%s)  expected HC:%04d  executed HC:%04d VC:%04d",
			eventname[CPU.WhichEvent], CPU.NextEvent, CPU.Cycles, CPU.V_Counter);
#endif

	switch (CPU.WhichEvent)
	{
		case HC_HBLANK_START_EVENT:
			S9xReschedule();
			break;

		case HC_HDMA_START_EVENT:
			S9xReschedule();

			if (PPU.HDMA && CPU.V_Counter <= PPU.ScreenHeight)
			{
			#ifdef DEBUGGER
				S9xTraceFormattedMessage("*** HDMA Transfer HC:%04d, Channel:%02x", CPU.Cycles, PPU.HDMA);
			#endif
				PPU.HDMA = S9xDoHDMA(PPU.HDMA);
			}

			break;

		case HC_HCOUNTER_MAX_EVENT:
			if (Settings.SuperFX)
			{
				if (!SuperFX.oneLineDone)
					S9xSuperFXExec();
				SuperFX.oneLineDone = FALSE;
			}

			// Per-scanline GB sync in BIOS-released mode. Replaces the
			// old per-opcode hook for performance (millions of calls/sec
			// → ~262×60 = 15k/sec). ICD2-register accesses in getset.h
			// also call SyncToSnesCycle inline so $7800/$6002 reads
			// still see freshest GB state mid-opcode; this scanline-end
			// sync just guarantees forward progress on scanlines that
			// have no ICD2 traffic.
			if (Settings.SGB_BIOSModeActive && S9xSGBBIOSGBIsReleased())
				S9xSGBSyncToSnesCycle(CPU.Cycles);

			// SFC-Box: the supervisor HD64180 runs its slice of every
			// scanline whether or not the SNES itself is executing.
			if (Settings.SFCBox)
				S9xSFCBoxEndScanline();

			S9xAPUEndScanline();
			CPU.Cycles -= Timings.H_Max;
			// Keep the SGB sync anchor continuous across the wrap so
			// multi-scanline deltas (DMA bursts) aren't discarded.
			if (Settings.SGB_BIOSModeActive)
				S9xSGBNotifyScanlineWrap(Timings.H_Max);
			if (Timings.NMITriggerPos != 0xffff)
				Timings.NMITriggerPos -= Timings.H_Max;
			if (Timings.NextIRQTimer != 0x0fffffff)
				Timings.NextIRQTimer -= Timings.H_Max;
			S9xAPUSetReferenceTime(CPU.Cycles);

			if (Settings.SA1)
				SA1.Cycles -= Timings.H_Max * 3;

			CPU.V_Counter++;
			if (CPU.V_Counter >= Timings.V_Max)	// V ranges from 0 to Timings.V_Max - 1
			{
				CPU.V_Counter = 0;

				// From byuu:
				// [NTSC]
				// interlace mode has 525 scanlines: 263 on the even frame, and 262 on the odd.
				// non-interlace mode has 524 scanlines: 262 scanlines on both even and odd frames.
				// [PAL] <PAL info is unverified on hardware>
				// interlace mode has 625 scanlines: 313 on the even frame, and 312 on the odd.
				// non-interlace mode has 624 scanlines: 312 scanlines on both even and odd frames.
				if (IPPU.Interlace && S9xInterlaceField())
					Timings.V_Max = Timings.V_Max_Master + 1;	// 263 (NTSC), 313?(PAL)
				else
					Timings.V_Max = Timings.V_Max_Master;		// 262 (NTSC), 312?(PAL)

				Memory.FillRAM[0x213F] ^= 0x80;
				PPU.RangeTimeOver = 0;

				// FIXME: reading $4210 will wait 2 cycles, then perform reading, then wait 4 more cycles.
				Memory.FillRAM[0x4210] = Model->_5A22;

				ICPU.Frame++;
				PPU.HVBeamCounterLatched = 0;
			}

			// From byuu:
			// In non-interlace mode, there are 341 dots per scanline, and 262 scanlines per frame.
			// On odd frames, scanline 240 is one dot short.
			// In interlace mode, there are always 341 dots per scanline. Even frames have 263 scanlines,
			// and odd frames have 262 scanlines.
			// Interlace mode scanline 240 on odd frames is not missing a dot.
			if (CPU.V_Counter == 240 && !IPPU.Interlace && S9xInterlaceField())	// V=240
				Timings.H_Max = Timings.H_Max_Master - ONE_DOT_CYCLE;	// HC=1360
			else
				Timings.H_Max = Timings.H_Max_Master;					// HC=1364

			if (Model->_5A22 == 2)
			{
				if (CPU.V_Counter != 240 || IPPU.Interlace || !S9xInterlaceField())	// V=240
				{
					if (Timings.WRAMRefreshPos == SNES_WRAM_REFRESH_HC_v2 - ONE_DOT_CYCLE)	// HC=534
						Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2;					// HC=538
					else
						Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v2 - ONE_DOT_CYCLE;	// HC=534
				}
			}
			else
				Timings.WRAMRefreshPos = SNES_WRAM_REFRESH_HC_v1;

			if (CPU.V_Counter == PPU.ScreenHeight + FIRST_VISIBLE_LINE)	// VBlank starts from V=225(240).
			{
				S9xEndScreenRefresh();
				#ifdef DEBUGGER
					if (!(CPU.Flags & FRAME_ADVANCE_FLAG))
				#endif
				{
					if (!Settings.InRunAhead)
						S9xSyncSpeed();
				}

				CPU.Flags |= SCAN_KEYS_FLAG;

				PPU.HDMA = 0;
				// Bits 7 and 6 of $4212 are computed when read in S9xGetPPU.
			#ifdef DEBUGGER
				missing.dma_this_frame = 0;
			#endif
				IPPU.MaxBrightness = PPU.Brightness;
				PPU.ForcedBlanking = (Memory.FillRAM[0x2100] >> 7) & 1;

				if (!PPU.ForcedBlanking)
				{
					PPU.OAMAddr = PPU.SavedOAMAddr;

					uint8	tmp = 0;

					if (PPU.OAMPriorityRotation)
						tmp = (PPU.OAMAddr & 0xFE) >> 1;
					if ((PPU.OAMFlip & 1) || PPU.FirstSprite != tmp)
					{
						PPU.FirstSprite = tmp;
						IPPU.OBJChanged = TRUE;
					}

					PPU.OAMFlip = 0;
				}

				// FIXME: writing to $4210 will wait 6 cycles.
				Memory.FillRAM[0x4210] = 0x80 | Model->_5A22;
				if (Memory.FillRAM[0x4200] & 0x80)
				{
#ifdef DEBUGGER
					if (Settings.TraceHCEvent)
					    S9xTraceFormattedMessage ("NMI Scheduled for next scanline.");
#endif
					// FIXME: triggered at HC=6, checked just before the final CPU cycle,
					// then, when to call S9xOpcode_NMI()?
					CPU.NMIPending = TRUE;
					Timings.NMITriggerPos = 6 + 6;
				}

			}

			if (CPU.V_Counter == PPU.ScreenHeight + 3)	// FIXME: not true
			{
				if (Memory.FillRAM[0x4200] & 1)
					S9xDoAutoJoypad();
			}

			if (CPU.V_Counter == FIRST_VISIBLE_LINE)	// V=1
				S9xStartScreenRefresh();

			S9xReschedule();

			break;

		case HC_HDMA_INIT_EVENT:
			S9xReschedule();

			if (CPU.V_Counter == 0)
			{
			#ifdef DEBUGGER
				S9xTraceFormattedMessage("*** HDMA Init     HC:%04d, Channel:%02x", CPU.Cycles, PPU.HDMA);
			#endif
				S9xStartHDMA();
			}

			break;

		case HC_RENDER_EVENT:
			if (CPU.V_Counter >= FIRST_VISIBLE_LINE && CPU.V_Counter <= PPU.ScreenHeight)
				RenderLine((uint8) (CPU.V_Counter - FIRST_VISIBLE_LINE));

			S9xReschedule();

			break;

		case HC_WRAM_REFRESH_EVENT:
		#ifdef DEBUGGER
			S9xTraceFormattedMessage("*** WRAM Refresh  HC:%04d", CPU.Cycles);
		#endif

			CPU.Cycles += SNES_WRAM_REFRESH_CYCLES;

			S9xReschedule();

			break;
	}

#ifdef DEBUGGER
	if (Settings.TraceHCEvent)
		S9xTraceFormattedMessage("--- HC event rescheduled (%s)  expected HC:%04d  current  HC:%04d",
			eventname[CPU.WhichEvent], CPU.NextEvent, CPU.Cycles);
#endif
}
