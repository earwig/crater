;; Copyright (C) 2016 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; 08-instructions.asm
; Exhaustive test of instruction syntax

.define	COUNTER	$C010
.define	OFFSET	6
.define	OFFSET2	157

.org $1000

str1: .ascii	"Hello, world!"

.org $0000
main:
	di
	call	inst
	halt

.org $0066
nmi:
	retn

.org $2000
inst:
	inc	a
	inc	c
	inc	hl
	inc	sp
	inc	(hl)
	inc	( hl    )
	inc	ix
	inc	iyl
	inc	(ix)
	inc	( ix )
	inc	(iy+0)
	inc	(ix+3)
	inc	(ix+OFFSET)
	inc	(ix-OFFSET)
	inc	( ix + 7 )
	inc	(ix-8)
	inc	(ix - +9)
	inc	(iy+127)
	inc	(iy-128)

	add	a, e
	add	a, 5
	add	a, $12
	add	a, (hl)
	add	a, (ix+4)
	add	hl, bc
	add	ix, de

	adc	a, e
	adc	a, 5
	adc	a, $12
	adc	a, (hl)
	adc	a, (ix+4)
	adc	hl, bc

	rlca
	rrca
	rla
	rra
	daa
	cpl
	scf
	ccf
	halt
	exx
	ei
	di
	cpd
	cpdr
	cpi
	cpir
	ind
	indr
	ini
	inir
	ldd
	lddr
	ldi
	ldir
	otdr
	otir
	outd
	outi
	rrd
	rld

	ld	hl, COUNTER	; H contains $C0, L contains $10
	ld	d, (hl)		; D contains $FF (garbage)
	ld	c, $3B		; C contains $3B
	ld	(hl), c		; memory address $C010 contains $3B

	ld	hl, str1
	ld	b, (hl)		; B contains 'H'
	ld	hl, (str1)	; H contains 'e', L contains 'H'
	ld	(hl), b		; error, writing to cartridge ROM
