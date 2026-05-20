# Olympic Summer Games — SGB BIOS-Mode Hang

## Symptom

`Olympic Summer Games (USA, Europe) (SGB Enhanced).gb` stalls on the SGB
yellow palette page in authentic-BIOS mode. The GB side advances (you can
watch `handoff_frames` and GB `VRAM hash` change between debugger samples),
the SGB BIOS keeps assembling packets, but the SNES side stops driving the
BG3 char-data DMA so nothing renders into the GB display window. The game
loads fine in BIOS-less mode.

All other SGB-enhanced titles tested (Zelda DX, Donkey Kong, Tetris Plus,
Pokemon Red, Pro Mahjong Kiwame, Toy Story, Animaniacs, Balloon Fight)
boot cleanly after the per-opcode sync work — Olympic was the lone holdout.

## Diagnostic trail

We extended the CPU debugger (`Emulation → CPU Debugger`) along the way so
the symptoms could be read off live state. The GB+SGB tab now exposes:

- Per-cmd packet counters: `DATA_SND`, `DATA_TRN`, `JUMP`, and the last
  destination address recorded for each
- A ring of the last 16 `DATA_SND` packets' full 16-byte payload, with
  the address bytes shown both as `(lo, mid, bank)` and `(lo, bank, hi)`
  so the pandocs encoding can be confirmed against actual writes
- WRAM peek windows on the SNES tab: `$7E:0000-$00FF`, `$7E:0200-$02FF`
  (covering the `$02CA` IRQ-CLI gate flag), `$7E:0800-$08FF`, and
  `$7E:0900-$09FF`

Two debugger papercuts were also fixed because they were hiding the bug:

- Instruction-byte preview used `MEMMAP_MASK` (`$0FFF`) to index into
  `Memory.Map[block]`, but the snes9x convention is `& $FFFF` (low 16 bits
  of the bank-relative address). With the wrong mask, `PC=$00:BA6A`
  displayed bytes `0A 0A 0A 0A` instead of the actual `A5 22 F0 FC`
  (`LDA $22; BEQ $-04`), completely obscuring the disassembly.
- Refresh timer was clobbering scroll position; now preserved via
  `EM_GETFIRSTVISIBLELINE` + `EM_LINESCROLL` with `WM_SETREDRAW` flicker
  suppression around `SetWindowText`.

## Root cause

The SNES CPU is parked at `$00:BA6A` in the SGB2 BIOS:

```
$00:BA67  STY $0216          ; sub_80BA14 — band-DMA wait
$00:BA6A  LDA $22            ; ← stuck here
$00:BA6C  BEQ $BA6A          ; spin until $22 != 0
$00:BA6E  STZ $0216
$00:BA71  RTS
```

The flag at WRAM `$7E:0022` is set only by the SGB BIOS's H/V-IRQ handler
at `$00:B59B`:

```
$00:B591  LDA #$0F / STA $2100   ; INIDISP brightness 15
$00:B596  LDA $4211              ; clear IRQ pending
$00:B599  LDA #$01
$00:B59B  STA $22                ; ← this is what the wait needs
$00:B59D  PLP PLB PLY PLX PLA RTI
```

The IRQ handler does the BG3 char-data DMA setup that drives the GB
framebuffer into VRAM `$7000` each frame. Without it firing, BG3 char
stays empty and the screen shows the yellow palette.

For the IRQ to fire, the CPU's `I` flag must be 0. In our snapshot at
the hang it is `1` (status `$37 = ..MX.IZC`). With `NMITIMEN=$31` NMI is
also off (bit 7 = 0), so no interrupt path can reach the handler.

The `I=1` source is the SGB BIOS's own `JUMP` packet handler at
`$00:C72B`, which Olympic triggers:

```
$00:C70D  LDA $0604 / STA $BB     ; \  install NMI vector from
$00:C710  LDA $0605 / STA $BC     ;  | packet bytes 4..6 into
$00:C715  LDA $0606 / STA $BD     ; /  DP $00BB-$00BD
$00:C71C  LDA $0601 / STA $B8     ; \  install JMP target from
$00:C71F  LDA $0602 / STA $B9     ;  | packet bytes 1..3 into
$00:C724  LDA $0603 / STA $BA     ; /  DP $00B8-$00BA
$00:C72B  SEI                     ; ← disable IRQs
$00:C72C  JMP [byte_7E00B8]       ; transfer control to user code
```

Per pandocs (`SGB_Command_System.html`):

> IRQs and COP and BRK instructions are not useful because their handlers
> still point into SGB ROM. **Use SEI WAI.**

So the BIOS deliberately leaves IRQ masked across the JUMP-target's
lifetime — the user code is supposed to manage interrupts itself.

Olympic ships its uploaded SNES code via 13 `DATA_SND` packets followed
by one `JUMP`. With pandocs' standard `pkt[1]=lo, pkt[2]=high, pkt[3]=bank`
encoding (the debugger confirms `byte0=$79, cmd=$0F, len=1` and raw
`1B 08 00 0B` for the first packet, landing at `$7E:081B`), the uploaded
code installs two hooks:

1. **`$7E:081B`** (5 NOPs then handler at `$7E:0820-$0860`) — the JUMP
   target. Compares DP `$C9/$CA/$CB` against fixed signatures; on match,
   fills `$7E:C001+0x28..0x8C` with `$E7` bytes (4-stride). Pure
   value-test then `RTS`. **No `CLI`.**
2. **`$7E:0900-$0921`** — a `SOU_TRN` (cmd `$09`) intercept reached via
   `JSR $0800` placed at the head of every packet dispatch by Olympic's
   third hook at `$7E:0800` (`JMP $0900`). The intercept calls the BIOS
   sound subroutines at `$00:C573` (SGB1) or `$00:C576` (SGB2). Both
   eventually call `sub_80C58D`, which **does `CLI`** when `$02CA != 0`
   — and `$02CA = $01` in our boot state.

So on real hardware Olympic restores `I=0` by piggybacking on its own
SOU_TRN packets: the BIOS processes SOU_TRN, Olympic's hook dispatches
into `sub_80C58D`, which CLIs before returning.

In our emulation that recovery never reaches the BIOS because the BIOS
hits the `$00:BA6A` wait first — the V-counter happens to land in the
`$DC..$DE` window when `sub_80BA14` is called, the wait entry condition
fires, and the BIOS sits there with `I=1` waiting for an IRQ that can
never come. The post-JUMP SOU_TRN packets that would CLI sit unprocessed
in the ICD2 queue (`queue_count` grows over time, last cmd IDs include
`$09`s, but `r_6002`/`r_7800`/`w_6001` counters stay frozen confirming
the BIOS isn't reading or driving anything).

## Fix

A single targeted `CLI` in `S9xMainLoop` (`cpuexec.cpp`), gated on the
exact wait address:

```cpp
if (Settings.SGB_BIOSModeActive &&
    Registers.PB == 0x00 && Registers.PCw == 0xBA6A &&
    CheckIRQ())
{
    ClearIRQ();
}
```

When the BIOS spins at `$BA6A` with `I=1`, this clears it; the very next
opcode dispatch lets the pending H/V IRQ fire, the handler writes
`$22 = 1`, the wait exits, BG3 char DMA resumes, and Olympic renders.

Non-JUMP-using SGB titles never reach `$BA6A` with `I=1` (the only `I=1`
source in the main-loop call chain is the `$C72B` SEI inside the JUMP
packet handler), so they are not affected.

Verified working on Olympic Summer Games after this change: tiles, custom
borders, audio all correct. Other SGB titles spot-checked for regression:
none.

## Why this is a fix, not a hack

The check restores the invariant the BIOS code at `$BA67` was written to
expect. `sub_80BA14`'s author assumed callers would have IRQs enabled
when reaching the wait — the wait literally cannot complete otherwise.
On real SGB hardware Olympic happens to satisfy that invariant via its
SOU_TRN-hook side-effect; our emulation arrives at the wait before that
side-effect fires. The CLI doesn't change behavior anywhere except the
one address that demands it.

## References

- SGB2 BIOS image: `win32/BIOS/SGB2.sfc` (LoROM, 512KB, header at
  `$7FC0` "Super GAMEBOY2")
- Full BIOS disassembly used for tracing the call chain:
  `C:\Users\shany\Downloads\SGB2.sfc.lst` (IDA-style listing)
- Pandocs SGB reference: `SGB_Command_System.html` (DATA_SND, JUMP),
  `SGB_Command_Packet.html` (transport layer)
- Relevant native-mode interrupt vectors (`$00:FFE0+`):
  - NMI `$FFEA` → `$00:8100` (`JML [$00BB]` indirect through WRAM —
    JUMP packet rewrites this pointer)
  - IRQ `$FFEE` → `$00:B50A` (fixed in ROM, runs the band-DMA setup
    and `STA $22` that the wait at `$BA6A` needs)
- Key BIOS subroutines for the call chain leading to the hang:
  - `sub_80AEFC` ($0101=5 game-mode entry; JMLed from `sub_808EA0`)
  - `sub_80B9BE` (band processor, called by `sub_80AEFC`)
  - `sub_80BA14` (band-DMA setup containing the `$BA6A` wait)
  - `sub_80BAA4` (per-iteration packet processing — calls
    `sub_80BBD9` twice and Olympic's `JSR $0810` hook)
  - `sub_80BC73` (`JSR $0800` Olympic's cmd dispatch hook entry,
    then `JML sub_80AF3C` normal dispatch)
  - `sub_80C58D` (the `SEI..CLI` routine Olympic's SOU_TRN hook
    eventually drives into; gates final CLI on `$02CA != 0`)
- Implementation: commit `6ad6c373` ("sgb: fix Olympic Summer Games
  hang in BIOS mode") in `cpuexec.cpp`.
