/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "rom.h"
#include "logging.h"

#define NUM_LOCATIONS 3
#define MAGIC_LEN 8
#define HEADER_SIZE 16

static size_t header_locations[NUM_LOCATIONS] = {0x7ff0, 0x1ff0, 0x3ff0};
static const char header_magic[MAGIC_LEN + 1] = "TMR SEGA";

/*
    Return whether or not the given ROM image size is valid.
*/
static bool validate_size(off_t size)
{
    if (size & (size - 1))
        return false;  // Ensure size is a power of two

    off_t kbytes = size >> 10;
    return kbytes >= 8 && kbytes <= 1024;
}

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Print out the header to stdout.
*/
static void print_header(const char *header)
{
    char header_hex[3 * HEADER_SIZE], header_chr[3 * HEADER_SIZE];

    for (int i = 0; i < HEADER_SIZE; i++) {
        snprintf(&header_hex[3 * i], 3, "%02x", header[i]);
        if (isprint(header[i]))
            snprintf(&header_chr[3 * i], 3, "%2c", header[i]);
        else {
            header_chr[3 * i]     = ' ';
            header_chr[3 * i + 1] = '?';
        }
        header_hex[3 * i + 2] = header_chr[3 * i + 2] = ' ';
    }
    header_hex[3 * HEADER_SIZE - 1] = header_chr[3 * HEADER_SIZE - 1] = '\0';
    DEBUG("- header dump (hex): %s", header_hex)
    DEBUG("- header dump (chr): %s", header_chr)
}
#endif

/*
    Read a ROM image's header, and return whether or not it is valid.
*/
static bool read_header(ROM *rom)
{
    size_t location, i;
    const char *header;

    DEBUG("- looking for header:")
    for (i = 0; i < NUM_LOCATIONS; i++) {
        location = header_locations[i];
        DEBUG("  - trying location 0x%zx:", location)
        header = &rom->data[location];
        if (memcmp(header, header_magic, MAGIC_LEN)) {
            DEBUG("    - magic not present")
        }
        else {
            DEBUG("    - magic found")
#ifdef DEBUG_MODE
            print_header(header);
#endif
            // TODO: parse header
            return true;
        }
    }
    DEBUG("  - could not find header")
    return false;
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
    DEBUG("Loading ROM %s:", rom->name)

    // Set rom->size:
    DEBUG("- size: %lld", st.st_size)
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

    // Parse the header:
    if (!read_header(rom)) {
        rom_close(rom);
        return rom_err_badheader;
    }

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
