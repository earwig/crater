;; Copyright (C) 2016 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; 05-includes.asm
; Source file inclusion test, involving cross-file label references and complex
; origins

.include	"05.inc1.asm"

.org $0000
main:
	di
	ld	c, $FA
	inc	c
	call	bar
