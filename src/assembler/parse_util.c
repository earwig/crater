/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "parse_util.h"
#include "directives.h"
#include "../logging.h"
#include "../util.h"

#define MAX_REGION_SIZE 32

#define LCASE(c) ((c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c)

#define DIRECTIVE_PARSE_FUNC(name, type)                                      \
    bool dparse_##name(type *result, const ASMLine *line, const char *directive)

/*
    All public functions in this file follow the same return conventions:

    - Return true on success and false on failure.
    - *result is only modified on success.
*/

/*
    Adjust *arg_ptr / *size_ptr for an indirect argument, like (hl) or (ix+*).

    *size_ptr must be > 2 to begin with, and is assured to be > 0 on success.
    The two arguments are not modified on failure.
*/
static bool adjust_for_indirection(const char **arg_ptr, ssize_t *size_ptr)
{
    const char *arg = *arg_ptr;
    ssize_t size = *size_ptr;

    if (arg[0] != '(' || arg[size - 1] != ')')
        return false;
    arg++;
    size -= 2;

    if (arg[0] == ' ') {
        arg++;
        if (--size <= 0)
            return false;
    }
    if (arg[size - 1] == ' ') {
        if (--size <= 0)
            return false;
    }

    *arg_ptr = arg;
    *size_ptr = size;
    return true;
}

/*
    Read in a boolean value and store it in *result.
*/
bool parse_bool(bool *result, const char *arg, ssize_t size)
{
    switch (size) {
        case 1:  // 0, 1
            if (*arg == '0' || *arg == '1')
                return (*result = *arg - '0'), true;
            return false;
        case 2:  // on
            if (!strncmp(arg, "on", 2))
                return (*result = true), true;
            return false;
        case 3:  // off
            if (!strncmp(arg, "off", 3))
                return (*result = false), true;
            return false;
        case 4:  // true
            if (!strncmp(arg, "true", 4))
                return (*result = true), true;
            return false;
        case 5:  // false
            if (!strncmp(arg, "false", 5))
                return (*result = false), true;
            return false;
    }
    return false;
}

/*
    Read in a 32-bit integer and store it in *result.
*/
bool parse_uint32_t(uint32_t *result, const char *arg, ssize_t size)
{
    if (size <= 0)
        return false;

    const char *end = arg + size;
    uint64_t value = 0;
    if (*arg == '$') {
        arg++;
        if (arg == end)
            return false;

        while (arg < end) {
            if (*arg >= '0' && *arg <= '9')
                value = (value * 0x10) + (*arg - '0');
            else if (*arg >= 'a' && *arg <= 'f')
                value = (value * 0x10) + 0xA + (*arg - 'a');
            else
                return false;
            if (value > UINT32_MAX)
                return false;
            arg++;
        }
    }
    else {
        while (arg < end) {
            if (*arg < '0' || *arg > '9')
                return false;
            value = (value * 10) + (*arg - '0');
            if (value > UINT32_MAX)
                return false;
            arg++;
        }
    }

    *result = value;
    return true;
}

/*
    Read in a string, possibly with escape sequences, and store it in *result.

    *length is also updated to the size of the string, which is *not*
    null-terminated. *result must be free()'d when finished.
*/
bool parse_string(char **result, size_t *length, const char *arg, ssize_t size)
{
    if (size < 2 || arg[0] != '"' || arg[size - 1] != '"')
        return false;

    ssize_t i, slashes = 0;
    for (i = 1; i < size; i++) {
        if (arg[i] == '"' && (slashes % 2) == 0)
            break;

        // TODO: parse escape codes here

        if (arg[i] == '\\')
            slashes++;
        else
            slashes = 0;
    }

    if (i != size - 1)  // Junk present after closing quote
        return false;

    *length = size - 2;
    *result = malloc(sizeof(char) * (*length));
    if (!*result)
        OUT_OF_MEMORY()
    memcpy(*result, arg + 1, *length);
    return true;
}

/*
    Read in a space-separated sequence of bytes and store it in *result.

    *length is also updated to the number of bytes in the array. *result must
    be free()'d when finished.
*/
bool parse_bytes(uint8_t **result, size_t *length, const char *arg, ssize_t size)
{
    if (size <= 0)
        return false;

    const char *end = arg + size;
    uint8_t *bytes = NULL;
    size_t nbytes = 0;

    while (arg < end) {
        const char *start = arg;
        while (arg != end && *arg != ' ' && *arg != ',')
            arg++;

        uint32_t temp;
        if (!parse_uint32_t(&temp, start, arg - start) || temp > UINT8_MAX) {
            free(bytes);
            return false;
        }

        nbytes++;
        bytes = realloc(bytes, sizeof(uint8_t) * nbytes);
        if (!bytes)
            OUT_OF_MEMORY()
        bytes[nbytes - 1] = temp;

        if (arg < end - 1 && *arg == ',' && *(arg + 1) == ' ')
            arg += 2;
        else if (arg++ >= end)
            break;
    }

    *result = bytes;
    *length = nbytes;
    return true;
}

/*
    Read in a register argument and store it in *result.
*/
bool argparse_register(ASMArgRegister *result, const char *arg, ssize_t size)
{
    if (size < 1 || size > 3)
        return false;

    char buf[3] = {'\0'};
    switch (size) {
        case 3: buf[2] = LCASE(arg[2]);
        case 2: buf[1] = LCASE(arg[1]);
        case 1: buf[0] = LCASE(arg[0]);
    }

    switch (size) {
        case 1:
            switch (buf[0]) {
                case 'a': return (*result = REG_A), true;
                case 'f': return (*result = REG_F), true;
                case 'b': return (*result = REG_B), true;
                case 'c': return (*result = REG_C), true;
                case 'd': return (*result = REG_D), true;
                case 'e': return (*result = REG_E), true;
                case 'h': return (*result = REG_H), true;
                case 'l': return (*result = REG_L), true;
                case 'i': return (*result = REG_I), true;
                case 'r': return (*result = REG_R), true;
            }
            return false;
        case 2:
            switch ((buf[0] << 8) + buf[1]) {
                case 0x6166: return (*result = REG_AF), true;
                case 0x6263: return (*result = REG_BC), true;
                case 0x6465: return (*result = REG_DE), true;
                case 0x686C: return (*result = REG_HL), true;
                case 0x6978: return (*result = REG_IX), true;
                case 0x6979: return (*result = REG_IY), true;
                case 0x7063: return (*result = REG_PC), true;
                case 0x7370: return (*result = REG_SP), true;
            }
            return false;
        case 3:
            switch ((buf[0] << 16) + (buf[1] << 8) + buf[2]) {
                case 0x616627: return (*result = REG_AF_), true;
                case 0x697868: return (*result = REG_IXH), true;
                case 0x69786C: return (*result = REG_IXL), true;
                case 0x697968: return (*result = REG_IYH), true;
                case 0x69796C: return (*result = REG_IYL), true;
            }
            return false;
    }
    return false;
}

/*
    Read in a condition argument and store it in *result.
*/
bool argparse_condition(ASMArgCondition *result, const char *arg, ssize_t size)
{
    if (size < 1 || size > 2)
        return false;

    char buf[2] = {'\0'};
    switch (size) {
        case 2: buf[1] = LCASE(arg[1]);
        case 1: buf[0] = LCASE(arg[0]);
    }

    switch (size) {
        case 1:
            switch (buf[0]) {
                case 'n': return (*result = COND_N), true;
                case 'c': return (*result = COND_C), true;
                case 'p': return (*result = COND_P), true;
                case 'm': return (*result = COND_M), true;
            }
            return false;
        case 2:
            switch ((buf[0] << 8) + buf[1]) {
                case 0x6E7A: return (*result = COND_NZ), true;
                case 0x6E63: return (*result = COND_NC), true;
                case 0x706F: return (*result = COND_PO), true;
                case 0x7065: return (*result = COND_PE), true;
            }
            return false;
    }
    return false;
}

/*
    Read in an immediate argument and store it in *result.
*/
bool argparse_immediate(ASMArgImmediate *result, const char *arg, ssize_t size)
{
    bool negative = false;
    ssize_t i = 0;

    if (size <= 0)
        return false;

    while (arg[i] == '-' || arg[i] == '+' || arg[i] == ' ') {
        if (arg[i] == '-')
            negative = !negative;
        if (++i >= size)
            return false;
    }

    uint32_t uval;
    if (!parse_uint32_t(&uval, arg + i, size - i) || uval > UINT16_MAX)
        return false;

    int32_t sval = negative ? -uval : uval;
    if (sval < INT16_MIN)
        return false;

    result->uval = uval;
    result->sval = sval;
    result->mask = 0;

    if (sval < 0) {
        if (sval >= INT8_MIN)
            result->mask |= IMM_S8;
        if (sval >= INT8_MIN + 2)
            result->mask |= IMM_REL;
    } else {
        result->mask = IMM_U16;
        if (uval <= UINT8_MAX)
            result->mask |= IMM_U8;
        if (uval <= INT8_MAX)
            result->mask |= IMM_S8;
        if (uval <= INT8_MAX + 2)
            result->mask |= IMM_REL;
        if (uval <= 7)
            result->mask |= IMM_BIT;
        if (!(uval & ~0x38))
            result->mask |= IMM_RST;
        if (uval <= 2)
            result->mask |= IMM_IM;
    }
    return true;
}

/*
    Read in an indirect argument and store it in *result.
*/
bool argparse_indirect(ASMArgIndirect *result, const char *arg, ssize_t size)
{
    if (size < 3 || !adjust_for_indirection(&arg, &size))
        return false;

    ASMArgRegister reg;
    ASMArgImmediate imm;
    if (argparse_register(&reg, arg, size)) {
        if (reg == REG_BC || reg == REG_DE || reg == REG_HL) {
            result->type = AT_REGISTER;
            result->addr.reg = reg;
            return true;
        }
    } else if (argparse_immediate(&imm, arg, size)) {
        if (imm.mask & IMM_U16) {
            result->type = AT_IMMEDIATE;
            result->addr.imm = imm;
            return true;
        }
    }
    return false;
}

/*
    Read in an indexed argument and store it in *result.
*/
bool argparse_indexed(ASMArgIndexed *result, const char *arg, ssize_t size)
{
    if (size < 4 || !adjust_for_indirection(&arg, &size) || size < 2)
        return false;

    ASMArgRegister reg;
    if (arg[0] != 'i')
        return false;
    if (arg[1] == 'x')
        reg = REG_IX;
    else if (arg[1] == 'y')
        reg = REG_IY;
    else
        return false;

    arg += 2;
    size -= 2;
    if (size > 0 && arg[0] == ' ') {
        arg++;
        size--;
    }

    if (size > 0) {
        ASMArgImmediate imm;
        if (!argparse_immediate(&imm, arg, size) || !(imm.mask & IMM_S8))
            return false;
        result->offset = imm.sval;
    } else {
        result->offset = 0;
    }
    result->reg = reg;
    return true;
}

/*
    Read in a label argument and store it in *result.
*/
bool argparse_label(ASMArgLabel *result, const char *arg, ssize_t size)
{
    if (size >= MAX_SYMBOL_SIZE)
        return false;

    for (const char *i = arg; i < arg + size; i++) {
        char c = *i;
        if (!((c >= 'a' && c <= 'z') || (i != arg && c >= '0' && c <= '9') ||
              c == '_' || c == '.'))
            return false;
    }

    strncpy(result->text, arg, size);
    result->text[size] = '\0';
    return true;
}

/*
    Read in a boolean argument from the given line and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(bool, bool)
{
    size_t offset = DIRECTIVE_OFFSET(line, directive) + 1;
    return parse_bool(result, line->data + offset, line->length - offset);
}

/*
    Read in a 32-bit int argument from the given line and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(uint32_t, uint32_t)
{
    size_t offset = DIRECTIVE_OFFSET(line, directive) + 1;
    return parse_uint32_t(result, line->data + offset, line->length - offset);
}

/*
    Read in a 16-bit int argument from the given line and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(uint16_t, uint16_t)
{
    uint32_t value;
    if (dparse_uint32_t(&value, line, directive) && value <= UINT16_MAX)
        return (*result = value), true;
    return false;
}

/*
    Read in an 8-bit int argument from the given line and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(uint8_t, uint8_t)
{
    uint32_t value;
    if (dparse_uint32_t(&value, line, directive) && value <= UINT8_MAX)
        return (*result = value), true;
    return false;
}

/*
    Parse a ROM size string in an ASMLine and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(rom_size, uint32_t)
{
    const char *arg = line->data + DIRECTIVE_OFFSET(line, directive) + 1;
    const char *end = line->data + line->length - 1;

    if (end - arg < 5)
        return false;
    if (*(arg++) != '"' || *(end--) != '"')
        return false;
    if (*end != 'B' && *end != 'b')
        return false;
    end--;

    uint32_t factor;
    if (*end == 'K' || *end == 'k')
        factor = 1 << 10;
    else if (*end == 'M' || *end == 'm')
        factor = 1 << 20;
    else
        return false;
    end--;

    if (*end != ' ')
        return false;

    uint32_t value = 0;
    while (arg < end) {
        if (*arg < '0' || *arg > '9')
            return false;
        value = (value * 10) + (*arg - '0');
        if (value > UINT16_MAX)
            return false;
        arg++;
    }

    *result = value * factor;
    return true;
}

/*
    Parse a region code string in an ASMLine and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(region_string, uint8_t)
{
    char buffer[MAX_REGION_SIZE];

    size_t offset = DIRECTIVE_OFFSET(line, directive) + 1;
    const char *arg = line->data + offset;
    ssize_t len = line->length - offset;

    if (len <= 2 || len >= MAX_REGION_SIZE + 2)  // Account for double quotes
        return false;
    if (arg[0] != '"' || arg[len - 1] != '"')
        return false;

    strncpy(buffer, arg + 1, len - 2);
    buffer[len - 2] = '\0';

    uint8_t code = region_string_to_code(buffer);
    if (code)
        return (*result = code), true;
    return false;
}

/*
    Parse a size code in an ASMLine and store it in *result.
*/
DIRECTIVE_PARSE_FUNC(size_code, uint8_t)
{
    uint32_t bytes;
    if (!dparse_uint32_t(&bytes, line, directive)) {
        if (!dparse_rom_size(&bytes, line, directive))
            return false;
    }

    uint8_t code = size_bytes_to_code(bytes);
    if (code != INVALID_SIZE_CODE)
        return (*result = code), true;
    return false;
}
