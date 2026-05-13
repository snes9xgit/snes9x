# SNES↔GB Clock Phase Bug — SGB BIOS-Mode

## Symptom

Tetris Plus shows ~2 frames of stripey/checkerboard artifacts in the GB-display
area around SNES frame 663–665, just after the SGB BIOS finishes its splash
fade-in to the GB-yellow logo screen. Mesen 2.1.1 does not show these
artifacts — its GB-area is clean at the same SNES frame.

Donkey Kong and Bomberman do **not** exhibit this artifact in steady state, but
prior masking attempts (zeroing `Memory.VRAM[0x7000]` and `[0xE000]` for
several frames) cause them to alternate between blank and game content.

## Investigation method

1. Per-packet OSD ring buffer logging cmd, param0, and `handoff_frames` at
   receipt — proved no SGB packets land near the corruption frame; all packets
   complete by `hf=30`.
2. SNES VRAM write hook (`S9xSgbTraceVramWrite` in `ppu.h`) logging
   PC + addr + byte + DMA-flag + `handoff_frames` per write to filtered
   regions — narrowed down where the VRAM corruption originates.
3. State dumps from both emulators at the same SNES frame:
   - SNES VRAM (64 KB), GB VRAM (8 KB), SNES OAM, GB OAM, SNES CGRAM, SNES WRAM
   - Triggered at first `hf >= 228` GB-frames-post-handoff
   - Mesen exports the same regions via Tools menu

## Diff results (us vs Mesen at SNES frame 663)

| Region | Our state | Mesen state | Verdict |
|---|---|---|---|
| **GB VRAM** (8 KB) | matches Mesen | reference | ✓ identical |
| **SNES OAM** (544 B) | all sprites parked at (232,232) | identical | ✓ identical |
| **SNES VRAM BG1 char** (`$C000-$DFFF`) | matches | reference | ✓ identical |
| **SNES VRAM BG1 tilemap** (`$7000-$77FF`) | zero (cleared at `hf=5`) | zero | ✓ identical |
| **SNES VRAM BG2 tilemap** (`$7800-$7FFF`) | SGB device-frame border | identical | ✓ identical |
| **SNES VRAM BG3 char** (`$E000-$F3FF`) | sequential `01 10 02 10 03 10…` | all zeros | ❌ **DIFFERS — 3623 bytes** |
| **SNES VRAM BG3 tilemap** (`$F800-$FFFF`) | zero | sparse `7F 01` padding + `DC 20 DD 20…` | ❌ differs |
| **SNES CGRAM palette 4** (`$80-$9F`) | color gradient | all zeros | ❌ differs (97 bytes) |
| **SNES WRAM `$7E:0280`** (state flag) | `0` | `1` | ❌ different swap state |
| **SNES WRAM `$7E:0282`** (DMA src ptr A) | `$5000` | `$6800` | ❌ unswapped |
| **SNES WRAM `$7E:0284`** (DMA src ptr B) | `$6800` | `$5000` | ❌ unswapped |
| **SNES WRAM `$7E:5000`** (buffer A) | zero | zero | ✓ identical |
| **SNES WRAM `$7E:6800`** (buffer B) | sequential `01 10 02 10 03 10…` | zero | ❌ stale BIOS-staged data |

## Visible-bug mechanism

`BGMODE=$01`, `BG3SC=$7C` → BG3 tilemap at byte `$F800`; `NBA34=$07` → BG3 char
at byte `$E000`. `TM=$17` enables BG1+BG2+BG3+sprites. With BG3 tilemap zero
in our emulator, every BG3 cell points to BG3 tile 0 (= byte `$E000-$E01F`).
Our tile 0 contains the sequential pattern `01 10 02 10 03 10 …` which, when
rendered as 2bpp tile data through palette 4 (which has a color gradient in
ours, but zeros in Mesen), produces visible vertical stripe content **as a
BG3 overlay covering the whole screen, including the GB-area**.

Mesen's BG3 char tile 0 is all zeros → BG3 layer renders blank → GB display
visible.

## Root cause — GB↔SNES phase drift

The IRQ at `$80:B50A` runs `sub_80B9BE` (at `$80:B9BE`) which does:

```
LDA $6000        ; ICD2 register: (sgb_row_latched & ~7) | sgb_bank_latched
STA $7E:0290
LDA $6000        ; debounce
CMP $7E:0290
BNE retry
LSR LSR LSR      ; >> 3 → band index (0–17)
SBC word_7E0294  ; if same band as last call, BEQ → return
                 ; else compute "bands to process" and loop sub_80BA14
```

Every 18 bands, calls `sub_80BA84` which **toggles `$7E:0280` and ping-pongs
`$7E:0282/$0284` between `$5000` and `$6800`**.

`sub_80BA14` is the band processor — it writes captured GB tile data into the
WRAM buffer pointed to by `$0282`. The IRQ DMA at `$80:B58E` then copies
`$1680` bytes from the buffer pointed to by `$0284` to VRAM `$E000` (BG3 char).

In normal operation the two pointers ping-pong every GB frame: while one
buffer is being written-to (current frame's captured tiles), the other is
DMA'd to VRAM (last frame's content).

### Trace evidence

`$6000` reads at the same code site, both emulators captured at SNES frame
~663:

| | `$6000` value | Implied `sgb_row_latched` |
|---|---|---|
| Mesen | `$00` (consistent across many reads) | 0–7 (start of GB visible frame) |
| Ours | `$88` (consistent across many reads) | 136–143 (end of GB visible frame) |

Both stable, but **~136 GB scanlines apart** — i.e., GB and SNES are in
different relative phases.

Effect on the BIOS swap logic:
- **Mesen** sees band 0 vs `$0294=9` → diff = 9 → loop processes 9 bands,
  hits the `$0294==18` wrap, **fires `sub_80BA84` (the swap)**.
- **Ours** sees band 17 vs `$0294=17` → diff = 0 → BEQ → returns immediately,
  **never reaches the swap path**.

So our IRQ samples `$6000` at a moment when the GB has already finished its
visible frame, while Mesen samples when the GB has just started one. Our
swap never triggers, so `$0282/$0284` stay at their post-init values
(`$5000/$6800`), and the IRQ DMA reads from the still-being-written
`$6800` buffer instead of the stable `$5000` zero buffer.

### Where the phase drift comes from

The 136-scanline offset is roughly **88% of a GB frame**. Plausible causes:

1. **Handoff timing** — `boot_handoff_captured` fires at `sgb.cpp:945`
   when GB boot ROM unmaps itself (`FF50` write). The number of SNES cycles
   elapsed during the SGB BIOS handshake before the GB starts running differs
   from Mesen's by a sizable fraction of a frame.

2. **Per-opcode SNES↔GB sync** — `S9xSGBSyncToSnesCycle` is called per SNES
   instruction with the cycle delta since the last sync. The cumulative GB
   advance over many SNES frames depends on this timing; Mesen runs each core
   on its own libco cothread (cycle-by-cycle interleave), which may produce
   slightly different timing accumulation than our per-opcode batching.

3. **GB clock ratio** — SGB2 mode uses `4194304/21477272 ≈ 0.1953` GB cycles
   per SNES master cycle. Cumulative rounding over thousands of frames can
   compound, but probably isn't the dominant factor for a fixed offset.

## Why the prior masking fix (`ed800de4`) broke DK/Bomberman

Commit `ed800de4` zeroed `Memory.VRAM[0x7000]` (BG1 tilemap, 2 KB) **and**
`Memory.VRAM[0xE000]` (BG3 char + tilemap, 8 KB) for SNES frames 650-670.

Two problems:
1. **BG1 tilemap zero is unrelated** — the actual visible-stripe content
   came from BG3, not BG1. Zeroing BG1 tilemap was a coincidental cleanup
   that happened to mask the symptom in Tetris (BG1 is sequential and shows
   captured GB pixels; zeroing it makes BG1 blank, which is what we wanted
   for Tetris but breaks the captured-tile display path that DK/Bomberman
   use for their non-zero GB content).
2. **`$E000-$FFFF` zero is too wide** — it includes BG3 tilemap (`$F800+`)
   and BG3 char (`$E000+`). BG3 char is what actually corrupts; BG3 tilemap
   in Mesen is non-zero and should not be zeroed.

## Mask approach without regression (proposed)

Surgical zeroing — target **only** the WRAM source buffer (`$7E:6800`)
at the corruption-window moment, so subsequent IRQ DMAs copy zeros into BG3
char and the layer goes blank (matching Mesen). Avoids touching BG1 or BG3
tilemap layers. DK/Bomberman should be unaffected if their borders go
through BG2 (custom-border PCT_TRN path).

Trigger: when `$7E:0284 == $6800` (we are in the unswapped state) AND the
buffer at `$7E:6800` matches the BIOS-staged sequential pattern
(`byte[0]==0x01 && byte[1]==0x10 && byte[2]==0x02 && byte[3]==0x10 …`).
Zero the buffer once. The IRQ DMA on the next VBlank copies zeros into BG3
char, BG3 layer renders blank, stripe artifact gone.

## Long-term fix — libco cothreads

Branch `sbg_fixes`, commit `e0822d59` (`sgb: libco cothreads + Mesen2 4-bank
ICD2 protocol fixes BIOS-mode artifacts`) replaces the per-opcode SNES↔GB
sync with libco cothreads (independent stacks, cycle-exact interleave).
This is what Mesen does internally, and is the right architectural fix for
the phase-drift class of bugs. Re-introducing it on this branch would
require rebasing the libco changes against current state.

## Useful references

- `sgb/sgb.cpp:1199-1207` — our `$6000` read implementation (returns
  `(sgb_row_latched & 0xF8) | (sgb_bank_latched & 0x03)`)
- `sgb/sgb.cpp:1377-1389` — `OnPpuHBlank` advances `sgb_row` once per
  visible GB scanline
- `sgb/sgb.cpp:1539` — `CaptureScanline` latches `sgb_row` → `sgb_row_latched`
  on each scanline, exposed at `$6000`
- `sgb/sgb.cpp:2059-2099` — `S9xSGBTickSnes` / SGB2 cycle ratio
- `sgb/sgb.cpp:945` — `boot_handoff_captured` set when GB boot ROM unmaps
- `sgb/sgb.cpp:2102-2129` — `S9xSGBSyncToSnesCycle` per-opcode sync
- `win32/Roms/SGB2.sfc.lst:11744` — `sub_80B9BE` (band processor)
- `win32/Roms/SGB2.sfc.lst:11876-11893` — `sub_80BA84` (the swap routine)
- `win32/Roms/SGB2.sfc.lst:11605-11620` — `sub_80B901` (initializer that
  seeds `$7E:0282=$5000` and `$7E:0284=$6800`)
- `win32/Roms/SGB2.sfc.lst:10940` — `Native_mode_IRQ` at `$80:B50A`

## Mesen export procedure (for future debugging)

In Mesen 2.1.1 with the SGB game running:
1. **Debug → Memory Tools → Memory Viewer**
2. Switch the dropdown at the top to the desired memory region
   (`SNES Memory`, `Snes Vram`, `Spc Ram`, `Snes Cg Ram`, `Snes Sprite Ram`,
   `Gameboy Memory`, `Gameboy Vram`, `Gameboy Oam`, etc.)
3. **File → Save Memory Dump** to write a raw `.dmp` file
4. For trace logging: **Debug → Trace Logger**, set the
   "Condition:" field (e.g., `pc == 0xb9be || pc == 0xb9c5`),
   click "Log to file…", run game, save log
