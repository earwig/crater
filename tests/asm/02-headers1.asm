;; Copyright (C) 2016 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; 02-headers1.asm
; Basic test for headers and other directives, mostly using default values

.rom_size	auto
.rom_header	auto
.rom_product	0
.rom_version	0
.rom_region	"GG Export"
.rom_checksum	off
.rom_declsize	auto
.cross_blocks	auto
