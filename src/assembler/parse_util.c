/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <limits.h>
#include <string.h>

#include "parse_util.h"
#include "directives.h"

/*
    Read in a boolean argument from the given line and store it in *result.

    auto_val is used if the argument's value is "auto". Return true on success
    and false on failure; in the latter case, *result is not modified.
*/
bool parse_bool(bool *result, const ASMLine *line, const char *directive, bool auto_val)
{
    const char *arg = line->data + (DIRECTIVE_OFFSET(line, directive) + 1);
    ssize_t len = line->length - (DIRECTIVE_OFFSET(line, directive) + 1);

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
        case 4:  // true, auto
            if (!strncmp(arg, "true", 4))
                return (*result = true), true;
            if (!strncmp(arg, "auto", 4))
                return (*result = auto_val), true;
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

    Return true on success and false on failure; in the latter case, *result is
    not modified.
*/
bool parse_uint32(uint32_t *result, const ASMLine *line, const char *directive)
{
    size_t offset = DIRECTIVE_OFFSET(line, directive) + 1;
    const char *str = line->data + offset;
    const char *end = str + line->length - offset;

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

    Return true on success and false on failure; in the latter case, *result is
    not modified.
*/
bool parse_uint16(uint16_t *result, const ASMLine *line, const char *directive)
{
    uint32_t value;
    if (parse_uint32(&value, line, directive) && value <= UINT16_MAX)
        return (*result = value), true;
    return false;
}

/*
    Read in an 8-bit int argument from the given line and store it in *result.

    Return true on success and false on failure; in the latter case, *result is
    not modified.
*/
bool parse_uint8(uint8_t *result, const ASMLine *line, const char *directive)
{
    uint32_t value;
    if (parse_uint32(&value, line, directive) && value <= UINT8_MAX)
        return (*result = value), true;
    return false;
}
