;; Copyright (C) 2016 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; 07-data.asm
; Test of data directives to fill ROM with non-code bytes

.org $1000

str1: .ascii	"Hello, world!"
str2: .asciz	"World, hello!"
str3: .asciiz	"foobar"

arr1: .byte	1 2 3 4 5 6
arr2: .byte	14 $14 $FF 255 $F0
arr3: .byte	8, $10, 4 5 6, 8, 10

void1: .space	3
void2: .space	6 $D0
void3: .space	128 $DE

.org $0000
main:
	di
loop:
	jp	loop

.org $0066
nmi:
	retn
