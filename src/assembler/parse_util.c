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

#define DIRECTIVE_PARSE_FUNC(name, type)                                      \
    bool dparse_##name(type *result, const ASMLine *line, const char *directive)

/*
    All public functions in this file follow the same return conventions:

    - Return true on success and false on failure.
    - *result is only modified on success.
*/

/*
    Read in a boolean value and store it in *result.
*/
bool parse_bool(bool *result, const char *arg, ssize_t size)
{
    if (size <= 0 || size > 5)
        return false;

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
