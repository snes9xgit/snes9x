; Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
;
; Super FX assembler emulator code
; (c) Copyright 1998, 1999 zsKnight and _Demo_.
;
; Permission to use, copy, modify and distribute Snes9x in both binary and
; source form, for non-commercial purposes, is hereby granted without fee,
; providing that this license information and copyright notice appear with
; all copies and any derived work.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event shall the authors be held liable for any damages
; arising from the use of this software.
;
; Snes9x is freeware for PERSONAL USE only. Commercial users should
; seek permission of the copyright holders first. Commercial use includes
; charging money for Snes9x or software derived from Snes9x.
;
; The copyright holders request that bug fixes and improvements to the code
; should be forwarded to them so everyone can benefit from the modifications
; in future versions.
;
; Super NES and Super Nintendo Entertainment System are trademarks of
; Nintendo Co., Limited and its subsidiary companies.
;
; ZSNES SuperFX emulation code and wrappers
; (C) Copyright 1997-8 zsKnight and _Demo_

; Used with the kind permission of the copyright holders.
;

%include "macros.mac"

EXTSYM InitC4, C4RegFunction, C4ReadReg, C4WriteReg
EXTSYM romdata

[BITS 32]
SECTION .data

NEWSYM C4Ram,	 dd 0
NEWSYM C4RamR,	 dd 0
NEWSYM C4RamW,	 dd 0

NEWSYM pressed,  dd 0		   ; used by C4Edit (unused function)
NEWSYM vidbuffer,dd 0		   ; used by C4Edit
NEWSYM oamram, times 544 db 0	   ; Sprite PPU data used by C4 routines

;
; Snes9x wrapper functions for the zsnes C4 code
; (C) Copyright 2000 Gary Henderson

SECTION .data

EXTSYM ROM,RegRAM

SECTION .text

NEWSYM S9xInitC4
	pushad
	mov eax,dword[ROM]
	mov dword[romdata],eax
	call InitC4
;	mov eax,dword[RegRAM]
;	add eax,06000h
;	mov dword[C4Ram],eax
	popad
	ret

NEWSYM S9xSetC4
NEWSYM S9xSetC4RAM
	mov ecx, dword[esp+8]
	mov eax, dword[esp+4]
	and ecx, 0ffffh
	push ebx
	push esi
	push edi
	push ebp
	sub ecx, 6000h
	call C4RegFunction
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

NEWSYM S9xGetC4
NEWSYM S9xGetC4RAM
	mov ecx, dword[esp+4]
	xor eax, eax
	and ecx, 0ffffh
	sub ecx, 06000h
	jmp C4ReadReg
