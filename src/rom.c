/* Copyright (C) 2014 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "rom.h"
#include "errors.h"

rom_type* load_rom(char *path)
{
    rom_type *rom;

    rom = malloc(sizeof(rom_type));
    if (!rom)
        out_of_memory();
    return rom;
}

void unload_rom(rom_type *rom)
{
    free(rom);
}
