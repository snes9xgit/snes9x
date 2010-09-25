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

EXTSYM SfxR1,SfxR2,InitFxTables,SfxSFR,SfxSCMR,initsfxregsw,initsfxregsr
EXTSYM SfxPBR,NumberOfOpcodes,SfxCLSR,MainLoop,SfxCROM
EXTSYM SfxRomBuffer,SfxRAMMem,SfxLastRamAdr,SfxR0
EXTSYM PHnum2writesfxreg, SfxCPB, SfxROMBR, SfxRAMBR, SfxnRamBanks
EXTSYM SfxSignZero

[BITS 32]
SECTION .data

NEWSYM flagnz,   dd 0
NEWSYM romdata,  dd 0              ; rom data  (4MB = 4194304)

NEWSYM regptr, dd 0              ; pointer to registers
NEWSYM regptw,  dd 0              ; pointer to registers
NEWSYM regptra, times 49152 db 0
NEWSYM regptwa, times 49152 db 0

NEWSYM debstop, db 0
NEWSYM sfxramdata, dd 0              ; SuperFX Ram Data
NEWSYM sfxplottable, dd 0
NEWSYM FxTable, times 256 dd 0
NEWSYM FxTableA1, times 256 dd 0
NEWSYM FxTableA2, times 256 dd 0
NEWSYM FxTableA3, times 256 dd 0
NEWSYM FxTableb, times 256 dd 0
NEWSYM FxTablebA1, times 256 dd 0
NEWSYM FxTablebA2, times 256 dd 0
NEWSYM FxTablebA3, times 256 dd 0
NEWSYM FxTablec, times 256 dd 0
NEWSYM FxTablecA1, times 256 dd 0
NEWSYM FxTablecA2, times 256 dd 0
NEWSYM FxTablecA3, times 256 dd 0
NEWSYM FxTabled, times 256 dd 0
NEWSYM FxTabledA1, times 256 dd 0
NEWSYM FxTabledA2, times 256 dd 0
NEWSYM FxTabledA3, times 256 dd 0
NEWSYM SfxMemTable, times 256 dd 0
NEWSYM fxxand,  times 256 dd 0
NEWSYM fxbit01, times 256 dd 0
NEWSYM fxbit23, times 256 dd 0
NEWSYM fxbit45, times 256 dd 0
NEWSYM fxbit67, times 256 dd 0
NEWSYM PLOTJmpa, times 64 dd 0
NEWSYM PLOTJmpb, times 64 dd 0

NEWSYM NumberOfOpcodes2, dd 350
NEWSYM SFXCounter, dd 0
NEWSYM SfxAC, dd 0

;%INCLUDE "fxtable.asm"
;%INCLUDE "sfxproc.asm"
;%INCLUDE "fxemu2.asm"
;%INCLUDE "fxemu2b.asm"
;%INCLUDE "fxemu2c.asm"

;
; Snes9x wrapper functions for the zsnes SuperFX code
; (C) Copyright 1998 Gary Henderson

SECTION .data

EXTSYM SRAM,ROM,RegRAM,SFXPlotTable

SECTION .text

NEWSYM StartSFX
NEWSYM StartSFXret
	jmp S9xSuperFXExec

NEWSYM S9xResetSuperFX
	push ebx
	push esi
	push edi
	push ebp

	mov dword[SfxR1],0
	mov dword[SfxR2],0
	mov dword[SfxSignZero],1	; clear z flag

	mov eax,dword[SRAM]
	mov dword[sfxramdata],eax
	mov eax,dword[ROM]
	mov dword[romdata],eax
	mov eax,dword[SFXPlotTable]
	mov dword[sfxplottable],eax

	call InitFxTables

	mov dword[SfxSFR],0
	mov byte[SfxSCMR],0
	mov dword[regptr],regptra
	sub dword[regptr],8000h   ; Since register address starts @ 2000h
	mov dword[regptw],regptwa
	sub dword[regptw],8000h   ; Since register address starts @ 2000h
	call initsfxregsw
	call initsfxregsr

NEWSYM preparesfx
	mov byte[SFXCounter],0
	mov esi,[romdata]
	add esi,07FC0h
	cmp dword[esi],'FX S'
    	je .yessfxcounter
    	cmp dword[esi],'Stun'
    	jne .nosfxcounter
.yessfxcounter
	mov byte[SFXCounter],1
.nosfxcounter

    	; make table
    	mov byte[SfxAC],0
    	mov eax,[romdata]
    	cmp dword[eax+02B80h],0AB6CAB6Ch
    	jne .noac
    	mov byte[SfxAC],1
.noac
	call UpdateSFX

	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

NEWSYM S9xSuperFXWriteReg
	mov eax,dword[esp+4]
	mov ecx,dword[esp+8]
	push ebx
	push esi
	push edi
	push ebp
	test dword[regptwa+ecx*4-8000h], ~0
	jz .skipwrite
	call dword[regptwa+ecx*4-8000h]
.skipwrite:
	pop ebp
	pop edi
	pop esi
	pop ebx
	ret

NEWSYM S9xSuperFXReadReg
	mov ecx,dword[esp+4]
	push ebx
	push esi
	push edi
	push ebp
	test dword[regptra+ecx*4-8000h], ~0
	jz .skipread
	call dword[regptra+ecx*4-8000h]
.skipread:
	pop ebp
	pop edi
	pop esi
	pop ebx
	and eax, 255
endfx:
	ret

EXTSYM SCBRrel,SfxSCBR,SfxCOLR,SfxPOR
EXTSYM sfx128lineloc,sfx160lineloc,sfx192lineloc,sfxobjlineloc,sfxclineloc
EXTSYM fxbit01pcal,fxbit23pcal,fxbit45pcal,fxbit67pcal

NEWSYM S9xSuperFXExec
    test byte[SfxSFR],20h
    jz endfx

    push ebx
    push esi
    push edi
    push ebp

    xor ebx,ebx
    mov bl,[SfxPBR]
    mov al,[SfxSCMR]
    and bl,7Fh
    cmp bl,70h
    jae .ram
    test al,10h
    jz .noaccess
    jmp .noram
.ram
    test al,08h
    jz .noaccess
.noram
    mov eax,[NumberOfOpcodes2]
    mov [NumberOfOpcodes],eax
    call MainLoop
.noaccess
    pop ebp
    pop edi
    pop esi
    pop edx
    ret

NEWSYM S9xSuperFXPreSaveState
    mov ecx,dword[SfxCROM]
    sub dword[SfxRomBuffer],ecx
    mov ecx,dword[SfxRAMMem]
    sub dword[SfxLastRamAdr],ecx
; Copy num2writesfxreg bytes from SfxR0 to a free area in RegRAM.
    push ebx
    mov ecx,SfxR0
    mov edx,dword[RegRAM]
    add edx,7000h
    mov ebx,[PHnum2writesfxreg]
.loop
    mov al,[ecx]
    mov [edx],al
    inc ecx
    inc edx
    dec ebx
    jnz .loop
    pop ebx
    mov ecx,dword[SfxCROM]
    add dword[SfxRomBuffer],ecx
    mov ecx,dword[SfxRAMMem]
    add dword[SfxLastRamAdr],ecx
    ret

NEWSYM S9xSuperFXPostSaveState
    ret

NEWSYM S9xSuperFXPostLoadState
; Copy num2writesfxreg bytes from a free area in RegRAM to SfxR0
    push ebx
    mov ecx,SfxR0
    mov edx,dword[RegRAM]
    add edx,7000h
    mov ebx,[PHnum2writesfxreg]
.loop
    mov al,[edx]
    mov [ecx],al
    inc ecx
    inc edx
    dec ebx
    jnz .loop
    pop ebx
    xor ecx,ecx
    mov cl,byte[SfxPBR]
    mov ecx,dword[SfxMemTable+ecx*4]
    mov dword[SfxCPB],ecx

    xor ecx,ecx
    mov cl,byte[SfxROMBR]
    mov ecx,dword[SfxMemTable+ecx*4]
    mov dword[SfxCROM],ecx

    xor ecx,ecx
    mov cl,byte[SfxRAMBR]
    shl ecx,16
    add ecx,dword[sfxramdata]
    mov dword [SfxRAMMem],ecx

    mov ecx,dword[SfxCROM]
    add dword[SfxRomBuffer],ecx
    mov ecx,dword[SfxRAMMem]
    add dword[SfxLastRamAdr],ecx
; Fix for mystery zeroing of SfxnRamBanks, allowing junk RAMBR values to be
; set in turn causing a crash when the code tries to access the junk RAM bank.
    mov dword [SfxnRamBanks], 4
    call UpdateSFX
    ret

NEWSYM UpdatePORSCMR
   push ebx
   push eax
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
   pop eax
   pop ebx
   ret

NEWSYM UpdateSCBRCOLR
   push eax
   push ebx
   mov ebx,[SfxSCBR]
   shl ebx,10
   add ebx,[sfxramdata]
   mov [SCBRrel],ebx
   mov eax,[SfxCOLR]
   mov ebx,[fxbit01+eax*4]
   mov [fxbit01pcal],ebx
   mov ebx,[fxbit23+eax*4]
   mov [fxbit23pcal],ebx
   mov ebx,[fxbit45+eax*4]
   mov [fxbit45pcal],ebx
   mov ebx,[fxbit67+eax*4]
   mov [fxbit67pcal],ebx
   pop ebx
   pop eax
   ret

NEWSYM UpdateCLSR
   mov dword [NumberOfOpcodes2],350 ; 0FFFFFFFh;350
   test byte[SfxCLSR],01h
   jz .nohighsfx
   mov dword [NumberOfOpcodes2],700 ;700
.nohighsfx
    cmp byte[SFXCounter],1
    je .noyi
    mov dword [NumberOfOpcodes2],0FFFFFFFh
.noyi
   ret

NEWSYM UpdateSFX
   call UpdatePORSCMR
   call UpdatePORSCMR
   call UpdateCLSR
   ret
