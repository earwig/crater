/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "disassembler.h"
#include "disassembler/arguments.h"
#include "disassembler/mnemonics.h"
#include "disassembler/sizes.h"
#include "mmu.h"
#include "rom.h"
#include "util.h"
#include "version.h"

#define HRULE \
 "----------------------------------------------------------------------------"

#define NUM_BANKS(rom) \
    (((rom)->size + MMU_ROM_BANK_SIZE - 1) / MMU_ROM_BANK_SIZE)
#define MAX_BYTES_PER_LINE 16

/* Structs and things */

typedef struct {
    size_t cap, len;
    char **lines;
} Disassembly;

typedef enum {
    DT_BINARY = 0,
    DT_CODE,
    DT_HEADER
} DataType;

typedef struct {
    size_t index;
    size_t size;
    int8_t slot;
    const uint8_t *data;
    DataType *types;
} ROMBank;

/*
    Format a sequence of bytes of a certain length as a pretty string.

    The result must be freed by the caller.
*/
static char* format_bytestring(const uint8_t *bytes, size_t size)
{
    if (!size)
        return NULL;

    char *str = cr_malloc(sizeof(char) * 16);
    size_t offset = 2, i;
    uint8_t b = bytes[0];

    if (b == 0xCB || b == 0xDD || b == 0xED || b == 0xFD) {
        offset--;
        if ((b == 0xDD || b == 0xFD) && bytes[1] == 0xCB)
            offset--;
    }

    for (i = 0; i < offset; i++) {
        str[3 * i] = str[3 * i + 1] = str[3 * i + 2] = ' ';
    }
    for (i = 0; i < size; i++) {
        snprintf(&str[3 * (i + offset)], 3, "%02X", bytes[i]);
        str[3 * (i + offset) + 2] = ' ';
    }
    str[3 * (size + offset) - 1] = '\0';
    return str;
}

/*
    Free the given DisasInstr struct.
*/
void disas_instr_free(DisasInstr *instr)
{
    free(instr->bytestr);
    free(instr->line);
    free(instr);
}

/*
    Disassemble a single instruction starting at the given address.

    Return a dynamically allocated structure containing various interesting
    fields. This must be freed by the user with disas_instr_free().
*/
DisasInstr* disassemble_instruction(const uint8_t *bytes)
{
    size_t size = get_instr_size(bytes);
    char *bytestr = format_bytestring(bytes, size);
    char *mnemonic = decode_mnemonic(bytes);
    char *args = decode_arguments(bytes);
    char *line;

    if (args) {
        line = cr_malloc(strlen(mnemonic) + strlen(args) + 2);
        sprintf(line, "%s\t%s", mnemonic, args);
        free(args);
    } else {
        line = cr_strdup(mnemonic);
    }

    DisasInstr *instr = cr_malloc(sizeof(DisasInstr));
    instr->size = size;
    instr->bytestr = bytestr;
    instr->line = line;
    return instr;
}

/*
    Append a line to the end of a disassembly.
*/
static void write_line(Disassembly *dis, char *line)
{
    dis->lines[dis->len++] = line;
    if (dis->len >= dis->cap) {
        dis->cap *= 2;
        dis->lines = cr_realloc(dis->lines, sizeof(char*) * dis->cap);
    }
}

/*
    Macro that wraps write_line() in a printf-like interface.
*/
#define WRITE_LINE_(dis, fmt, ...)                             \
    do {                                                       \
        char *tmp_buffer_;                                     \
        if (asprintf(&tmp_buffer_, fmt "\n", __VA_ARGS__) < 0) \
            OUT_OF_MEMORY()                                    \
        write_line(dis, tmp_buffer_);                          \
    } while(0);

#define WRITE_LINE(dis, ...) WRITE_LINE_(dis, __VA_ARGS__, NULL)

/*
    Write some metadata comments to the top of the disassembly.
*/
static void write_metadata(Disassembly *dis, const ROM *rom)
{
    time_t t;
    struct tm *tm_info;
    char buf[64];

    time(&t);
    tm_info = localtime(&t);
    strftime(buf, sizeof buf, "on %a %b %d, %Y at %H:%M:%S", tm_info);

    WRITE_LINE(dis, ";; GAME GEAR ROM DISASSEMBLY")
    WRITE_LINE(dis, ";; File: %s", rom->name)
    WRITE_LINE(dis, ";; Generated %s by crater %s", buf, CRATER_VERSION)
    WRITE_LINE(dis, ";; " HRULE)
    WRITE_LINE(dis, "")
}

/*
    Given a size, fill 'output' with a pretty string. Modified from rom.c.
*/
static char* size_to_string(char *output, size_t size)
{
    if (size >= (1 << 20))
        sprintf(output, "%zu MB", size >> 20);
    else
        sprintf(output, "%zu KB", size >> 10);
    return output;
}

/*
    Extract appropriate assembler directives from a ROM's header.
*/
static void disassemble_header(Disassembly *dis, const ROM *rom)
{
    char buf[64];
    const char *size, *product, *region, *declsize;

    DEBUG("Disassembling header")
    size = size_to_string(buf, rom->size);
    product = rom_product(rom);
    region = rom_region(rom);
    declsize = size_to_string(buf, size_code_to_bytes(rom->declared_size));

    WRITE_LINE(dis, ".rom_size\t\"%s\"%s\t; $%zX bytes in %zu banks",
        size, strlen(size) < 6 ? "\t" : "", rom->size, NUM_BANKS(rom))
    WRITE_LINE(dis, ".rom_header\t$%04X",
        rom->header_location)
    WRITE_LINE(dis, ".rom_checksum\t%s",
        (rom->reported_checksum == rom->expected_checksum) ? "on" : "off")
    WRITE_LINE(dis, ".rom_product\t%u\t\t; %s",
        rom->product_code, product ? product : "(unknown)")
    WRITE_LINE(dis, ".rom_version\t%u",
        rom->version)
    WRITE_LINE(dis, ".rom_region\t%u\t\t; %s",
        rom->region_code, region ? region : "(unknown)")
    WRITE_LINE(dis, ".rom_declsize\t$%X\t\t; %s",
        rom->declared_size, declsize)
}

/*
    Initialize and return an array of ROMBank objects for the given ROM.
*/
static ROMBank* init_banks(const ROM *rom)
{
    size_t nbanks = NUM_BANKS(rom), i;
    ROMBank *banks = cr_malloc(sizeof(ROMBank) * (nbanks + 1));
    DataType *types = cr_calloc(sizeof(DataType), rom->size);

    for (i = 0; i < nbanks; i++) {
        banks[i].index = i;
        if (i == nbanks - 1 && rom->size % MMU_ROM_BANK_SIZE)
            banks[i].size = rom->size % MMU_ROM_BANK_SIZE;
        else
            banks[i].size = MMU_ROM_BANK_SIZE;
        banks[i].slot = -1;
        banks[i].data = rom->data + (i * MMU_ROM_BANK_SIZE);
        banks[i].types = types + (i * MMU_ROM_BANK_SIZE);
    }

    banks[nbanks].data = NULL;  // Sentinel
    return banks;
}

/*
    Deallocate the given array of ROM banks.
*/
static void free_banks(ROMBank *banks)
{
    free(banks[0].types);
    free(banks);
}

/*
    Return the offset in bytes of the first address in the given bank.
*/
static size_t get_bank_offset(const ROMBank *bank)
{
    return MMU_ROM_BANK_SIZE * ((bank->slot >= 0) ? bank->slot :
                                (bank->index > 2) ? 2 : bank->index);
}

/*
    Mark the ROM's header as non-binary/non-code inside of the relevant bank.
*/
static void mark_header(const ROM *rom, ROMBank *banks)
{
    size_t i;
    for (i = 0; i < HEADER_SIZE; i++)
        banks[0].types[rom->header_location + i] = DT_HEADER;
}

/*
    Render a line of binary data within a block.
*/
static void render_binary(Disassembly *dis, size_t *idx, const ROMBank *bank)
{
    size_t span = 1, tabs, i;
    while (span < MAX_BYTES_PER_LINE && *idx + span < bank->size &&
           bank->types[*idx + span] == DT_BINARY)
        span++;

    char buf[4 * MAX_BYTES_PER_LINE + 1];
    for (i = 0; i < span; i++)
        sprintf(buf + 4 * i, "$%02X ", bank->data[*idx + i]);
    buf[4 * span - 1] = '\0';

    char padding[16];
    tabs = span <= 16 ? 8 - (span - 1) / 2 : 1;
    padding[tabs] = '\0';
    while (tabs-- > 0)
        padding[tabs] = '\t';

    WRITE_LINE(dis, ".byte %s%s; $%04zX", buf, padding, *idx)
    (*idx) += span;
}

/*
    Render a single instruction within a block.
*/
static void render_code(Disassembly *dis, size_t *idx, const ROMBank *bank)
{
    DisasInstr *instr = disassemble_instruction(bank->data + *idx);
    char padding[16], *split;

    if ((split = strchr(instr->line, '\t'))) {
        size_t tabs = (40 - (instr->line + strlen(instr->line) - split)) / 8;
        padding[tabs] = '\0';
        while (tabs-- > 0)
            padding[tabs] = '\t';
    } else {
        strcpy(padding, "\t\t\t\t\t");
    }

    WRITE_LINE(dis, "\t%s%s\t; $%04zX: %s",
        instr->line, padding, get_bank_offset(bank) + *idx, instr->bytestr)
    (*idx) += instr->size;
    disas_instr_free(instr);
}

/*
    Render fully analyzed banks into lines of disassembly.
*/
static void render_banks(Disassembly *dis, const ROMBank *banks)
{
    size_t bn = 0, idx;
    DEBUG("Rendering lines")

    while (banks[bn].data) {
        TRACE("Rendering bank 0x%02zX (0x%06zX-0x%06zX)", bn,
            bn * MMU_ROM_BANK_SIZE, bn * MMU_ROM_BANK_SIZE + banks[bn].size)
        WRITE_LINE(dis, "")
        WRITE_LINE(dis, ";; " HRULE)
        WRITE_LINE(dis, "")
        WRITE_LINE(dis, ".block $%02zX", bn)

        idx = 0;
        while (idx < banks[bn].size) {
            switch (banks[bn].types[idx]) {
                case DT_BINARY:
                    render_binary(dis, &idx, &banks[bn]);
                    break;
                case DT_CODE:
                    render_code(dis, &idx, &banks[bn]);
                    break;
                case DT_HEADER:
                    idx += HEADER_SIZE;
                    break;
                default:
                    FATAL("invalid data type %d at addr 0x%06zX",
                        banks[bn].types[idx], bn * MMU_ROM_BANK_SIZE + idx)
            }
        }
        bn++;
    }
}

/*
    Disassemble a ROM into an array of strings, each storing one source line.

    Each line is newline-terminated. The array itself is terminated with a NULL
    element. Each line, and the overall array, must be free()d by the caller.
*/
char** disassemble(const ROM *rom)
{
    Disassembly dis = {.cap = 16, .len = 0};
    dis.lines = cr_malloc(sizeof(char*) * dis.cap);

    write_metadata(&dis, rom);
    disassemble_header(&dis, rom);

    ROMBank *banks = init_banks(rom);
    mark_header(rom, banks);

    // TODO: analyze(): set DT_CODE (future: make labels, slots) where appropriate
    for (size_t i = 0; i < 0x1000; i++)
        banks[0].types[i] = DT_CODE;

    render_banks(&dis, banks);
    free_banks(banks);

    write_line(&dis, NULL);
    return dis.lines;
}

/*
    Write a disassembly created by disassemble() to the given output file.

    Return whether the file was written successfully. This function frees the
    disassembly along the way.
*/
static bool write_disassembly(const char *path, char **lines)
{
    FILE *fp;
    char **itr = lines;

    if (!(fp = fopen(path, "w"))) {
        ERROR_ERRNO("couldn't open destination file")
        return false;
    }

    while (*itr) {
        if (!fwrite(*itr, strlen(*itr), 1, fp)) {
            fclose(fp);
            do free(*itr); while (*(++itr));
            ERROR_ERRNO("couldn't write to destination file")
            return false;
        }
        free(*itr);
        itr++;
    }

    fclose(fp);
    free(lines);
    return true;
}

/*
    Disassemble the binary file at the input path into z80 source code.

    Return true if the operation was a success and false if it was a failure.
    Errors are printed to STDOUT; if the operation was successful then nothing
    is printed.
*/
bool disassemble_file(const char *src_path, const char *dst_path)
{
    ROM *rom;
    const char *errmsg;
    char **lines;

    DEBUG("Disassembling: %s -> %s", src_path, dst_path)
    if ((errmsg = rom_open(&rom, src_path))) {
        ERROR("couldn't load ROM image '%s': %s", src_path, errmsg)
        return false;
    }

    lines = disassemble(rom);
    rom_close(rom);

    DEBUG("Writing output file")
    return write_disassembly(dst_path, lines);
}
