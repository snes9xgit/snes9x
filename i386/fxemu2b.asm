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

EXTSYM FxTable,FxTableb,FxTablec,SfxB,SfxCPB,SfxCROM,SfxCarry,SfxOverflow
EXTSYM SfxR0,SfxR14,SfxR15,SfxRomBuffer,SfxSignZero,withr15sk

NEWSYM FxEmu2BAsmStart

%include "fxemu2.mac"
%include "fxemu2b.mac"






NEWSYM FxOpb05      ; BRA    branch always      ; Verified.
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   inc ebp
   add ebp,eax
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb06      ; BGE    branch on greater or equals        ; Verified.
   movsx eax,byte[ebp]
   mov ebx,[SfxSignZero]
   shr ebx,15
   inc ebp
   xor bl,[SfxOverflow]
   mov cl,[ebp]
   test bl,01h
   jnz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb07      ; BLT    branch on lesss than       ; Verified.
   movsx eax,byte[ebp]
   mov ebx,[SfxSignZero]
   shr ebx,15
   inc ebp
   xor bl,[SfxOverflow]
   mov cl,[ebp]
   test bl,01h
   jz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb08      ; BNE    branch on not equal        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb09      ; BEQ    branch on equal (z=1)      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb0A      ; BPL    branch on plus     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb0B      ; BMI    branch on minus    ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb0C      ; BCC    branch on carry clear      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb0D      ; BCS    branch on carry set        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb0E      ; BVC    branch on overflow clear   ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb0F      ; BVS    branch on overflow set     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTableb+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTableb+ecx*4]
   ret

NEWSYM FxOpb10      ; TO RN  set register n as destination register
   TORNb 0
NEWSYM FxOpb11      ; TO RN  set register n as destination register
   TORNb 1
NEWSYM FxOpb12      ; TO RN  set register n as destination register
   TORNb 2
NEWSYM FxOpb13      ; TO RN  set register n as destination register
   TORNb 3
NEWSYM FxOpb14      ; TO RN  set register n as destination register
   TORNb 4
NEWSYM FxOpb15      ; TO RN  set register n as destination register
   TORNb 5
NEWSYM FxOpb16      ; TO RN  set register n as destination register
   TORNb 6
NEWSYM FxOpb17      ; TO RN  set register n as destination register
   TORNb 7
NEWSYM FxOpb18      ; TO RN  set register n as destination register
   TORNb 8
NEWSYM FxOpb19      ; TO RN  set register n as destination register
   TORNb 9
NEWSYM FxOpb1A      ; TO RN  set register n as destination register
   TORNb 10
NEWSYM FxOpb1B      ; TO RN  set register n as destination register
   TORNb 11
NEWSYM FxOpb1C      ; TO RN  set register n as destination register
   TORNb 12
NEWSYM FxOpb1D      ; TO RN  set register n as destination register
   TORNb 13
NEWSYM FxOpb1E      ; TO RN  set register n as destination register
   FETCHPIPE
   test dword [SfxB],1
   jnz .VersionB
   mov edi,SfxR0+14*4
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov dword[withr15sk],1
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov edi,SfxR0
   UpdateR14
   ret
.VersionB
   mov eax,[esi]            ; Read Source
   mov dword[withr15sk],1
   mov [SfxR0+14*4],eax             ; Write
   CLRFLAGS
   UpdateR14
   inc ebp                ; Increase program counter
   ret
NEWSYM FxOpb1F      ; TO RN  set register n as destination register
   FETCHPIPE
   test dword [SfxB],1
   jnz .VersionB
   mov edi,SfxR0+15*4
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov ebp,[SfxCPB]
   mov dword[withr15sk],1
   add ebp,[SfxR15]
   mov edi,SfxR0
   ret
.VersionB
   mov eax,[esi]            ; Read Source
   mov ebp,[SfxCPB]
   mov dword[withr15sk],1
   add ebp,eax
   CLRFLAGS
   ret

NEWSYM FxOpb3D      ; ALT1   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,01h
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpb3E      ; ALT2   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,02h
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpb3F      ; ALT3   set alt3 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,03h
   inc ebp
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTable+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpbB0      ; FROM rn   set source register
   FROMRNb 0
NEWSYM FxOpbB1      ; FROM rn   set source register
   FROMRNb 1
NEWSYM FxOpbB2      ; FROM rn   set source register
   FROMRNb 2
NEWSYM FxOpbB3      ; FROM rn   set source register
   FROMRNb 3
NEWSYM FxOpbB4      ; FROM rn   set source register
   FROMRNb 4
NEWSYM FxOpbB5      ; FROM rn   set source register
   FROMRNb 5
NEWSYM FxOpbB6      ; FROM rn   set source register
   FROMRNb 6
NEWSYM FxOpbB7      ; FROM rn   set source register
   FROMRNb 7
NEWSYM FxOpbB8      ; FROM rn   set source register
   FROMRNb 8
NEWSYM FxOpbB9      ; FROM rn   set source register
   FROMRNb 9
NEWSYM FxOpbBA      ; FROM rn   set source register
   FROMRNb 10
NEWSYM FxOpbBB      ; FROM rn   set source register
   FROMRNb 11
NEWSYM FxOpbBC      ; FROM rn   set source register
   FROMRNb 12
NEWSYM FxOpbBD      ; FROM rn   set source register
   FROMRNb 13
NEWSYM FxOpbBE      ; FROM rn   set source register
   FROMRNb 14
NEWSYM FxOpbBF      ; FROM rn   set source register
   test dword [SfxB],1
   jnz .VersionB
   mov esi,SfxR0+15*4
   inc ebp                ; Increase program counter
   mov eax,ebp
   sub eax,[SfxCPB]
   mov [SfxR15],eax
   call [FxTableb+ecx*4]
   mov esi,SfxR0
   ret
.VersionB
   FETCHPIPE
   mov eax,ebp
   sub eax,[SfxCPB]
   inc ebp
   mov [edi],eax        ; Write Destination
   mov [SfxSignZero],eax
   shr al,7
   mov byte[SfxOverflow],al
   CLRFLAGS
   ret

NEWSYM FxOpc05      ; BRA    branch always      ; Verified.
   movsx eax,byte[ebp]
   mov cl,[ebp+1]
   inc ebp
   add ebp,eax
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc06      ; BGE    branch on greater or equals        ; Verified.
   movsx eax,byte[ebp]
   mov ebx,[SfxSignZero]
   shr ebx,15
   inc ebp
   xor bl,[SfxOverflow]
   mov cl,[ebp]
   test bl,01h
   jnz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc07      ; BLT    branch on lesss than       ; Verified.
   movsx eax,byte[ebp]
   mov ebx,[SfxSignZero]
   shr ebx,15
   inc ebp
   xor bl,[SfxOverflow]
   mov cl,[ebp]
   test bl,01h
   jz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc08      ; BNE    branch on not equal        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc09      ; BEQ    branch on equal (z=1)      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],0FFFFh
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc0A      ; BPL    branch on plus     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc0B      ; BMI    branch on minus    ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test dword[SfxSignZero],088000h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc0C      ; BCC    branch on carry clear      ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc0D      ; BCS    branch on carry set        ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxCarry],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc0E      ; BVC    branch on overflow clear   ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jnz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc0F      ; BVS    branch on overflow set     ; Verified.
   movsx eax,byte[ebp]
   inc ebp
   test byte[SfxOverflow],01h
   mov cl,[ebp]
   jz .nojump
   add ebp,eax
   call [FxTablec+ecx*4]
   ret
.nojump
   inc ebp
   call [FxTablec+ecx*4]
   ret

NEWSYM FxOpc10      ; TO RN  set register n as destination register
   TORNc 0
NEWSYM FxOpc11      ; TO RN  set register n as destination register
   TORNc 1
NEWSYM FxOpc12      ; TO RN  set register n as destination register
   TORNc 2
NEWSYM FxOpc13      ; TO RN  set register n as destination register
   TORNc 3
NEWSYM FxOpc14      ; TO RN  set register n as destination register
   TORNc 4
NEWSYM FxOpc15      ; TO RN  set register n as destination register
   TORNc 5
NEWSYM FxOpc16      ; TO RN  set register n as destination register
   TORNc 6
NEWSYM FxOpc17      ; TO RN  set register n as destination register
   TORNc 7
NEWSYM FxOpc18      ; TO RN  set register n as destination register
   TORNc 8
NEWSYM FxOpc19      ; TO RN  set register n as destination register
   TORNc 9
NEWSYM FxOpc1A      ; TO RN  set register n as destination register
   TORNc 10
NEWSYM FxOpc1B      ; TO RN  set register n as destination register
   TORNc 11
NEWSYM FxOpc1C      ; TO RN  set register n as destination register
   TORNc 12
NEWSYM FxOpc1D      ; TO RN  set register n as destination register
   TORNc 13
NEWSYM FxOpc1E      ; TO RN  set register n as destination register
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov [SfxR0+14*4],eax             ; Write
   CLRFLAGS
   UpdateR14
   inc ebp                ; Increase program counter
   ret
NEWSYM FxOpc1F      ; TO RN  set register n as destination register
   FETCHPIPE
   mov eax,[esi]            ; Read Source
   mov ebp,[SfxCPB]
   mov [SfxR15],eax
   add ebp,eax
   CLRFLAGS
   ret

NEWSYM FxOpc3D      ; ALT1   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,01h
   inc ebp
   call [FxTablec+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpc3E      ; ALT2   set alt1 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,02h
   inc ebp
   call [FxTablec+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpc3F      ; ALT3   set alt3 mode      ; Verified.
   FETCHPIPE
   mov dword [SfxB],0
   or ch,03h
   inc ebp
   call [FxTablec+ecx*4]
   xor ch,ch
   ret

NEWSYM FxOpcB0      ; FROM rn   set source register
   FROMRNc 0
NEWSYM FxOpcB1      ; FROM rn   set source register
   FROMRNc 1
NEWSYM FxOpcB2      ; FROM rn   set source register
   FROMRNc 2
NEWSYM FxOpcB3      ; FROM rn   set source register
   FROMRNc 3
NEWSYM FxOpcB4      ; FROM rn   set source register
   FROMRNc 4
NEWSYM FxOpcB5      ; FROM rn   set source register
   FROMRNc 5
NEWSYM FxOpcB6      ; FROM rn   set source register
   FROMRNc 6
NEWSYM FxOpcB7      ; FROM rn   set source register
   FROMRNc 7
NEWSYM FxOpcB8      ; FROM rn   set source register
   FROMRNc 8
NEWSYM FxOpcB9      ; FROM rn   set source register
   FROMRNc 9
NEWSYM FxOpcBA      ; FROM rn   set source register
   FROMRNc 10
NEWSYM FxOpcBB      ; FROM rn   set source register
   FROMRNc 11
NEWSYM FxOpcBC      ; FROM rn   set source register
   FROMRNc 12
NEWSYM FxOpcBD      ; FROM rn   set source register
   FROMRNc 13
NEWSYM FxOpcBE      ; FROM rn   set source register
   FROMRNc 14
NEWSYM FxOpcBF      ; FROM rn   set source register
   FETCHPIPE
   mov eax,ebp
   sub eax,[SfxCPB]
   inc ebp
   mov [edi],eax        ; Write Destination
   mov [SfxSignZero],eax
   shr al,7
   mov byte[SfxOverflow],al
   CLRFLAGS
   ret

NEWSYM FxEmu2BAsmEnd
