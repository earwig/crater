/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
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
#define SIZE_CODE_BUF 8

static size_t header_locations[NUM_LOCATIONS] = {0x7FF0, 0x3FF0, 0x1FF0};

/*
    @DEBUG_LEVEL
    Given a ROM size, return a pretty string.
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

/*
    @DEBUG_LEVEL
    Print out the raw header to stdout.
*/
static void print_header_dump(const uint8_t *header)
{
    char header_hex[3 * HEADER_SIZE], header_chr[3 * HEADER_SIZE];

    for (int i = 0; i < HEADER_SIZE; i++) {
        snprintf(&header_hex[3 * i], 3, "%02X", header[i]);
        if (isprint(header[i]))
            snprintf(&header_chr[3 * i], 3, "%2c", header[i]);
        else {
            header_chr[3 * i]     = ' ';
            header_chr[3 * i + 1] = '.';
        }
        header_hex[3 * i + 2] = header_chr[3 * i + 2] = ' ';
    }
    header_hex[3 * HEADER_SIZE - 1] = header_chr[3 * HEADER_SIZE - 1] = '\0';
    DEBUG("- header dump (hex): %s", header_hex)
    DEBUG("- header dump (chr): %s", header_chr)
}

/*
    @DEBUG_LEVEL
    Print out the analyzed header to stdout.
*/
static void print_header_contents(const ROM *rom, const uint8_t *header)
{
    DEBUG("- header info:")
    if (rom->reported_checksum == rom->expected_checksum)
        DEBUG("  - checksum:      0x%04X (valid)", rom->reported_checksum)
    else
        DEBUG("  - checksum:      0x%04X (invalid, expected 0x%04X)",
              rom->reported_checksum, rom->expected_checksum)
    DEBUG("  - product code:  %u (%s)", rom->product_code,
          rom_product(rom) ? rom_product(rom) : "unknown")
    DEBUG("  - version:       %u", rom->version)
    DEBUG("  - region code:   %u (%s)", rom->region_code,
          rom_region(rom) ? rom_region(rom) : "unknown")
    DEBUG("  - reported size: %s",
          size_to_string(size_code_to_bytes(header[0xF] & 0xF)))
}

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
    if (DEBUG_LEVEL)
        print_header_dump(header);

    rom->reported_checksum = header[0xA] + (header[0xB] << 8);
    rom->expected_checksum = compute_checksum(rom->data, rom->size, header[0xF]);
    rom->product_code = bcd_decode(header[0xC]) +
        (bcd_decode(header[0xD]) * 100) + ((header[0xE] >> 4) * 10000);
    rom->version = header[0xE] & 0x0F;
    rom->region_code = header[0xF] >> 4;

    if (DEBUG_LEVEL)
        print_header_contents(rom, header);
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
        if (memcmp(header, rom_header_magic, HEADER_MAGIC_LEN)) {
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

    rom = cr_malloc(sizeof(ROM));

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
    rom->name = cr_malloc(sizeof(char) * (strlen(path) + 1));
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
    rom->data = cr_malloc(sizeof(uint8_t) * st.st_size);
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

    if (rom->region_code == 3 || rom->region_code == 4) {
        // TODO: support SMS ROMs eventually?
        rom_close(rom);
        return rom_err_sms;
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
    Return a string explanation of this ROM's product code.

    NULL is returned if the product code is not known.

    Information from: http://www.smspower.org/Development/ProductCodes
*/
const char* rom_product(const ROM *rom)
{
    uint32_t developer = rom->product_code / 1000;
    if (developer == 2)
        return "Sega of America";
    if (developer == 3)
        return "Sega of Japan";
    if (developer > 10 && developer < 160)
        return get_third_party_developer(developer);
    return NULL;
}

/*
    Return the region this ROM was intended for, based on header information.

    NULL is returned if the region code is invalid.
*/
const char* rom_region(const ROM *rom)
{
    return region_code_to_string(rom->region_code);
}
