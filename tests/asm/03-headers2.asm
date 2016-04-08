;; Copyright (C) 2016 Ben Kurtovic <ben.kurtovic@gmail.com>
;; Released under the terms of the MIT License. See LICENSE for details.

; ----- CRATER UNIT TESTING SUITE ---------------------------------------------

; 03-headers2.asm
; Header/directive test using non-default values

.rom_size	"64 KB"
.rom_header	$7FF0
.rom_product	101893
.rom_version	3
.rom_region	"GG International"
.rom_checksum	on
.rom_declsize	"32 KB"
.cross_blocks	off
