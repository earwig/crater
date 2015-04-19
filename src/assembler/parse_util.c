/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <limits.h>
#include <string.h>

#include "parse_util.h"
#include "directives.h"
#include "../util.h"

#define MAX_REGION_SIZE 32

#define PARSE_FUNC_HEADER(name, type)                                         \
    bool parse_##name(type *result, const ASMLine *line, const char *directive)

/*
    All public functions in this file follow the same return conventions:

    - Return true on success and false on failure.
    - *result is only modified on success.
*/

/*
    Read in a boolean argument from the given line and store it in *result.
*/
PARSE_FUNC_HEADER(bool, bool)
{
    size_t offset = DIRECTIVE_OFFSET(line, directive) + 1;
    const char *arg = line->data + offset;
    ssize_t len = line->length - offset;

    if (len <= 0 || len > 5)
        return false;

    switch (len) {
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
    Read in a 32-bit int argument from the given line and store it in *result.
*/
PARSE_FUNC_HEADER(uint32_t, uint32_t)
{
    const char *str = line->data + DIRECTIVE_OFFSET(line, directive) + 1;
    const char *end = line->data + line->length;

    if (end - str <= 0)
        return false;

    uint64_t value = 0;
    if (*str == '$') {
        str++;
        if (str == end)
            return false;

        while (str < end) {
            if (*str >= '0' && *str <= '9')
                value = value * 16 + (*str - '0');
            else if (*str >= 'a' && *str <= 'f')
                value = (value * 0x10) + 0xA + (*str - 'a');
            else
                return false;
            if (value > UINT32_MAX)
                return false;
            str++;
        }
    }
    else {
        while (str < end) {
            if (*str < '0' || *str > '9')
                return false;
            value = (value * 10) + (*str - '0');
            if (value > UINT32_MAX)
                return false;
            str++;
        }
    }

    *result = value;
    return true;
}

/*
    Read in a 16-bit int argument from the given line and store it in *result.
*/
PARSE_FUNC_HEADER(uint16_t, uint16_t)
{
    uint32_t value;
    if (parse_uint32_t(&value, line, directive) && value <= UINT16_MAX)
        return (*result = value), true;
    return false;
}

/*
    Read in an 8-bit int argument from the given line and store it in *result.
*/
PARSE_FUNC_HEADER(uint8_t, uint8_t)
{
    uint32_t value;
    if (parse_uint32_t(&value, line, directive) && value <= UINT8_MAX)
        return (*result = value), true;
    return false;
}

/*
    Parse a ROM size string in an ASMLine and store it in *result.
*/
PARSE_FUNC_HEADER(rom_size, uint32_t)
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
PARSE_FUNC_HEADER(region_string, uint8_t)
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
PARSE_FUNC_HEADER(size_code, uint8_t)
{
    uint32_t bytes;
    if (!parse_uint32_t(&bytes, line, directive)) {
        if (!parse_rom_size(&bytes, line, directive))
            return false;
    }

    uint8_t code = size_bytes_to_code(bytes);
    if (code)
        return (*result = code), true;
    return false;
}
