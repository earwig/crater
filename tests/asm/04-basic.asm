;; Copyright (C) 2016 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; 04-basic.asm
; Basic instruction test

.org $0000
main:
	di
	ld	a, $23
	inc	a
	call	foo

foo:
	push	bc
	xor	c
	ret	z
	rrca
	ret
