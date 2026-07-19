# Super Famicom Box (SFC-Box) emulation

Implements snes9xgit/snes9x#1046. The Super Famicom Box (Nintendo/HAL, 1993)
is a hotel pay-per-play unit: a SNES plus a supervisor board built around a
Hitachi HD64180 (Z180-compatible) CPU running a 64K BIOS ("KROM 1"). The
KROM owns game selection, session timing, coin/keyswitch input, an RTC, an
on-screen-display chip, and — critically — two mapping registers that decide
which cartridge ROM the SNES sees. Previously emulated only by no$sns and a
skeleton MAME driver (`sfcbox.cpp`).

References:
- fullsnes "SNES Hotel Boxes and Arcade Machines" (problemkaputt.de/fullsnes.htm)
  — authoritative register-level docs; everything below derives from it.
- nesdev thread https://forums.nesdev.org/viewtopic.php?t=9597 (dumping and
  hardware research: nocash, Markfrizb, gorgyrip, TechNVGM).
- MAME `src/mame/nintendo/sfcbox.cpp` (romset structure).
- BIOS files (no$sns naming, LuigiBlood's preservation upload):
  https://archive.org/details/super-famicom-box-bios
  `KROM1.BIN` (65536 bytes) + `MB90082.BIN` (9216 bytes, OSD font).
- Cart images: https://archive.org/details/super-famicom-box-no-sns
  (merged GROM+ROM images per the no$sns/fullsnes convention).

## Hardware summary

Console: stock SNES A/V chain + supervisor daughterboard:
- HD64180 @ PHI = 4.608 MHz (9.216 MHz / 2). Traps ALL undocumented opcodes
  (incl. SLL, IXH/IXL access). 64K KROM EPROM, 32K battery-backed work RAM,
  Seiko S-3520CF serial RTC, Fujitsu MB90082 OSD chip (video overlay, fed via
  Z180 CSI/O serial), front-panel keyswitch + GAME/TV + RESET buttons,
  external watchdog reloaded by pulsing port [81h].W bit6.
- Two cartridge slots for special multicarts. Slot 0 must hold PSS-61
  (attraction menu "ATROM" + Mario Kart + Mario Collection + Star Fox + DSP1
  + GSU + 128K SRAM). Slot 1 optionally PSS-62/63/64 (2 games each, no SRAM
  of their own — they borrow PSS-61's).
- Every cart has a 32K "GROM" EPROM: directory of the games (titles, mapping
  info, demo joypad data, shift-JIS instructions) plus ~256 bytes of HD64180
  code the KROM calls to program the mapping registers.

### HD64180 memory map (physical, 19-bit via Z180 MMU)

    00000-0FFFF  KROM (64K)
    20000-27FFF  WRAM 32K (200000-203FFF: save area, write-protected
                 unless [A0h].W bit7 set; battery-backed -> persisted)
    40000-47FFF  GROM slot 0
    60000-67FFF  GROM slot 1

I/O (8-bit decode, mirrored):

    00-3F  Z180 on-chip (ASCI0/1, CSI/O, PRT timers, MMU, INT)
    80.R   keyswitch/buttons  bit0-5 switch pins (0=selected), bit6 TV/GAME,
           bit7 RESET button (0=pressed)
    80.W   SNES transfer: bit0 STAT->WRIO2, bit1 CLOCK->WRIO5, bit2 DATA->WRIO1
    81.R   bit0 /INT0 coin, bit1 ACK<-WRIO3, bit2 DATA<-WRIO4, bit3 boot flag,
           bit6 /INT1 joypad, bit7 vblank-ish toggle (100-200 Hz)
    81.W   bit0 SNES /RESET (0=hold reset), bit3 INT1 ack, bit6 watchdog
           reload (pulse), bit7 OSD chip-select for CSI/O
    83-87  joypad sniff/inject (manual/auto forwarding, swap)
    A0.R   bit0 RTC data-in
    A0.W   bit0 RTC /CS, bit1 direction (1=read), bit2 data, bit3 /CLK,
           bit7 unlock save area of WRAM
    C0.W   SNES mapping reg 0: bit0-1 ROM socket (0=ROM5/menu, 1=ROM1/7/12,
           2=ROM3/9, 3=IC20/GSU), bit2 ROM slot, bit3 SRAM enable, bit4 SRAM
           slot, bit5 DSP enable, bit6 DSP slot, bit7 LoROM/HiROM
    C1.W   SNES mapping reg 1: bit0-1 map mode (1=GSU, 2=LoROM, 3=HiROM),
           bit2-3 SRAM base (32K units), bit4 GSU slot, bit6-7 SRAM size
           (0=2K, 1=8K, 3=32K)

Verified against the real KROM: at power-on it enables manual joypad
forwarding ([83h]=0Eh) and parks FFh in all four latches, runs its RAM/GROM
self-tests (including a sweep of all four SRAM banks via [C1h]), walks every
populated socket by remapping it live while the SNES executes a stub from
WRAM, then programs the menu mapping and pulses the SNES reset line.

### SNES <-> KROM link (WRIO $4201 / RDIO $4213)

Bit-banged serial through the SNES programmable I/O port. From the SNES's
view (default WRIO output E6h):

    bit0 out  ready-ish (KROM sees it on [81h].R bit3?)
    bit1 in   data from KROM        ([80h].W bit2)
    bit2 in   status/ready from KROM ([80h].W bit0)
    bit3 out  clock/ack to KROM     ([81h].R bit1)
    bit4 out  data to KROM          ([81h].R bit2)
    bit5 in   clock from KROM       ([80h].W bit1)
    bit6/7    normal joypad IOBit lines

The menu program (ATROM) receives 32-bit packets (1 command + 3 parameter
bytes); bulk transfers are pushed through the joypad shift registers
([84h-87h] on the KROM side) as a "hardware accelerated" path.

### Cartridge image format (no$sns / fullsnes convention)

One file per cart: `GROM + ROM0 (+ROM1 (+ROM2...)) (+DSP1 program ROM)`,
all chip-order per the GROM directory. Two carts may be concatenated in one
file (slot0 image then slot1 image). GROM layout:

    [0000] number of ROMs (1..8)     [0001] GROM size (1<<n KB, usually 5=32K)
    [0004] chipset (bit0 SRAM, bit1 DSP, bit2 GSU)
    [0006] offset of HD64180 mapping-helper code
    [0008] offset of ROM directory: NumROMs words (info block index, *1000h)
           then NumROMs bytes (physical socket id 0..3)
    [7FFC] 16-bit sum of [0000-7FFB], [7FFE] its complement  <- detection

Per-ROM info block at [n*1000h]: +0 word offset P0 of title/config
(22-byte ASCII title; +16h mapmode 0=LoROM 1=HiROM 2=GSU; +17h SRAM size
1<<n KB; +18h DSP1 flag; +19h ROM size in 128K units; +1Ch SRAM base;
+26h game flag), P1/P2/P3 title bitmap, P4 instructions, P5 demo joypad data.

### S-3520CF RTC serial protocol

4-bit nibbles LSB-first. /CS low starts a transfer; writes send index nibble
+ data nibble; reads send index then clock out garbage,garbage,data (data is
the 4th nibble). Registers 0-C: BCD time/date (sec lo/hi, min lo/hi, hour
lo/hi + PM, weekday, day lo/hi, month lo/hi, year lo/hi); D control
(TPS/30ADJ/CNTR/24H), E status (STA/LOST), F mode (0=RTC, 2/3=SRAM pages,
2x15 nibbles battery RAM).

### MB90082 OSD

24x12 character overlay, 12x18-pixel cells (OSD dot clock ~1.5x SNES: 12 OSD
dots span ~8 SNES dots). Commands arrive as 2-byte pairs over CSI/O
(cmd 0 set VRAM addr / fill mode, 1 select color, 2 write char, 4/5 screen
control incl. display enable, 6 line control incl. per-line zoom, 7/8 screen
offsets, A under-color). Font: `MB90082.BIN` = 256 chars x 18 rows x 2
bytes; byte 0 holds dots 0-7 MSB-first, byte 1 holds dots 8-11 in its LOW
nibble. Charset is JIS-ish katakana + ASCII + symbols; FFh is the
transparent space.

## Boot flow (what the KROM does)

Power-on: Z180 boots from KROM with SNES held in reset. Self-checks, reads
GROMs (mapped at 40000h/60000h), talks to the RTC, draws boot progress via
OSD, programs C0h/C1h (via the GROM's embedded helper), releases SNES reset
-> SNES boots the ATROM attraction menu. Menu and KROM then converse over
WRIO/RDIO; picking a game makes the KROM reprogram the mapping and pulse the
SNES reset line. KROM remaps DURING SNES execution (SNES executes garbage,
BRK loops — harmless; the reset that follows restores order). Emulation must
therefore never abort on wild execution while the box is active.

## Implementation map (this repo)

    hd64180.{h,cpp}   Z180 core: full documented Z80 set + Z180 extensions
                      (MLT/TST/TSTIO/IN0/OUT0/OTIM(R)/OTDM(R)/SLP), TRAP on
                      undocumented opcodes, MMU, PRT timers, CSI/O, ASCI
                      stubs, INT0/1/2 + internal vectors, IM0/1/2.
    sfcbox.{h,cpp}    board glue: custom I/O ports, RTC S-3520CF, OSD chip +
                      overlay renderer, WRIO/RDIO bridge, joypad forward,
                      mapping regs -> Memory.Map rebuild, SNES reset line,
                      KROM WRAM persistence (.box file next to the save).
    memmap.cpp        LoadSFCBox(): GROM checksum sniffing (single or
                      concatenated two-cart images), socket table, KROM1.BIN
                      + MB90082.BIN from the BIOS dir (falling back to the
                      ROM's directory), InitROM/DSP1 arming.
    cpuexec.cpp       per-scanline Z180 execution (HC_HCOUNTER_MAX), SNES
                      reset-line handling (hold = skip SNES ops, rising edge
                      = S9xSoftReset while preserving box state).
    ppu.cpp           $4201 writes forwarded to the box; $4213 reads composed
                      from the Z180's transfer outputs.

Front-panel defaults: keyswitch parked at the "ON" play position
(`SFCBox.Keyswitch` = bit index pulled low on [80h].R); the KROM boots with
factory-fresh battery RAM straight into the attraction menu and launches
games without coins. `S9xSFCBoxInsertCoin()` closes the coin switch for the
documented 44-80ms if a pay-mode configuration ever needs it.

## Usage

Put `KROM1.BIN` (and `MB90082.BIN` for the boot/operator overlay) in the
BIOS directory or next to the cart image, then load a merged image
(`pss61_merged.sfc`, `pss61+63_merged.sfc`, ...) like any ROM. Detection is
content-based: a valid GROM checksum at 7FFCh diverts the loader. Nothing
else to configure.

    snes_harness pss61_merged.sfc frames=3600 "fb=2400:menu.ppm"
    SFCBOX_TRACE=1 ...   # logs mapping/reset/serial traffic + Z180 PC samples

## Verified

Booted with the real KROM 1.00: OSD boot screen + progress bar, full
self-test, attraction menu with title bitmaps for both carts, game launch
via the per-game menu — Super Mario Kart (HiROM + DSP-1, slot 0) and Super
Donkey Kong (4MB HiROM, slot 1) reach gameplay/attract. Boot needs three
pieces beyond the obvious ports, all discovered against the real BIOS:

- INT1 must fire when the SNES strobes the joypads: the KROM's ISR streams
  menu bulk data (title bitmaps etc.) through [84h-87h] one word per SNES
  read. Without it the KROM reboots the SNES forever and starves the
  watchdog.
- The [84h-87h] latches must actually replace the SNES-side pads whenever
  [83h].W bit3 manual mode is on (the KROM enables it at power-on), with
  wire polarity inverted vs $4218.
- Emulation must keep running while the SNES executes garbage mid-remap —
  the KROM's reset pulse restores order (matches fullsnes's crash notes).

## Known gaps

- Star Fox: the GSU mapping mode falls back to plain LoROM (see the
  follow-up note in the tracker) — selecting it will misbehave.
- Savestates don't capture the supervisor board (Z180/RTC/OSD) yet.
- OSD sprite commands (3/C/D/B) are accepted but not rendered; zoom is
  rendered, blink/shading are not.
- RTC time-set writes bump digit counters per-field (no cross-field carry);
  the clock itself is seeded from the host at power-on.
