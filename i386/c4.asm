%include "macros.mac"
;*******************************************************
; Register & Memory Access Banks (0 - 3F) / (80 - BF)
;*******************************************************
; enter : BL = bank number, CX = address location
; leave : AL = value read

EXTSYM C4WFXVal,C4WFYVal,C4WFX2Val,C4WFY2Val,C4CalcWireFrame,C4WFDist,C4WFScale,C4TransfWireFrame
EXTSYM C4TransfWireFrame2,C4WFZVal,C41FXVal,C41FYVal,C41FAngleRes,C41FDist,C4Op1F,C4Op15,C41FDistVal
EXTSYM C4Op0D,C4LoaDMem

%ifdef ZSNES_FX
EXTSYM romdata
%else
NEWSYM romdata,  dd 0              ; rom data  (4MB = 4194304)
%endif

EXTSYM pressed,vidbuffer,oamram
EXTSYM C4Ram,C4RamR,C4RamW

C4ProcessVectors:
    mov esi,[C4Ram]
    mov edi,esi
    add edi,1F8Ch
    xor edx,edx
    mov dx,[esi+1F8Ah]
    cmp dx,128
    ja .ret
    cmp dx,0
    jne .nozero
.ret
    ret
.nozero
    cmp dx,10h
    jb .less
    mov dx,10h
.less
    mov esi,[C4Ram]
    add esi,800h
.loop
    mov ecx,100h
    xor ebx,ebx
    xor eax,eax
    movsx bx,byte[edi]
.spotloop
    add ah,80h
    mov byte[esi],ah
    sub ah,80h
    add ax,bx
    inc esi
    loop .spotloop
    add edi,3
    dec dx
    jnz .loop
    ret

SECTION .data
C4ObjDisp dd 0
C4ColRot db 1
NEWSYM C4ObjSelec, db 0
NEWSYM C4SObjSelec, db 0
NEWSYM C4Pause, db 0
C4DataCopy times 64 db 0
;NEWSYM C4Data times 64*4096 db 0        ; 15 sprites, 4 bytes each
                                        ; x,y,oamptr,stat (b0=oamb8,b1=16x16)
                                        ; 4 byte header (#sobj,?,?,?)
CObjNum dw 0
C4Temp dd 0

SECTION .text

C4Edit:
    ; C4 editing routines
    ; Register keyboard presses
    ;  [ = prev object, ] = next object
    ;  p = pause/unpause
    cmp byte[pressed+1Ah],0
    je .notpressed
    mov byte[pressed+1Ah],0
    inc byte[C4ObjSelec]
    inc byte[C4Temp]
.notpressed
    cmp byte[pressed+1Bh],0
    je .notpressed2
    mov byte[pressed+1Bh],0
    dec byte[C4ObjSelec]
    dec byte[C4Temp]
.notpressed2
    cmp byte[pressed+19h],0
    je .notpressed3
    mov byte[pressed+19h],0
    xor byte[C4Pause],1
.notpressed3

    ; Setup variables
    mov esi,[C4Ram]
    add byte[C4ColRot],16
    mov al,[esi+620h]
    cmp byte[C4ObjSelec],0FFh
    jne .notneg
    dec al
    mov byte[C4ObjSelec],al
    jmp .notof
.notneg
    cmp byte[C4ObjSelec],al
    jb .notof
    xor al,al
    mov [C4ObjSelec],al
.notof

    ; Draw the dots on-screen
    xor eax,eax
    mov al,[C4ObjSelec]
    shl eax,4
    add eax,[C4Ram]
    add eax,220h
    mov byte[.flipped],0
    test byte[eax+6],40h
    jz .notflip
    mov byte[.flipped],1
.notflip

;              00/01 - x position relative to BG scroll value
;              02/03 - y position relative to BG scroll value
;              04    - palette/priority settings
;              05    - OAM pointer value
;              06    - flip settings : b6 = flipx, b7 = flipy
;              07    - looks like some sprite displacement values
;              08/09 - ???
;              0A-0F - unused
    xor ebx,ebx
    mov bx,[eax+8]
    mov [CObjNum],bx
    cmp bx,4096
    jae near .skipall
    shl ebx,6
    add ebx,[C4Data]

    ; t,f,g,h = move current object
    ; q = copy current object structure, w = paste current object structure
    cmp byte[pressed+14h],0
    je .notmove
    mov byte[pressed+14h],0
    pushad
    mov ecx,15
.next
    add ebx,4
    dec byte[ebx+1]
    loop .next
    popad
.notmove
    cmp byte[pressed+21h],0
    je .notmove2
    mov byte[pressed+21h],0
    pushad
    mov ecx,15
.next2
    add ebx,4
    cmp byte[.flipped],0
    je .noflipx
    add byte[ebx],2
.noflipx
    dec byte[ebx]
    loop .next2
    popad
.notmove2
    cmp byte[pressed+22h],0
    je .notmove3
    mov byte[pressed+22h],0
    pushad
    mov ecx,15
.next3
    add ebx,4
    inc byte[ebx+1]
    loop .next3
    popad
.notmove3
    cmp byte[pressed+23h],0
    je .notmove4
    mov byte[pressed+23h],0
    pushad
    mov ecx,15
.next4
    add ebx,4
    cmp byte[.flipped],0
    je .noflipx2
    sub byte[ebx],2
.noflipx2
    inc byte[ebx]
    loop .next4
    popad
.notmove4
    cmp byte[pressed+10h],0
    je .notcopy
    mov byte[pressed+10h],0
    pushad
    mov edx,C4DataCopy
    mov ecx,64
.copylp
    mov al,[ebx]
    mov [edx],al
    inc ebx
    inc edx
    loop .copylp
    popad
.notcopy
    cmp byte[pressed+11h],0
    je .notpaste
    mov byte[pressed+11h],0
    pushad
    mov edx,C4DataCopy
    mov ecx,64
.pastelp
    mov al,[edx]
    mov [ebx],al
    inc ebx
    inc edx
    loop .pastelp
    popad
.notpaste

    ;  - = remove sub-object, + = add sub-object
    ;  ; = previous sub-object, ' = next sub-object
    cmp byte[pressed+0Ch],0
    je .notpressed4
    mov byte[pressed+0Ch],0
    cmp byte[ebx],0
    je .notpressed4
    dec byte[ebx]
.notpressed4
    cmp byte[pressed+0Dh],0
    je .notpressed5
    mov byte[pressed+0Dh],0
    cmp byte[ebx],15
    je .notpressed5
    inc byte[ebx]
.notpressed5
    cmp byte[pressed+27h],0
    je .notpressed6
    mov byte[pressed+27h],0
    dec byte[C4SObjSelec]
.notpressed6
    cmp byte[pressed+28h],0
    je .notpressed7
    mov byte[pressed+28h],0
    inc byte[C4SObjSelec]
.notpressed7

    ; get current sub-object displacement (0 if no sub-objects)
    xor ecx,ecx
    cmp byte[ebx],0
    je near .nosubobjs

    mov cl,[ebx]
    cmp byte[C4ObjSelec],0FFh
    jne .sobjokay2
    dec cl
    mov byte[C4SObjSelec],cl
    jmp .sobjokay
.sobjokay2
    cmp byte[C4SObjSelec],cl
    jb .sobjokay
    mov byte[C4SObjSelec],0
.sobjokay

    xor ecx,ecx
    mov cl,byte[C4SObjSelec]
    shl ecx,2
    add ebx,ecx
    add ebx,4

    ; i,j,k,l = move current sub-object (17,24,25,26)
    ; u = toggle between 8x8 and 16x16 tiles
    ; o = toggle between high/low oam value
    ; . = decrease oam value, / = increase oam value of sub-object
    cmp byte[pressed+17h],0
    je .notpressed8
    mov byte[pressed+17h],0
    dec byte[ebx+1]
.notpressed8
    cmp byte[pressed+24h],0
    je .notpressed9
    mov byte[pressed+24h],0
    dec byte[ebx]
    cmp byte[.flipped],0
    je .notpressed9
    add byte[ebx],2
.notpressed9
    cmp byte[pressed+26h],0
    je .notpressed11
    mov byte[pressed+26h],0
    inc byte[ebx]
    cmp byte[.flipped],0
    je .notpressed11
    sub byte[ebx],2
.notpressed11
    cmp byte[pressed+25h],0
    je .notpressed10
    mov byte[pressed+25h],0
    inc byte[ebx+1]
.notpressed10
    cmp byte[pressed+16h],0
    je .notpressed12
    mov byte[pressed+16h],0
    xor byte[ebx+3],2
.notpressed12
    cmp byte[pressed+18h],0
    je .notpressed13
    mov byte[pressed+18h],0
    xor byte[ebx+3],1
.notpressed13
    cmp byte[pressed+34h],0
    je .notpressed14
    mov byte[pressed+34h],0
    dec byte[ebx+2]
.notpressed14
    cmp byte[pressed+35h],0
    je .notpressed15
    mov byte[pressed+35h],0
    inc byte[ebx+2]
.notpressed15

    mov cl,[ebx]
    mov ch,[ebx+1]
.nosubobjs
    mov edx,ecx
    xor ebx,ebx
    xor ecx,ecx
    mov bl,[eax]
    sub bl,[esi+621h]
    add bl,dl
    mov cl,[eax+2]
    sub cl,[esi+623h]
    add cl,dh
    mov esi,[vidbuffer]
    add esi,16*2+256*2+32*2
    add esi,ebx
    add esi,ebx
    mov ebx,ecx
    shl ebx,9
    shl ecx,6
    add esi,ebx
    add esi,ecx
    mov al,[C4ColRot]
    mov ah,al
    xor ah,0FFh
    mov [esi],ax
    mov [esi+16],ax
    mov [esi+288*8*2],ax
    mov [esi+16+288*8*2],ax
.skipall
    ret

SECTION .data
.flipped db 0
SECTION .text

C4AddSprite:
    cmp dword[C4count],0
    je near .nosprite
    mov [edi],ax
    mov [edi+2],bx
    mov ebx,[C4usprptr]
    and [ebx],dl
    mov al,dl
    xor al,0FFh
    and dh,al
    or [ebx],dh
    add edi,4
    rol dl,2
    rol dh,2
    dec dword[C4count]
    cmp dl,0FCh
    jne .nosprite
    inc dword[C4usprptr]
.nosprite
    ret

C4ConvOAM:
    inc byte[C4Timer]
    and byte[C4Timer],15
    inc byte[C4Timer2]
    and byte[C4Timer2],7
    mov esi,[C4Ram]
    xor ecx,ecx
    mov edi,esi
    mov cl,[esi+620h]
    mov bx,[esi+621h]
    mov [.addx],bx
    mov bx,[esi+623h]
    mov [.addy],bx
    mov [C4usprptr],esi
    add dword[C4usprptr],200h
    mov eax,[C4ObjDisp]
    add edi,eax
    shr eax,4
    add dword[C4usprptr],eax
    add esi,220h
    ; Convert from esi to edi
    mov dl,0FCh
    push ecx
    mov cl,byte[C4sprites]
    and cl,3
    add cl,cl
    rol dl,cl
    pop ecx
    cmp cl,0
    je near .none
    mov dword[C4count],128
    mov eax,[C4sprites]
    sub dword[C4count],eax

.loop
    push ecx
    push esi
;              00/01 - x position relative to BG scroll value
;              02/03 - y position relative to BG scroll value
;              04    - palette/priority settings
;              05    - OAM pointer value
;              06    - flip settings : b6 = flipx, b7 = flipy
;              07    - ???
;              08/09 - Pointer to Sprite Structure
;              0A-0F - unused
;bit 1-3 = palette number bit 4,5 = playfield priority
;bit 6   = horizontal flip bit 7   = horizonal flip
    mov ax,[esi]
    sub ax,[.addx]
    mov [C4SprX],ax
    mov ax,[esi+2]
    sub ax,[.addy]
    mov [C4SprY],ax
    mov al,[esi+5]
    mov [C4SprOAM],al
    mov al,[esi+4]
    mov ah,al
    and ah,0Eh
    cmp ah,0
    jmp .notstage2
    jne .notstage1
    cmp byte[C4Timer],0
    je .flash
    jmp .noflash
.notstage1
    jmp .notstage2
    cmp ah,4
    jne .notstage2
    cmp byte[C4Timer2],0
    je .flash
.noflash
    and al,0F1h
    or al,2
    jmp .notstage2
.flash
    and al,0F1h
.notstage2
    mov [C4SprAttr],al
    mov al,[esi+6]
    or [C4SprAttr],al
;    mov [C4SprFlip],al

    xor ecx,ecx
    mov cl,[esi+9]
    shl ecx,16
    mov cx,[esi+7]
    add cx,cx
    shr ecx,1
    add ecx,[romdata]

    mov al,[ecx]
    or al,al
    jz near .singlespr
    mov [C4SprCnt],al
    inc ecx
.nextspr
    xor ebx,ebx
    movsx bx,byte[ecx+1]
    test byte[C4SprAttr],40h
    jz .notflipx
    neg bx
    sub bx,8
.notflipx
    add bx,[C4SprX]
    xor dh,dh
    test byte[ecx],20h
    jz .no16x16
    or dh,10101010b
    test byte[C4SprAttr],40h
    jz .no16x16
    sub bx,8
.no16x16
    cmp bx,-16
    jl near .nosprite
    cmp bx,272
    jg near .nosprite
    mov al,bl
    test bx,100h
    jz .not512b
    or dh,01010101b
.not512b
    xor ebx,ebx
    movsx bx,byte[ecx+2]
    test byte[C4SprAttr],80h
    jz .notflipy
    neg bx
    sub bx,8
.notflipy
    add bx,[C4SprY]
    test byte[ecx],20h
    jz .no16x16b
    test byte[C4SprAttr],80h
    jz .no16x16b
    sub bx,8
.no16x16b
    cmp bx,-16
    jl near .nosprite
    cmp bx,224
    jg near .nosprite
    mov ah,bl
    mov bh,[C4SprAttr]
    mov bl,[ecx]
    and bl,0C0h
    xor bh,bl
    mov bl,[C4SprOAM]
    add bl,[ecx+3]
    call C4AddSprite
.nosprite
    add ecx,4
    dec byte[C4SprCnt]
    jnz near .nextspr
    jmp .donemultispr
.singlespr
    mov dh,10101010b
    test byte[C4SprX+1],1
    jz .not512
    or dh,01010101b
.not512
    mov al,[C4SprX]
    mov ah,[C4SprY]
    mov bl,[C4SprOAM]
    mov bh,[C4SprAttr]
    call C4AddSprite
.donemultispr
    pop esi
    pop ecx

;NEWSYM C4Data times 64*4096 db 0       ; 15 sprites, 4 bytes each
                                        ; x,y,oamptr,stat (b0=oamb8,b1=16x16)
                                        ; 4 byte header (#sobj,?,?,?)

    add esi,16
    dec cl
    jnz near .loop
.none
    mov esi,oamram
    mov edi,[C4Ram]
    mov ecx,544
.next
    mov al,[edi]
    mov [esi],al
    inc edi
    inc esi
    loop .next
    ret

SECTION .data
.addx dw 0
.addy dw 0

C4count   dd 0
C4usprptr dd 0
C4SprX    dw 0
C4SprY    dw 0
C4SprCnt  db 0
C4SprAttr db 0
C4SprOAM  db 0
C4SprFlip db 0
C4Timer   db 0
C4Timer2  db 0

SECTION .text

NEWSYM C4VBlank
    ret
NEWSYM C4ProcessSprites
    push ecx
    push esi
    push edi
    push ebx
    push edx
;    call C4ProcessVectors

;    call C4Edit

    mov esi,[C4Ram]
    mov dword[C4count],8
    mov cl,[esi+626h]
    mov byte[C4sprites],cl
    mov ecx,[C4sprites]
    shl ecx,2
    mov dword[C4ObjDisp],ecx
    mov ecx,128
;    cmp byte[esi+65],50h
;    jne .noincdisp
    mov dword[C4count],32
    sub ecx,[C4sprites]
.noincdisp
    add esi,[C4ObjDisp]
    ; Clear OAM to-be ram
.next
    mov byte[esi+1],0E0h
    add esi,4
    dec ecx
    jnz .next

    call C4ConvOAM

    pop edx
    pop ebx
    pop edi
    pop esi
    pop ecx
    ret

SECTION .data
NEWSYM SprValAdd, db 0
C4Data dd 0
C4sprites dd 0
SECTION .text

NEWSYM InitC4
    pushad
    mov esi,[romdata]
    add esi,4096*1024
    mov [C4Data],esi
    add dword[C4Data],128*1024
    mov [C4RamR],esi
    mov [C4RamW],esi
    mov [C4Ram],esi
    add dword[C4RamW],8192*4
    add dword[C4Ram],8192*8
    mov ecx,8192
.c4loop
    mov dword[esi],C4ReadReg
    mov dword[esi+8192*4],C4WriteReg
    mov dword[esi+8192*8],0
    add esi,4
    dec ecx
    jnz .c4loop
    mov esi,[C4RamW]
    mov dword[esi+1F4Fh*4],C4RegFunction
    mov esi,[C4Data]
    mov ecx,16*4096
.c4loopb
    mov dword[esi],0
    add esi,4
    loop .c4loopb
    popad
    ret

C4ClearSpr:
    mov esi,ebx
    mov edi,eax
;    xor ecx,ecx
;    mov cx,[eax+1F44h]
;    sub cx,6000h
;    add eax,ecx
    shl ch,3
.scloop2
    mov cl,byte[C4SprPos]
    shl cl,2
.scloop
    mov byte[edi],0
    mov byte[edi+2000h],0
    inc edi
    dec cl
    jnz .scloop
    dec ch
    jnz .scloop2
    ret

C4SprBitPlane:
    mov edi,eax
    shl ebx,2
.scloop3
    mov ch,[C4SprPos]
    push esi
.scloop4
    push esi
    mov cl,8
.loop
    mov dh,8
    mov dl,80h
    mov eax,[esi]
.nextd
    test al,1
    jz .not0
    or byte[edi],dl
.not0
    test al,2
    jz .not1
    or byte[edi+1],dl
.not1
    test al,4
    jz .not2
    or byte[edi+16],dl
.not2
    test al,8
    jz .not3
    or byte[edi+17],dl
.not3
    shr eax,4
    shr dl,1
    dec dh
    jnz .nextd
    add esi,ebx
    add edi,2
    dec cl
    jnz .loop
    add edi,16
    pop esi
    add esi,4
    dec ch
    jnz .scloop4
    pop esi
    shl ebx,3
    add esi,ebx
    add edi,dword[C4SprPtrInc]
    shr ebx,3
    dec byte[C4SprPos+1]
    jnz .scloop3
.end
    ret

SECTION .data
C4XXScale dw 0
C4XYScale dw 0
C4YXScale dw 0
C4YYScale dw 0
C4CXPos dw 0
C4CYPos dw 0
C4CXMPos dd 0
C4CYMPos dd 0
C4PCXMPos dd 0
C4PCYMPos dd 0
SECTION .text

DoScaleRotate:
    pushad
    mov esi,eax
    ; Calculate X scaler
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[CosTable+eax*2]
    mov bx,[esi+1F8Fh]
    test bx,8000h
    jz .notover
    mov bx,7FFFh
.notover
    imul bx
    add ax,ax
    adc dx,dx
    mov [C4XXScale],dx
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[SinTable+eax*2]
    imul bx
    add ax,ax
    adc dx,dx
    mov [C4XYScale],dx
    ; Calculate Y scaler
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[CosTable+eax*2]
    mov bx,[esi+1F92h]
    test bx,8000h
    jz .notoverb
    mov bx,7FFFh
.notoverb
    imul bx
    add ax,ax
    add dx,dx
    mov [C4YYScale],dx
    mov ax,[esi+1F80h]
    and eax,01FFh
    mov ax,[SinTable+eax*2]
    imul bx
    add ax,ax
    adc dx,dx
    neg dx
    mov [C4YXScale],dx
    cmp word[esi+1F80h],0
    jne .effect
    cmp word[esi+1F92h],1000h
    jne .effect
    mov word[C4YYScale],1000h
    mov word[C4YXScale],0
.effect
    ; Calculate Pixel Resolution
    mov cl,byte[C4SprPos]
    shl cl,3
    mov byte[C4SprPos+2],cl
    mov cl,byte[C4SprPos+1]
    shl cl,3
    mov byte[C4SprPos+3],cl
    ; Calculate Positions
    ; (1-scale)*(pos/2)
    xor eax,eax
    mov al,[C4SprPos+2]
    shl eax,11
    mov [C4PCXMPos],eax
    xor eax,eax
    mov al,[C4SprPos+3]
    shl eax,11
    mov [C4PCYMPos],eax

    mov bx,[C4XXScale]
    xor eax,eax
    mov al,[C4SprPos+2]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCXMPos],edx
    mov bx,[C4YXScale]
    xor eax,eax
    mov al,[C4SprPos+3]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCXMPos],edx

    mov bx,[C4XYScale]
    xor eax,eax
    mov al,[C4SprPos+2]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCYMPos],edx
    mov bx,[C4YYScale]
    xor eax,eax
    mov al,[C4SprPos+3]
    shr ax,1
    imul bx
    shl edx,16
    mov dx,ax
    sub [C4PCYMPos],edx

    ; Start loop
    mov word[C4CYPos],0
    xor edi,edi
.loop
    mov ecx,[C4PCXMPos]
    mov [C4CXMPos],ecx
    mov ecx,[C4PCYMPos]
    mov [C4CYMPos],ecx
    mov al,[C4SprPos+2]
    mov byte[C4CXPos],al
.loop2
    xor eax,eax
    mov al,[C4SprPos+2]
    mov ebx,[C4CXMPos]
    sar ebx,12
    cmp ebx,eax
    jae near .blank
    xor eax,eax
    mov al,[C4SprPos+3]
    mov ebx,[C4CYMPos]
    sar ebx,12
    cmp ebx,eax
    jae near .blank
    ; Get pixel value
    mov ebx,[C4CYMPos]
    xor eax,eax
    shr ebx,12
    mov al,[C4SprPos+2]
    mul ebx
    mov ebx,[C4CXMPos]
    shr ebx,12
    add eax,ebx
    mov ebx,[C4SprPtr]
    test al,1
    jnz .upperb
    shr eax,1
    add ebx,eax
    mov al,[ebx]
    jmp .lowerb
.upperb
    shr eax,1
    add ebx,eax
    mov al,[ebx]
    shr al,4
.lowerb
    mov ebx,edi
    shr ebx,1
    add ebx,esi
    test edi,1
    jnz .upperb2
    and al,0Fh
    and byte[ebx+2000h],0F0h
    or byte[ebx+2000h],al
    jmp .done
.upperb2
    shl al,4
    and byte[ebx+2000h],0Fh
    or byte[ebx+2000h],al
    jmp .done
.blank
    mov eax,edi
    shr eax,1
    add eax,esi
    test edi,1
    jnz .upper
    and byte[eax+2000h],0F0h
    jmp .done
.upper
    and byte[eax+2000h],0Fh
.done
    movsx eax,word[C4XXScale]
    add [C4CXMPos],eax
    movsx eax,word[C4XYScale]
    add [C4CYMPos],eax
    inc edi
    dec byte[C4CXPos]
    jne near .loop2
    movsx eax,word[C4YXScale]
    add [C4PCXMPos],eax
    movsx eax,word[C4YYScale]
    add [C4PCYMPos],eax
    inc word[C4CYPos]
    mov al,[C4SprPos+3]
    cmp byte[C4CYPos],al
    jne near .loop
.noimage
    popad
    ret

DoScaleRotate2:
    pushad
    xor ebx,ebx
    mov bx,[eax+1F8Fh]
    cmp bx,1000h
    ja .scaled
    mov bx,1000h
.scaled
    mov [C4SprScale],ebx
    xor ebx,ebx
    mov bx,[eax+1F92h]
    cmp bx,1000h
    ja .scaledb
    mov bx,1000h
.scaledb
    mov [C4SprScaleY],ebx
    mov cl,[C4SprPos]
    shl cl,3
    mov ch,cl
    xor ebx,ebx
.leftovercheck
    dec ch
    add ebx,[C4SprScale]
.leftovercheckb
    cmp ebx,1000h
    jb .leftovercheck
    sub ebx,1000h
    dec cl
    jz .donecheck
    jmp .leftovercheckb
.donecheck
    shr ch,1
    mov cl,ch
    and ecx,0FFh
    mov esi,ecx

    mov cl,[C4SprPos+1]
    shl cl,3
    mov ch,cl
    xor ebx,ebx
.leftovercheckc
    dec ch
    add ebx,[C4SprScaleY]
.leftovercheckd
    cmp ebx,1000h
    jb .leftovercheckc
    sub ebx,1000h
    dec cl
    jz .donecheckc
    jmp .leftovercheckd
.donecheckc
    shr ch,1
    mov cl,ch
    and ecx,0FFh
    push eax
    xor eax,eax
    mov al,[C4SprPos]
    shl al,3
    mul ecx
    add esi,eax
    pop eax

    mov dword[C4SprScalerY],0
    xor edi,edi
    mov cl,[C4SprPos+1]
    shl cl,3
    mov [C4SprPos+3],cl
.next
    push esi
    push edi
    xor ecx,ecx
    mov cl,[C4SprPos]
    shl cl,3
    mov ch,cl
    mov dword[C4SprScaler],0
    xor edx,edx
.loop
    mov edx,edi
    shr edx,1
    add edx,[C4SprPtr]
    mov bl,[edx]
    test esi,1
    jz .notupper
    shr bl,4
.notupper
    and bl,0Fh
    mov edx,esi
    shr edx,1
    test esi,1
    jz .notupperb
    shl bl,4
    or byte[eax+edx+2000h],bl
    jmp .notlowerb
.notupperb
    or byte[eax+edx+2000h],bl
.notlowerb
    inc esi
    mov ebx,[C4SprScale]
    add dword[C4SprScaler],ebx
    dec ch
.nextcheck
    cmp dword[C4SprScaler],1000h
    jb near .loop
    sub dword[C4SprScaler],1000h
    inc edi
    dec cl
    jz .done
    jmp .nextcheck
.done
    pop edi
    pop esi
    xor edx,edx
    mov dl,[C4SprPos]
    shl dl,3
    add esi,edx

    mov ebx,[C4SprScaleY]
    add dword[C4SprScalerY],ebx
.nextcheckb
    cmp dword[C4SprScalerY],1000h
    jb near .next
    sub dword[C4SprScalerY],1000h
    add edi,edx
    dec byte[C4SprPos+3]
    jz .doneb
    jmp .nextcheckb
.doneb

    popad
    ret

C4SprScaleR:
    push ecx
    push ebx
    push edx
    push esi
    push edi
    mov dword[C4SprPtrInc],0
    xor ebx,ebx
    mov bl,[eax+1F42h]
    shl ebx,16
    mov bx,[eax+1F40h]
    add bx,bx
    shr ebx,1
    add ebx,[romdata]
    mov ch,[eax+1F8Ch]
    shr ch,3
    mov cl,[eax+1F89h]
    shr cl,3
    mov [C4SprPos],cx
    mov [C4SprPtr],ebx

    call C4ClearSpr

    call DoScaleRotate

    mov esi,eax
    add esi,2000h
    xor ebx,ebx
    mov bl,[C4SprPos]
    call C4SprBitPlane
    pop edi
    pop esi
    pop edx
    pop ebx
    pop ecx
    ret

C4SprRotateR:
    push ecx
    push ebx
    push edx
    push esi
    push edi
    xor ebx,ebx
    mov ebx,600h
    add ebx,[C4Ram]
    mov [C4SprPtr],esi
    mov ch,[eax+1F8Ch]
    shr ch,3
    mov cl,[eax+1F89h]
    shr cl,3
    add ch,2
    mov [C4SprPos],cx
    mov dword[C4SprPtrInc],64
    mov [C4SprPtr],ebx
    sub byte[C4SprPos+1],2
    call C4ClearSpr

    call DoScaleRotate
    mov esi,eax
    add esi,2000h
    xor ebx,ebx
    mov bl,[C4SprPos]
    add byte[C4SprPos+1],2
    call C4SprBitPlane
    pop edi
    pop esi
    pop edx
    pop ebx
    pop ecx
    ret

C4SprDisintegrate:
    pushad
    mov dword[C4SprPtrInc],0
    xor ebx,ebx
    mov bl,[eax+1F42h]
    shl ebx,16
    mov bx,[eax+1F40h]
    add bx,bx
    shr ebx,1
    add ebx,[romdata]
    mov ch,[eax+1F8Ch]
    shr ch,3
    mov cl,[eax+1F89h]
    shr cl,3
    mov [C4SprPos],cx
    mov [C4SprPtr],ebx

    call C4ClearSpr

    mov esi,[C4Ram]
    xor ebx,ebx
    mov bx,[esi+1F86h]
    xor eax,eax
    mov al,[esi+1F89h]
    shr al,1
    mul ebx
    neg eax
    xor ebx,ebx
    mov bl,[esi+1F89h]
    shr bl,1
    shl ebx,8
    add eax,ebx
    push eax
    xor ebx,ebx
    mov bx,[esi+1F8Fh]
    xor eax,eax
    mov al,[esi+1F8Ch]
    shr al,1
    mul ebx
    neg eax
    xor ebx,ebx
    mov bl,[esi+1F8Ch]
    shr bl,1
    shl ebx,8
    add ebx,eax
    mov edx,ebx
    pop ebx
    mov esi,[C4Ram]
    mov cl,[esi+1F89h]
    mov ch,[esi+1F8Ch]
    mov [C4SprPos+2],cx
    movsx eax,word[esi+1F86h]
    mov [.scalex],eax
    movsx eax,word[esi+1F8Fh]
    mov [.scaley],eax
    mov esi,[C4SprPtr]
    mov edi,[C4Ram]
    add edi,2000h

    ; convert to 8-bit bitmap
    mov cx,[C4SprPos+2]
    shr cl,1
.loop2
    mov al,[esi]
    mov [edi],al
    mov al,[esi]
    shr al,4
    mov [edi+1],al
    inc esi
    add edi,2
    dec cl
    jnz .loop2
    dec ch
    jnz .loop2

    mov edi,[C4Ram]
    add edi,4000h
    mov ecx,2000h
.lp
    mov byte[edi],0
    inc edi
    loop .lp

    mov esi,[C4Ram]
    add esi,2000h
    mov edi,[C4Ram]
    add edi,4000h

    mov cx,[C4SprPos+2]
.next2
    push ebx
.next
    xor eax,eax
    mov ah,[C4SprPos+2]
    cmp ebx,eax
    jae .fail
    xor eax,eax
    mov ah,[C4SprPos+3]
    cmp edx,eax
    jae .fail
    push ecx
    push edx
    xor eax,eax
    mov al,[C4SprPos+2]
    xor ecx,ecx
    mov cl,dh
    mul ecx
    mov ecx,ebx
    shr ecx,8
    add eax,ecx
    mov dl,[esi]
    cmp eax,2000h
    jae .skipdisi
    mov [edi+eax],dl
.skipdisi
    pop edx
    pop ecx
.fail
    inc esi
    add ebx,[.scalex]
    dec cl
    jnz near .next
    pop ebx
    add edx,[.scaley]
    mov cl,[C4SprPos+2]
    dec ch
    jnz near .next2

.skipall
    ; convert to 4-bit bitmap
    mov esi,[C4Ram]
    add esi,4000h
    mov edi,[C4Ram]
    add edi,6000h
    mov cx,[C4SprPos+2]
    shr cl,1
.loop
    mov al,[esi]
    mov [edi],al
    mov al,[esi+1]
    shl al,4
    or [edi],al
    inc edi
    add esi,2
    dec cl
    jnz .loop
    dec ch
    jnz .loop

    mov esi,[C4Ram]
    add esi,6000h
;    mov esi,[C4SprPtr]
    mov eax,[C4Ram]
    xor ebx,ebx
    mov bl,[C4SprPos]
    call C4SprBitPlane

    popad
    ret

SECTION .data
.scalex dd 0
.scaley dd 0
SECTION .text

C4BitPlaneWave:
    pushad
    mov esi,[C4Ram]
    mov dword[.temp],10h
    xor eax,eax
    mov al,[esi+1F83h]
    mov dword[.waveptr],eax
    mov word[.temp+4],0C0C0h
    mov word[.temp+6],03F3Fh
.bmloopb
    mov edi,[C4Ram]
    add edi,[.waveptr]
    xor eax,eax
    movsx ax,byte[edi+$0B00]
    neg ax
    sub ax,16
    mov edi,[C4Ram]
    add edi,0A00h
    xor ecx,ecx
.bmloopa
    mov ebx,[.bmptr+ecx*4]
    mov dx,[.temp+6]
    and [esi+ebx],dx
    xor dx,dx
    cmp ax,0
    jl .less
    mov dx,0FF00h
    cmp ax,8
    jae .less
    mov dx,[edi+eax*2]
.less
    and dx,[.temp+4]
    or [esi+ebx],dx
    inc ax
    inc ecx
    cmp ecx,28h
    jne .bmloopa
    add dword[.waveptr],1
    and dword[.waveptr],07Fh
    ror word[.temp+4],2
    ror word[.temp+6],2
    cmp word[.temp+4],0C0C0h
    jne near .bmloopb
    add esi,16
.bmloopa2b
    mov edi,[C4Ram]
    add edi,[.waveptr]
    xor eax,eax
    movsx ax,byte[edi+$0B00]
    neg ax
    sub ax,16
    mov edi,[C4Ram]
    add edi,0A00h
    xor ecx,ecx
.bmloopa2
    mov ebx,[.bmptr+ecx*4]
    mov dx,[.temp+6]
    and [esi+ebx],dx
    xor dx,dx
    cmp ax,0
    jl .less2
    mov dx,0FF00h
    cmp ax,8
    jae .less2
    mov dx,[edi+eax*2+16]
.less2
    and dx,[.temp+4]
    or [esi+ebx],dx
    inc ax
    inc ecx
    cmp ecx,28h
    jne .bmloopa2
    add dword[.waveptr],1
    and dword[.waveptr],07Fh
    ror word[.temp+4],2
    ror word[.temp+6],2
    cmp word[.temp+4],0C0C0h
    jne near .bmloopa2b
    add esi,16
    dec dword[.temp]
    jnz near .bmloopb
    mov esi,[C4Ram]
;    mov cx,[esi+1F80h]
;    mov [C4values],cx
;    mov cx,[esi+1F83h]
;    mov [C4values+2],cx
    popad
    ret

SECTION .data
.bmptr dd 0000h,0002h,0004h,0006h,0008h,000Ah,000Ch,000Eh
       dd 0200h,0202h,0204h,0206h,0208h,020Ah,020Ch,020Eh
       dd 0400h,0402h,0404h,0406h,0408h,040Ah,040Ch,040Eh
       dd 0600h,0602h,0604h,0606h,0608h,060Ah,060Ch,060Eh
       dd 0800h,0802h,0804h,0806h,0808h,080Ah,080Ch,080Eh
.temp dd 0,0
.waveptr dd 0
;C4X1 dw 0
;C4Y1 dw 0
;C4Z1 dw 0
;C4X2 dw 0
;C4Y2 dw 0
;C4Z2 dw 0
;C4Col dw 0

SECTION .text
C4DrawLine:
    pushad

    ; transform both coordinates
    push esi
    mov ax,word[C4X1]
    mov [C4WFXVal],ax
    mov ax,word[C4Y1]
    mov [C4WFYVal],ax
    mov ax,word[C4Z1]
    mov [C4WFZVal],ax
    mov al,[esi+1F90h]
    mov [C4WFScale],al
    mov al,[esi+1F86h]
    mov [C4WFX2Val],al
    mov al,[esi+1F87h]
    mov [C4WFY2Val],al
    mov al,[esi+1F88h]
    mov [C4WFDist],al
    call C4TransfWireFrame2
    mov ax,[C4WFXVal]
    mov word[C4X1],ax
    mov ax,[C4WFYVal]
    mov word[C4Y1],ax

    mov ax,word[C4X2]
    mov [C4WFXVal],ax
    mov ax,word[C4Y2]
    mov [C4WFYVal],ax
    mov ax,word[C4Z2]
    mov [C4WFZVal],ax
    call C4TransfWireFrame2
    mov ax,[C4WFXVal]
    mov word[C4X2],ax
    mov ax,[C4WFYVal]
    mov word[C4Y2],ax

    add word[C4X1],48
    add word[C4Y1],48
    add word[C4X2],48
    add word[C4Y2],48
    shl dword[C4X1],8
    shl dword[C4X2],8
    shl dword[C4Y1],8
    shl dword[C4Y2],8
    ; get line info
    mov ax,[C4X1+1]
    mov [C4WFXVal],ax
    mov ax,[C4Y1+1]
    mov [C4WFYVal],ax
    mov ax,[C4X2+1]
    mov [C4WFX2Val],ax
    mov ax,[C4Y2+1]
    mov [C4WFY2Val],ax
    call C4CalcWireFrame
    xor ecx,ecx
    mov cx,[C4WFDist]
    or ecx,ecx
    jnz .not0
    mov ecx,1
.not0
    movsx eax,word[C4WFXVal]
    mov [C4X2],eax
    movsx eax,word[C4WFYVal]
    mov [C4Y2],eax
    pop esi
    ; render line
.loop
    ; plot pixel
    cmp word[C4X1+1],0
    jl near .noplot
    cmp word[C4Y1+1],0
    jl near .noplot
    cmp word[C4X1+1],95
    jg near .noplot
    cmp word[C4Y1+1],95
    jg near .noplot
    xor eax,eax
    mov dx,[C4Y1+1]
    shr dx,3
    mov ax,dx
    shl ax,6
    shl dx,8
    sub dx,ax
    mov ax,[C4X1+1]
    shr ax,3
    shl ax,4
    add ax,dx
    mov dx,[C4Y1+1]
    and dx,07h
    add dx,dx
    add ax,dx
    mov dl,07Fh
    push ecx
    mov cl,[C4X1+1]
    and cl,07h
    ror dl,cl
    pop ecx
    and byte[esi+eax+300h],dl
    and byte[esi+eax+301h],dl
    xor dl,0FFh
    test byte[C4Col],1
    jz .nocolor0
    or byte[esi+eax+300h],dl
.nocolor0
    test byte[C4Col],2
    jz .nocolor1
    or byte[esi+eax+301h],dl
.nocolor1
.noplot
    mov eax,[C4X2]
    add [C4X1],eax
    mov eax,[C4Y2]
    add [C4Y1],eax
    dec ecx
    jnz near .loop
    popad
    ret

DrawWireFrame:
    mov esi,[C4Ram]
    mov edi,esi
    xor ebx,ebx
    mov bl,[esi+1F82h]
    shl ebx,16
    mov bx,[esi+1F80h]
    add bx,bx
    shr ebx,1
    add ebx,[romdata]
    mov edi,ebx
    xor ecx,ecx
    mov cl,[esi+295h]
.loop
    xor eax,eax
    mov al,[esi+1F82h]
    shl eax,16
    mov al,[edi+1]
    mov ah,[edi+0]
    mov edx,edi
.nextprev
    cmp ax,0FFFFh
    jne .notprev
    sub edx,5
    mov al,[edx+3]
    mov ah,[edx+2]
    jmp .nextprev
.notprev
    add ax,ax
    shr eax,1
    add eax,[romdata]
    xor edx,edx
    mov dl,[esi+1F82h]
    shl edx,16
    mov dl,[edi+3]
    mov dh,[edi+2]
;    mov [C4values+6],dx
    add dx,dx
    shr edx,1
    add edx,[romdata]
    xor ebx,ebx
    mov bh,[eax]
    mov bl,[eax+1]
    mov [C4X1],ebx
    mov bh,[eax+2]
    mov bl,[eax+3]
    mov [C4Y1],ebx
    mov bh,[eax+4]
    mov bl,[eax+5]
    mov [C4Z1],ebx
    mov bh,[edx]
    mov bl,[edx+1]
    mov [C4X2],ebx
    mov bh,[edx+2]
    mov bl,[edx+3]
    mov [C4Y2],ebx
    mov bh,[edx+4]
    mov bl,[edx+5]
    mov [C4Z2],ebx
    mov al,[edi+4]
    mov [C4Col],al
    add edi,5
    call C4DrawLine
    dec ecx
    jnz near .loop
    ret

SECTION .data
C4X1 dd 0
C4Y1 dd 0
C4Z1 dd 0
C4X2 dd 0
C4Y2 dd 0
C4Z2 dd 0
C4Col dd 0
SECTION .text

WireFrameB:
    pushad
    ; 28EECA
    ; 7F80 (3bytes) = pointer to data
    ; 7F86-7F88 = rotation, 7F90 = scale (/7A?)
    ; 6295 = # of lines, 7FA5 = ???
    mov esi,[C4Ram]
    add esi,300h
    mov ecx,16*12*3
.loop
    mov dword[esi],0
    add esi,4
    loop .loop
    call DrawWireFrame

    mov esi,[C4Ram]
    mov cx,[esi+1FA5h]
;    mov [C4values],cx
;    mov cx,[esi+1F86h]
;    mov [C4values],cx
;    mov cx,[esi+1F88h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F90h]
;    mov [C4values+4],cx
    popad
    ret

WireFrameB2:
    pushad
    call DrawWireFrame
    popad
    ret

C4WireFrame:
    pushad
    mov esi,[C4Ram]
    mov ax,[esi+1F83h]
    and ax,0FFh
    mov [C4WFX2Val],ax
;    mov [C4values],ax
    mov ax,[esi+1F86h]
    and ax,0FFh
    mov [C4WFY2Val],ax
;    mov [C4values+2],ax
    mov ax,[esi+1F89h]
    and ax,0FFh
    mov [C4WFDist],ax
;    mov [C4values+4],ax
    mov ax,[esi+1F8Ch]
    and ax,0FFh
    mov [C4WFScale],ax
;    mov [C4values+6],ax

    ; transform vertices (MMX2 - 36 vertices, 54 lines)
    xor ecx,ecx
    mov cx,[esi+1F80h]
    xor al,al
.loop
    mov ax,[esi+1]
    mov [C4WFXVal],ax
    mov ax,[esi+5]
    mov [C4WFYVal],ax
    mov ax,[esi+9]
    mov [C4WFZVal],ax
    push esi
    push ecx
    call C4TransfWireFrame
    pop ecx
    pop esi
    ; Displace
    mov ax,[C4WFXVal]
    add ax,80h
    mov [esi+1],ax
    mov ax,[C4WFYVal]
    add ax,50h
    mov [esi+5],ax
    add esi,10h
    loop .loop
    ; Uses 6001,6005,6600,6602,6605

    mov esi,[C4Ram]
    mov word[esi+$600],23
    mov word[esi+$602],60h
    mov word[esi+$605],40h
    mov word[esi+$600+8],23
    mov word[esi+$602+8],60h
    mov word[esi+$605+8],40h

    xor ecx,ecx
    mov cx,[esi+0B00h]
    mov edi,esi
    add edi,0B02h
.lineloop
    xor eax,eax
    mov al,[edi]
    shl eax,4
    add eax,[C4Ram]
    mov bx,[eax+1]
    mov [C4WFXVal],bx
    mov bx,[eax+5]
    mov [C4WFYVal],bx
    xor eax,eax
    mov al,[edi+1]
    shl eax,4
    add eax,[C4Ram]
    mov bx,[eax+1]
    mov [C4WFX2Val],bx
    mov bx,[eax+5]
    mov [C4WFY2Val],bx
    push esi
    push edi
    push ecx
    call C4CalcWireFrame
    pop ecx
    pop edi
    pop esi
    mov ax,[C4WFDist]
    or ax,ax
    jnz .yeswire
    mov ax,1
.yeswire
    mov word[esi+$600],ax
    mov ax,[C4WFXVal]
    mov word[esi+$602],ax
    mov ax,[C4WFYVal]
    mov word[esi+$605],ax
    add edi,2
    add esi,8
    dec ecx
    jnz near .lineloop
.done
    popad
    ret

C4Transform:
    ; 7F81,4,7,9,A,B,0,1,D
    pushad
    mov esi,[C4Ram]
    mov ax,word[esi+1F81h]
    mov [C4WFXVal],ax
    mov ax,word[esi+1F84h]
    mov [C4WFYVal],ax
    mov ax,word[esi+1F87h]
    mov [C4WFZVal],ax
    mov al,[esi+1F90h]
    mov [C4WFScale],al
    mov al,[esi+1F89h]
    mov [C4WFX2Val],al
    mov al,[esi+1F8Ah]
    mov [C4WFY2Val],al
    mov al,[esi+1F8Bh]
    mov [C4WFDist],al
    call C4TransfWireFrame2
    mov ax,[C4WFXVal]
    mov word[esi+1F80h],ax
    mov ax,[C4WFYVal]
    mov word[esi+1F83h],ax
    popad
    ret

SECTION .data
C4SprPos dd 0
C4SprScale dd 0
C4SprScaleY dd 0
C4SprScaler dd 0
C4SprScalerY dd 0
C4SprPtr dd 0
C4SprPtrInc dd 0
NEWSYM C4values, dd 0,0,0
SECTION .text

C4activate:
    cmp ecx,1F4Fh
    jne .noc4test
    push esi
    mov esi,[C4Ram]
    cmp byte[esi+1F4Dh],0Eh
    jne .befnoc4test
    test al,0C3h
    jnz .befnoc4test
    shr al,2
    mov [esi+1F80h],al
    pop esi
    ret
.befnoc4test
    pop esi
.noc4test
    cmp al,00h
    je near .dosprites
    cmp al,01h
    je near .dowireframe
    cmp al,05h          ; ?
    je near .propulsion
    cmp al,0Dh          ; ?
    je near .equatevelocity
    cmp al,10h          ; supply angle+distance, return x/y displacement
    je near .direction
    cmp al,13h          ; Convert polar coordinates to rectangular 2 (similar to 10)
    je near .polarcord2
    cmp al,15h          ; ?
    je near .calcdistance
    cmp al,1Fh          ; supply x/y displacement, return angle (+distance?)
    je near .calcangle
    cmp al,22h          ; supply x/y displacement, return angle (+distance?)
    je near .linearray
    cmp al,25h
    je near .multiply
    cmp al,2Dh          ; ???
    je near .transform
    cmp al,40h
    je near .sum
    cmp al,54h
    je near .square
    cmp al,5Ch
    je near .immediatereg
    cmp al,89h
    je near .immediaterom
    ret
.dowireframe
    call WireFrameB
    ret
.linearray
    pushad
    ; C,F,0,3,6,9 -> 6800 (E1h bytes)
    ; 0,3 = screen scroll coordinates
    ; 6,9 = light source coordinates
    ; C,F = angle of both arrays
    mov esi,[C4Ram]
    xor ecx,ecx
.loopline
    ; process position
    xor eax,eax
    mov al,[esi+1F8Ch]
    or ecx,ecx
    jz .secondlineb
    mov al,[esi+1F8Fh]
.secondlineb
    test al,80h
    jz .notua
    or ah,1
.notua
    movsx ebx,word[CosTable+eax*2]
    mov ax,word[SinTable+eax*2]
    shl eax,16
    cmp ebx,0
    je near .finish
    xor edx,edx
    test eax,80000000h
    jz .notnegline
    mov edx,0FFFFFFFFh
.notnegline
    idiv ebx
    mov [C4Temp],eax
    xor edx,edx
    mov bx,[esi+1F83h]
    sub bx,[esi+1F89h]
    dec bx
    movsx ebx,bx
.nextline
    test ebx,80000000h
    jnz .none
    mov eax,[C4Temp]
    imul eax,ebx
    sar eax,16
    sub ax,[esi+1F80h]
    add ax,[esi+1F86h]
    inc ax
    add ax,cx
    cmp ax,0
    jge .not0line
    xor ax,ax
    or ecx,ecx
    jz .not0line
    mov byte[esi+edx+$800],1
.not0line
    cmp ax,255
    jl .not255line
    mov ax,255
.not255line
    jmp .doneline
.none
    mov al,1
    sub al,cl
.doneline
    or ecx,ecx
    jnz .secondline
    mov [esi+edx+$800],al
    jmp .firstline
.secondline
    mov [esi+edx+$900],al
.firstline
    inc ebx
    inc edx
    cmp edx,0E1h
    jne .nextline
    or ecx,ecx
    jnz .finish
    mov ecx,1
    jmp .loopline
.finish
    mov cx,[C4Temp]
;    mov [C4values],cx
    mov cx,[C4Temp+2]
;    mov [C4values+2],cx
    mov cx,[esi+1F8Ch]
;    mov [C4values+4],cx
    mov cx,[esi+1F8Fh]
;    mov [C4values+6],cx
    popad
    ret
.propulsion
    pushad
    ; 81 = 5B, 83 = 0x300
    ; 0x300 = /1, 0x280 = /4
    mov esi,[C4Ram]

    mov cx,[esi+1F83h]
    mov [C4values+2],cx
    mov cx,[esi+1F81h]
    mov [C4values],cx
    xor bx,bx

;    mov ax,256*256
    xor ax,ax
    mov dx,1
    mov bx,[esi+1F83h]
    or dx,dx
    jz .done
    idiv bx
    mov [C4values+6],ax
    mov bx,[esi+1F81h]
    imul bx
    shl edx,16
    mov dx,ax
    sar edx,8
.done
    mov word[esi+1F80h],dx
    mov [C4values+4],dx

;    and eax,1FFh
;    mov bx,[SinTable+eax*2]
;    mov ax,[esi+1F81h]          ; distance?
;    imul bx
;    mov ax,dx
;    shl ax,1
;    shl dx,3
;    add dx,ax

    popad
    ret
.polarcord2
    pushad
    mov esi,[C4Ram]
    xor ecx,ecx
    mov cx,[esi+1F80h]
    and ecx,1FFh
    movsx eax,word[esi+1F83h]
    add eax,eax
    movsx ebx,word[CosTable+ecx*2]
    imul ebx,eax
    sar ebx,8
    adc ebx,0
    mov [esi+1F86h],ebx
    movsx ebx,word[SinTable+ecx*2]
    imul ebx,eax
    sar ebx,8
    adc ebx,0
    mov [esi+1F89h],bx
    sar ebx,16
    mov [esi+1F8Bh],bl
    popad
    ret
.dosprites
;    mov byte[debstop3],0
    push eax
    mov eax,[C4Ram]
    cmp byte[eax+1F4Dh],0
    je near .sprites
    cmp byte[eax+1F4Dh],3
    je near .scaler
    cmp byte[eax+1F4Dh],5
    je near .lines
    cmp byte[eax+1F4Dh],7
    je near .rotater
    cmp byte[eax+1F4Dh],8
    je near .wireframeb
    cmp byte[eax+1F4Dh],0Bh
    je near .disintegrate
    cmp byte[eax+1F4Dh],0Ch
    je near .bitmap
    pop eax
    ret
.wireframeb
    pop eax
    call WireFrameB2
    ret
.sprites
    pop eax
    call C4ProcessSprites
    ret
.disintegrate
    call C4SprDisintegrate
    pop eax
    ret
.dolines
;    mov byte[debstop3],0
    ret
.bitmap
    call C4BitPlaneWave
    pop eax
    ret
.calcdistance
    pushad
    mov esi,[C4Ram]
    mov bx,[esi+1F80h]
    mov [C41FXVal],bx
    mov bx,[esi+1F83h]
    mov [C41FYVal],bx
;    mov eax,[C4Ram]
;    mov cx,[eax+1F80h]
;    mov [C4values+0],cx
;    mov cx,[eax+1F83h]
;    mov [C4values+2],cx
    call C4Op15
    mov eax,[C4Ram]
    mov bx,[C41FDist]
    mov [eax+1F80h],bx
;    mov word[eax+1F80h],50
;    mov cx,[eax+1F80h]
;    mov [C4values+4],cx
    popad
    ret
.calcangle
    pushad
    mov esi,[C4Ram]
    mov bx,[esi+1F80h]
    mov [C41FXVal],bx
    mov bx,[esi+1F83h]
    mov [C41FYVal],bx
    call C4Op1F
    mov eax,[C4Ram]
    mov bx,[C41FAngleRes]
    mov [eax+1F86h],bx
;    mov esi,[C4Ram]
;    mov cx,[esi+1F86h]
;    mov [C4values],cx
;    mov cx,[esi+1F80h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F83h]
;    mov [C4values+4],cx
    popad
    ret
.transform
    ; 7F81,4,7,9,A,B,0,1,D
;    mov byte[debstop3],0
    pushad
;    mov eax,[C4Ram]
    call C4Transform
;    mov word[eax+1F80h],0
;    mov word[eax+1F83h],0
    popad
    ret
.multiply
    pushad
    mov esi,[C4Ram]
    mov eax,[esi+1F80h]
    and eax,0FFFFFFh
    mov ebx,[esi+1F83h]
    and ebx,0FFFFFFh
    imul eax,ebx
    mov [esi+1F80h],eax
    popad
    ret
.sum
    pushad
    xor eax,eax
    xor ebx,ebx
    mov esi,[C4Ram]
    mov ecx,800h
.sumloop
    mov bl,byte[esi]
    inc esi
    add ax,bx
    dec ecx
    jnz .sumloop
    mov [esi+1F80h-0800h],ax
    popad
    ret
.square
    pushad
    xor edx,edx
    mov esi,[C4Ram]
    mov eax,[esi+1F80h]
    shl eax,8
    sar eax,8
    imul eax
    mov [esi+1F83h],eax
    mov [esi+1F87h],dx
    popad
    ret
.equatevelocity
    pushad
    mov esi,[C4Ram]
    mov bx,[esi+1F80h]
    mov [C41FXVal],bx
    mov bx,[esi+1F83h]
    mov [C41FYVal],bx
    mov bx,[esi+1F86h]
    mov [C41FDistVal],bx
    call C4Op0D
    mov bx,[C41FXVal]
    mov [esi+1F89h],bx
    mov bx,[C41FYVal]
    mov [esi+1F8Ch],bx
    popad
    ret


    pushad
    mov esi,[C4Ram]
    mov cx,[esi+$1F86]
    cmp cx,40h
    jb .nomult
    shr cx,7
.nomult
    mov ax,[esi+$1F80]
;    imul cx
    shl ax,4
    mov word[esi+$1F89],ax
    mov ax,[esi+$1F83]
;    imul cx
    shl ax,4
    mov word[esi+$1F8C],ax
;    mov cx,[esi+$1F80]
;    mov [C4values],cx
;    mov cx,[esi+$1F83]
;    mov [C4values+2],cx
;    mov cx,[esi+$1F86]
;    mov [C4values+4],cx
    popad
    ret
.lines
    call C4WireFrame
    pop eax
    ret
.scaler
    push esi
    push ecx
    mov esi,[C4Ram]
;    mov cx,[esi+1F8Fh]
;    mov [C4values],cx
;    mov cx,[esi+1F92h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F80h]
;    mov [C4values+4],cx
    pop ecx
    pop esi
    call C4SprScaleR
    pop eax
    ret
.rotater
    push esi
    push ecx
    mov esi,[C4Ram]
;    mov cx,[esi+1F8Fh]
;    mov [C4values],cx
;    mov cx,[esi+1F92h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F80h]
;    mov [C4values+4],cx
    pop ecx
    pop esi
    call C4SprRotateR
    pop eax
    ret
.direction
    push eax
    push ebx
    push esi
    push edx
    push ecx
    mov esi,[C4Ram]
    xor ecx,ecx
    mov ax,[esi+1F80h]
    and eax,1FFh
    mov bx,[CosTable+eax*2]
    mov ax,[esi+1F83h]
    imul bx
    add ax,ax
    adc dx,dx
    mov ax,dx
    movsx edx,dx
    mov [esi+1F86h],edx
    mov ax,[esi+1F80h]
    and eax,1FFh
    mov bx,[SinTable+eax*2]
    mov ax,[esi+1F83h]
    imul bx
    add ax,ax
    adc dx,dx
    mov ax,dx
    movsx edx,dx
    mov eax,edx
    sar eax,6
    sub edx,eax
    mov al,[esi+198Ch]
    mov [esi+1F89h],edx
    mov [esi+198Ch],al
;    mov cx,[esi+1F80h]
;    mov [C4values],cx
;    mov cx,[esi+1F83h]
;    mov [C4values+2],cx
;    mov cx,[esi+1F86h]
;    mov [C4values+4],cx
    pop ecx
    pop edx
    pop esi
    pop ebx
    pop eax
    ret
.immediaterom
    push eax
    mov eax,[C4Ram]
    mov byte[eax+1F80h],36h
    mov byte[eax+1F81h],43h
    mov byte[eax+1F82h],05h
    pop eax
    ret
.immediatereg
    push eax
    mov eax,[C4Ram]
    mov dword[eax+0*4],0FF000000h
    mov dword[eax+1*4],0FF00FFFFh
    mov dword[eax+2*4],0FF000000h
    mov dword[eax+3*4],00000FFFFh
    mov dword[eax+4*4],00000FFFFh
    mov dword[eax+5*4],07FFFFF80h
    mov dword[eax+6*4],0FF008000h
    mov dword[eax+7*4],07FFF007Fh
    mov dword[eax+8*4],0FFFF7FFFh
    mov dword[eax+9*4],0FF010000h
    mov dword[eax+10*4],00100FEFFh
    mov dword[eax+11*4],000FEFF00h
    pop eax
    ret

;NEWSYM C4RegFunction
    add ecx,[C4Ram]
    mov [ecx],al
    sub ecx,[C4Ram]
    cmp ecx,1F4Fh
    je near C4activate
    ret

NEWSYM C4ReadReg
    add ecx,[C4Ram]
    mov al,[ecx]
    sub ecx,[C4Ram]
    ret

NEWSYM C4RegFunction
NEWSYM C4WriteReg
    add ecx,[C4Ram]
    mov [ecx],al
    sub ecx,[C4Ram]
    cmp ecx,1F4Fh
    je near C4activate
    cmp ecx,1F47h
    je .C4Memcpy
    ret

.C4Memcpy
    pushad
    push dword [C4Ram]
    call C4LoaDMem
    pop eax
    popad
    ret

SECTION .data
SinTable:
dw $00000,$00192,$00324,$004B6,$00647,$007D9,$0096A,$00AFB,$00C8B,$00E1B,$00FAB
dw $01139,$012C8,$01455,$015E2,$0176D,$018F8,$01A82,$01C0B,$01D93,$01F19,$0209F
dw $02223,$023A6,$02528,$026A8,$02826,$029A3,$02B1F,$02C98,$02E11,$02F87,$030FB
dw $0326E,$033DE,$0354D,$036BA,$03824,$0398C,$03AF2,$03C56,$03DB8,$03F17,$04073
dw $041CE,$04325,$0447A,$045CD,$0471C,$04869,$049B4,$04AFB,$04C3F,$04D81,$04EBF
dw $04FFB,$05133,$05269,$0539B,$054CA,$055F5,$0571D,$05842,$05964,$05A82,$05B9D
dw $05CB4,$05DC7,$05ED7,$05FE3,$060EC,$061F1,$062F2,$063EF,$064E8,$065DD,$066CF
dw $067BD,$068A6,$0698C,$06A6D,$06B4A,$06C24,$06CF9,$06DCA,$06E96,$06F5F,$07023
dw $070E2,$0719E,$07255,$07307,$073B5,$0745F,$07504,$075A5,$07641,$076D9,$0776C
dw $077FA,$07884,$07909,$0798A,$07A05,$07A7D,$07AEF,$07B5D,$07BC5,$07C29,$07C89
dw $07CE3,$07D39,$07D8A,$07DD6,$07E1D,$07E5F,$07E9D,$07ED5,$07F09,$07F38,$07F62
dw $07F87,$07FA7,$07FC2,$07FD8,$07FE9,$07FF6,$07FFD,$07FFF,$07FFD,$07FF6,$07FE9
dw $07FD8,$07FC2,$07FA7,$07F87,$07F62,$07F38,$07F09,$07ED5,$07E9D,$07E5F,$07E1D
dw $07DD6,$07D8A,$07D39,$07CE3,$07C89,$07C29,$07BC5,$07B5D,$07AEF,$07A7D,$07A05
dw $0798A,$07909,$07884,$077FA,$0776C,$076D9,$07641,$075A5,$07504,$0745F,$073B5
dw $07307,$07255,$0719E,$070E2,$07023,$06F5F,$06E96,$06DCA,$06CF9,$06C24,$06B4A
dw $06A6D,$0698C,$068A6,$067BD,$066CF,$065DD,$064E8,$063EF,$062F2,$061F1,$060EC
dw $05FE3,$05ED7,$05DC7,$05CB4,$05B9D,$05A82,$05964,$05842,$0571D,$055F5,$054CA
dw $0539B,$05269,$05133,$04FFB,$04EBF,$04D81,$04C3F,$04AFB,$049B4,$04869,$0471C
dw $045CD,$0447A,$04325,$041CE,$04073,$03F17,$03DB8,$03C56,$03AF2,$0398C,$03824
dw $036BA,$0354D,$033DE,$0326E,$030FB,$02F87,$02E11,$02C98,$02B1F,$029A3,$02826
dw $026A8,$02528,$023A6,$02223,$0209F,$01F19,$01D93,$01C0B,$01A82,$018F8,$0176D
dw $015E2,$01455,$012C8,$01139,$00FAB,$00E1B,$00C8B,$00AFB,$0096A,$007D9,$00647
dw $004B6,$00324,$00192
dw $00000,$0FE6E,$0FCDC,$0FB4A,$0F9B9,$0F827,$0F696,$0F505,$0F375,$0F1E5,$0F055
dw $0EEC7,$0ED38,$0EBAB,$0EA1E,$0E893,$0E708,$0E57E,$0E3F5,$0E26D,$0E0E7,$0DF61
dw $0DDDD,$0DC5A,$0DAD8,$0D958,$0D7DA,$0D65D,$0D4E1,$0D368,$0D1EF,$0D079,$0CF05
dw $0CD92,$0CC22,$0CAB3,$0C946,$0C7DC,$0C674,$0C50E,$0C3AA,$0C248,$0C0E9,$0BF8D
dw $0BE32,$0BCDB,$0BB86,$0BA33,$0B8E4,$0B797,$0B64C,$0B505,$0B3C1,$0B27F,$0B141
dw $0B005,$0AECD,$0AD97,$0AC65,$0AB36,$0AA0B,$0A8E3,$0A7BE,$0A69C,$0A57E,$0A463
dw $0A34C,$0A239,$0A129,$0A01D,$09F14,$09E0F,$09D0E,$09C11,$09B18,$09A23,$09931
dw $09843,$0975A,$09674,$09593,$094B6,$093DC,$09307,$09236,$0916A,$090A1,$08FDD
dw $08F1E,$08E62,$08DAB,$08CF9,$08C4B,$08BA1,$08AFC,$08A5B,$089BF,$08927,$08894
dw $08806,$0877C,$086F7,$08676,$085FB,$08583,$08511,$084A3,$0843B,$083D7,$08377
dw $0831D,$082C7,$08276,$0822A,$081E3,$081A1,$08163,$0812B,$080F7,$080C8,$0809E
dw $08079,$08059,$0803E,$08028,$08017,$0800A,$08003,$08001,$08003,$0800A,$08017
dw $08028,$0803E,$08059,$08079,$0809E,$080C8,$080F7,$0812B,$08163,$081A1,$081E3
dw $0822A,$08276,$082C7,$0831D,$08377,$083D7,$0843B,$084A3,$08511,$08583,$085FB
dw $08676,$086F7,$0877C,$08806,$08894,$08927,$089BF,$08A5B,$08AFC,$08BA1,$08C4B
dw $08CF9,$08DAB,$08E62,$08F1E,$08FDD,$090A1,$0916A,$09236,$09307,$093DC,$094B6
dw $09593,$09674,$0975A,$09843,$09931,$09A23,$09B18,$09C11,$09D0E,$09E0F,$09F14
dw $0A01D,$0A129,$0A239,$0A34C,$0A463,$0A57E,$0A69C,$0A7BE,$0A8E3,$0AA0B,$0AB36
dw $0AC65,$0AD97,$0AECD,$0B005,$0B141,$0B27F,$0B3C1,$0B505,$0B64C,$0B797,$0B8E4
dw $0BA33,$0BB86,$0BCDB,$0BE32,$0BF8D,$0C0E9,$0C248,$0C3AA,$0C50E,$0C674,$0C7DC
dw $0C946,$0CAB3,$0CC22,$0CD92,$0CF05,$0D079,$0D1EF,$0D368,$0D4E1,$0D65D,$0D7DA
dw $0D958,$0DAD8,$0DC5A,$0DDDD,$0DF61,$0E0E7,$0E26D,$0E3F5,$0E57E,$0E708,$0E893
dw $0EA1E,$0EBAB,$0ED38,$0EEC7,$0F055,$0F1E5,$0F375,$0F505,$0F696,$0F827,$0F9B9
dw $0FB4A,$0FCDC,$0FE6E


CosTable:
dw $07FFF,$07FFD,$07FF6,$07FE9,$07FD8,$07FC2,$07FA7,$07F87,$07F62,$07F38,$07F09
dw $07ED5,$07E9D,$07E5F,$07E1D,$07DD6,$07D8A,$07D39,$07CE3,$07C89,$07C29,$07BC5
dw $07B5D,$07AEF,$07A7D,$07A05,$0798A,$07909,$07884,$077FA,$0776C,$076D9,$07641
dw $075A5,$07504,$0745F,$073B5,$07307,$07255,$0719E,$070E2,$07023,$06F5F,$06E96
dw $06DCA,$06CF9,$06C24,$06B4A,$06A6D,$0698C,$068A6,$067BD,$066CF,$065DD,$064E8
dw $063EF,$062F2,$061F1,$060EC,$05FE3,$05ED7,$05DC7,$05CB4,$05B9D,$05A82,$05964
dw $05842,$0571D,$055F5,$054CA,$0539B,$05269,$05133,$04FFB,$04EBF,$04D81,$04C3F
dw $04AFB,$049B4,$04869,$0471C,$045CD,$0447A,$04325,$041CE,$04073,$03F17,$03DB8
dw $03C56,$03AF2,$0398C,$03824,$036BA,$0354D,$033DE,$0326E,$030FB,$02F87,$02E11
dw $02C98,$02B1F,$029A3,$02826,$026A8,$02528,$023A6,$02223,$0209F,$01F19,$01D93
dw $01C0B,$01A82,$018F8,$0176D,$015E2,$01455,$012C8,$01139,$00FAB,$00E1B,$00C8B
dw $00AFB,$0096A,$007D9,$00647,$004B6,$00324,$00192,$00000,$0FE6E,$0FCDC,$0FB4A
dw $0F9B9,$0F827,$0F696,$0F505,$0F375,$0F1E5,$0F055,$0EEC7,$0ED38,$0EBAB,$0EA1E
dw $0E893,$0E708,$0E57E,$0E3F5,$0E26D,$0E0E7,$0DF61,$0DDDD,$0DC5A,$0DAD8,$0D958
dw $0D7DA,$0D65D,$0D4E1,$0D368,$0D1EF,$0D079,$0CF05,$0CD92,$0CC22,$0CAB3,$0C946
dw $0C7DC,$0C674,$0C50E,$0C3AA,$0C248,$0C0E9,$0BF8D,$0BE32,$0BCDB,$0BB86,$0BA33
dw $0B8E4,$0B797,$0B64C,$0B505,$0B3C1,$0B27F,$0B141,$0B005,$0AECD,$0AD97,$0AC65
dw $0AB36,$0AA0B,$0A8E3,$0A7BE,$0A69C,$0A57E,$0A463,$0A34C,$0A239,$0A129,$0A01D
dw $09F14,$09E0F,$09D0E,$09C11,$09B18,$09A23,$09931,$09843,$0975A,$09674,$09593
dw $094B6,$093DC,$09307,$09236,$0916A,$090A1,$08FDD,$08F1E,$08E62,$08DAB,$08CF9
dw $08C4B,$08BA1,$08AFC,$08A5B,$089BF,$08927,$08894,$08806,$0877C,$086F7,$08676
dw $085FB,$08583,$08511,$084A3,$0843B,$083D7,$08377,$0831D,$082C7,$08276,$0822A
dw $081E3,$081A1,$08163,$0812B,$080F7,$080C8,$0809E,$08079,$08059,$0803E,$08028
dw $08017,$0800A,$08003
dw $08001,$08003,$0800A,$08017,$08028,$0803E,$08059,$08079,$0809E,$080C8,$080F7,
dw $0812B,$08163,$081A1,$081E3,$0822A,$08276,$082C7,$0831D,$08377,$083D7,$0843B,
dw $084A3,$08511,$08583,$085FB,$08676,$086F7,$0877C,$08806,$08894,$08927,$089BF,
dw $08A5B,$08AFC,$08BA1,$08C4B,$08CF9,$08DAB,$08E62,$08F1E,$08FDD,$090A1,$0916A,
dw $09236,$09307,$093DC,$094B6,$09593,$09674,$0975A,$09843,$09931,$09A23,$09B18,
dw $09C11,$09D0E,$09E0F,$09F14,$0A01D,$0A129,$0A239,$0A34C,$0A463,$0A57E,$0A69C,
dw $0A7BE,$0A8E3,$0AA0B,$0AB36,$0AC65,$0AD97,$0AECD,$0B005,$0B141,$0B27F,$0B3C1,
dw $0B505,$0B64C,$0B797,$0B8E4,$0BA33,$0BB86,$0BCDB,$0BE32,$0BF8D,$0C0E9,$0C248,
dw $0C3AA,$0C50E,$0C674,$0C7DC,$0C946,$0CAB3,$0CC22,$0CD92,$0CF05,$0D079,$0D1EF,
dw $0D368,$0D4E1,$0D65D,$0D7DA,$0D958,$0DAD8,$0DC5A,$0DDDD,$0DF61,$0E0E7,$0E26D,
dw $0E3F5,$0E57E,$0E708,$0E893,$0EA1E,$0EBAB,$0ED38,$0EEC7,$0F055,$0F1E5,$0F375,
dw $0F505,$0F696,$0F827,$0F9B9,$0FB4A,$0FCDC,$0FE6E,$00000,$00192,$00324,$004B6,
dw $00647,$007D9,$0096A,$00AFB,$00C8B,$00E1B,$00FAB,$01139,$012C8,$01455,$015E2,
dw $0176D,$018F8,$01A82,$01C0B,$01D93,$01F19,$0209F,$02223,$023A6,$02528,$026A8,
dw $02826,$029A3,$02B1F,$02C98,$02E11,$02F87,$030FB,$0326E,$033DE,$0354D,$036BA,
dw $03824,$0398C,$03AF2,$03C56,$03DB8,$03F17,$04073,$041CE,$04325,$0447A,$045CD,
dw $0471C,$04869,$049B4,$04AFB,$04C3F,$04D81,$04EBF,$04FFB,$05133,$05269,$0539B,
dw $054CA,$055F5,$0571D,$05842,$05964,$05A82,$05B9D,$05CB4,$05DC7,$05ED7,$05FE3,
dw $060EC,$061F1,$062F2,$063EF,$064E8,$065DD,$066CF,$067BD,$068A6,$0698C,$06A6D,
dw $06B4A,$06C24,$06CF9,$06DCA,$06E96,$06F5F,$07023,$070E2,$0719E,$07255,$07307,
dw $073B5,$0745F,$07504,$075A5,$07641,$076D9,$0776C,$077FA,$07884,$07909,$0798A,
dw $07A05,$07A7D,$07AEF,$07B5D,$07BC5,$07C29,$07C89,$07CE3,$07D39,$07D8A,$07DD6,
dw $07E1D,$07E5F,$07E9D,$07ED5,$07F09,$07F38,$07F62,$07F87,$07FA7,$07FC2,$07FD8,
dw $07FE9,$07FF6,$07FFD

SECTION .text
