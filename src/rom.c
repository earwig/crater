/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "rom.h"
#include "logging.h"

/*
    Return whether or not the given ROM image size is valid.
*/
static bool validate_size(off_t size)
{
    off_t kbytes = size >> 10;
    if (kbytes << 10 != size)
        return false;
    if (kbytes < 8 || kbytes > 1024)
        return false;
    return !(kbytes & (kbytes - 1));
}

/*
    Create and load a ROM image located at the given path.

    rom_ptr will point to the new object if created successfully, and NULL will
    be returned. Otherwise, rom_ptr will not be modified and an error string
    will be returned. The error string should not be freed.
*/
const char* rom_open(ROM **rom_ptr, const char *path)
{
    ROM *rom;
    FILE* fp;
    struct stat st;

    if (!(fp = fopen(path, "r")))
        return strerror(errno);

    if (fstat(fileno(fp), &st)) {
        fclose(fp);
        return strerror(errno);
    }
    if (!(st.st_mode & S_IFREG)) {
        fclose(fp);
        return (st.st_mode & S_IFDIR) ? rom_err_isdir : rom_err_notfile;
    }

    if (!(rom = malloc(sizeof(ROM))))
        OUT_OF_MEMORY()

    // Set defaults:
    rom->name = NULL;
    rom->data = NULL;
    rom->size = 0;

    // Set rom->name:
    if (!(rom->name = malloc(sizeof(char) * (strlen(path) + 1))))
        OUT_OF_MEMORY()
    strcpy(rom->name, path);

    // Set rom->size:
    if (!validate_size(st.st_size)) {
        rom_close(rom);
        fclose(fp);
        return rom_err_badsize;
    }
    rom->size = st.st_size;

    // Set rom->data:
    if (!(rom->data = malloc(sizeof(char) * st.st_size)))
        OUT_OF_MEMORY()
    if (!(fread(rom->data, st.st_size, 1, fp))) {
        rom_close(rom);
        fclose(fp);
        return rom_err_badread;
    }

    fclose(fp);
    *rom_ptr = rom;
    return NULL;
}

/*
    Free a ROM object previously created with rom_open().
*/
void rom_close(ROM *rom)
{
    if (rom->name)
        free(rom->name);
    if (rom->data)
        free(rom->data);
    free(rom);
}
