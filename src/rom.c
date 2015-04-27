/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "rom.h"
#include "logging.h"
#include "util.h"

#define NUM_LOCATIONS 3
#define MAGIC_LEN 8
#define SIZE_CODE_BUF 8

static size_t header_locations[NUM_LOCATIONS] = {0x7FF0, 0x3FF0, 0x1FF0};
static const char header_magic[MAGIC_LEN + 1] = "TMR SEGA";

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Print out the header to stdout.
*/
static void print_header(const uint8_t *header)
{
    char header_hex[3 * HEADER_SIZE], header_chr[3 * HEADER_SIZE];

    for (int i = 0; i < HEADER_SIZE; i++) {
        snprintf(&header_hex[3 * i], 3, "%02X", header[i]);
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
    Compute the correct ROM data checksum.

    If the summable region (as specified by the range parameter) is too large,
    we'll compute the checksum over the default range (0x0000-0x7FF0), or the
    largest possible range.
*/
static uint16_t compute_checksum(const uint8_t *data, size_t size, uint8_t range)
{
    size_t low_region, high_region;
    switch (range & 0xF) {
        case 0xA: low_region = 0x1FEF; high_region = 0;       break;
        case 0xB: low_region = 0x3FEF; high_region = 0;       break;
        case 0xC: low_region = 0x7FEF; high_region = 0;       break;
        case 0xD: low_region = 0xBFEF; high_region = 0;       break;
        case 0xE: low_region = 0x7FEF; high_region = 0x0FFFF; break;
        case 0xF: low_region = 0x7FEF; high_region = 0x1FFFF; break;
        case 0x0: low_region = 0x7FEF; high_region = 0x3FFFF; break;
        case 0x1: low_region = 0x7FEF; high_region = 0x7FFFF; break;
        case 0x2: low_region = 0x7FEF; high_region = 0xFFFFF; break;
        default:  low_region = 0x7FEF; high_region = 0;       break;
    }

    if (low_region >= size)
        low_region = (size >= 0x4000) ? 0x3FEF : 0x1FEF;
    if (high_region >= size)
        high_region = 0;

    uint16_t sum = 0;
    for (size_t index = 0; index <= low_region; index++)
        sum += data[index];
    if (high_region) {
        for (size_t index = 0x08000; index <= high_region; index++)
            sum += data[index];
    }
    return sum;
}

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Given a ROM size, return a pretty string.
*/
static const char* size_to_string(size_t size)
{
    static char buffer[SIZE_CODE_BUF];

    if (!size)
        strncpy(buffer, "unknown", SIZE_CODE_BUF);
    else if (size >= (1 << 20))
        snprintf(buffer, SIZE_CODE_BUF, "%zu MB", size >> 20);
    else
        snprintf(buffer, SIZE_CODE_BUF, "%zu KB", size >> 10);

    return buffer;
}
#endif

/*
    Parse a ROM image's header, and return whether or not it is valid.

    The header is 16 bytes long, consisting of:
    byte 0:             magic ('T')
    byte 1:             magic ('M')
    byte 2:             magic ('R')
    byte 3:             magic (' ')
    byte 4:             magic ('S')
    byte 5:             magic ('E')
    byte 6:             magic ('G')
    byte 7:             magic ('A')
    byte 8:             unused
    byte 9:             unused
    byte A:             checksum (LSB)
    byte B:             checksum (MSB)
    byte C:             product code (LSB)
    byte D:             product code (middle byte)
    byte E (hi nibble): product code (most-significant nibble)
    byte E (lo nibble): version
    byte F (hi nibble): region code
    byte F (lo nibble): ROM size

    (Based on: http://www.smspower.org/Development/ROMHeader)
*/
static bool parse_header(ROM *rom, const uint8_t *header)
{
#ifdef DEBUG_MODE
    print_header(header);
#endif

    rom->reported_checksum = header[0xA] + (header[0xB] << 8);
    rom->expected_checksum = compute_checksum(rom->data, rom->size, header[0xF]);
    rom->product_code = bcd_decode(header[0xC]) +
        (bcd_decode(header[0xD]) * 100) + ((header[0xE] >> 4) * 10000);
    rom->version = header[0xE] & 0x0F;
    rom->region_code = header[0xF] >> 4;

    DEBUG("- header info:")
    if (rom->reported_checksum == rom->expected_checksum)
        DEBUG("  - checksum:      0x%04X (valid)", rom->reported_checksum)
    else
        DEBUG("  - checksum:      0x%04X (invalid, expected 0x%04X)",
              rom->reported_checksum, rom->expected_checksum)
    DEBUG("  - product code:  %u", rom->product_code)
    DEBUG("  - version:       %u", rom->version)
    DEBUG("  - region code:   %u (%s)", rom->region_code,
          rom_region(rom) ? rom_region(rom) : "unknown")
    DEBUG("  - reported size: %s",
          size_to_string(size_code_to_bytes(header[0xF] & 0xF)))
    return true;
}

/*
    Find and read a ROM image's header, and return whether or not it is valid.
*/
static bool find_and_read_header(ROM *rom)
{
    size_t location, i;
    const uint8_t *header;

    DEBUG("- looking for header:")
    for (i = 0; i < NUM_LOCATIONS; i++) {
        location = header_locations[i];
        if (location + HEADER_SIZE > rom->size) {
            DEBUG("  - skipping location 0x%zX, out of range", location)
            continue;
        }
        DEBUG("  - trying location 0x%zX:", location)
        header = &rom->data[location];
        if (memcmp(header, header_magic, MAGIC_LEN)) {
            DEBUG("    - magic not present")
        }
        else {
            DEBUG("    - magic found")
            return parse_header(rom, header);
        }
    }
    DEBUG("  - couldn't find header")
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
    FILE *fp;
    struct stat st;

    if (!(fp = fopen(path, "rb")))
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
    rom->reported_checksum = 0;
    rom->expected_checksum = 0;
    rom->product_code = 0;
    rom->version = 0;
    rom->region_code = 0;

    // Set rom->name:
    if (!(rom->name = malloc(sizeof(char) * (strlen(path) + 1))))
        OUT_OF_MEMORY()
    strcpy(rom->name, path);
    DEBUG("Loading ROM %s:", rom->name)

    // Set rom->size:
    DEBUG("- size: %lld bytes (%s)", st.st_size, size_to_string(st.st_size))
    if (size_bytes_to_code(st.st_size) == INVALID_SIZE_CODE) {
        rom_close(rom);
        fclose(fp);
        return rom_err_badsize;
    }
    rom->size = st.st_size;

    // Set rom->data:
    if (!(rom->data = malloc(sizeof(uint8_t) * st.st_size)))
        OUT_OF_MEMORY()
    if (!(fread(rom->data, st.st_size, 1, fp))) {
        rom_close(rom);
        fclose(fp);
        return rom_err_badread;
    }
    fclose(fp);

    // Parse the header:
    if (!find_and_read_header(rom)) {
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
    free(rom->name);
    free(rom->data);
    free(rom);
}

/*
    Return the region this ROM was intended for, based on header information.

    NULL is returned if the region code is invalid.
*/
const char* rom_region(const ROM *rom)
{
    return region_code_to_string(rom->region_code);
}
