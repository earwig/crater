;; Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; This file contains basic header code for standard assembly files in the unit
; testing suite. It sets values for the ROM header, and contains basic test
; runner code.

.rom_size	auto		; Smallest possible ROM size >= 32 KB
.rom_header	auto		; Standard header location (0x7FF0)
.rom_checksum	off		; Don't write a ROM checksum to the header
.rom_product	0		; Zero product code
.rom_version	0		; Zero version number
.rom_region	"GG Export"	; Common region code for Western ROMs
.rom_declsize	auto		; Set declared size to actual ROM size
.cross_blocks	auto		; Do not allow data to cross between blocks

; Main routine (execution begins here)
.org $0000
main:
	di			; Disable maskable interrupts
	call	test		; Run test subroutine
	emu	except(done)	; Signal to emulator that test is done

; Non-maskable interrupt handler (should not happen; raise an exception)
.org $0066
nmi_handler:
	emu	except(nmi)	; Signal to emulator that an NMI was received
