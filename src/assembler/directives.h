/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <string.h>

#define DIRECTIVE_MARKER '.'
#define NUM_DIRECTIVES   16

#define DIR_INCLUDE      ".include"

#define DIR_ROM_SIZE     ".rom_size"
#define DIR_ROM_HEADER   ".rom_header"
#define DIR_ROM_CHECKSUM ".rom_checksum"
#define DIR_ROM_PRODUCT  ".rom_product"
#define DIR_ROM_VERSION  ".rom_version"
#define DIR_ROM_REGION   ".rom_region"
#define DIR_ROM_DECLSIZE ".rom_declsize"
#define DIR_CROSS_BLOCKS ".cross_blocks"

#define DIR_ORIGIN       ".org"
#define DIR_BLOCK        ".block"
#define DIR_BYTE         ".byte"
#define DIR_SPACE        ".space"
#define DIR_ASCII        ".ascii"
#define DIR_ASCIZ        ".asciz"
#define DIR_ASCIIZ       ".asciiz"

#define DIRECTIVE_HAS_ARG(line, d) ((line)->length > strlen(d))

#define IS_DIRECTIVE(line, d)                                                 \
    (((line)->length >= strlen(d)) &&                                         \
    !strncmp((line)->data, d, strlen(d)) &&                                   \
    (!DIRECTIVE_HAS_ARG(line, d) || (line)->data[strlen(d)] == ' '))

#define IS_LOCAL_DIRECTIVE(line)                                              \
    (IS_DIRECTIVE(line, DIR_ORIGIN) || IS_DIRECTIVE(line, DIR_BLOCK) ||       \
     IS_DIRECTIVE(line, DIR_BYTE)   || IS_DIRECTIVE(line, DIR_SPACE) ||       \
     IS_DIRECTIVE(line, DIR_ASCII)  || IS_DIRECTIVE(line, DIR_ASCIZ) ||       \
     IS_DIRECTIVE(line, DIR_ASCIIZ))

#define DIRECTIVE_OFFSET(line, d)                                             \
    (DIRECTIVE_HAS_ARG(line, d) ? strlen(d) : 0)

#define DIRECTIVE_IS_AUTO(line, d)                                            \
    (line->length - (DIRECTIVE_OFFSET(line, d) + 1) == 4 &&                   \
    !strncmp(line->data + (DIRECTIVE_OFFSET(line, d) + 1), "auto", 4))
