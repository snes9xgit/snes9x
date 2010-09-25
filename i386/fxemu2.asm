;Copyright (C) 1997-2001 ZSNES Team ( zsknight@zsnes.com / _demo_@zsnes.com )
;
;This program is free software; you can redistribute it and/or
;modify it under the terms of the GNU General Public License
;as published by the Free Software Foundation; either
;version 2 of the License, or (at your option) any later
;version.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program; if not, write to the Free Software
;Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

%include "macros.mac"

EXTSYM FxTable,FxTableb,FxTablec,FxTabled,SfxMemTable,flagnz,fxbit01,fxbit23
EXTSYM fxxand,sfx128lineloc,sfx160lineloc,sfx192lineloc,sfxobjlineloc
EXTSYM sfxramdata,fxbit45,fxbit67,SFXProc,ChangeOps
EXTSYM PLOTJmpa,PLOTJmpb

NEWSYM FxEmu2AsmStart

%include "fxemu2.mac"






; * Optimise PLOT, COLOR!
SECTION .text
NEWSYM FlushCache
   ; Copy 512 bytes from pb:eax to SfxCACHERAM
   ret

SECTION .bss
NEWSYM tempsfx, resb 3

SECTION .data ;ALIGN=32

; FxChip emulation by _Demo_
; Optimised by zsKnight
; based on fxemu by lestat

NEWSYM SfxR0,    dd 0     ; default source/destination register
NEWSYM SfxR1,    dd 0     ; pixel plot X position register
NEWSYM SfxR2,    dd 0     ; pixel plot Y position register
NEWSYM SfxR3,    dd 0     ;
NEWSYM SfxR4,    dd 0     ; lower 16 bit result of lmult
NEWSYM SfxR5,    dd 0     ;
NEWSYM SfxR6,    dd 0     ; multiplier for fmult and lmult
NEWSYM SfxR7,    dd 0     ; fixed point texel X position for merge
NEWSYM SfxR8,    dd 0     ; fixed point texel Y position for merge
NEWSYM SfxR9,    dd 0     ;
NEWSYM SfxR10,   dd 0     ;
NEWSYM SfxR11,   dd 0     ; return address set by link
NEWSYM SfxR12,   dd 0     ; loop counter
NEWSYM SfxR13,   dd 0     ; loop point address
NEWSYM SfxR14,   dd 0     ; rom address for getb, getbh, getbl, getbs
NEWSYM SfxR15,   dd 0     ; program counter

NEWSYM SfxSFR,   dd 0     ; status flag register (16bit)
;SFR status flag register bits:
; 0   -
; 1   Z   Zero flag
; 2   CY  Carry flag
; 3   S   Sign flag
; 4   OV  Overflow flag
; 5   G   Go flag (set to 1 when the GSU is running)
; 6   R   Set to 1 when reading ROM using R14 address
; 7   -
; 8   ALT1   Mode set-up flag for the next instruction
; 9   ALT2   Mode set-up flag for the next instruction
;10   IL  Immediate lower 8-bit flag
;11   IH  Immediate higher 8-bit flag
;12   B   Set to 1 when the WITH instruction is executed
;13   -
;14   -
;15   IRQ Set to 1 when GSU caused an interrupt
;                Set to 0 when read by 658c16

NEWSYM SfxBRAMR,    dd 0  ; backup ram read only on/off (8bit)
NEWSYM SfxPBR,      dd 0  ; program bank register (8bit)
NEWSYM SfxROMBR,    dd 0  ; rom bank register (8bit)
NEWSYM SfxCFGR,     dd 0  ; control flags register (8bit)
NEWSYM SfxSCBR,     dd 0  ; screen bank register (8bit)
NEWSYM SfxCLSR,     dd 0  ; clock speed register (8bit)
NEWSYM SfxSCMR,     dd 0  ; screen mode register (8bit)
NEWSYM SfxVCR,      dd 0  ; version code register (8bit)
NEWSYM SfxRAMBR,    dd 0  ; ram bank register (8bit)
NEWSYM SfxCBR,      dd 0  ; cache base register (16bit)

NEWSYM SfxCOLR,     dd 0  ; Internal color register
NEWSYM SfxPOR,      dd 0  ; Plot option register

NEWSYM SfxCacheFlags,  dd 0  ; Saying what parts of the cache was written to
NEWSYM SfxLastRamAdr,  dd 0  ; Last RAM address accessed
NEWSYM SfxDREG,        dd 0  ; Current destination register index
NEWSYM SfxSREG,        dd 0  ; Current source register index
NEWSYM SfxRomBuffer,   dd 0  ; Current byte read by R14
NEWSYM SfxPIPE,        dd 0  ; Instructionset pipe
NEWSYM SfxPipeAdr,     dd 0  ; The address of where the pipe was read from

NEWSYM SfxnRamBanks,   dd 4  ; Number of 64kb-banks in FxRam (Don't confuse it with SNES-Ram!!!)
NEWSYM SfxnRomBanks,   dd 0  ; Number of 32kb-banks in Cart-ROM

NEWSYM SfxvScreenHeight, dd 0 ; 128, 160 or 192
NEWSYM SfxvScreenSize, dd 0

NEWSYM SfxCacheActive, dd 0  ; Cache Active

NEWSYM SfxCarry,       dd 0  ; Carry flag
NEWSYM SfxSignZero,    dd 1  ; Sign and Zero flag
NEWSYM SfxB,           dd 0  ; B flag  (1 when with instruction executed)
NEWSYM SfxOverflow,    dd 0  ; Overflow flag

NEWSYM SfxCACHERAM, times 512 db 0    ; 512 bytes of GSU cache memory
num2writesfxreg  equ $-SfxR0
; pharos equ hack *sigh*
NEWSYM PHnum2writesfxreg, dd num2writesfxreg

NEWSYM SfxCPB,         dd 0
NEWSYM SfxCROM,        dd 0
NEWSYM SfxRAMMem,      dd 0
NEWSYM withr15sk,      dd 0
NEWSYM sfxclineloc,       dd 0
NEWSYM SCBRrel, dd 0
NEWSYM fxbit01pcal, dd 0
NEWSYM fxbit23pcal, dd 0
NEWSYM fxbit45pcal, dd 0
NEWSYM fxbit67pcal, dd 0

;SfxRAM times 256*1024 db 0

; If we need this later...

SECTION .text
NEWSYM FxOp00     ; STOP   stop GSU execution (and maybe generate an IRQ)     ; Verified.
   FETCHPIPE
   mov [SfxPIPE],cl
   and dword [SfxSFR],0FFFFh-32     ; Clear Go flag (set to 1 when the GSU is running)
   test dword [SfxCFGR],080h        ; Check if the interrupt generation is on
   jnz .NoIRQ
   or dword [SfxSFR],08000h         ; Set IRQ Flag
.NoIRQ
   CLRFLAGS
   inc ebp
   mov eax,[NumberOfOpcodes]
   add eax,0F0000000h
   add [ChangeOps],eax
   mov dword [NumberOfOpcodes],1
   mov dword[SFXProc],0
   xor cl,cl
   ret

NEWSYM FxOp01      ; NOP    no operation       ; Verified.
   FETCHPIPE
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret

NEWSYM FxOp02      ; CACHE  reintialize GSU cache
   mov eax,ebp
   FETCHPIPE
   sub eax,[SfxCPB]
   and eax,0FFF0h
   cmp dword [SfxCBR],eax
   je .SkipUpdate
   cmp byte [SfxCacheActive],1
   je .SkipUpdate
   mov dword [SfxCBR],eax
   mov dword [SfxCacheActive],1
   call FlushCache
.SkipUpdate
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret

NEWSYM FxOp03      ; LSR    logic shift right  ; Verified.
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   mov [SfxCarry],al
   and byte[SfxCarry],1
   shr ax,1                      ; logic shift right
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp04      ; ROL    rotate left (RCL?) ; V
   shr byte[SfxCarry],1
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   rcl ax,1
   rcl byte[SfxCarry],1
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp05      ; BRA    branch always      ; Verified.
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   inc ebp
   add ebp,eax
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp06      ; BGE    branch on greater or equals        ; Verified.
   movsx eax,byte[ebp]
   mov ebx,[SfxSignZero]
   shr ebx,15
   inc ebp
   xor bl,[SfxOverflow]
   mov cl,[ebp]
   test bl,01h
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp07      ; BLT    branch on lesss than       ; Verified.
   movsx eax,byte[ebp]
   mov ebx,[SfxSignZero]
   shr ebx,15
   inc ebp
   xor bl,[SfxOverflow]
   mov cl,[ebp]
   test bl,01h
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp08      ; BNE    branch on not equal        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp09      ; BEQ    branch on equal (z=1)      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp0A      ; BPL    branch on plus     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp0B      ; BMI    branch on minus    ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp0C      ; BCC    branch on carry clear      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp0D      ; BCS    branch on carry set        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp0E      ; BVC    branch on overflow clear   ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp0F      ; BVS    branch on overflow set     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTable+ecx*4]
   ret

NEWSYM FxOp10      ; TO RN  set register n as destination register
   TORN 0
NEWSYM FxOp11      ; TO RN  set register n as destination register
   TORN 1
NEWSYM FxOp12      ; TO RN  set register n as destination register
   TORN 2
NEWSYM FxOp13      ; TO RN  set register n as destination register
   TORN 3
NEWSYM FxOp14      ; TO RN  set register n as destination register
   TORN 4
NEWSYM FxOp15      ; TO RN  set register n as destination register
   TORN 5
NEWSYM FxOp16      ; TO RN  set register n as destination register
   TORN 6
NEWSYM FxOp17      ; TO RN  set register n as destination register
   TORN 7
NEWSYM FxOp18      ; TO RN  set register n as destination register
   TORN 8
NEWSYM FxOp19      ; TO RN  set register n as destination register
   TORN 9
NEWSYM FxOp1A      ; TO RN  set register n as destination register
   TORN 10
NEWSYM FxOp1B      ; TO RN  set register n as destination register
   TORN 11
NEWSYM FxOp1C      ; TO RN  set register n as destination register
   TORN 12
NEWSYM FxOp1D      ; TO RN  set register n as destination register
   TORN 13
NEWSYM FxOp1E      ; TO RN  set register n as destination register
   FETCHPIPE
   mov edi,SfxR0+14*4
   inc ebp
   call [FxTable+ecx*4]
   mov edi,SfxR0
   UpdateR14
   ret
NEWSYM FxOp1F      ; TO RN  set register n as destination register
   FETCHPIPE
   mov edi,SfxR0+15*4
   inc ebp
   call [FxTable+ecx*4]
   mov ebp,[SfxCPB]
   add ebp,[SfxR15]
   mov edi,SfxR0
   ret

NEWSYM FxOp20      ; WITH  set register n as source and destination register
   WITH 0
NEWSYM FxOp21      ; WITH  set register n as source and destination register
   WITH 1
NEWSYM FxOp22      ; WITH  set register n as source and destination register
   WITH 2
NEWSYM FxOp23      ; WITH  set register n as source and destination register
   WITH 3
NEWSYM FxOp24      ; WITH  set register n as source and destination register
   WITH 4
NEWSYM FxOp25      ; WITH  set register n as source and destination register
   WITH 5
NEWSYM FxOp26      ; WITH  set register n as source and destination register
   WITH 6
NEWSYM FxOp27      ; WITH  set register n as source and destination register
   WITH 7
NEWSYM FxOp28      ; WITH  set register n as source and destination register
   WITH 8
NEWSYM FxOp29      ; WITH  set register n as source and destination register
   WITH 9
NEWSYM FxOp2A      ; WITH  set register n as source and destination register
   WITH 10
NEWSYM FxOp2B      ; WITH  set register n as source and destination register
   WITH 11
NEWSYM FxOp2C      ; WITH  set register n as source and destination register
   WITH 12
NEWSYM FxOp2D      ; WITH  set register n as source and destination register
   WITH 13
NEWSYM FxOp2E      ; WITH  set register n as source and destination register
   FETCHPIPE
   mov esi,SfxR0+14*4
   mov edi,SfxR0+14*4
   mov dword [SfxB],1
   inc ebp
   call [FxTablec+ecx*4]
   mov dword [SfxB],0         ; Clear B Flag
   mov esi,SfxR0
   mov edi,SfxR0
   UpdateR14
   ret
NEWSYM FxOp2F      ; WITH  set register n as source and destination register
   FETCHPIPE
   mov esi,SfxR0+15*4
   mov edi,SfxR0+15*4
   mov dword [SfxB],1
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov dword[withr15sk],0
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   cmp dword[withr15sk],1
   je .skip
   mov ebp,[SfxCPB]
   add ebp,[SfxR15]
.skip
   mov dword [SfxB],0         ; Clear B Flag
   mov esi,SfxR0
   mov edi,SfxR0
   ret

NEWSYM FxOp30      ; STW RN store word
   STWRN 0
NEWSYM FxOp31      ; STW RN store word
   STWRN 1
NEWSYM FxOp32      ; STW RN store word
   STWRN 2
NEWSYM FxOp33      ; STW RN store word
   STWRN 3
NEWSYM FxOp34      ; STW RN store word
   STWRN 4
NEWSYM FxOp35      ; STW RN store word
   STWRN 5
NEWSYM FxOp36      ; STW RN store word
   STWRN 6
NEWSYM FxOp37      ; STW RN store word
   STWRN 7
NEWSYM FxOp38      ; STW RN store word
   STWRN 8
NEWSYM FxOp39      ; STW RN store word
   STWRN 9
NEWSYM FxOp3A      ; STW RN store word
   STWRN 10
NEWSYM FxOp3B      ; STW RN store word
   STWRN 11

NEWSYM FxOp30A1    ; STB RN store byte
   STBRN 0
NEWSYM FxOp31A1    ; STB RN store byte
   STBRN 1
NEWSYM FxOp32A1    ; STB RN store byte
   STBRN 2
NEWSYM FxOp33A1    ; STB RN store byte
   STBRN 3
NEWSYM FxOp34A1    ; STB RN store byte
   STBRN 4
NEWSYM FxOp35A1    ; STB RN store byte
   STBRN 5
NEWSYM FxOp36A1    ; STB RN store byte
   STBRN 6
NEWSYM FxOp37A1    ; STB RN store byte
   STBRN 7
NEWSYM FxOp38A1    ; STB RN store byte
   STBRN 8
NEWSYM FxOp39A1    ; STB RN store byte
   STBRN 9
NEWSYM FxOp3AA1    ; STB RN store byte
   STBRN 10
NEWSYM FxOp3BA1    ; STB RN store byte
   STBRN 11

NEWSYM FxOp3C      ; LOOP   decrement loop counter, and branch on not zero ; V
   dec word [SfxR12]       ; decrement loop counter
   FETCHPIPE
   mov eax,[SfxR12]
   mov [SfxSignZero],eax
   or eax,eax
   jz .NoBranch
   mov eax,dword [SfxR13]
   mov ebp,[SfxCPB]
   add ebp,eax
   CLRFLAGS
   ret
.NoBranch
   inc ebp
   CLRFLAGS
   ret

NEWSYM FxOp3D      ; ALT1   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,01h
   inc ebp
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOp3E      ; ALT2   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,02h
   inc ebp
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOp3F      ; ALT3   set alt3 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,03h
   inc ebp
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOp40      ; LDW RN load word from RAM
   LDWRN 0
NEWSYM FxOp41      ; LDW RN load word from RAM
   LDWRN 1
NEWSYM FxOp42      ; LDW RN load word from RAM
   LDWRN 2
NEWSYM FxOp43      ; LDW RN load word from RAM
   LDWRN 3
NEWSYM FxOp44      ; LDW RN load word from RAM
   LDWRN 4
NEWSYM FxOp45      ; LDW RN load word from RAM
   LDWRN 5
NEWSYM FxOp46      ; LDW RN load word from RAM
   LDWRN 6
NEWSYM FxOp47      ; LDW RN load word from RAM
   LDWRN 7
NEWSYM FxOp48      ; LDW RN load word from RAM
   LDWRN 8
NEWSYM FxOp49      ; LDW RN load word from RAM
   LDWRN 9
NEWSYM FxOp4A      ; LDW RN load word from RAM
   LDWRN 10
NEWSYM FxOp4B      ; LDW RN load word from RAM
   LDWRN 11

NEWSYM FxOp40A1    ; LDB RN load byte from RAM
   LDBRN 0
NEWSYM FxOp41A1    ; LDB RN load byte from RAM
   LDBRN 1
NEWSYM FxOp42A1    ; LDB RN load byte from RAM
   LDBRN 2
NEWSYM FxOp43A1    ; LDB RN load byte from RAM
   LDBRN 3
NEWSYM FxOp44A1    ; LDB RN load byte from RAM
   LDBRN 4
NEWSYM FxOp45A1    ; LDB RN load byte from RAM
   LDBRN 5
NEWSYM FxOp46A1    ; LDB RN load byte from RAM
   LDBRN 6
NEWSYM FxOp47A1    ; LDB RN load byte from RAM
   LDBRN 7
NEWSYM FxOp48A1    ; LDB RN load byte from RAM
   LDBRN 8
NEWSYM FxOp49A1    ; LDB RN load byte from RAM
   LDBRN 9
NEWSYM FxOp4AA1    ; LDB RN load byte from RAM
   LDBRN 10
NEWSYM FxOp4BA1    ; LDB RN load byte from RAM
   LDBRN 11

NEWSYM FxOp4C1284b       ; PLOT 4bit
   plotlines4b plotb
NEWSYM FxOp4C1284bz      ; PLOT 4bit, zero check
   plotlines4b plotbz
NEWSYM FxOp4C1284bd      ; PLOT 4bit, dither
   plotlines4b plotbd
NEWSYM FxOp4C1284bzd     ; PLOT 4bit, zero check + dither
   plotlines4b plotbzd

NEWSYM FxOp4C1282b       ; PLOT 2bit
   plotlines2b plotb
NEWSYM FxOp4C1282bz      ; PLOT 2bit, zero check
   plotlines2b plotbz
NEWSYM FxOp4C1282bd      ; PLOT 2bit, dither
   plotlines2b plotbd
NEWSYM FxOp4C1282bzd     ; PLOT 2bit, zero check + dither
   plotlines2b plotbzd

NEWSYM FxOp4C1288b       ; PLOT 8bit
   plotlines8b plotb
NEWSYM FxOp4C1288bz      ; PLOT 8bit, zero check
   plotlines8b plotbz
NEWSYM FxOp4C1288bd      ; PLOT 8bit, dither
   plotlines8b plotbd
NEWSYM FxOp4C1288bzd     ; PLOT 8bit, zero check + dither
   plotlines8b plotbzd

NEWSYM FxOp4C1288bl       ; PLOT 8bit
   plotlines8bl plotb
NEWSYM FxOp4C1288bzl      ; PLOT 8bit, zero check
   plotlines8bl plotbz
NEWSYM FxOp4C1288bdl      ; PLOT 8bit, dither
   plotlines8bl plotbd
NEWSYM FxOp4C1288bzdl     ; PLOT 8bit, zero check + dither
   plotlines8bl plotbzd

NEWSYM FxOp4C      ; PLOT   plot pixel with R1,R2 as x,y and the color register as the color
   jmp FxOp4C1284b
   FETCHPIPE
   inc ebp
   CLRFLAGS
   mov ebx,[SfxR2]
   mov bh,[SfxR1]
   mov eax,[sfxclineloc]
   mov ebx,[eax+ebx*4]
   cmp ebx,0FFFFFFFFh
   je near .nodraw
   xor eax,eax
   ; bits 5/2 : 00 = 128 pixels, 01 = 160 pixels, 10 = 192 pixels, 11 = obj
   ; bits 1/0 : 00 = 4 color, 01 = 16-color, 10 = not used, 11 = 256 color
   ; 192 pixels = 24 tiles, 160 pixels = 20 tiles, 128 pixels = 16 tiles
   ;              16+8(4/3)              16+4(4/2)              16(4/0)
   push ecx
   mov al,[SfxSCMR]
   and al,00000011b     ; 4 + 32
   cmp al,0
   je near .colors4
   cmp al,3
   je near .colors256

   shl ebx,5    ; x32 (16 colors)
   mov al,[SfxSCBR]
   shl eax,10   ; Get SFX address
   add eax,ebx
   add eax,[sfxramdata]
   mov ebx,[SfxR2]
   and ebx,07h
   shl ebx,1
   add eax,ebx
   mov cl,[SfxR1]
   and cl,07h
   xor cl,07h
   mov bl,1
   shl bl,cl
   mov bh,bl
   xor bh,0FFh
   pop ecx
   test byte[SfxPOR],01h
   jnz .nozerocheck_16
   test byte[SfxCOLR],0Fh
   jz .nodraw
.nozerocheck_16
   mov dl,[SfxCOLR]
   test byte[SfxPOR],02h
   jz .nodither4b
   mov dh,[SfxR1]
   xor dh,[SfxR2]
   test dh,01h
   jz .nodither4b
   shr dh,4
.nodither4b
   and byte[eax],bh
   and byte[eax+1],bh
   and byte[eax+16],bh
   and byte[eax+17],bh
   test dl,01h
   jz .nodraw_16
   or byte[eax],   bl
.nodraw_16
   test dl,02h
   jz .nodraw2_16
   or byte[eax+1], bl
.nodraw2_16
   test dl,04h
   jz .nodraw3_16
   or byte[eax+16],bl
.nodraw3_16
   test dl,08h
   jz .nodraw4_16
   or byte[eax+17],bl
.nodraw4_16
.nodraw
   inc word [SfxR1]
   ret

.colors4
   shl ebx,4    ; x16 (4 colors)
   mov al,[SfxSCBR]
   shl eax,10   ; Get SFX address
   add eax,ebx
   add eax,[sfxramdata]
   mov ebx,[SfxR2]
   and ebx,07h
   shl ebx,1
   add eax,ebx
   mov cl,[SfxR1]
   and cl,07h
   xor cl,07h
   mov bl,1
   shl bl,cl
   mov bh,bl
   xor bh,0FFh
   pop ecx
   test byte[SfxPOR],01h
   jnz .nozerocheck_4
   test byte[SfxCOLR],03h
   jz .noplot_4
.nozerocheck_4
   mov dl,[SfxCOLR]
   test byte[SfxPOR],02h
   jz .nodither2b
   mov dh,[SfxR1]
   xor dh,[SfxR2]
   test dh,01h
   jz .nodither2b
   shr dh,4
.nodither2b
   and byte[eax],bh
   and byte[eax+1],bh
   test dl,01h
   jz .nodraw_4
   or byte[eax],   bl
.nodraw_4
   test dl,02h
   jz .nodraw2_4
   or byte[eax+1], bl
.nodraw2_4
.noplot_4
   inc word [SfxR1]
   ret

.colors256
   shl ebx,6    ; x64 (256 colors)
   mov al,[SfxSCBR]
   shl eax,10   ; Get SFX address
   add eax,ebx
   add eax,[sfxramdata]
   mov ebx,[SfxR2]
   and ebx,07h
   shl ebx,1
   add eax,ebx
   mov cl,[SfxR1]
   and cl,07h
   xor cl,07h
   mov bl,1
   shl bl,cl
   mov bh,bl
   xor bh,0FFh
   pop ecx
   test byte[SfxPOR],01h
   jnz .nozerocheck_256
   mov dl,0FFh
   test byte[SfxPOR],08h
   jz .nozerocheckb_256
   mov dl,0Fh
.nozerocheckb_256
   test byte[SfxCOLR],dl
   jz .noplot_256
.nozerocheck_256
   mov dl,[SfxCOLR]
   and byte[eax],bh
   and byte[eax+1],bh
   and byte[eax+16],bh
   and byte[eax+17],bh
   and byte[eax+32],bh
   and byte[eax+33],bh
   and byte[eax+48],bh
   and byte[eax+49],bh
   test dl,01h
   jz .nodraw_256
   or byte[eax],   bl
.nodraw_256
   test dl,02h
   jz .nodraw2_256
   or byte[eax+1], bl
.nodraw2_256
   test dl,04h
   jz .nodraw3_256
   or byte[eax+16],bl
.nodraw3_256
   test dl,08h
   jz .nodraw4_256
   or byte[eax+17],bl
.nodraw4_256
   test dl,10h
   jz .nodraw5_256
   or byte[eax+32],   bl
.nodraw5_256
   test dl,20h
   jz .nodraw6_256
   or byte[eax+33], bl
.nodraw6_256
   test dl,40h
   jz .nodraw7_256
   or byte[eax+48],bl
.nodraw7_256
   test dl,80h
   jz .nodraw8_256
   or byte[eax+49],bl
.nodraw8_256
.noplot_256
   inc word [SfxR1]
   ret

SECTION .bss
.prevx resw 1
.prevy resw 1

sfxwarning resb 1

SECTION .text

NEWSYM FxOp4CA1    ; RPIX   read color of the pixel with R1,R2 as x,y
   FETCHPIPE
   mov ebx,[SfxR2]
   mov bh,[SfxR1]
   test byte[SfxPOR],10h
   jnz .objmode
   mov al,[SfxSCMR]
   and al,00100100b     ; 4 + 32
   cmp al,4
   je .lines160
   cmp al,32
   je .lines192
   cmp al,36
   je .objmode
   mov eax,[sfx128lineloc]
   jmp .donelines
.lines160
   mov eax,[sfx160lineloc]
   jmp .donelines
.lines192
   mov eax,[sfx192lineloc]
   jmp .donelines
.objmode
   mov eax,[sfxobjlineloc]
.donelines
   mov ebx,[eax+ebx*4]
   cmp ebx,0FFFFFFFFh
   je near .nodraw
   xor eax,eax
   ; bits 5/2 : 00 = 128 pixels, 01 = 160 pixels, 10 = 192 pixels, 11 = obj
   ; bits 1/0 : 00 = 4 color, 01 = 16-color, 10 = not used, 11 = 256 color
   ; 192 pixels = 24 tiles, 160 pixels = 20 tiles, 128 pixels = 16 tiles
   ;              16+8(4/3)              16+4(4/2)              16(4/0)
   push ecx
   mov al,[SfxSCMR]
   and al,00000011b     ; 4 + 32

   cmp al,0
   je near .colors4
   cmp al,3
   je near .colors256

   shl ebx,5    ; x32 (16 colors)
   mov al,[SfxSCBR]
   shl eax,10   ; Get SFX address
   add eax,ebx
   add eax,[sfxramdata]
   mov ebx,[SfxR2]
   and ebx,07h
   shl ebx,1
   add eax,ebx
   mov cl,[SfxR1]
   and cl,07h
   xor cl,07h
   mov bl,1
   shl bl,cl
   pop ecx
   xor bh,bh
   test byte[eax],bl
   jz .nodraw_16
   or bh,01h
.nodraw_16
   test byte[eax+1],bl
   jz .nodraw2_16
   or bh,02h
.nodraw2_16
   test byte[eax+16],bl
   jz .nodraw3_16
   or bh,04h
.nodraw3_16
   test byte[eax+17],bl
   jz .nodraw4_16
   or bh,08h
.nodraw4_16
.nodraw
   mov bl,bh
   and ebx,0FFh
   inc ebp
;   UpdateR14
   CLRFLAGS
   mov [edi],ebx            ; Write Destination
   mov [flagnz],ebx
   ret

.colors4
   shl ebx,4    ; x16 (4 colors)
   mov al,[SfxSCBR]
   shl eax,10   ; Get SFX address
   add eax,ebx
   add eax,[sfxramdata]
   mov ebx,[SfxR2]
   and ebx,07h
   shl ebx,1
   add eax,ebx
   mov cl,[SfxR1]
   and cl,07h
   xor cl,07h
   mov bl,1
   shl bl,cl
   mov bh,bl
   xor bh,0FFh
   pop ecx
   xor bh,bh
   test byte[eax],bl
   jz .nodraw_4
   or bh,01h
.nodraw_4
   test byte[eax+1],bl
   jz .nodraw2_4
   or bh,02h
.nodraw2_4
   mov bl,bh
   and ebx,0FFh
   inc ebp
;   UpdateR14
   CLRFLAGS
   mov [edi],ebx            ; Write Destination
   mov [flagnz],ebx
   ret

.colors256
   shl ebx,6    ; x64 (256 colors)
   mov al,[SfxSCBR]
   shl eax,10   ; Get SFX address
   add eax,ebx
   add eax,[sfxramdata]
   mov ebx,[SfxR2]
   and ebx,07h
   shl ebx,1
   add eax,ebx
   mov cl,[SfxR1]
   and cl,07h
   xor cl,07h
   mov bl,1
   shl bl,cl
   mov bh,bl
   xor bh,0FFh
   pop ecx
   xor bh,bh
   test byte[eax],bl
   jz .nodraw_256
   or bh,01h
.nodraw_256
   test byte[eax+1],bl
   jz .nodraw2_256
   or bh,02h
.nodraw2_256
   test byte[eax+16],bl
   jz .nodraw3_256
   or bh,04h
.nodraw3_256
   test byte[eax+17],bl
   jz .nodraw4_256
   or bh,08h
.nodraw4_256
   test byte[eax+32],bl
   jz .nodraw5_256
   or bh,10h
.nodraw5_256
   test byte[eax+33],bl
   jz .nodraw6_256
   or bh,20h
.nodraw6_256
   test byte[eax+48],bl
   jz .nodraw7_256
   or bh,40h
.nodraw7_256
   test byte[eax+49],bl
   jz .nodraw8_256
   or bh,80h
.nodraw8_256
   mov bl,bh
   and ebx,0FFh
   inc ebp
;   UpdateR14
   CLRFLAGS
   mov [edi],ebx            ; Write Destination
   mov [flagnz],ebx
   ret

NEWSYM FxOp4D      ; SWAP   swap upper and lower byte of a register    ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   ror ax,8
   inc ebp                ; Increase program counter
   mov [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp4E      ; COLOR  copy source register to color register     ; V
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   ; if bit 3 of SfxPOR is set, then don't modify the upper 4 bits
   test byte[SfxPOR],04h
   jz .nohighnibble
   mov bl,al
   shr bl,4
   and al,0F0h
   or al,bl
.nohighnibble
   test byte[SfxPOR],08h
   jnz .preserveupper
   cmp [SfxCOLR],al
   je .nocolchange
   mov [SfxCOLR],al
   and eax,0FFh
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
.nocolchange
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret
.preserveupper
   mov bl,[SfxCOLR]
   and al,0Fh
   and bl,0F0h
   or al,bl
   cmp [SfxCOLR],al
   je .nocolchange
   mov [SfxCOLR],al
   and eax,0FFh
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret

NEWSYM FxOp4EA1    ; CMODE  set plot option register ; V
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   inc ebp                ; Increase program counter
   mov dword [SfxPOR],eax

   test byte[SfxPOR],10h
   jnz .objmode
   mov al,[SfxSCMR]
   and al,00100100b     ; 4 + 32
   cmp al,4
   je .lines160
   cmp al,32
   je .lines192
   cmp al,36
   je .objmode
   mov eax,[sfx128lineloc]
   jmp .donelines
.lines160
   mov eax,[sfx160lineloc]
   jmp .donelines
.lines192
   mov eax,[sfx192lineloc]
   jmp .donelines
.objmode
   mov eax,[sfxobjlineloc]
.donelines
   mov [sfxclineloc],eax


   push ebx
   mov al,[SfxSCMR]
   and eax,00000011b
   mov bl,[SfxPOR]
   and bl,0Fh
   shl bl,2
   or al,bl
   mov ebx,[PLOTJmpb+eax*4]
   mov eax,[PLOTJmpa+eax*4]
   mov dword [FxTable+4Ch*4],eax
   mov dword [FxTableb+4Ch*4],eax
   mov dword [FxTablec+4Ch*4],eax
   mov dword [FxTabled+4Ch*4],ebx
   pop ebx

   CLRFLAGS
   ret

NEWSYM FxOp4F      ; NOT    perform exclusive exor with 1 on all bits  ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   xor eax,0FFFFh
   inc ebp                ; Increase program counter
   mov [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp50      ; ADD RN add, register + register
   ADDRN 0
NEWSYM FxOp51      ; ADD RN add, register + register
   ADDRN 1
NEWSYM FxOp52      ; ADD RN add, register + register
   ADDRN 2
NEWSYM FxOp53      ; ADD RN add, register + register
   ADDRN 3
NEWSYM FxOp54      ; ADD RN add, register + register
   ADDRN 4
NEWSYM FxOp55      ; ADD RN add, register + register
   ADDRN 5
NEWSYM FxOp56      ; ADD RN add, register + register
   ADDRN 6
NEWSYM FxOp57      ; ADD RN add, register + register
   ADDRN 7
NEWSYM FxOp58      ; ADD RN add, register + register
   ADDRN 8
NEWSYM FxOp59      ; ADD RN add, register + register
   ADDRN 9
NEWSYM FxOp5A      ; ADD RN add, register + register
   ADDRN 10
NEWSYM FxOp5B      ; ADD RN add, register + register
   ADDRN 11
NEWSYM FxOp5C      ; ADD RN add, register + register
   ADDRN 12
NEWSYM FxOp5D      ; ADD RN add, register + register
   ADDRN 13
NEWSYM FxOp5E      ; ADD RN add, register + register
   ADDRN 14
NEWSYM FxOp5F      ; ADD RN add, register + register
   FETCHPIPE
   mov eax, [esi]    ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   add ax,bx
   seto byte[SfxOverflow]
   setc byte[SfxCarry]
   mov [SfxSignZero],eax
   inc ebp                ; Increase program counter
   mov [edi],eax      ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp50A1    ; ADC RN add with carry, register + register
   ADCRN 0
NEWSYM FxOp51A1    ; ADC RN add with carry, register + register
   ADCRN 1
NEWSYM FxOp52A1    ; ADC RN add with carry, register + register
   ADCRN 2
NEWSYM FxOp53A1    ; ADC RN add with carry, register + register
   ADCRN 3
NEWSYM FxOp54A1    ; ADC RN add with carry, register + register
   ADCRN 4
NEWSYM FxOp55A1    ; ADC RN add with carry, register + register
   ADCRN 5
NEWSYM FxOp56A1    ; ADC RN add with carry, register + register
   ADCRN 6
NEWSYM FxOp57A1    ; ADC RN add with carry, register + register
   ADCRN 7
NEWSYM FxOp58A1    ; ADC RN add with carry, register + register
   ADCRN 8
NEWSYM FxOp59A1    ; ADC RN add with carry, register + register
   ADCRN 9
NEWSYM FxOp5AA1    ; ADC RN add with carry, register + register
   ADCRN 10
NEWSYM FxOp5BA1    ; ADC RN add with carry, register + register
   ADCRN 11
NEWSYM FxOp5CA1    ; ADC RN add with carry, register + register
   ADCRN 12
NEWSYM FxOp5DA1    ; ADC RN add with carry, register + register
   ADCRN 13
NEWSYM FxOp5EA1    ; ADC RN add with carry, register + register
   ADCRN 14
NEWSYM FxOp5FA1    ; ADC RN add with carry, register + register
   FETCHPIPE
   mov eax, [esi]    ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   shr byte[SfxCarry],1
   adc ax,bx
   seto byte[SfxOverflow]
   setc byte[SfxCarry]
   mov [SfxSignZero],eax
   inc ebp                ; Increase program counter
   mov [edi],eax      ; Write Destination
   CLRFLAGS
   ret

; Weird opcode (FxOp50A2, add 0, wow!)
NEWSYM FxOp50A2    ; ADI RN add, register + immediate
   ADIRN 0
NEWSYM FxOp51A2    ; ADI RN add, register + immediate
   ADIRN 1
NEWSYM FxOp52A2    ; ADI RN add, register + immediate
   ADIRN 2
NEWSYM FxOp53A2    ; ADI RN add, register + immediate
   ADIRN 3
NEWSYM FxOp54A2    ; ADI RN add, register + immediate
   ADIRN 4
NEWSYM FxOp55A2    ; ADI RN add, register + immediate
   ADIRN 5
NEWSYM FxOp56A2    ; ADI RN add, register + immediate
   ADIRN 6
NEWSYM FxOp57A2    ; ADI RN add, register + immediate
   ADIRN 7
NEWSYM FxOp58A2    ; ADI RN add, register + immediate
   ADIRN 8
NEWSYM FxOp59A2    ; ADI RN add, register + immediate
   ADIRN 9
NEWSYM FxOp5AA2    ; ADI RN add, register + immediate
   ADIRN 10
NEWSYM FxOp5BA2    ; ADI RN add, register + immediate
   ADIRN 11
NEWSYM FxOp5CA2    ; ADI RN add, register + immediate
   ADIRN 12
NEWSYM FxOp5DA2    ; ADI RN add, register + immediate
   ADIRN 13
NEWSYM FxOp5EA2    ; ADI RN add, register + immediate
   ADIRN 14
NEWSYM FxOp5FA2    ; ADI RN add, register + immediate
   ADIRN 15

; Another very useful opcode
NEWSYM FxOp50A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 0
NEWSYM FxOp51A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 1
NEWSYM FxOp52A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 2
NEWSYM FxOp53A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 3
NEWSYM FxOp54A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 4
NEWSYM FxOp55A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 5
NEWSYM FxOp56A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 6
NEWSYM FxOp57A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 7
NEWSYM FxOp58A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 8
NEWSYM FxOp59A3    ; ADCIRN add with carry, register + immediate
   ADCIRN 9
NEWSYM FxOp5AA3    ; ADCIRN add with carry, register + immediate
   ADCIRN 10
NEWSYM FxOp5BA3    ; ADCIRN add with carry, register + immediate
   ADCIRN 11
NEWSYM FxOp5CA3    ; ADCIRN add with carry, register + immediate
   ADCIRN 12
NEWSYM FxOp5DA3    ; ADCIRN add with carry, register + immediate
   ADCIRN 13
NEWSYM FxOp5EA3    ; ADCIRN add with carry, register + immediate
   ADCIRN 14
NEWSYM FxOp5FA3    ; ADCIRN add with carry, register + immediate
   ADCIRN 15

NEWSYM FxOp60      ; SUBRN  subtract, register - register
   SUBRN 0
NEWSYM FxOp61      ; SUBRN  subtract, register - register
   SUBRN 1
NEWSYM FxOp62      ; SUBRN  subtract, register - register
   SUBRN 2
NEWSYM FxOp63      ; SUBRN  subtract, register - register
   SUBRN 3
NEWSYM FxOp64      ; SUBRN  subtract, register - register
   SUBRN 4
NEWSYM FxOp65      ; SUBRN  subtract, register - register
   SUBRN 5
NEWSYM FxOp66      ; SUBRN  subtract, register - register
   SUBRN 6
NEWSYM FxOp67      ; SUBRN  subtract, register - register
   SUBRN 7
NEWSYM FxOp68      ; SUBRN  subtract, register - register
   SUBRN 8
NEWSYM FxOp69      ; SUBRN  subtract, register - register
   SUBRN 9
NEWSYM FxOp6A      ; SUBRN  subtract, register - register
   SUBRN 10
NEWSYM FxOp6B      ; SUBRN  subtract, register - register
   SUBRN 11
NEWSYM FxOp6C      ; SUBRN  subtract, register - register
   SUBRN 12
NEWSYM FxOp6D      ; SUBRN  subtract, register - register
   SUBRN 13
NEWSYM FxOp6E      ; SUBRN  subtract, register - register
   SUBRN 14
NEWSYM FxOp6F      ; SUBRN  subtract, register - register
   FETCHPIPE
   mov eax,[esi]    ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   sub ax,bx
   seto byte[SfxOverflow]
   setc byte[SfxCarry]
   xor byte[SfxCarry],1
   inc ebp                   ; Increase program counter
   mov [edi],eax                        ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp60A1    ; SBCRN  subtract with carry, register - register
   SBCRN 0
NEWSYM FxOp61A1    ; SBCRN  subtract with carry, register - register
   SBCRN 1
NEWSYM FxOp62A1    ; SBCRN  subtract with carry, register - register
   SBCRN 2
NEWSYM FxOp63A1    ; SBCRN  subtract with carry, register - register
   SBCRN 3
NEWSYM FxOp64A1    ; SBCRN  subtract with carry, register - register
   SBCRN 4
NEWSYM FxOp65A1    ; SBCRN  subtract with carry, register - register
   SBCRN 5
NEWSYM FxOp66A1    ; SBCRN  subtract with carry, register - register
   SBCRN 6
NEWSYM FxOp67A1    ; SBCRN  subtract with carry, register - register
   SBCRN 7
NEWSYM FxOp68A1    ; SBCRN  subtract with carry, register - register
   SBCRN 8
NEWSYM FxOp69A1    ; SBCRN  subtract with carry, register - register
   SBCRN 9
NEWSYM FxOp6AA1    ; SBCRN  subtract with carry, register - register
   SBCRN 10
NEWSYM FxOp6BA1    ; SBCRN  subtract with carry, register - register
   SBCRN 11
NEWSYM FxOp6CA1    ; SBCRN  subtract with carry, register - register
   SBCRN 12
NEWSYM FxOp6DA1    ; SBCRN  subtract with carry, register - register
   SBCRN 13
NEWSYM FxOp6EA1    ; SBCRN  subtract with carry, register - register
   SBCRN 14
NEWSYM FxOp6FA1    ; SBCRN  subtract with carry, register - register
   mov eax,[esi]    ; Read Source
   mov ebx,ebp
   FETCHPIPE
   sub ebx,[SfxCPB]
   cmp byte[SfxCarry],1
   sbb ax,bx
   seto byte[SfxOverflow]
   setc byte[SfxCarry]
   xor byte[SfxCarry],1
   inc ebp                ; Increase program counter
   mov [edi],eax      ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp60A2    ; SUBIRN subtract, register - immediate
   SUBIRN 0
NEWSYM FxOp61A2    ; SUBIRN subtract, register - immediate
   SUBIRN 1
NEWSYM FxOp62A2    ; SUBIRN subtract, register - immediate
   SUBIRN 2
NEWSYM FxOp63A2    ; SUBIRN subtract, register - immediate
   SUBIRN 3
NEWSYM FxOp64A2    ; SUBIRN subtract, register - immediate
   SUBIRN 4
NEWSYM FxOp65A2    ; SUBIRN subtract, register - immediate
   SUBIRN 5
NEWSYM FxOp66A2    ; SUBIRN subtract, register - immediate
   SUBIRN 6
NEWSYM FxOp67A2    ; SUBIRN subtract, register - immediate
   SUBIRN 7
NEWSYM FxOp68A2    ; SUBIRN subtract, register - immediate
   SUBIRN 8
NEWSYM FxOp69A2    ; SUBIRN subtract, register - immediate
   SUBIRN 9
NEWSYM FxOp6AA2    ; SUBIRN subtract, register - immediate
   SUBIRN 10
NEWSYM FxOp6BA2    ; SUBIRN subtract, register - immediate
   SUBIRN 11
NEWSYM FxOp6CA2    ; SUBIRN subtract, register - immediate
   SUBIRN 12
NEWSYM FxOp6DA2    ; SUBIRN subtract, register - immediate
   SUBIRN 13
NEWSYM FxOp6EA2    ; SUBIRN subtract, register - immediate
   SUBIRN 14
NEWSYM FxOp6FA2    ; SUBIRN subtract, register - immediate
   SUBIRN 15

NEWSYM FxOp60A3    ; CMPRN  compare, register, register
   CMPRN 0
NEWSYM FxOp61A3    ; CMPRN  compare, register, register
   CMPRN 1
NEWSYM FxOp62A3    ; CMPRN  compare, register, register
   CMPRN 2
NEWSYM FxOp63A3    ; CMPRN  compare, register, register
   CMPRN 3
NEWSYM FxOp64A3    ; CMPRN  compare, register, register
   CMPRN 4
NEWSYM FxOp65A3    ; CMPRN  compare, register, register
   CMPRN 5
NEWSYM FxOp66A3    ; CMPRN  compare, register, register
   CMPRN 6
NEWSYM FxOp67A3    ; CMPRN  compare, register, register
   CMPRN 7
NEWSYM FxOp68A3    ; CMPRN  compare, register, register
   CMPRN 8
NEWSYM FxOp69A3    ; CMPRN  compare, register, register
   CMPRN 9
NEWSYM FxOp6AA3    ; CMPRN  compare, register, register
   CMPRN 10
NEWSYM FxOp6BA3    ; CMPRN  compare, register, register
   CMPRN 11
NEWSYM FxOp6CA3    ; CMPRN  compare, register, register
   CMPRN 12
NEWSYM FxOp6DA3    ; CMPRN  compare, register, register
   CMPRN 13
NEWSYM FxOp6EA3    ; CMPRN  compare, register, register
   CMPRN 14
NEWSYM FxOp6FA3    ; CMPRN  compare, register, register
   FETCHPIPE
   mov eax,[esi]    ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   sub ax,bx
   seto byte[SfxOverflow]
   setc byte[SfxCarry]
   xor byte[SfxCarry],1
   mov [SfxSignZero],eax
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret

NEWSYM FxOp70      ; MERGE  R7 as upper byte, R8 as lower byte (used for texture-mapping) */
            ; V
   xor eax,eax
   FETCHPIPE
   mov ah,byte [SfxR7+1]
   mov al,byte [SfxR8+1]
   inc ebp
   mov [edi],eax            ; Write Destination
   mov dword[SfxSignZero],0001h
   test eax,0F0F0h
   jz .nozero
   mov dword[SfxSignZero],0000h
.nozero
   test eax,08080h
   jz .nosign
   or dword [SfxSignZero],80000h
.nosign
   mov dword [SfxOverflow],1
   test ax,0c0c0h
   jnz .Overflow
   mov dword [SfxOverflow],0
.Overflow
   mov dword [SfxCarry],1
   test ax,0e0e0h
   jnz .Carry
   mov dword [SfxCarry],0
.Carry
   CLRFLAGS
   ret

NEWSYM FxOp71      ; AND RN register & register
   ANDRN 1
NEWSYM FxOp72      ; AND RN register & register
   ANDRN 2
NEWSYM FxOp73      ; AND RN register & register
   ANDRN 3
NEWSYM FxOp74      ; AND RN register & register
   ANDRN 4
NEWSYM FxOp75      ; AND RN register & register
   ANDRN 5
NEWSYM FxOp76      ; AND RN register & register
   ANDRN 6
NEWSYM FxOp77      ; AND RN register & register
   ANDRN 7
NEWSYM FxOp78      ; AND RN register & register
   ANDRN 8
NEWSYM FxOp79      ; AND RN register & register
   ANDRN 9
NEWSYM FxOp7A      ; AND RN register & register
   ANDRN 10
NEWSYM FxOp7B      ; AND RN register & register
   ANDRN 11
NEWSYM FxOp7C      ; AND RN register & register
   ANDRN 12
NEWSYM FxOp7D      ; AND RN register & register
   ANDRN 13
NEWSYM FxOp7E      ; AND RN register & register
   ANDRN 14
NEWSYM FxOp7F      ; AND RN register & register
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   and eax,ebx
   inc ebp
   mov dword [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp71A1    ; BIC RN register & ~register
   BICRN 1
NEWSYM FxOp72A1    ; BIC RN register & ~register
   BICRN 2
NEWSYM FxOp73A1    ; BIC RN register & ~register
   BICRN 3
NEWSYM FxOp74A1    ; BIC RN register & ~register
   BICRN 4
NEWSYM FxOp75A1    ; BIC RN register & ~register
   BICRN 5
NEWSYM FxOp76A1    ; BIC RN register & ~register
   BICRN 6
NEWSYM FxOp77A1    ; BIC RN register & ~register
   BICRN 7
NEWSYM FxOp78A1    ; BIC RN register & ~register
   BICRN 8
NEWSYM FxOp79A1    ; BIC RN register & ~register
   BICRN 9
NEWSYM FxOp7AA1    ; BIC RN register & ~register
   BICRN 10
NEWSYM FxOp7BA1    ; BIC RN register & ~register
   BICRN 11
NEWSYM FxOp7CA1    ; BIC RN register & ~register
   BICRN 12
NEWSYM FxOp7DA1    ; BIC RN register & ~register
   BICRN 13
NEWSYM FxOp7EA1    ; BIC RN register & ~register
   BICRN 14
NEWSYM FxOp7FA1    ; BIC RN register & ~register
   FETCHPIPE
   mov ebx,ebp
   sub ebx,[SfxCPB]
   mov eax,[esi]            ; Read Source
   xor ebx,0FFFFh
   and eax,ebx
   inc ebp
   mov dword [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp71A2    ; ANDIRN and #n - register & immediate
   ANDIRN 1
NEWSYM FxOp72A2    ; ANDIRN and #n - register & immediate
   ANDIRN 2
NEWSYM FxOp73A2    ; ANDIRN and #n - register & immediate
   ANDIRN 3
NEWSYM FxOp74A2    ; ANDIRN and #n - register & immediate
   ANDIRN 4
NEWSYM FxOp75A2    ; ANDIRN and #n - register & immediate
   ANDIRN 5
NEWSYM FxOp76A2    ; ANDIRN and #n - register & immediate
   ANDIRN 6
NEWSYM FxOp77A2    ; ANDIRN and #n - register & immediate
   ANDIRN 7
NEWSYM FxOp78A2    ; ANDIRN and #n - register & immediate
   ANDIRN 8
NEWSYM FxOp79A2    ; ANDIRN and #n - register & immediate
   ANDIRN 9
NEWSYM FxOp7AA2    ; ANDIRN and #n - register & immediate
   ANDIRN 10
NEWSYM FxOp7BA2    ; ANDIRN and #n - register & immediate
   ANDIRN 11
NEWSYM FxOp7CA2    ; ANDIRN and #n - register & immediate
   ANDIRN 12
NEWSYM FxOp7DA2    ; ANDIRN and #n - register & immediate
   ANDIRN 13
NEWSYM FxOp7EA2    ; ANDIRN and #n - register & immediate
   ANDIRN 14
NEWSYM FxOp7FA2    ; ANDIRN and #n - register & immediate
   ANDIRN 15

NEWSYM FxOp71A3    ; BICIRN register & ~immediate
   BICIRN 1 ^ 0FFFFh
NEWSYM FxOp72A3    ; BICIRN register & ~immediate
   BICIRN 2 ^ 0FFFFh
NEWSYM FxOp73A3    ; BICIRN register & ~immediate
   BICIRN 3 ^ 0FFFFh
NEWSYM FxOp74A3    ; BICIRN register & ~immediate
   BICIRN 4 ^ 0FFFFh
NEWSYM FxOp75A3    ; BICIRN register & ~immediate
   BICIRN 5 ^ 0FFFFh
NEWSYM FxOp76A3    ; BICIRN register & ~immediate
   BICIRN 6 ^ 0FFFFh
NEWSYM FxOp77A3    ; BICIRN register & ~immediate
   BICIRN 7 ^ 0FFFFh
NEWSYM FxOp78A3    ; BICIRN register & ~immediate
   BICIRN 8 ^ 0FFFFh
NEWSYM FxOp79A3    ; BICIRN register & ~immediate
   BICIRN 9 ^ 0FFFFh
NEWSYM FxOp7AA3    ; BICIRN register & ~immediate
   BICIRN 10 ^ 0FFFFh
NEWSYM FxOp7BA3    ; BICIRN register & ~immediate
   BICIRN 11 ^ 0FFFFh
NEWSYM FxOp7CA3    ; BICIRN register & ~immediate
   BICIRN 12 ^ 0FFFFh
NEWSYM FxOp7DA3    ; BICIRN register & ~immediate
   BICIRN 13 ^ 0FFFFh
NEWSYM FxOp7EA3    ; BICIRN register & ~immediate
   BICIRN 14 ^ 0FFFFh
NEWSYM FxOp7FA3    ; BICIRN register & ~immediate
   BICIRN 15 ^ 0FFFFh

NEWSYM FxOp80      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 0
NEWSYM FxOp81      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 1
NEWSYM FxOp82      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 2
NEWSYM FxOp83      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 3
NEWSYM FxOp84      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 4
NEWSYM FxOp85      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 5
NEWSYM FxOp86      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 6
NEWSYM FxOp87      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 7
NEWSYM FxOp88      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 8
NEWSYM FxOp89      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 9
NEWSYM FxOp8A      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 10
NEWSYM FxOp8B      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 11
NEWSYM FxOp8C      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 12
NEWSYM FxOp8D      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 13
NEWSYM FxOp8E      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   MULTRN 14
NEWSYM FxOp8F      ; MULTRN 8 bit to 16 bit signed multiply, register * register
   FETCHPIPE
   mov ebx,ebp
   mov al,byte [esi]     ; Read Source
   sub ebx,[SfxCPB]
   imul bl
   inc ebp
   and eax,0FFFFh
   mov [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp80A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 0
NEWSYM FxOp81A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 1
NEWSYM FxOp82A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 2
NEWSYM FxOp83A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 3
NEWSYM FxOp84A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 4
NEWSYM FxOp85A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 5
NEWSYM FxOp86A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 6
NEWSYM FxOp87A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 7
NEWSYM FxOp88A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 8
NEWSYM FxOp89A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 9
NEWSYM FxOp8AA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 10
NEWSYM FxOp8BA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 11
NEWSYM FxOp8CA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 12
NEWSYM FxOp8DA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 13
NEWSYM FxOp8EA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRN 14
NEWSYM FxOp8FA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   FETCHPIPE
   mov ebx,ebp
   mov al,byte [esi]     ; Read Source
   sub ebx,[SfxCPB]
   mul bl
   inc ebp
   and eax,0FFFFh
   mov [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   ret

NEWSYM FxOp80A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 0
NEWSYM FxOp81A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 1
NEWSYM FxOp82A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 2
NEWSYM FxOp83A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 3
NEWSYM FxOp84A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 4
NEWSYM FxOp85A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 5
NEWSYM FxOp86A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 6
NEWSYM FxOp87A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 7
NEWSYM FxOp88A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 8
NEWSYM FxOp89A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 9
NEWSYM FxOp8AA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 10
NEWSYM FxOp8BA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 11
NEWSYM FxOp8CA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 12
NEWSYM FxOp8DA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 13
NEWSYM FxOp8EA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 14
NEWSYM FxOp8FA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRN 15

NEWSYM FxOp80A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 0
NEWSYM FxOp81A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 1
NEWSYM FxOp82A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 2
NEWSYM FxOp83A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 3
NEWSYM FxOp84A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 4
NEWSYM FxOp85A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 5
NEWSYM FxOp86A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 6
NEWSYM FxOp87A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 7
NEWSYM FxOp88A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 8
NEWSYM FxOp89A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 9
NEWSYM FxOp8AA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 10
NEWSYM FxOp8BA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 11
NEWSYM FxOp8CA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 12
NEWSYM FxOp8DA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 13
NEWSYM FxOp8EA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 14
NEWSYM FxOp8FA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRN 15

NEWSYM FxOp90      ; SBK    store word to last accessed RAM address    ; V
   mov ebx,[SfxLastRamAdr]   ; Load last ram address
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   mov [ebx],al         ; Store Word
   sub ebx,[SfxRAMMem]
   xor ebx,1
   add ebx,[SfxRAMMem]
   inc ebp                ; Increase program counter
   mov [ebx],ah         ; Store Word
   CLRFLAGS
   ret

NEWSYM FxOp91      ; LINK#n R11 = R15 + immediate
   LINK 1
NEWSYM FxOp92      ; LINK#n R11 = R15 + immediate
   LINK 2
NEWSYM FxOp93      ; LINK#n R11 = R15 + immediate
   LINK 3
NEWSYM FxOp94      ; LINK#n R11 = R15 + immediate
   LINK 4

NEWSYM FxOp95      ; SEX    sign extend 8 bit to 16 bit        ; V
   movsx eax, byte [esi]     ; Read Source
   FETCHPIPE
   and eax,0FFFFh
   inc ebp
   mov [edi],eax            ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp96      ; ASR    aritmethic shift right by one      ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   mov [SfxCarry],al
   and byte[SfxCarry],1
   sar ax,1                      ; logic shift right
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp96A1    ; DIV2   aritmethic shift right by one      ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   cmp ax,-1
   je .minusone
   mov [SfxCarry],al
   and byte[SfxCarry],1
   sar ax,1                      ; logic shift right
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   ret
.minusone
   mov byte[SfxCarry],1
   xor eax,eax
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp97      ; ROR    rotate right by one        ; V
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   shr byte[SfxCarry],1
   rcr ax,1
   setc byte[SfxCarry]
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp98      ; JMPRN  jump to address of register
   JMPRN 8
NEWSYM FxOp99      ; JMPRN  jump to address of register
   JMPRN 9
NEWSYM FxOp9A      ; JMPRN  jump to address of register
   JMPRN 10
NEWSYM FxOp9B      ; JMPRN  jump to address of register
   JMPRN 11
NEWSYM FxOp9C      ; JMPRN  jump to address of register
   JMPRN 12
NEWSYM FxOp9D      ; JMPRN  jump to address of register
   JMPRN 13

NEWSYM FxOp98A1    ; LJMPRN set program bank to source register and jump to address of register
   LJMPRN 8
NEWSYM FxOp99A1    ; LJMPRN set program bank to source register and jump to address of register
   LJMPRN 9
NEWSYM FxOp9AA1    ; LJMPRN set program bank to source register and jump to address of register
   LJMPRN 10
NEWSYM FxOp9BA1    ; LJMPRN set program bank to source register and jump to address of register
   LJMPRN 11
NEWSYM FxOp9CA1    ; LJMPRN set program bank to source register and jump to address of register
   LJMPRN 12
NEWSYM FxOp9DA1    ; LJMPRN set program bank to source register and jump to address of register
   LJMPRN 13

NEWSYM FxOp9E      ; LOB    set upper byte to zero (keep low byte) ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   and eax,0FFh
   inc ebp
   mov [edi],eax            ; Write Destination
   shl eax,8
   mov dword [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOp9F      ; FMULT  16 bit to 32 bit signed multiplication, upper 16 bits only
            ; V
   mov eax,[esi]             ; Read Source
   mov ebx,[SfxR6]
   FETCHPIPE
   imul bx
   inc ebp
   and edx,0FFFFh
   mov [SfxSignZero],edx
   mov [edi],edx        ; Write Destination
   shr ax,15
   mov [SfxCarry],al
   CLRFLAGS
   ret

NEWSYM FxOp9FA1    ; LMULT  16 bit to 32 bit signed multiplication     ; V
   mov eax,[esi]             ; Read Source
   mov ebx,[SfxR6]
   FETCHPIPE
   imul bx
   and edx,0FFFFh
   inc ebp
   mov [edi],edx        ; Write Destination
   mov [SfxR4],ax
   mov [SfxSignZero],edx
   shr ax,15
   mov [SfxCarry],al
   CLRFLAGS
   ret

NEWSYM FxOpA0      ; IBTRN,#PP immediate byte transfer
   IBTRN 0
NEWSYM FxOpA1      ; IBTRN,#PP immediate byte transfer
   IBTRN 1
NEWSYM FxOpA2      ; IBTRN,#PP immediate byte transfer
   IBTRN 2
NEWSYM FxOpA3      ; IBTRN,#PP immediate byte transfer
   IBTRN 3
NEWSYM FxOpA4      ; IBTRN,#PP immediate byte transfer
   IBTRN 4
NEWSYM FxOpA5      ; IBTRN,#PP immediate byte transfer
   IBTRN 5
NEWSYM FxOpA6      ; IBTRN,#PP immediate byte transfer
   IBTRN 6
NEWSYM FxOpA7      ; IBTRN,#PP immediate byte transfer
   IBTRN 7
NEWSYM FxOpA8      ; IBTRN,#PP immediate byte transfer
   IBTRN 8
NEWSYM FxOpA9      ; IBTRN,#PP immediate byte transfer
   IBTRN 9
NEWSYM FxOpAA      ; IBTRN,#PP immediate byte transfer
   IBTRN 10
NEWSYM FxOpAB      ; IBTRN,#PP immediate byte transfer
   IBTRN 11
NEWSYM FxOpAC      ; IBTRN,#PP immediate byte transfer
   IBTRN 12
NEWSYM FxOpAD      ; IBTRN,#PP immediate byte transfer
   IBTRN 13
NEWSYM FxOpAE      ; IBTRN,#PP immediate byte transfer
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   add ebp,2
   mov [SfxR0+14*4],ax
   UpdateR14
   CLRFLAGS
   ret
NEWSYM FxOpAF      ; IBTRN,#PP immediate byte transfer
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   and eax,0FFFFh
   mov ebp,[SfxCPB]
   add ebp,eax
   CLRFLAGS
   ret

NEWSYM FxOpA0A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 0
NEWSYM FxOpA1A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 1
NEWSYM FxOpA2A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 2
NEWSYM FxOpA3A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 3
NEWSYM FxOpA4A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 4
NEWSYM FxOpA5A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 5
NEWSYM FxOpA6A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 6
NEWSYM FxOpA7A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 7
NEWSYM FxOpA8A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 8
NEWSYM FxOpA9A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 9
NEWSYM FxOpAAA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 10
NEWSYM FxOpABA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 11
NEWSYM FxOpACA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 12
NEWSYM FxOpADA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRN 13
NEWSYM FxOpAEA1    ; LMS rn,(yy)  load word from RAM (short address)
   xor eax,eax
   mov al,[ebp]
   add eax,eax
   inc ebp
   add eax,[SfxRAMMem]
   mov cl,[ebp]
   mov dword [SfxLastRamAdr],eax
   mov ebx,[eax]              ; Read word from ram
   inc ebp
   mov [SfxR0+14*4],bx              ; Write data
   UpdateR14
   CLRFLAGS
   ret
NEWSYM FxOpAFA1    ; LMS rn,(yy)  load word from RAM (short address)
   xor eax,eax
   mov al,[ebp]
   add eax,eax
   inc ebp
   add eax,[SfxRAMMem]
   mov cl,[ebp]
   mov dword [SfxLastRamAdr],eax
   mov ebx,[eax]              ; Read word from ram
   and ebx,0FFFFh
   mov ebp,[SfxCPB]
   add ebp,ebx
   CLRFLAGS
   ret

NEWSYM FxOpA0A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 0
NEWSYM FxOpA1A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 1
NEWSYM FxOpA2A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 2
NEWSYM FxOpA3A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 3
NEWSYM FxOpA4A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 4
NEWSYM FxOpA5A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 5
NEWSYM FxOpA6A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 6
NEWSYM FxOpA7A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 7
NEWSYM FxOpA8A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 8
NEWSYM FxOpA9A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 9
NEWSYM FxOpAAA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 10
NEWSYM FxOpABA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 11
NEWSYM FxOpACA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 12
NEWSYM FxOpADA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 13
NEWSYM FxOpAEA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRN 14
NEWSYM FxOpAFA2    ; SMS (yy),rn  store word in RAM (short address)
   xor eax,eax
   mov ebx,ebp
   sub ebx,[SfxCPB]
   mov al,[ebp]
   inc ebp
   add eax,eax
   FETCHPIPE
   add eax,[SfxRAMMem]
   mov dword [SfxLastRamAdr],eax
   inc ebp
   mov [eax],bx              ; Write word to ram
   CLRFLAGS
   ret

NEWSYM FxOpB0      ; FROM rn   set source register
   FROMRN 0
NEWSYM FxOpB1      ; FROM rn   set source register
   FROMRN 1
NEWSYM FxOpB2      ; FROM rn   set source register
   FROMRN 2
NEWSYM FxOpB3      ; FROM rn   set source register
   FROMRN 3
NEWSYM FxOpB4      ; FROM rn   set source register
   FROMRN 4
NEWSYM FxOpB5      ; FROM rn   set source register
   FROMRN 5
NEWSYM FxOpB6      ; FROM rn   set source register
   FROMRN 6
NEWSYM FxOpB7      ; FROM rn   set source register
   FROMRN 7
NEWSYM FxOpB8      ; FROM rn   set source register
   FROMRN 8
NEWSYM FxOpB9      ; FROM rn   set source register
   FROMRN 9
NEWSYM FxOpBA      ; FROM rn   set source register
   FROMRN 10
NEWSYM FxOpBB      ; FROM rn   set source register
   FROMRN 11
NEWSYM FxOpBC      ; FROM rn   set source register
   FROMRN 12
NEWSYM FxOpBD      ; FROM rn   set source register
   FROMRN 13
NEWSYM FxOpBE      ; FROM rn   set source register
   FROMRN 14
NEWSYM FxOpBF      ; FROM rn   set source register
   FETCHPIPE
   mov esi,SfxR0+15*4
   inc ebp                ; Increase program counter
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov esi,SfxR0
   ret

NEWSYM FxOpC0      ; HIB       move high-byte to low-byte      ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   and eax,0FF00h
   mov dword [SfxSignZero],eax
   shr eax,8
   inc ebp
   mov [edi],eax
   CLRFLAGS
   ret

NEWSYM FxOpC1      ; OR rn     or rn
   ORRN 1
NEWSYM FxOpC2      ; OR rn     or rn
   ORRN 2
NEWSYM FxOpC3      ; OR rn     or rn
   ORRN 3
NEWSYM FxOpC4      ; OR rn     or rn
   ORRN 4
NEWSYM FxOpC5      ; OR rn     or rn
   ORRN 5
NEWSYM FxOpC6      ; OR rn     or rn
   ORRN 6
NEWSYM FxOpC7      ; OR rn     or rn
   ORRN 7
NEWSYM FxOpC8      ; OR rn     or rn
   ORRN 8
NEWSYM FxOpC9      ; OR rn     or rn
   ORRN 9
NEWSYM FxOpCA      ; OR rn     or rn
   ORRN 10
NEWSYM FxOpCB      ; OR rn     or rn
   ORRN 11
NEWSYM FxOpCC      ; OR rn     or rn
   ORRN 12
NEWSYM FxOpCD      ; OR rn     or rn
   ORRN 13
NEWSYM FxOpCE      ; OR rn     or rn
   ORRN 14
NEWSYM FxOpCF      ; OR rn     or rn
   mov eax,[esi]            ; Read Source
   mov ebx,ebp
   FETCHPIPE
   sub ebx,[SfxCPB]
   or eax,ebx
   inc ebp
   mov [edi],eax            ; Write DREG
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOpC1A1    ; XOR rn    xor rn
   XORRN 1
NEWSYM FxOpC2A1    ; XOR rn    xor rn
   XORRN 2
NEWSYM FxOpC3A1    ; XOR rn    xor rn
   XORRN 3
NEWSYM FxOpC4A1    ; XOR rn    xor rn
   XORRN 4
NEWSYM FxOpC5A1    ; XOR rn    xor rn
   XORRN 5
NEWSYM FxOpC6A1    ; XOR rn    xor rn
   XORRN 6
NEWSYM FxOpC7A1    ; XOR rn    xor rn
   XORRN 7
NEWSYM FxOpC8A1    ; XOR rn    xor rn
   XORRN 8
NEWSYM FxOpC9A1    ; XOR rn    xor rn
   XORRN 9
NEWSYM FxOpCAA1    ; XOR rn    xor rn
   XORRN 10
NEWSYM FxOpCBA1    ; XOR rn    xor rn
   XORRN 11
NEWSYM FxOpCCA1    ; XOR rn    xor rn
   XORRN 12
NEWSYM FxOpCDA1    ; XOR rn    xor rn
   XORRN 13
NEWSYM FxOpCEA1    ; XOR rn    xor rn
   XORRN 14
NEWSYM FxOpCFA1    ; XOR rn    xor rn
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   xor eax,ebx
   inc ebp
   mov [edi],eax            ; Write DREG
   mov [SfxSignZero],eax
   CLRFLAGS
   ret

NEWSYM FxOpC1A2    ; OR #n     OR #n
   ORI 1
NEWSYM FxOpC2A2    ; OR #n     OR #n
   ORI 2
NEWSYM FxOpC3A2    ; OR #n     OR #n
   ORI 3
NEWSYM FxOpC4A2    ; OR #n     OR #n
   ORI 4
NEWSYM FxOpC5A2    ; OR #n     OR #n
   ORI 5
NEWSYM FxOpC6A2    ; OR #n     OR #n
   ORI 6
NEWSYM FxOpC7A2    ; OR #n     OR #n
   ORI 7
NEWSYM FxOpC8A2    ; OR #n     OR #n
   ORI 8
NEWSYM FxOpC9A2    ; OR #n     OR #n
   ORI 9
NEWSYM FxOpCAA2    ; OR #n     OR #n
   ORI 10
NEWSYM FxOpCBA2    ; OR #n     OR #n
   ORI 11
NEWSYM FxOpCCA2    ; OR #n     OR #n
   ORI 12
NEWSYM FxOpCDA2    ; OR #n     OR #n
   ORI 13
NEWSYM FxOpCEA2    ; OR #n     OR #n
   ORI 14
NEWSYM FxOpCFA2    ; OR #n     OR #n
   ORI 15

NEWSYM FxOpC1A3    ; XOR #n    xor #n
   XORI 1
NEWSYM FxOpC2A3    ; XOR #n    xor #n
   XORI 2
NEWSYM FxOpC3A3    ; XOR #n    xor #n
   XORI 3
NEWSYM FxOpC4A3    ; XOR #n    xor #n
   XORI 4
NEWSYM FxOpC5A3    ; XOR #n    xor #n
   XORI 5
NEWSYM FxOpC6A3    ; XOR #n    xor #n
   XORI 6
NEWSYM FxOpC7A3    ; XOR #n    xor #n
   XORI 7
NEWSYM FxOpC8A3    ; XOR #n    xor #n
   XORI 8
NEWSYM FxOpC9A3    ; XOR #n    xor #n
   XORI 9
NEWSYM FxOpCAA3    ; XOR #n    xor #n
   XORI 10
NEWSYM FxOpCBA3    ; XOR #n    xor #n
   XORI 11
NEWSYM FxOpCCA3    ; XOR #n    xor #n
   XORI 12
NEWSYM FxOpCDA3    ; XOR #n    xor #n
   XORI 13
NEWSYM FxOpCEA3    ; XOR #n    xor #n
   XORI 14
NEWSYM FxOpCFA3    ; XOR #n    xor #n
   XORI 15

NEWSYM FxOpD0      ; INC rn    increase by one
   INCRN 0
NEWSYM FxOpD1      ; INC rn    increase by one
   INCRN 1
NEWSYM FxOpD2      ; INC rn    increase by one
   INCRN 2
NEWSYM FxOpD3      ; INC rn    increase by one
   INCRN 3
NEWSYM FxOpD4      ; INC rn    increase by one
   INCRN 4
NEWSYM FxOpD5      ; INC rn    increase by one
   INCRN 5
NEWSYM FxOpD6      ; INC rn    increase by one
   INCRN 6
NEWSYM FxOpD7      ; INC rn    increase by one
   INCRN 7
NEWSYM FxOpD8      ; INC rn    increase by one
   INCRN 8
NEWSYM FxOpD9      ; INC rn    increase by one
   INCRN 9
NEWSYM FxOpDA      ; INC rn    increase by one
   INCRN 10
NEWSYM FxOpDB      ; INC rn    increase by one
   INCRN 11
NEWSYM FxOpDC      ; INC rn    increase by one
   INCRN 12
NEWSYM FxOpDD      ; INC rn    increase by one
   INCRN 13
NEWSYM FxOpDE      ; INC rn    increase by one
   FETCHPIPE
   mov eax,[SfxR0+14*4]            ; Read Source
   inc ax
   mov [SfxR0+14*4],eax
   mov [SfxSignZero],eax
   CLRFLAGS
   inc ebp
   UpdateR14
   ret

NEWSYM FxOpDF      ; GETC      transfer ROM buffer to color register
   mov eax,[SfxRomBuffer]
   FETCHPIPE
   mov eax,[eax]
   test byte[SfxPOR],04h
   jz .nohighnibble
   mov bl,al
   shr bl,4
   and al,0F0h
   or al,bl
.nohighnibble
   test byte[SfxPOR],08h
   jnz .preserveupper
   cmp [SfxCOLR],al
   je .nocolchange
   mov [SfxCOLR],al
   and eax,0FFh
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
.nocolchange
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret
.preserveupper
   mov bl,[SfxCOLR]
   and al,0Fh
   and bl,0F0h
   or al,bl
   cmp [SfxCOLR],al
   je .nocolchange
   mov [SfxCOLR],al
   and eax,0FFh
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
   CLRFLAGS
   inc ebp                ; Increase program counter
   ret

NEWSYM FxOpDFA2    ; RAMB      set current RAM bank    ; Verified
   mov eax,[esi]            ; Read Source
   mov ebx,[SfxnRamBanks]
   FETCHPIPE
   dec ebx
   and eax,ebx
   mov dword [SfxRAMBR],eax
   shl eax,16
   add eax,[sfxramdata]
   mov dword [SfxRAMMem],eax
   CLRFLAGS
   inc ebp
   ret

NEWSYM FxOpDFA3    ; ROMB      set current ROM bank    ; Verified
   mov eax,[esi]            ; Read Source
   and eax,07Fh
   FETCHPIPE
   mov dword [SfxROMBR],eax
   mov eax,[SfxMemTable+eax*4]
   mov [SfxCROM],eax
   CLRFLAGS
   inc ebp
   ret

NEWSYM FxOpE0      ; DEC rn    decrement by one
   DECRN 0
NEWSYM FxOpE1      ; DEC rn    decrement by one
   DECRN 1
NEWSYM FxOpE2      ; DEC rn    decrement by one
   DECRN 2
NEWSYM FxOpE3      ; DEC rn    decrement by one
   DECRN 3
NEWSYM FxOpE4      ; DEC rn    decrement by one
   DECRN 4
NEWSYM FxOpE5      ; DEC rn    decrement by one
   DECRN 5
NEWSYM FxOpE6      ; DEC rn    decrement by one
   DECRN 6
NEWSYM FxOpE7      ; DEC rn    decrement by one
   DECRN 7
NEWSYM FxOpE8      ; DEC rn    decrement by one
   DECRN 8
NEWSYM FxOpE9      ; DEC rn    decrement by one
   DECRN 9
NEWSYM FxOpEA      ; DEC rn    decrement by one
   DECRN 10
NEWSYM FxOpEB      ; DEC rn    decrement by one
   DECRN 11
NEWSYM FxOpEC      ; DEC rn    decrement by one
   DECRN 12
NEWSYM FxOpED      ; DEC rn    decrement by one
   DECRN 13
NEWSYM FxOpEE      ; DEC rn    decrement by one
   dec word[SfxR0+14*4]
   FETCHPIPE
   mov eax,[SfxR0+14*4]            ; Read Source
   mov [SfxSignZero],eax
   UpdateR14
   CLRFLAGS
   inc ebp
   ret

NEWSYM FxOpEF      ; getb      get byte from ROM at address R14        ; V
   FETCHPIPE
   mov eax,[SfxRomBuffer]
   inc ebp
   mov eax,[eax]
   and eax,0FFh
;   cmp edi,SfxR15
;   je .nor15
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   ret
.nor15
;   mov eax,ebp
;   sub eax,[SfxCPB]
;   mov [SfxR15],al
   or eax,8000h
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   ret

NEWSYM FxOpEFA1    ; getbh     get high-byte from ROM at address R14   ; V
   mov eax,[esi]            ; Read Source
   mov ebx,[SfxRomBuffer]
   and eax,0FFh
   FETCHPIPE
   mov ah,[ebx]
   inc ebp
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   ret

NEWSYM FxOpEFA2    ; getbl     get low-byte from ROM at address R14    ; V
   mov eax,[esi]            ; Read Source
   mov ebx,[SfxRomBuffer]
   and eax,0FF00h
   FETCHPIPE
   mov al,[ebx]
   inc ebp
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   ret

NEWSYM FxOpEFA3    ; getbs     get sign extended byte from ROM at address R14  ; V
   mov ebx,[SfxRomBuffer]
   FETCHPIPE
   movsx eax,byte[ebx]
   inc ebp
   mov [edi],ax            ; Write DREG
   CLRFLAGS
   ret

NEWSYM FxOpF0      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 0
NEWSYM FxOpF1      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 1
NEWSYM FxOpF2      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 2
NEWSYM FxOpF3      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 3
NEWSYM FxOpF4      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 4
NEWSYM FxOpF5      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 5
NEWSYM FxOpF6      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 6
NEWSYM FxOpF7      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 7
NEWSYM FxOpF8      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 8
NEWSYM FxOpF9      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 9
NEWSYM FxOpFA      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 10
NEWSYM FxOpFB      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 11
NEWSYM FxOpFC      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 12
NEWSYM FxOpFD      ; IWT RN,#xx   immediate word transfer to register
   IWTRN 13
NEWSYM FxOpFE      ; IWT RN,#xx   immediate word transfer to register
   mov eax,[ebp]
   mov cl,[ebp+2]
   and eax,0FFFFh
   add ebp,3
   mov [SfxR0+14*4],eax
   UpdateR14
   CLRFLAGS
   ret
NEWSYM FxOpFF      ; IWT RN,#xx   immediate word transfer to register
   mov eax,[ebp]
   mov cl,[ebp+2]
   and eax,0FFFFh
   mov ebp,[SfxCPB]
   add ebp,eax
   CLRFLAGS
   ret

NEWSYM FxOpF0A1    ; LM RN,(XX)   load word from RAM
   LMRN 0
NEWSYM FxOpF1A1    ; LM RN,(XX)   load word from RAM
   LMRN 1
NEWSYM FxOpF2A1    ; LM RN,(XX)   load word from RAM
   LMRN 2
NEWSYM FxOpF3A1    ; LM RN,(XX)   load word from RAM
   LMRN 3
NEWSYM FxOpF4A1    ; LM RN,(XX)   load word from RAM
   LMRN 4
NEWSYM FxOpF5A1    ; LM RN,(XX)   load word from RAM
   LMRN 5
NEWSYM FxOpF6A1    ; LM RN,(XX)   load word from RAM
   LMRN 6
NEWSYM FxOpF7A1    ; LM RN,(XX)   load word from RAM
   LMRN 7
NEWSYM FxOpF8A1    ; LM RN,(XX)   load word from RAM
   LMRN 8
NEWSYM FxOpF9A1    ; LM RN,(XX)   load word from RAM
   LMRN 9
NEWSYM FxOpFAA1    ; LM RN,(XX)   load word from RAM
   LMRN 10
NEWSYM FxOpFBA1    ; LM RN,(XX)   load word from RAM
   LMRN 11
NEWSYM FxOpFCA1    ; LM RN,(XX)   load word from RAM
   LMRN 12
NEWSYM FxOpFDA1    ; LM RN,(XX)   load word from RAM
   LMRN 13
NEWSYM FxOpFEA1    ; LM RN,(XX)   load word from RAM
   xor eax,eax
   mov cl,[ebp+2]
   mov ax,[ebp]
   mov ebx,[SfxRAMMem]
   mov [SfxLastRamAdr],eax
   add [SfxLastRamAdr],ebx
   mov dl,[eax+ebx]
   xor eax,1
   add ebp,3
   mov dh,[eax+ebx]
   mov word [SfxR0+14*4],dx         ; Store Word
   UpdateR14
   CLRFLAGS
   ret
NEWSYM FxOpFFA1    ; LM RN,(XX)   load word from RAM
   FETCHPIPE
   mov eax,ecx
   inc ebp
   FETCHPIPE
   inc ebp
   mov ah,cl
   FETCHPIPE
   mov ebx,[SfxRAMMem]
   mov [SfxLastRamAdr],eax
   add [SfxLastRamAdr],ebx
   mov dl,[eax+ebx]
   xor eax,1
   mov dh,[eax+ebx]
   and edx,0FFFFh
   mov ebp,[SfxCPB]
   add ebp,edx
   CLRFLAGS
   ret

NEWSYM FxOpF0A2    ; SM (XX),RN   store word in RAM
   SMRN 0
NEWSYM FxOpF1A2    ; SM (XX),RN   store word in RAM
   SMRN 1
NEWSYM FxOpF2A2    ; SM (XX),RN   store word in RAM
   SMRN 2
NEWSYM FxOpF3A2    ; SM (XX),RN   store word in RAM
   SMRN 3
NEWSYM FxOpF4A2    ; SM (XX),RN   store word in RAM
   SMRN 4
NEWSYM FxOpF5A2    ; SM (XX),RN   store word in RAM
   SMRN 5
NEWSYM FxOpF6A2    ; SM (XX),RN   store word in RAM
   SMRN 6
NEWSYM FxOpF7A2    ; SM (XX),RN   store word in RAM
   SMRN 7
NEWSYM FxOpF8A2    ; SM (XX),RN   store word in RAM
   SMRN 8
NEWSYM FxOpF9A2    ; SM (XX),RN   store word in RAM
   SMRN 9
NEWSYM FxOpFAA2    ; SM (XX),RN   store word in RAM
   SMRN 10
NEWSYM FxOpFBA2    ; SM (XX),RN   store word in RAM
   SMRN 11
NEWSYM FxOpFCA2    ; SM (XX),RN   store word in RAM
   SMRN 12
NEWSYM FxOpFDA2    ; SM (XX),RN   store word in RAM
   SMRN 13
NEWSYM FxOpFEA2    ; SM (XX),RN   store word in RAM
   SMRN 14
NEWSYM FxOpFFA2    ; SM (XX),RN   store word in RAM
   FETCHPIPE
   mov ebx,ebp
   sub ebx,[SfxCPB]
   mov eax,ecx
   inc ebp
   FETCHPIPE
   inc ebp
   mov ah,cl
   FETCHPIPE
   mov dx,bx
   mov ebx,[SfxRAMMem]
   mov [SfxLastRamAdr],eax
   add [SfxLastRamAdr],ebx
   mov [eax+ebx],dl
   xor eax,1
   inc ebp
   mov [eax+ebx],dh
   CLRFLAGS
   ret

SECTION .bss ;ALIGN=32

NEWSYM NumberOfOpcodes, resd 1    ; Number of opcodes to execute
NEWSYM NumberOfOpcodesBU, resd 1  ; Number of opcodes to execute backup value
NEWSYM sfxwarningb, resb 1

SECTION .text

NEWSYM MainLoop
   mov eax,[SfxPBR]
   and eax,0FFh
;   mov byte[fxtrace+eax],1
   mov ebp,[SfxCPB]
   add ebp,[SfxR15]
   xor ecx,ecx
   mov cl,[SfxPIPE]
   mov ch,[SfxSFR+1]
   and ch,03h
   ; pack esi/edi
   PackEsiEdi
   jmp [FxTabled+ecx*4]
   jmp .LoopAgain
ALIGN16
.LoopAgain
   call [FxTable+ecx*4]
   dec dword [NumberOfOpcodes]
   jnz .LoopAgain
.EndLoop
NEWSYM FXEndLoop
   sub ebp,[SfxCPB]
   mov [SfxR15],ebp
   mov [SfxPIPE],cl
   and byte[SfxSFR+1],0FFh-03h
   or [SfxSFR+1],ch
   UnPackEsiEdi
   ret

SECTION .data
NEWSYM fxtrace, db 0; times 65536 db 0
SECTION .text

NEWSYM FxEmu2AsmEnd
