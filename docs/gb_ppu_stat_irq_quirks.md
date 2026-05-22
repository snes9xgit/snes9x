# GB PPU STAT IRQ hardware quirks

Three real-hardware behaviors that affect the STAT (LCDSTAT, IRQ vector
$0048) interrupt on DMG. All three are now modeled in `sgb/gb_ppu.cpp`.
The third one was load-bearing for Zerd no Densetsu (J) — without it the
game's intro animation crashes ~1 second after boot into an RST 38 loop.

## Background: the STAT IRQ line

STAT is an edge-triggered interrupt. The hardware computes a single
"STAT line" as the OR of four source conditions, each gated by an enable
bit in the STAT register (`$FF41`):

| STAT bit | Source            | Condition                |
| -------- | ----------------- | ------------------------ |
| 3        | Mode 0 (HBlank)   | PPU is in HBlank         |
| 4        | Mode 1 (VBlank)   | PPU is in VBlank         |
| 5        | Mode 2 (OAM scan) | PPU is in OAM scan       |
| 6        | LYC match         | `LY == LYC`              |

`IF.STAT` (bit 1 of `$FF0F`) is set on the rising edge of this combined
line. Once set, `IF.STAT` stays set until the IRQ is dispatched or the
game clears it.

In our code the line is computed by `RecomputeStatLine` (gb_ppu.cpp).
It runs on PPU mode transitions and on CPU writes to `$FF41` / `$FF44` /
`$FF45`.

---

## 1. LCD-off forces the STAT line low

When the LCD is disabled (`LCDC.bit7 = 0`), real DMG holds the STAT IRQ
line low regardless of the STAT enable bits or any "remembered" mode/
LY/LYC values. `PpuStep` already parks the PPU in mode 0 with `LY=0`
while the LCD is off, but the source-condition check in
`RecomputeStatLine` is also called from `PpuWriteReg` for `$FF41` /
`$FF44` / `$FF45`. Without an LCD-on guard, those writes would compute
"mode 0 active + bit 3 enabled" or "LY=LYC=0 + bit 6 enabled" and
spuriously raise STAT IRQ while the LCD is disabled — something real
hardware never does.

Fix: gate the four source checks inside `RecomputeStatLine` with
`LCDC.bit7`. When the LCD is off, `line_high` stays false and no IRQ
rises.

Exposed by: Zerd no Densetsu's bank-1 init, which writes `STAT.bit6`
inside a DI region with the LCD off (turned off by `$02CF` earlier in
the same init routine).

---

## 2. LY=153 hardware quirk

Per Pan Docs §STAT.lyc-glitch:

> On real DMG, LY only reads as 153 for the first 4 dots of scanline
> 153. For the remaining 452 dots of scanline 153 the PPU keeps mode = 1
> (VBlank) but LY visibly becomes 0.

So a game with `LYC = 0` and bit 6 enabled sees the LYC=match rising
edge at scanline 153 cycle 4 — mid-VBlank — not at scanline 0 cycle 0
as a naive scanline-end-only counter would imply.

A handler scheduled this way can race with code that runs immediately
after VBlank: the IRQ fires *during* VBlank and the game's handler runs
before the next frame's mode 2 begins.

Fix: in `PpuStep`'s VBlank case, when `p.ly == 153 && p.mode_clock == 4`,
flip `p.ly` to 0 and call `RecomputeStatLine` (via `transitioned`). The
scanline-153 → scanline-0 mode transition then happens 452 dots later
as usual; the LY-counter update is suppressed because LY is already 0.

---

## 3. DMG STAT-write spurious IRQ

Per Pan Docs §STAT.spurious-stat-interrupts:

> Because of how the LCD STAT IRQ line is wired on DMG, when STAT is
> written the existing logic momentarily applies the rising-edge
> detector to ALL four source conditions, regardless of their actual
> values. The "edge" the detector sees is whether any current condition
> matches — not whether the bit being written enables it.

In practice: writing `$FF41` while the STAT line is low raises
`IF.STAT` immediately if **any** of (mode 0, mode 1, mode 2, LYC match)
is currently true. The value being written doesn't matter for the
spurious raise — only for what stays "actually enabled" afterwards.

Fix: in `PpuWriteReg` case `$FF41`, before performing the write:

```cpp
if (!p.stat_line_high && (p.lcdc & 0x80))
{
    const bool any_source_active =
        p.mode == PpuMode::HBlank ||
        p.mode == PpuMode::VBlank ||
        p.mode == PpuMode::OamScan ||
        p.ly == p.lyc;
    if (any_source_active)
        mem.if_ |= IRQ_LCDSTAT;
}
// then write STAT and call RecomputeStatLine as before
```

### Why this matters for Zerd no Densetsu

Zerd's bank-1 init (`$6257`) does the following sequence near the end:

```
$62C0: DI                  ; disable IRQs
$62C1-$62CE: write JP $64D6 at $C2CC ; install STAT trampoline
$62D7-$62DB: SET 6, A;
             LDH ($41), A  ; enable LYC IRQ source
$62DD-$62E1: enable IE.STAT
$62E3-$62E5: $FFA7 = $B0
$62E7: EI
$62E8: CALL $021E           ; enter the wait loop
```

The wait loop's `$021E` calls `$01EA` (1-frame delay routine), which
internally switches to bank 7 (`CALL $03DF`), calls the bank-7 sound
update at `$4023` (which `DI`s), then `CALL $03F7` (restore bank 1),
then `HALT`.

**Without the spurious-write quirk:** `IF.STAT` is not raised at `$62DB`.
The next rising-edge opportunity is the next LYC=0 match. With the LY=153
quirk that's about 3000 cycles later, by which time the CPU is deep
inside `$4023`'s `DI` region with bank 7 mapped. `$4023` eventually
re-enables IME via its `EI`+`RET`; the IRQ dispatches at the next CPU
step (which is `$01F8`, the instruction after the CALL site in `$01EA`).
**Bank 7 is still mapped because `$03F7` has not run yet.** The
trampoline at `$C2CC` reads `JP $64D6`, but `$64D6` in bank 7 is `7B 20
F0 …` (`LD A, E`; `JR NZ, -16`; `RST 38h`) — garbage. The CPU executes
those bytes and falls into the `RST 38` trap at `$0038` (where the cart
ROM has `AA FF` = `XOR D; RST 38h`), looping forever.

**With the spurious-write quirk:** the `LDH ($41), A` at `$62DB` runs
with mode = VBlank (LY ≈ 147). The quirk sets `IF.STAT` *immediately*.
After the `EI` delay completes (one instruction past `$62E7`, i.e. after
`$62E8 CALL` finishes), the IRQ dispatches at the first instruction of
`$021E` — **bank 1 is still mapped**, the trampoline reads the real
handler, LYC is updated to `$28`, and subsequent IRQs fire mid-screen
(LY=$28 / LY=$48) where they don't interact with the bank-7 sound CALL.

### Caveat: SGB1 inherits DMG behavior

The STAT-write quirk is a DMG-specific bug. SGB1 and SGB2 are DMG-based
and exhibit it; CGB does not (its STAT line is wired differently). Our
SGB emulation hits this path because Zerd is a plain DMG cart
(`CGB flag = $00`, `SGB flag = $00`) running through the SGB BIOS.

---

## References

- Pan Docs: <https://gbdev.io/pandocs/STAT.html>
- Pan Docs §LCD Status Register Bug: <https://gbdev.io/pandocs/STAT.html#spurious-stat-interrupts>
- AntonioND's GB-CTR (cycle-accurate STAT IRQ docs):
  <https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf>

## Test ROMs to spot-check after changes

- Pokemon Yellow — Pikachu voice routine disables the LCD with STAT bit
  3 (HBlank) still enabled; relies on the LCD-off guard.
- Altered Space — title-screen wait-for-LY=$90 loop; uses the existing
  24-dot VBlank IRQ delay, orthogonal to these three fixes.
- Alleyway — D-pad serial latch path; orthogonal but check for
  regressions.
- Zerd no Densetsu (J) — full crash repro; intro must reach title
  screen.
- Games with LYC raster effects (Donkey Kong, Killer Instinct 95) —
  exercise the LYC IRQ path with non-zero LYC.
