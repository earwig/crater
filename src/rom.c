/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "rom.h"

/* Create and return a ROM object located at the given path. Return NULL if
   there was an error; errno will be set appropriately. */
rom_type* rom_open(const char *path)
{
    rom_type *rom;
    FILE* fp;
    struct stat s;

    if (!(fp = fopen(path, "r")))
        return NULL;

    if (fstat(fileno(fp), &s)) {
        fclose(fp);
        return NULL;
    }
    if (!(s.st_mode & S_IFREG)) {
        if (s.st_mode & S_IFDIR)
            errno = EISDIR;
        fclose(fp);
        return NULL;
    }

    if (!(rom = malloc(sizeof(rom_type)))) {
        fclose(fp);
        return NULL;
    }
    rom->name = malloc(sizeof(char) * (strlen(path) + 1));
    strcpy(rom->name, path);

    // TODO: load data from file into a buffer

    fclose(fp);
    return rom;
}

/* Free a ROM object previously created with rom_open(). */
void rom_close(rom_type *rom)
{
    free(rom);
}
