/* Copyright (C) 2014 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdio.h>
#include <stdlib.h>

#include "rom.h"

/* Create and return a ROM object located at the given path. Return NULL if
   there was an error; errno will be set appropriately. */
rom_type* open_rom(char *path)
{
    rom_type *rom;
    FILE* fp;

    if (!(fp = fopen(path, "r")))
        return NULL;
    if (!(rom = malloc(sizeof(rom_type))))
        return NULL;

    // load data from file into a buffer

    return rom;
}

/* Free a ROM object previously created with open_rom(). */
void close_rom(rom_type *rom)
{
    free(rom);
}
