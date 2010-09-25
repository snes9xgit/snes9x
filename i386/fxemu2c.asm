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

EXTSYM FXEndLoop,FlushCache,FxOp02,FxTable,FxTableb,FxTablec,FxTabled
EXTSYM NumberOfOpcodes,SfxB,SfxCBR,SfxCFGR,SfxCOLR,SfxCPB,SfxCROM
EXTSYM SfxCacheActive,SfxCarry,SfxLastRamAdr,SfxMemTable,SfxOverflow
EXTSYM SfxPBR,SfxPIPE,SfxPOR,SfxR0,SfxR1,SfxR11,SfxR12,SfxR13,SfxR14
EXTSYM SfxR15,SfxR2,SfxR4,SfxR6,SfxR7,SfxR8,SfxRAMBR,SfxRAMMem,SfxROMBR
EXTSYM SfxRomBuffer,SfxSCBR,SfxSCMR,SfxSFR,SfxSignZero,SfxnRamBanks,flagnz
EXTSYM sfx128lineloc,sfx160lineloc,sfx192lineloc,sfxobjlineloc,sfxramdata
EXTSYM withr15sk,sfxclineloc,SCBRrel,ChangeOps
EXTSYM fxbit01pcal,fxbit23pcal,fxbit45pcal,fxbit67pcal
EXTSYM fxbit01,fxbit23,fxbit45,fxbit67
EXTSYM fxxand
EXTSYM PLOTJmpa,PLOTJmpb

NEWSYM FxEmu2CAsmStart

%include "fxemu2.mac"
%include "fxemu2b.mac"
%include "fxemu2c.mac"




SECTION .text ;ALIGN=32

ALIGN32
NEWSYM FxOpd00      ; STOP   stop GSU execution (and maybe generate an IRQ)     ; Verified.
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
   jmp FXEndLoop
   FXReturn

NEWSYM FxOpd01      ; NOP    no operation       ; Verified.
   FETCHPIPE
   CLRFLAGS
   inc ebp                ; Increase program counter
   FXReturn

NEWSYM FxOpd02      ; CACHE  reintialize GSU cache
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
   FXReturn

NEWSYM FxOpd03      ; LSR    logic shift right  ; Verified.
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   mov [SfxCarry],al
   and byte[SfxCarry],1
   shr ax,1                      ; logic shift right
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd04      ; ROL    rotate left (RCL?) ; V
   shr byte[SfxCarry],1
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   rcl ax,1
   rcl byte[SfxCarry],1
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd05      ; BRA    branch always      ; Verified.
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   inc ebp
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd06      ; BGE    branch on greater or equals        ; Verified.
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
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd07      ; BLT    branch on lesss than       ; Verified.
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
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd08      ; BNE    branch on not equal        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd09      ; BEQ    branch on equal (z=1)      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd0A      ; BPL    branch on plus     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd0B      ; BMI    branch on minus    ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd0C      ; BCC    branch on carry clear      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd0D      ; BCS    branch on carry set        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd0E      ; BVC    branch on overflow clear   ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd0F      ; BVS    branch on overflow set     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTable+ecx*4]
   FXReturn2
.nojump
   inc ebp
   call [FxTable+ecx*4]
   FXReturn2

NEWSYM FxOpd10      ; TO RN  set register n as destination register
   TORNd 0
NEWSYM FxOpd11      ; TO RN  set register n as destination register
   TORNd 1
NEWSYM FxOpd12      ; TO RN  set register n as destination register
   TORNd 2
NEWSYM FxOpd13      ; TO RN  set register n as destination register
   TORNd 3
NEWSYM FxOpd14      ; TO RN  set register n as destination register
   TORNd 4
NEWSYM FxOpd15      ; TO RN  set register n as destination register
   TORNd 5
NEWSYM FxOpd16      ; TO RN  set register n as destination register
   TORNd 6
NEWSYM FxOpd17      ; TO RN  set register n as destination register
   TORNd 7
NEWSYM FxOpd18      ; TO RN  set register n as destination register
   TORNd 8
NEWSYM FxOpd19      ; TO RN  set register n as destination register
   TORNd 9
NEWSYM FxOpd1A      ; TO RN  set register n as destination register
   TORNd 10
NEWSYM FxOpd1B      ; TO RN  set register n as destination register
   TORNd 11
NEWSYM FxOpd1C      ; TO RN  set register n as destination register
   TORNd 12
NEWSYM FxOpd1D      ; TO RN  set register n as destination register
   TORNd 13
NEWSYM FxOpd1E      ; TO RN  set register n as destination register
   FETCHPIPE
   mov edi,SfxR0+14*4
   inc ebp
   call [FxTable+ecx*4]
   mov edi,SfxR0
   UpdateR14
   FXReturn
NEWSYM FxOpd1F      ; TO RN  set register n as destination register
   FETCHPIPE
   mov edi,SfxR0+15*4
   inc ebp
   call [FxTable+ecx*4]
   mov ebp,[SfxCPB]
   add ebp,[SfxR15]
   mov edi,SfxR0
   FXReturn

NEWSYM FxOpd20      ; WITH  set register n as source and destination register
   WITHc 0
NEWSYM FxOpd21      ; WITH  set register n as source and destination register
   WITHc 1
NEWSYM FxOpd22      ; WITH  set register n as source and destination register
   WITHc 2
NEWSYM FxOpd23      ; WITH  set register n as source and destination register
   WITHc 3
NEWSYM FxOpd24      ; WITH  set register n as source and destination register
   WITHc 4
NEWSYM FxOpd25      ; WITH  set register n as source and destination register
   WITHc 5
NEWSYM FxOpd26      ; WITH  set register n as source and destination register
   WITHc 6
NEWSYM FxOpd27      ; WITH  set register n as source and destination register
   WITHc 7
NEWSYM FxOpd28      ; WITH  set register n as source and destination register
   WITHc 8
NEWSYM FxOpd29      ; WITH  set register n as source and destination register
   WITHc 9
NEWSYM FxOpd2A      ; WITH  set register n as source and destination register
   WITHc 10
NEWSYM FxOpd2B      ; WITH  set register n as source and destination register
   WITHc 11
NEWSYM FxOpd2C      ; WITH  set register n as source and destination register
   WITHc 12
NEWSYM FxOpd2D      ; WITH  set register n as source and destination register
   WITHc 13
NEWSYM FxOpd2E      ; WITH  set register n as source and destination register
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
   FXReturn
NEWSYM FxOpd2F      ; WITH  set register n as source and destination register
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
   FXReturn

NEWSYM FxOpd30      ; STW RN store word
   STWRNc 0
NEWSYM FxOpd31      ; STW RN store word
   STWRNc 1
NEWSYM FxOpd32      ; STW RN store word
   STWRNc 2
NEWSYM FxOpd33      ; STW RN store word
   STWRNc 3
NEWSYM FxOpd34      ; STW RN store word
   STWRNc 4
NEWSYM FxOpd35      ; STW RN store word
   STWRNc 5
NEWSYM FxOpd36      ; STW RN store word
   STWRNc 6
NEWSYM FxOpd37      ; STW RN store word
   STWRNc 7
NEWSYM FxOpd38      ; STW RN store word
   STWRNc 8
NEWSYM FxOpd39      ; STW RN store word
   STWRNc 9
NEWSYM FxOpd3A      ; STW RN store word
   STWRNc 10
NEWSYM FxOpd3B      ; STW RN store word
   STWRNc 11

NEWSYM FxOpd30A1    ; STB RN store byte
   STBRNc 0
NEWSYM FxOpd31A1    ; STB RN store byte
   STBRNc 1
NEWSYM FxOpd32A1    ; STB RN store byte
   STBRNc 2
NEWSYM FxOpd33A1    ; STB RN store byte
   STBRNc 3
NEWSYM FxOpd34A1    ; STB RN store byte
   STBRNc 4
NEWSYM FxOpd35A1    ; STB RN store byte
   STBRNc 5
NEWSYM FxOpd36A1    ; STB RN store byte
   STBRNc 6
NEWSYM FxOpd37A1    ; STB RN store byte
   STBRNc 7
NEWSYM FxOpd38A1    ; STB RN store byte
   STBRNc 8
NEWSYM FxOpd39A1    ; STB RN store byte
   STBRNc 9
NEWSYM FxOpd3AA1    ; STB RN store byte
   STBRNc 10
NEWSYM FxOpd3BA1    ; STB RN store byte
   STBRNc 11

NEWSYM FxOpd3C      ; LOOP   decrement loop counter, and branch on not zero ; V
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
   FXReturn
.NoBranch
   inc ebp
   CLRFLAGS
   FXReturn

NEWSYM FxOpd3D      ; ALT1   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,01h
   inc ebp
   call [FxTable+ecx*4]
   xor ch,ch
   FXReturn

NEWSYM FxOpd3E      ; ALT2   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,02h
   inc ebp
   call [FxTable+ecx*4]
   xor ch,ch
   FXReturn

NEWSYM FxOpd3F      ; ALT3   set alt3 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,03h
   inc ebp
   call [FxTable+ecx*4]
   xor ch,ch
   FXReturn

NEWSYM FxOpd40      ; LDW RN load word from RAM
   LDWRNc 0
NEWSYM FxOpd41      ; LDW RN load word from RAM
   LDWRNc 1
NEWSYM FxOpd42      ; LDW RN load word from RAM
   LDWRNc 2
NEWSYM FxOpd43      ; LDW RN load word from RAM
   LDWRNc 3
NEWSYM FxOpd44      ; LDW RN load word from RAM
   LDWRNc 4
NEWSYM FxOpd45      ; LDW RN load word from RAM
   LDWRNc 5
NEWSYM FxOpd46      ; LDW RN load word from RAM
   LDWRNc 6
NEWSYM FxOpd47      ; LDW RN load word from RAM
   LDWRNc 7
NEWSYM FxOpd48      ; LDW RN load word from RAM
   LDWRNc 8
NEWSYM FxOpd49      ; LDW RN load word from RAM
   LDWRNc 9
NEWSYM FxOpd4A      ; LDW RN load word from RAM
   LDWRNc 10
NEWSYM FxOpd4B      ; LDW RN load word from RAM
   LDWRNc 11

NEWSYM FxOpd40A1    ; LDB RN load byte from RAM
   LDBRNc 0
NEWSYM FxOpd41A1    ; LDB RN load byte from RAM
   LDBRNc 1
NEWSYM FxOpd42A1    ; LDB RN load byte from RAM
   LDBRNc 2
NEWSYM FxOpd43A1    ; LDB RN load byte from RAM
   LDBRNc 3
NEWSYM FxOpd44A1    ; LDB RN load byte from RAM
   LDBRNc 4
NEWSYM FxOpd45A1    ; LDB RN load byte from RAM
   LDBRNc 5
NEWSYM FxOpd46A1    ; LDB RN load byte from RAM
   LDBRNc 6
NEWSYM FxOpd47A1    ; LDB RN load byte from RAM
   LDBRNc 7
NEWSYM FxOpd48A1    ; LDB RN load byte from RAM
   LDBRNc 8
NEWSYM FxOpd49A1    ; LDB RN load byte from RAM
   LDBRNc 9
NEWSYM FxOpd4AA1    ; LDB RN load byte from RAM
   LDBRNc 10
NEWSYM FxOpd4BA1    ; LDB RN load byte from RAM
   LDBRNc 11


NEWSYM FxOpd4C1284b       ; PLOT 4bit
   plotlines4bb plotb
NEWSYM FxOpd4C1284bz      ; PLOT 4bit, zero check
   plotlines4bb plotbz
NEWSYM FxOpd4C1284bd      ; PLOT 4bit, dither
   plotlines4bb plotbd
NEWSYM FxOpd4C1284bzd     ; PLOT 4bit, zero check + dither
   plotlines4bb plotbzd

NEWSYM FxOpd4C1282b       ; PLOT 2bit
   plotlines2bb plotb
NEWSYM FxOpd4C1282bz      ; PLOT 2bit, zero check
   plotlines2bb plotbz
NEWSYM FxOpd4C1282bd      ; PLOT 2bit, dither
   plotlines2bb plotbd
NEWSYM FxOpd4C1282bzd     ; PLOT 2bit, zero check + dither
   plotlines2bb plotbzd

NEWSYM FxOpd4C1288b       ; PLOT 8bit
   plotlines8bb plotb
NEWSYM FxOpd4C1288bz      ; PLOT 8bit, zero check
   plotlines8bb plotbz
NEWSYM FxOpd4C1288bd      ; PLOT 8bit, dither
   plotlines8bb plotb
NEWSYM FxOpd4C1288bzd     ; PLOT 8bit, zero check + dither
   plotlines8bb plotbz

NEWSYM FxOpd4C1288bl       ; PLOT 8bit
   plotlines8bbl plotb
NEWSYM FxOpd4C1288bzl      ; PLOT 8bit, zero check
   plotlines8bbl plotbz
NEWSYM FxOpd4C1288bdl      ; PLOT 8bit, dither
   plotlines8bbl plotb
NEWSYM FxOpd4C1288bzdl     ; PLOT 8bit, zero check + dither
   plotlines8bbl plotbz

NEWSYM FxOpd4C      ; PLOT   plot pixel with R1,R2 as x,y and the color register as the color
   jmp FxOpd4C1284b

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
   FXReturn

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
   FXReturn

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
   FXReturn

SECTION .bss
.prevx resw 1
.prevy resw 1

SECTION .text

NEWSYM FxOpd4CA1    ; RPIX   read color of the pixel with R1,R2 as x,y
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
   FXReturn

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
   FXReturn

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
   FXReturn

NEWSYM FxOpd4D      ; SWAP   swap upper and lower byte of a register    ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   ror ax,8
   inc ebp                ; Increase program counter
   mov [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   FXReturn

NEWSYM FxOpd4E      ; COLOR  copy source register to color register     ; V
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
   FXReturn
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
   FXReturn

NEWSYM FxOpd4EA1    ; CMODE  set plot option register ; V
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
   FXReturn

NEWSYM FxOpd4F      ; NOT    perform exclusive exor with 1 on all bits  ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   xor eax,0FFFFh
   inc ebp                ; Increase program counter
   mov [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   FXReturn

NEWSYM FxOpd50      ; ADD RN add, register + register
   ADDRNc 0
NEWSYM FxOpd51      ; ADD RN add, register + register
   ADDRNc 1
NEWSYM FxOpd52      ; ADD RN add, register + register
   ADDRNc 2
NEWSYM FxOpd53      ; ADD RN add, register + register
   ADDRNc 3
NEWSYM FxOpd54      ; ADD RN add, register + register
   ADDRNc 4
NEWSYM FxOpd55      ; ADD RN add, register + register
   ADDRNc 5
NEWSYM FxOpd56      ; ADD RN add, register + register
   ADDRNc 6
NEWSYM FxOpd57      ; ADD RN add, register + register
   ADDRNc 7
NEWSYM FxOpd58      ; ADD RN add, register + register
   ADDRNc 8
NEWSYM FxOpd59      ; ADD RN add, register + register
   ADDRNc 9
NEWSYM FxOpd5A      ; ADD RN add, register + register
   ADDRNc 10
NEWSYM FxOpd5B      ; ADD RN add, register + register
   ADDRNc 11
NEWSYM FxOpd5C      ; ADD RN add, register + register
   ADDRNc 12
NEWSYM FxOpd5D      ; ADD RN add, register + register
   ADDRNc 13
NEWSYM FxOpd5E      ; ADD RN add, register + register
   ADDRNc 14
NEWSYM FxOpd5F      ; ADD RN add, register + register
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
   FXReturn

NEWSYM FxOpd50A1    ; ADC RN add with carry, register + register
   ADCRNc 0
NEWSYM FxOpd51A1    ; ADC RN add with carry, register + register
   ADCRNc 1
NEWSYM FxOpd52A1    ; ADC RN add with carry, register + register
   ADCRNc 2
NEWSYM FxOpd53A1    ; ADC RN add with carry, register + register
   ADCRNc 3
NEWSYM FxOpd54A1    ; ADC RN add with carry, register + register
   ADCRNc 4
NEWSYM FxOpd55A1    ; ADC RN add with carry, register + register
   ADCRNc 5
NEWSYM FxOpd56A1    ; ADC RN add with carry, register + register
   ADCRNc 6
NEWSYM FxOpd57A1    ; ADC RN add with carry, register + register
   ADCRNc 7
NEWSYM FxOpd58A1    ; ADC RN add with carry, register + register
   ADCRNc 8
NEWSYM FxOpd59A1    ; ADC RN add with carry, register + register
   ADCRNc 9
NEWSYM FxOpd5AA1    ; ADC RN add with carry, register + register
   ADCRNc 10
NEWSYM FxOpd5BA1    ; ADC RN add with carry, register + register
   ADCRNc 11
NEWSYM FxOpd5CA1    ; ADC RN add with carry, register + register
   ADCRNc 12
NEWSYM FxOpd5DA1    ; ADC RN add with carry, register + register
   ADCRNc 13
NEWSYM FxOpd5EA1    ; ADC RN add with carry, register + register
   ADCRNc 14
NEWSYM FxOpd5FA1    ; ADC RN add with carry, register + register
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
   FXReturn

; Weird Opdode (FxOpd50A2, add 0, wow!)
NEWSYM FxOpd50A2    ; ADI RN add, register + immediate
   ADIRNc 0
NEWSYM FxOpd51A2    ; ADI RN add, register + immediate
   ADIRNc 1
NEWSYM FxOpd52A2    ; ADI RN add, register + immediate
   ADIRNc 2
NEWSYM FxOpd53A2    ; ADI RN add, register + immediate
   ADIRNc 3
NEWSYM FxOpd54A2    ; ADI RN add, register + immediate
   ADIRNc 4
NEWSYM FxOpd55A2    ; ADI RN add, register + immediate
   ADIRNc 5
NEWSYM FxOpd56A2    ; ADI RN add, register + immediate
   ADIRNc 6
NEWSYM FxOpd57A2    ; ADI RN add, register + immediate
   ADIRNc 7
NEWSYM FxOpd58A2    ; ADI RN add, register + immediate
   ADIRNc 8
NEWSYM FxOpd59A2    ; ADI RN add, register + immediate
   ADIRNc 9
NEWSYM FxOpd5AA2    ; ADI RN add, register + immediate
   ADIRNc 10
NEWSYM FxOpd5BA2    ; ADI RN add, register + immediate
   ADIRNc 11
NEWSYM FxOpd5CA2    ; ADI RN add, register + immediate
   ADIRNc 12
NEWSYM FxOpd5DA2    ; ADI RN add, register + immediate
   ADIRNc 13
NEWSYM FxOpd5EA2    ; ADI RN add, register + immediate
   ADIRNc 14
NEWSYM FxOpd5FA2    ; ADI RN add, register + immediate
   ADIRNc 15

; Another very useful Opdode
NEWSYM FxOpd50A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 0
NEWSYM FxOpd51A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 1
NEWSYM FxOpd52A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 2
NEWSYM FxOpd53A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 3
NEWSYM FxOpd54A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 4
NEWSYM FxOpd55A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 5
NEWSYM FxOpd56A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 6
NEWSYM FxOpd57A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 7
NEWSYM FxOpd58A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 8
NEWSYM FxOpd59A3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 9
NEWSYM FxOpd5AA3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 10
NEWSYM FxOpd5BA3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 11
NEWSYM FxOpd5CA3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 12
NEWSYM FxOpd5DA3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 13
NEWSYM FxOpd5EA3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 14
NEWSYM FxOpd5FA3    ; ADCIRN add with carry, register + immediate
   ADCIRNc 15

NEWSYM FxOpd60      ; SUBRN  subtract, register - register
   SUBRNc 0
NEWSYM FxOpd61      ; SUBRN  subtract, register - register
   SUBRNc 1
NEWSYM FxOpd62      ; SUBRN  subtract, register - register
   SUBRNc 2
NEWSYM FxOpd63      ; SUBRN  subtract, register - register
   SUBRNc 3
NEWSYM FxOpd64      ; SUBRN  subtract, register - register
   SUBRNc 4
NEWSYM FxOpd65      ; SUBRN  subtract, register - register
   SUBRNc 5
NEWSYM FxOpd66      ; SUBRN  subtract, register - register
   SUBRNc 6
NEWSYM FxOpd67      ; SUBRN  subtract, register - register
   SUBRNc 7
NEWSYM FxOpd68      ; SUBRN  subtract, register - register
   SUBRNc 8
NEWSYM FxOpd69      ; SUBRN  subtract, register - register
   SUBRNc 9
NEWSYM FxOpd6A      ; SUBRN  subtract, register - register
   SUBRNc 10
NEWSYM FxOpd6B      ; SUBRN  subtract, register - register
   SUBRNc 11
NEWSYM FxOpd6C      ; SUBRN  subtract, register - register
   SUBRNc 12
NEWSYM FxOpd6D      ; SUBRN  subtract, register - register
   SUBRNc 13
NEWSYM FxOpd6E      ; SUBRN  subtract, register - register
   SUBRNc 14
NEWSYM FxOpd6F      ; SUBRN  subtract, register - register
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
   FXReturn

NEWSYM FxOpd60A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 0
NEWSYM FxOpd61A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 1
NEWSYM FxOpd62A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 2
NEWSYM FxOpd63A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 3
NEWSYM FxOpd64A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 4
NEWSYM FxOpd65A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 5
NEWSYM FxOpd66A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 6
NEWSYM FxOpd67A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 7
NEWSYM FxOpd68A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 8
NEWSYM FxOpd69A1    ; SBCRN  subtract with carry, register - register
   SBCRNc 9
NEWSYM FxOpd6AA1    ; SBCRN  subtract with carry, register - register
   SBCRNc 10
NEWSYM FxOpd6BA1    ; SBCRN  subtract with carry, register - register
   SBCRNc 11
NEWSYM FxOpd6CA1    ; SBCRN  subtract with carry, register - register
   SBCRNc 12
NEWSYM FxOpd6DA1    ; SBCRN  subtract with carry, register - register
   SBCRNc 13
NEWSYM FxOpd6EA1    ; SBCRN  subtract with carry, register - register
   SBCRNc 14
NEWSYM FxOpd6FA1    ; SBCRN  subtract with carry, register - register
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
   FXReturn

NEWSYM FxOpd60A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 0
NEWSYM FxOpd61A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 1
NEWSYM FxOpd62A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 2
NEWSYM FxOpd63A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 3
NEWSYM FxOpd64A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 4
NEWSYM FxOpd65A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 5
NEWSYM FxOpd66A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 6
NEWSYM FxOpd67A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 7
NEWSYM FxOpd68A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 8
NEWSYM FxOpd69A2    ; SUBIRN subtract, register - immediate
   SUBIRNc 9
NEWSYM FxOpd6AA2    ; SUBIRN subtract, register - immediate
   SUBIRNc 10
NEWSYM FxOpd6BA2    ; SUBIRN subtract, register - immediate
   SUBIRNc 11
NEWSYM FxOpd6CA2    ; SUBIRN subtract, register - immediate
   SUBIRNc 12
NEWSYM FxOpd6DA2    ; SUBIRN subtract, register - immediate
   SUBIRNc 13
NEWSYM FxOpd6EA2    ; SUBIRN subtract, register - immediate
   SUBIRNc 14
NEWSYM FxOpd6FA2    ; SUBIRN subtract, register - immediate
   SUBIRNc 15

NEWSYM FxOpd60A3    ; CMPRN  compare, register, register
   CMPRNc 0
NEWSYM FxOpd61A3    ; CMPRN  compare, register, register
   CMPRNc 1
NEWSYM FxOpd62A3    ; CMPRN  compare, register, register
   CMPRNc 2
NEWSYM FxOpd63A3    ; CMPRN  compare, register, register
   CMPRNc 3
NEWSYM FxOpd64A3    ; CMPRN  compare, register, register
   CMPRNc 4
NEWSYM FxOpd65A3    ; CMPRN  compare, register, register
   CMPRNc 5
NEWSYM FxOpd66A3    ; CMPRN  compare, register, register
   CMPRNc 6
NEWSYM FxOpd67A3    ; CMPRN  compare, register, register
   CMPRNc 7
NEWSYM FxOpd68A3    ; CMPRN  compare, register, register
   CMPRNc 8
NEWSYM FxOpd69A3    ; CMPRN  compare, register, register
   CMPRNc 9
NEWSYM FxOpd6AA3    ; CMPRN  compare, register, register
   CMPRNc 10
NEWSYM FxOpd6BA3    ; CMPRN  compare, register, register
   CMPRNc 11
NEWSYM FxOpd6CA3    ; CMPRN  compare, register, register
   CMPRNc 12
NEWSYM FxOpd6DA3    ; CMPRN  compare, register, register
   CMPRNc 13
NEWSYM FxOpd6EA3    ; CMPRN  compare, register, register
   CMPRNc 14
NEWSYM FxOpd6FA3    ; CMPRN  compare, register, register
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
   FXReturn

NEWSYM FxOpd70      ; MERGE  R7 as upper byte, R8 as lower byte (used for texture-mapping) */
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
   FXReturn

NEWSYM FxOpd71      ; AND RN register & register
   ANDRNc 1
NEWSYM FxOpd72      ; AND RN register & register
   ANDRNc 2
NEWSYM FxOpd73      ; AND RN register & register
   ANDRNc 3
NEWSYM FxOpd74      ; AND RN register & register
   ANDRNc 4
NEWSYM FxOpd75      ; AND RN register & register
   ANDRNc 5
NEWSYM FxOpd76      ; AND RN register & register
   ANDRNc 6
NEWSYM FxOpd77      ; AND RN register & register
   ANDRNc 7
NEWSYM FxOpd78      ; AND RN register & register
   ANDRNc 8
NEWSYM FxOpd79      ; AND RN register & register
   ANDRNc 9
NEWSYM FxOpd7A      ; AND RN register & register
   ANDRNc 10
NEWSYM FxOpd7B      ; AND RN register & register
   ANDRNc 11
NEWSYM FxOpd7C      ; AND RN register & register
   ANDRNc 12
NEWSYM FxOpd7D      ; AND RN register & register
   ANDRNc 13
NEWSYM FxOpd7E      ; AND RN register & register
   ANDRNc 14
NEWSYM FxOpd7F      ; AND RN register & register
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   and eax,ebx
   inc ebp
   mov dword [SfxSignZero],eax
   mov [edi],eax            ; Write Destination
   CLRFLAGS
   FXReturn

NEWSYM FxOpd71A1    ; BIC RN register & ~register
   BICRNc 1
NEWSYM FxOpd72A1    ; BIC RN register & ~register
   BICRNc 2
NEWSYM FxOpd73A1    ; BIC RN register & ~register
   BICRNc 3
NEWSYM FxOpd74A1    ; BIC RN register & ~register
   BICRNc 4
NEWSYM FxOpd75A1    ; BIC RN register & ~register
   BICRNc 5
NEWSYM FxOpd76A1    ; BIC RN register & ~register
   BICRNc 6
NEWSYM FxOpd77A1    ; BIC RN register & ~register
   BICRNc 7
NEWSYM FxOpd78A1    ; BIC RN register & ~register
   BICRNc 8
NEWSYM FxOpd79A1    ; BIC RN register & ~register
   BICRNc 9
NEWSYM FxOpd7AA1    ; BIC RN register & ~register
   BICRNc 10
NEWSYM FxOpd7BA1    ; BIC RN register & ~register
   BICRNc 11
NEWSYM FxOpd7CA1    ; BIC RN register & ~register
   BICRNc 12
NEWSYM FxOpd7DA1    ; BIC RN register & ~register
   BICRNc 13
NEWSYM FxOpd7EA1    ; BIC RN register & ~register
   BICRNc 14
NEWSYM FxOpd7FA1    ; BIC RN register & ~register
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
   FXReturn

NEWSYM FxOpd71A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 1
NEWSYM FxOpd72A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 2
NEWSYM FxOpd73A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 3
NEWSYM FxOpd74A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 4
NEWSYM FxOpd75A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 5
NEWSYM FxOpd76A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 6
NEWSYM FxOpd77A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 7
NEWSYM FxOpd78A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 8
NEWSYM FxOpd79A2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 9
NEWSYM FxOpd7AA2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 10
NEWSYM FxOpd7BA2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 11
NEWSYM FxOpd7CA2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 12
NEWSYM FxOpd7DA2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 13
NEWSYM FxOpd7EA2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 14
NEWSYM FxOpd7FA2    ; ANDIRNc and #n - register & immediate
   ANDIRNc 15

NEWSYM FxOpd71A3    ; BICIRNc register & ~immediate
   BICIRNc 1 ^ 0FFFFh
NEWSYM FxOpd72A3    ; BICIRNc register & ~immediate
   BICIRNc 2 ^ 0FFFFh
NEWSYM FxOpd73A3    ; BICIRNc register & ~immediate
   BICIRNc 3 ^ 0FFFFh
NEWSYM FxOpd74A3    ; BICIRNc register & ~immediate
   BICIRNc 4 ^ 0FFFFh
NEWSYM FxOpd75A3    ; BICIRNc register & ~immediate
   BICIRNc 5 ^ 0FFFFh
NEWSYM FxOpd76A3    ; BICIRNc register & ~immediate
   BICIRNc 6 ^ 0FFFFh
NEWSYM FxOpd77A3    ; BICIRNc register & ~immediate
   BICIRNc 7 ^ 0FFFFh
NEWSYM FxOpd78A3    ; BICIRNc register & ~immediate
   BICIRNc 8 ^ 0FFFFh
NEWSYM FxOpd79A3    ; BICIRNc register & ~immediate
   BICIRNc 9 ^ 0FFFFh
NEWSYM FxOpd7AA3    ; BICIRNc register & ~immediate
   BICIRNc 10 ^ 0FFFFh
NEWSYM FxOpd7BA3    ; BICIRNc register & ~immediate
   BICIRNc 11 ^ 0FFFFh
NEWSYM FxOpd7CA3    ; BICIRNc register & ~immediate
   BICIRNc 12 ^ 0FFFFh
NEWSYM FxOpd7DA3    ; BICIRNc register & ~immediate
   BICIRNc 13 ^ 0FFFFh
NEWSYM FxOpd7EA3    ; BICIRNc register & ~immediate
   BICIRNc 14 ^ 0FFFFh
NEWSYM FxOpd7FA3    ; BICIRNc register & ~immediate
   BICIRNc 15 ^ 0FFFFh

NEWSYM FxOpd80      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 0
NEWSYM FxOpd81      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 1
NEWSYM FxOpd82      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 2
NEWSYM FxOpd83      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 3
NEWSYM FxOpd84      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 4
NEWSYM FxOpd85      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 5
NEWSYM FxOpd86      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 6
NEWSYM FxOpd87      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 7
NEWSYM FxOpd88      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 8
NEWSYM FxOpd89      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 9
NEWSYM FxOpd8A      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 10
NEWSYM FxOpd8B      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 11
NEWSYM FxOpd8C      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 12
NEWSYM FxOpd8D      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 13
NEWSYM FxOpd8E      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
   MULTRNc 14
NEWSYM FxOpd8F      ; MULTRNc 8 bit to 16 bit signed multiply, register * register
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
   FXReturn

NEWSYM FxOpd80A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 0
NEWSYM FxOpd81A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 1
NEWSYM FxOpd82A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 2
NEWSYM FxOpd83A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 3
NEWSYM FxOpd84A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 4
NEWSYM FxOpd85A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 5
NEWSYM FxOpd86A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 6
NEWSYM FxOpd87A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 7
NEWSYM FxOpd88A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 8
NEWSYM FxOpd89A1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 9
NEWSYM FxOpd8AA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 10
NEWSYM FxOpd8BA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 11
NEWSYM FxOpd8CA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 12
NEWSYM FxOpd8DA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 13
NEWSYM FxOpd8EA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
   UMULTRNc 14
NEWSYM FxOpd8FA1    ; UMULRN 8 bit to 16 bit unsigned multiply, register * register
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
   FXReturn

NEWSYM FxOpd80A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 0
NEWSYM FxOpd81A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 1
NEWSYM FxOpd82A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 2
NEWSYM FxOpd83A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 3
NEWSYM FxOpd84A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 4
NEWSYM FxOpd85A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 5
NEWSYM FxOpd86A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 6
NEWSYM FxOpd87A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 7
NEWSYM FxOpd88A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 8
NEWSYM FxOpd89A2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 9
NEWSYM FxOpd8AA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 10
NEWSYM FxOpd8BA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 11
NEWSYM FxOpd8CA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 12
NEWSYM FxOpd8DA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 13
NEWSYM FxOpd8EA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 14
NEWSYM FxOpd8FA2    ; MULIRN 8 bit to 16 bit signed multiply, register * immediate
   MULTIRNc 15

NEWSYM FxOpd80A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 0
NEWSYM FxOpd81A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 1
NEWSYM FxOpd82A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 2
NEWSYM FxOpd83A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 3
NEWSYM FxOpd84A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 4
NEWSYM FxOpd85A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 5
NEWSYM FxOpd86A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 6
NEWSYM FxOpd87A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 7
NEWSYM FxOpd88A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 8
NEWSYM FxOpd89A3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 9
NEWSYM FxOpd8AA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 10
NEWSYM FxOpd8BA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 11
NEWSYM FxOpd8CA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 12
NEWSYM FxOpd8DA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 13
NEWSYM FxOpd8EA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 14
NEWSYM FxOpd8FA3    ;UMULIRN 8 bit to 16 bit unsigned multiply, register * immediate
   UMULTIRNc 15

NEWSYM FxOpd90      ; SBK    store word to last accessed RAM address    ; V
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
   FXReturn

NEWSYM FxOpd91      ; LINKc#n R11 = R15 + immediate
   LINKc 1
NEWSYM FxOpd92      ; LINKc#n R11 = R15 + immediate
   LINKc 2
NEWSYM FxOpd93      ; LINKc#n R11 = R15 + immediate
   LINKc 3
NEWSYM FxOpd94      ; LINKc#n R11 = R15 + immediate
   LINKc 4

NEWSYM FxOpd95      ; SEX    sign extend 8 bit to 16 bit        ; V
   movsx eax, byte [esi]     ; Read Source
   FETCHPIPE
   and eax,0FFFFh
   inc ebp
   mov [edi],eax            ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd96      ; ASR    aritmethic shift right by one      ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   mov [SfxCarry],al
   and byte[SfxCarry],1
   sar ax,1                      ; logic shift right
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd96A1    ; DIV2   aritmethic shift right by one      ; V
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
   FXReturn
.minusone
   mov byte[SfxCarry],1
   xor eax,eax
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov dword [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd97      ; ROR    rotate right by one        ; V
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   shr byte[SfxCarry],1
   rcr ax,1
   setc byte[SfxCarry]
   inc ebp                ; Increase program counter
   mov [edi],eax            ; Write Destination
   mov [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd98      ; JMPRNc  jump to address of register
   JMPRNc 8
NEWSYM FxOpd99      ; JMPRNc  jump to address of register
   JMPRNc 9
NEWSYM FxOpd9A      ; JMPRNc  jump to address of register
   JMPRNc 10
NEWSYM FxOpd9B      ; JMPRNc  jump to address of register
   JMPRNc 11
NEWSYM FxOpd9C      ; JMPRNc  jump to address of register
   JMPRNc 12
NEWSYM FxOpd9D      ; JMPRNc  jump to address of register
   JMPRNc 13

NEWSYM FxOpd98A1    ; LJMPRNc set program bank to source register and jump to address of register
   LJMPRNc 8
NEWSYM FxOpd99A1    ; LJMPRNc set program bank to source register and jump to address of register
   LJMPRNc 9
NEWSYM FxOpd9AA1    ; LJMPRNc set program bank to source register and jump to address of register
   LJMPRNc 10
NEWSYM FxOpd9BA1    ; LJMPRNc set program bank to source register and jump to address of register
   LJMPRNc 11
NEWSYM FxOpd9CA1    ; LJMPRNc set program bank to source register and jump to address of register
   LJMPRNc 12
NEWSYM FxOpd9DA1    ; LJMPRNc set program bank to source register and jump to address of register
   LJMPRNc 13

NEWSYM FxOpd9E      ; LOB    set upper byte to zero (keep low byte) ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   and eax,0FFh
   inc ebp
   mov [edi],eax            ; Write Destination
   shl eax,8
   mov dword [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpd9F      ; FMULT  16 bit to 32 bit signed multiplication, upper 16 bits only
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
   FXReturn

NEWSYM FxOpd9FA1    ; LMULT  16 bit to 32 bit signed multiplication     ; V
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
   FXReturn

NEWSYM FxOpdA0      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 0
NEWSYM FxOpdA1      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 1
NEWSYM FxOpdA2      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 2
NEWSYM FxOpdA3      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 3
NEWSYM FxOpdA4      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 4
NEWSYM FxOpdA5      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 5
NEWSYM FxOpdA6      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 6
NEWSYM FxOpdA7      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 7
NEWSYM FxOpdA8      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 8
NEWSYM FxOpdA9      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 9
NEWSYM FxOpdAA      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 10
NEWSYM FxOpdAB      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 11
NEWSYM FxOpdAC      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 12
NEWSYM FxOpdAD      ; IBTRNc,#PP immediate byte transfer
   IBTRNc 13
NEWSYM FxOpdAE      ; IBTRNc,#PP immediate byte transfer
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   add ebp,2
   mov [SfxR0+14*4],ax
   UpdateR14
   CLRFLAGS
   FXReturn
NEWSYM FxOpdAF      ; IBTRNc,#PP immediate byte transfer
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   and eax,0FFFFh
   mov ebp,[SfxCPB]
   add ebp,eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpdA0A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 0
NEWSYM FxOpdA1A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 1
NEWSYM FxOpdA2A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 2
NEWSYM FxOpdA3A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 3
NEWSYM FxOpdA4A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 4
NEWSYM FxOpdA5A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 5
NEWSYM FxOpdA6A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 6
NEWSYM FxOpdA7A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 7
NEWSYM FxOpdA8A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 8
NEWSYM FxOpdA9A1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 9
NEWSYM FxOpdAAA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 10
NEWSYM FxOpdABA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 11
NEWSYM FxOpdACA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 12
NEWSYM FxOpdADA1    ; LMS rn,(yy)  load word from RAM (short address)
   LMSRNc 13
NEWSYM FxOpdAEA1    ; LMS rn,(yy)  load word from RAM (short address)
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
   FXReturn
NEWSYM FxOpdAFA1    ; LMS rn,(yy)  load word from RAM (short address)
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
   FXReturn

NEWSYM FxOpdA0A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 0
NEWSYM FxOpdA1A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 1
NEWSYM FxOpdA2A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 2
NEWSYM FxOpdA3A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 3
NEWSYM FxOpdA4A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 4
NEWSYM FxOpdA5A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 5
NEWSYM FxOpdA6A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 6
NEWSYM FxOpdA7A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 7
NEWSYM FxOpdA8A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 8
NEWSYM FxOpdA9A2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 9
NEWSYM FxOpdAAA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 10
NEWSYM FxOpdABA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 11
NEWSYM FxOpdACA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 12
NEWSYM FxOpdADA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 13
NEWSYM FxOpdAEA2    ; SMS (yy),rn  store word in RAM (short address)
   SMSRNc 14
NEWSYM FxOpdAFA2    ; SMS (yy),rn  store word in RAM (short address)
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
   FXReturn

NEWSYM FxOpdB0      ; FROM rn   set source register
   FROMRNd 0
NEWSYM FxOpdB1      ; FROM rn   set source register
   FROMRNd 1
NEWSYM FxOpdB2      ; FROM rn   set source register
   FROMRNd 2
NEWSYM FxOpdB3      ; FROM rn   set source register
   FROMRNd 3
NEWSYM FxOpdB4      ; FROM rn   set source register
   FROMRNd 4
NEWSYM FxOpdB5      ; FROM rn   set source register
   FROMRNd 5
NEWSYM FxOpdB6      ; FROM rn   set source register
   FROMRNd 6
NEWSYM FxOpdB7      ; FROM rn   set source register
   FROMRNd 7
NEWSYM FxOpdB8      ; FROM rn   set source register
   FROMRNd 8
NEWSYM FxOpdB9      ; FROM rn   set source register
   FROMRNd 9
NEWSYM FxOpdBA      ; FROM rn   set source register
   FROMRNd 10
NEWSYM FxOpdBB      ; FROM rn   set source register
   FROMRNd 11
NEWSYM FxOpdBC      ; FROM rn   set source register
   FROMRNd 12
NEWSYM FxOpdBD      ; FROM rn   set source register
   FROMRNd 13
NEWSYM FxOpdBE      ; FROM rn   set source register
   FROMRNd 14
NEWSYM FxOpdBF      ; FROM rn   set source register
   FETCHPIPE
   mov esi,SfxR0+15*4
   inc ebp                ; Increase program counter
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov esi,SfxR0
   FXReturn

NEWSYM FxOpdC0      ; HIB       move high-byte to low-byte      ; V
   mov eax,[esi]            ; Read Source
   FETCHPIPE
   and eax,0FF00h
   mov dword [SfxSignZero],eax
   shr eax,8
   inc ebp
   mov [edi],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpdC1      ; OR rn     or rn
   ORRNc 1
NEWSYM FxOpdC2      ; OR rn     or rn
   ORRNc 2
NEWSYM FxOpdC3      ; OR rn     or rn
   ORRNc 3
NEWSYM FxOpdC4      ; OR rn     or rn
   ORRNc 4
NEWSYM FxOpdC5      ; OR rn     or rn
   ORRNc 5
NEWSYM FxOpdC6      ; OR rn     or rn
   ORRNc 6
NEWSYM FxOpdC7      ; OR rn     or rn
   ORRNc 7
NEWSYM FxOpdC8      ; OR rn     or rn
   ORRNc 8
NEWSYM FxOpdC9      ; OR rn     or rn
   ORRNc 9
NEWSYM FxOpdCA      ; OR rn     or rn
   ORRNc 10
NEWSYM FxOpdCB      ; OR rn     or rn
   ORRNc 11
NEWSYM FxOpdCC      ; OR rn     or rn
   ORRNc 12
NEWSYM FxOpdCD      ; OR rn     or rn
   ORRNc 13
NEWSYM FxOpdCE      ; OR rn     or rn
   ORRNc 14
NEWSYM FxOpdCF      ; OR rn     or rn
   mov eax,[esi]            ; Read Source
   mov ebx,ebp
   FETCHPIPE
   sub ebx,[SfxCPB]
   or eax,ebx
   inc ebp
   mov [edi],eax            ; Write DREG
   mov [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpdC1A1    ; XOR rn    xor rn
   XORRNc 1
NEWSYM FxOpdC2A1    ; XOR rn    xor rn
   XORRNc 2
NEWSYM FxOpdC3A1    ; XOR rn    xor rn
   XORRNc 3
NEWSYM FxOpdC4A1    ; XOR rn    xor rn
   XORRNc 4
NEWSYM FxOpdC5A1    ; XOR rn    xor rn
   XORRNc 5
NEWSYM FxOpdC6A1    ; XOR rn    xor rn
   XORRNc 6
NEWSYM FxOpdC7A1    ; XOR rn    xor rn
   XORRNc 7
NEWSYM FxOpdC8A1    ; XOR rn    xor rn
   XORRNc 8
NEWSYM FxOpdC9A1    ; XOR rn    xor rn
   XORRNc 9
NEWSYM FxOpdCAA1    ; XOR rn    xor rn
   XORRNc 10
NEWSYM FxOpdCBA1    ; XOR rn    xor rn
   XORRNc 11
NEWSYM FxOpdCCA1    ; XOR rn    xor rn
   XORRNc 12
NEWSYM FxOpdCDA1    ; XOR rn    xor rn
   XORRNc 13
NEWSYM FxOpdCEA1    ; XOR rn    xor rn
   XORRNc 14
NEWSYM FxOpdCFA1    ; XOR rn    xor rn
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov ebx,ebp
   sub ebx,[SfxCPB]
   xor eax,ebx
   inc ebp
   mov [edi],eax            ; Write DREG
   mov [SfxSignZero],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpdC1A2    ; OR #n     OR #n
   ORIc 1
NEWSYM FxOpdC2A2    ; OR #n     OR #n
   ORIc 2
NEWSYM FxOpdC3A2    ; OR #n     OR #n
   ORIc 3
NEWSYM FxOpdC4A2    ; OR #n     OR #n
   ORIc 4
NEWSYM FxOpdC5A2    ; OR #n     OR #n
   ORIc 5
NEWSYM FxOpdC6A2    ; OR #n     OR #n
   ORIc 6
NEWSYM FxOpdC7A2    ; OR #n     OR #n
   ORIc 7
NEWSYM FxOpdC8A2    ; OR #n     OR #n
   ORIc 8
NEWSYM FxOpdC9A2    ; OR #n     OR #n
   ORIc 9
NEWSYM FxOpdCAA2    ; OR #n     OR #n
   ORIc 10
NEWSYM FxOpdCBA2    ; OR #n     OR #n
   ORIc 11
NEWSYM FxOpdCCA2    ; OR #n     OR #n
   ORIc 12
NEWSYM FxOpdCDA2    ; OR #n     OR #n
   ORIc 13
NEWSYM FxOpdCEA2    ; OR #n     OR #n
   ORIc 14
NEWSYM FxOpdCFA2    ; OR #n     OR #n
   ORIc 15

NEWSYM FxOpdC1A3    ; XOR #n    xor #n
   XORIc 1
NEWSYM FxOpdC2A3    ; XOR #n    xor #n
   XORIc 2
NEWSYM FxOpdC3A3    ; XOR #n    xor #n
   XORIc 3
NEWSYM FxOpdC4A3    ; XOR #n    xor #n
   XORIc 4
NEWSYM FxOpdC5A3    ; XOR #n    xor #n
   XORIc 5
NEWSYM FxOpdC6A3    ; XOR #n    xor #n
   XORIc 6
NEWSYM FxOpdC7A3    ; XOR #n    xor #n
   XORIc 7
NEWSYM FxOpdC8A3    ; XOR #n    xor #n
   XORIc 8
NEWSYM FxOpdC9A3    ; XOR #n    xor #n
   XORIc 9
NEWSYM FxOpdCAA3    ; XOR #n    xor #n
   XORIc 10
NEWSYM FxOpdCBA3    ; XOR #n    xor #n
   XORIc 11
NEWSYM FxOpdCCA3    ; XOR #n    xor #n
   XORIc 12
NEWSYM FxOpdCDA3    ; XOR #n    xor #n
   XORIc 13
NEWSYM FxOpdCEA3    ; XOR #n    xor #n
   XORIc 14
NEWSYM FxOpdCFA3    ; XOR #n    xor #n
   XORIc 15

NEWSYM FxOpdD0      ; INC rn    increase by one
   INCRNc 0
NEWSYM FxOpdD1      ; INC rn    increase by one
   INCRNc 1
NEWSYM FxOpdD2      ; INC rn    increase by one
   INCRNc 2
NEWSYM FxOpdD3      ; INC rn    increase by one
   INCRNc 3
NEWSYM FxOpdD4      ; INC rn    increase by one
   INCRNc 4
NEWSYM FxOpdD5      ; INC rn    increase by one
   INCRNc 5
NEWSYM FxOpdD6      ; INC rn    increase by one
   INCRNc 6
NEWSYM FxOpdD7      ; INC rn    increase by one
   INCRNc 7
NEWSYM FxOpdD8      ; INC rn    increase by one
   INCRNc 8
NEWSYM FxOpdD9      ; INC rn    increase by one
   INCRNc 9
NEWSYM FxOpdDA      ; INC rn    increase by one
   INCRNc 10
NEWSYM FxOpdDB      ; INC rn    increase by one
   INCRNc 11
NEWSYM FxOpdDC      ; INC rn    increase by one
   INCRNc 12
NEWSYM FxOpdDD      ; INC rn    increase by one
   INCRNc 13
NEWSYM FxOpdDE      ; INC rn    increase by one
   FETCHPIPE
   mov eax,[SfxR0+14*4]            ; Read Source
   inc ax
   mov [SfxR0+14*4],eax
   mov [SfxSignZero],eax
   CLRFLAGS
   inc ebp
   UpdateR14
   FXReturn

NEWSYM FxOpdDF      ; GETC      transfer ROM buffer to color register
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
   FXReturn
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
   FXReturn

NEWSYM FxOpdDFA2    ; RAMB      set current RAM bank    ; Verified
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
   FXReturn

NEWSYM FxOpdDFA3    ; ROMB      set current ROM bank    ; Verified
   mov eax,[esi]            ; Read Source
   and eax,07Fh
   FETCHPIPE
   mov dword [SfxROMBR],eax
   mov eax,[SfxMemTable+eax*4]
   mov [SfxCROM],eax
   CLRFLAGS
   inc ebp
   FXReturn

NEWSYM FxOpdE0      ; DEC rn    decrement by one
   DECRNc 0
NEWSYM FxOpdE1      ; DEC rn    decrement by one
   DECRNc 1
NEWSYM FxOpdE2      ; DEC rn    decrement by one
   DECRNc 2
NEWSYM FxOpdE3      ; DEC rn    decrement by one
   DECRNc 3
NEWSYM FxOpdE4      ; DEC rn    decrement by one
   DECRNc 4
NEWSYM FxOpdE5      ; DEC rn    decrement by one
   DECRNc 5
NEWSYM FxOpdE6      ; DEC rn    decrement by one
   DECRNc 6
NEWSYM FxOpdE7      ; DEC rn    decrement by one
   DECRNc 7
NEWSYM FxOpdE8      ; DEC rn    decrement by one
   DECRNc 8
NEWSYM FxOpdE9      ; DEC rn    decrement by one
   DECRNc 9
NEWSYM FxOpdEA      ; DEC rn    decrement by one
   DECRNc 10
NEWSYM FxOpdEB      ; DEC rn    decrement by one
   DECRNc 11
NEWSYM FxOpdEC      ; DEC rn    decrement by one
   DECRNc 12
NEWSYM FxOpdED      ; DEC rn    decrement by one
   DECRNc 13
NEWSYM FxOpdEE      ; DEC rn    decrement by one
   dec word[SfxR0+14*4]
   FETCHPIPE
   mov eax,[SfxR0+14*4]            ; Read Source
   mov [SfxSignZero],eax
   UpdateR14
   CLRFLAGS
   inc ebp
   FXReturn

NEWSYM FxOpdEF      ; getb      get byte from ROM at address R14        ; V
   FETCHPIPE
   mov eax,[SfxRomBuffer]
   inc ebp
   mov eax,[eax]
   and eax,0FFh
;   cmp edi,SfxR15
;   je .nor15
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   FXReturn
.nor15
   or eax,8000h
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   FXReturn

   add eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpdEFA1    ; getbh     get high-byte from ROM at address R14   ; V
   mov eax,[esi]            ; Read Source
   mov ebx,[SfxRomBuffer]
   and eax,0FFh
   FETCHPIPE
   mov ah,[ebx]
   inc ebp
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   FXReturn

NEWSYM FxOpdEFA2    ; getbl     get low-byte from ROM at address R14    ; V
   mov eax,[esi]            ; Read Source
   mov ebx,[SfxRomBuffer]
   and eax,0FF00h
   FETCHPIPE
   mov al,[ebx]
   inc ebp
   mov [edi],eax            ; Write DREG
   CLRFLAGS
   FXReturn

NEWSYM FxOpdEFA3    ; getbs     get sign extended byte from ROM at address R14  ; V
   mov ebx,[SfxRomBuffer]
   FETCHPIPE
   movsx eax,byte[ebx]
   inc ebp
   mov [edi],ax            ; Write DREG
   CLRFLAGS
   FXReturn

NEWSYM FxOpdF0      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 0
NEWSYM FxOpdF1      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 1
NEWSYM FxOpdF2      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 2
NEWSYM FxOpdF3      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 3
NEWSYM FxOpdF4      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 4
NEWSYM FxOpdF5      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 5
NEWSYM FxOpdF6      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 6
NEWSYM FxOpdF7      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 7
NEWSYM FxOpdF8      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 8
NEWSYM FxOpdF9      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 9
NEWSYM FxOpdFA      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 10
NEWSYM FxOpdFB      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 11
NEWSYM FxOpdFC      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 12
NEWSYM FxOpdFD      ; IWT RN,#xx   immediate word transfer to register
   IWTRNc 13
NEWSYM FxOpdFE      ; IWT RN,#xx   immediate word transfer to register
   mov eax,[ebp]
   mov cl,[ebp+2]
   and eax,0FFFFh
   add ebp,3
   mov [SfxR0+14*4],eax
   UpdateR14
   CLRFLAGS
   FXReturn
NEWSYM FxOpdFF      ; IWT RN,#xx   immediate word transfer to register
   mov eax,[ebp]
   mov cl,[ebp+2]
   and eax,0FFFFh
   mov ebp,[SfxCPB]
   add ebp,eax
   CLRFLAGS
   FXReturn

NEWSYM FxOpdF0A1    ; LM RN,(XX)   load word from RAM
   LMRNc 0
NEWSYM FxOpdF1A1    ; LM RN,(XX)   load word from RAM
   LMRNc 1
NEWSYM FxOpdF2A1    ; LM RN,(XX)   load word from RAM
   LMRNc 2
NEWSYM FxOpdF3A1    ; LM RN,(XX)   load word from RAM
   LMRNc 3
NEWSYM FxOpdF4A1    ; LM RN,(XX)   load word from RAM
   LMRNc 4
NEWSYM FxOpdF5A1    ; LM RN,(XX)   load word from RAM
   LMRNc 5
NEWSYM FxOpdF6A1    ; LM RN,(XX)   load word from RAM
   LMRNc 6
NEWSYM FxOpdF7A1    ; LM RN,(XX)   load word from RAM
   LMRNc 7
NEWSYM FxOpdF8A1    ; LM RN,(XX)   load word from RAM
   LMRNc 8
NEWSYM FxOpdF9A1    ; LM RN,(XX)   load word from RAM
   LMRNc 9
NEWSYM FxOpdFAA1    ; LM RN,(XX)   load word from RAM
   LMRNc 10
NEWSYM FxOpdFBA1    ; LM RN,(XX)   load word from RAM
   LMRNc 11
NEWSYM FxOpdFCA1    ; LM RN,(XX)   load word from RAM
   LMRNc 12
NEWSYM FxOpdFDA1    ; LM RN,(XX)   load word from RAM
   LMRNc 13
NEWSYM FxOpdFEA1    ; LM RN,(XX)   load word from RAM
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
   FXReturn
NEWSYM FxOpdFFA1    ; LM RN,(XX)   load word from RAM
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
   FXReturn

NEWSYM FxOpdF0A2    ; SM (XX),RN   store word in RAM
   SMRNc 0
NEWSYM FxOpdF1A2    ; SM (XX),RN   store word in RAM
   SMRNc 1
NEWSYM FxOpdF2A2    ; SM (XX),RN   store word in RAM
   SMRNc 2
NEWSYM FxOpdF3A2    ; SM (XX),RN   store word in RAM
   SMRNc 3
NEWSYM FxOpdF4A2    ; SM (XX),RN   store word in RAM
   SMRNc 4
NEWSYM FxOpdF5A2    ; SM (XX),RN   store word in RAM
   SMRNc 5
NEWSYM FxOpdF6A2    ; SM (XX),RN   store word in RAM
   SMRNc 6
NEWSYM FxOpdF7A2    ; SM (XX),RN   store word in RAM
   SMRNc 7
NEWSYM FxOpdF8A2    ; SM (XX),RN   store word in RAM
   SMRNc 8
NEWSYM FxOpdF9A2    ; SM (XX),RN   store word in RAM
   SMRNc 9
NEWSYM FxOpdFAA2    ; SM (XX),RN   store word in RAM
   SMRNc 10
NEWSYM FxOpdFBA2    ; SM (XX),RN   store word in RAM
   SMRNc 11
NEWSYM FxOpdFCA2    ; SM (XX),RN   store word in RAM
   SMRNc 12
NEWSYM FxOpdFDA2    ; SM (XX),RN   store word in RAM
   SMRNc 13
NEWSYM FxOpdFEA2    ; SM (XX),RN   store word in RAM
   SMRNc 14
NEWSYM FxOpdFFA2    ; SM (XX),RN   store word in RAM
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
   FXReturn

NEWSYM FxEmu2CAsmEnd
