;Ejemplo de ensamblador para gb-z80
;
;
;         test.asm
;
;         gbz80_asm test.asm test.gb
;*********************************************
;
main:     ;Main proces
        ; Wait for VBL
ldh     a,($40) ;Gets the status of the lcd
add     a,a     ;Multiply it by 2
;ret     nc      ;If there is no carry, return

notyet:                         ;
ldh     a, ($44)        ;Check if ly is over 144(0x90)
cp      $90             ; $90 and bigger = in V-blank
jr      nz,notyet       ; Loop until it $90
;
ld a,0x13; prepeare the lcd
ldh [0x40],a;write it
ld a,0
ldh [0x41],a         ; LCDC Status
nop                     ; wait (just in case h/ware is slow)
ldh     ($42),a         ; Screen scroll Y=0
nop                     ; wait (just in case h/ware is slow)
ldh     ($43),a         ; Screen scroll X=0

ld a,$E4
ldh (0x47),a

         ; Must be in V-BLank before turning the screen On.
ldh     a, ($40)        ; load old val
set     7, a            ; SET bit 7 which turns screen on
ldh     ($40), a        ; write back
;ret

ld b,$20
ld c,$20
ld h,0x98
ld l,0

;put all the 32 tiles map to tile 0
ld a,0xA
columb:
ld b,$20
line:
ld (hli),a
dec b
jp nz,line
dec c
jp nz,columb

ld h,0x80
ld l,0

;0
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;1
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x30
ldi (hl),a
ld a, 0x30
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;2
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x0C
ldi (hl),a
ld a, 0x0C
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x7C
ldi (hl),a
ld a, 0x7C
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;3
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x18
ldi (hl),a
ld a, 0x18
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;4
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x48
ldi (hl),a
ld a, 0x48
ldi (hl),a
ld a, 0x48
ldi (hl),a
ld a, 0x48
ldi (hl),a
ld a, 0x78
ldi (hl),a
ld a, 0x78
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a



;5
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a

;6
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x40
ldi (hl),a
ld a, 0x40
ldi (hl),a
ld a, 0x78
ldi (hl),a
ld a, 0x78
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;7
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x7C
ldi (hl),a
ld a, 0x7C
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x08
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x10
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x20
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;8
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;9
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x44
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x3C
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x04
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x38
ldi (hl),a
ld a, 0x0
ldi (hl),a
ld a, 0x0
ldi (hl),a

;empty
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a
ld a, 0x00
ldi (hl),a

;compilado

ld b, 0x03
ld e, b
ld b, 0x02
ld c, b
ld b, e
ld a, b
add a, c
ld b, a
ld e, b
ld b, 0x0a
ld c, b
ld b, e
ld a, c
ld d, 0x00
cp 0x00
jp z, inexacta-div6
ld a, b
cp 0x00
jp z, inexacta-div6
ld a, c
restar-div6:
sub b
jp c, inexacta-div6
jp z, exacta-div6
inc d
jp restar-div6
exacta-div6:
inc d
inexacta-div6:
ld b, d

ld h,0x98
ld l,0
ld a,b;
srl a
srl a
srl a
srl a
ld (hli), a
ld a,b
and 0x0F
ld (hli), a

end:
jp end


;ends main
