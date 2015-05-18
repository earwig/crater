/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdarg.h>
#include <stdlib.h>

#include "instructions.h"
#include "inst_args.h"
#include "../util.h"

/* Helper macros for get_inst_parser() and lookup_parser() */

#define JOIN(a, b, c, d) ((uint32_t) ((a << 24) + (b << 16) + (c << 8) + d))

#define DISPATCH_(s, z) (                                                     \
    (z) == 2 ? JOIN(s[0], s[1], 0x00, 0x00) :                                 \
    (z) == 3 ? JOIN(s[0], s[1], s[2], 0x00) :                                 \
               JOIN(s[0], s[1], s[2], s[3]))                                  \

#define MAKE_CMP_(s) DISPATCH_(s, sizeof(s) / sizeof(char) - 1)

#define HANDLE(m) if (key == MAKE_CMP_(#m)) return parse_inst_##m;

/* Helper macro for parse_arg() */

#define TRY_PARSER(func, argtype, field)                                      \
    if (mask & argtype && argparse_##func(&arg->data.field, info)) {          \
        arg->type = argtype;                                                  \
        return ED_NONE;                                                       \
    }

/* Internal helper macros */

#define INST_ALLOC_(len)                                                      \
    *length = len;                                                            \
    *bytes = cr_malloc(sizeof(uint8_t) * (len));

#define INST_SET_(b, val) ((*bytes)[b] = val)
#define INST_SET1_(b1) INST_SET_(0, b1)
#define INST_SET2_(b1, b2) INST_SET1_(b1), INST_SET_(1, b2)
#define INST_SET3_(b1, b2, b3) INST_SET2_(b1, b2), INST_SET_(2, b3)
#define INST_SET4_(b1, b2, b3, b4) INST_SET3_(b1, b2, b3), INST_SET_(3, b4)

#define INST_DISPATCH_(a, b, c, d, target, ...) target

#define INST_FILL_BYTES_(len, ...)                                            \
    ((len > 4) ? fill_bytes_variadic(*bytes, len, __VA_ARGS__) :              \
    INST_DISPATCH_(__VA_ARGS__, INST_SET4_, INST_SET3_, INST_SET2_,           \
                   INST_SET1_, __VA_ARGS__)(__VA_ARGS__));

#define INST_PREFIX_(reg)                                                     \
    (((reg) == REG_IX || (reg) == REG_IXH || (reg) == REG_IXL) ?              \
     INST_IX_PREFIX : INST_IY_PREFIX)

/* Helper macros for instruction parsers */

#define INST_FUNC(mnemonic)                                                   \
static ASMErrorDesc parse_inst_##mnemonic(                                    \
    uint8_t **bytes, size_t *length, char **symbol, ASMArgParseInfo ap_info)  \

#define INST_ERROR(desc) return ED_PS_##desc;

#define INST_TAKES_NO_ARGS                                                    \
    if (ap_info.arg)                                                          \
        INST_ERROR(TOO_MANY_ARGS)

#define INST_TAKES_ARGS(a0, a1, a2)                                           \
    if (!ap_info.arg)                                                         \
        INST_ERROR(TOO_FEW_ARGS)                                              \
    ASMInstArg args[3];                                                       \
    size_t nargs;                                                             \
    ASMArgType masks[] = {a0, a1, a2};                                        \
    ASMErrorDesc err = parse_args(args, &nargs, ap_info, masks);              \
    if (err)                                                                  \
        return err;                                                           \

#define INST_NARGS nargs
#define INST_TYPE(n) args[n].type
#define INST_REG(n) args[n].data.reg
#define INST_IMM(n) args[n].data.imm
#define INST_INDIRECT(n) args[n].data.indirect
#define INST_INDEX(n) args[n].data.index
#define INST_COND(n) args[n].data.cond
#define INST_PORT(n) args[n].data.port

#define INST_IX_PREFIX 0xDD
#define INST_IY_PREFIX 0xFD

#define INST_RETURN(len, ...) {                                               \
        (void) symbol;                                                        \
        INST_ALLOC_(len)                                                      \
        INST_FILL_BYTES_(len, __VA_ARGS__)                                    \
        return ED_NONE;                                                       \
    }

#define INST_IMM_U16_B1(imm)                                                  \
    ((imm).is_label ? (*symbol = cr_strdup((imm).label), 0) : (imm).uval >> 8)
#define INST_IMM_U16_B2(imm)                                                  \
    ((imm).is_label ? 0 : (imm).uval & 0xFF)

#define INST_INDEX_PREFIX(n) INST_PREFIX_(INST_INDEX(n).reg)

/* ----------------------------- END WORK BLOCK ---------------------------- */

/*
    Fill an instruction's byte array with the given data.

    This internal function is only called for instructions longer than four
    bytes (of which there is only one: the fake emulator debugging/testing
    opcode with mnemonic "emu"), so it does not get used in normal situations.

    Return the value of the last byte inserted, for compatibility with the
    INST_SETn_ family of macros.
*/
static uint8_t fill_bytes_variadic(uint8_t *bytes, size_t len, ...)
{
    va_list vargs;
    va_start(vargs, len);
    for (size_t i = 0; i < len; i++)
        bytes[i] = va_arg(vargs, unsigned);
    va_end(vargs);
    return bytes[len - 1];
}

/*
    Parse a single instruction argument into an ASMInstArg object.

    Return ED_NONE (0) on success or an error code on failure.
*/
static ASMErrorDesc parse_arg(
    ASMInstArg *arg, const char *str, size_t size, ASMDefineTable *deftable,
    ASMArgType mask)
{
    ASMArgParseInfo info = {.arg = str, .size = size, .deftable = deftable};
    TRY_PARSER(register, AT_REGISTER, reg)
    TRY_PARSER(condition, AT_CONDITION, cond)
    TRY_PARSER(indexed, AT_INDEXED, index)
    TRY_PARSER(indirect, AT_INDIRECT, indirect)
    TRY_PARSER(port, AT_PORT, port)
    TRY_PARSER(immediate, AT_IMMEDIATE, imm)
    return ED_PS_ARG_SYNTAX;
}

/*
    Parse an argument string into ASMInstArg objects.

    Return ED_NONE (0) on success or an error code on failure.
*/
static ASMErrorDesc parse_args(
    ASMInstArg args[3], size_t *nargs, ASMArgParseInfo ap_info,
    ASMArgType masks[3])
{
    ASMErrorDesc err;
    ASMDefineTable *dt = ap_info.deftable;
    const char *str = ap_info.arg;
    size_t size = ap_info.size, start = 0, i = 0, n = 0;

    while (i < size) {
        char c = str[i];
        if (c == ',') {
            if (i == start)
                return ED_PS_ARG_SYNTAX;
            if (masks[n] == AT_NONE)
                return ED_PS_TOO_MANY_ARGS;
            err = parse_arg(&args[n], str + start, i - start, dt, masks[n]);
            if (err)
                return err;
            n++;

            i++;
            if (i < size && str[i] == ' ')
                i++;
            start = i;
            if (i == size)
                return ED_PS_ARG_SYNTAX;
            if (n >= 3)
                return ED_PS_TOO_MANY_ARGS;
        } else {
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                 c == ' ' || c == '+' || c == '-' || c == '(' || c == ')' ||
                 c == '$' || c == '_' || c == '.')
                i++;
            else
                return ED_PS_ARG_SYNTAX;
        }
    }

    if (i > start) {
        if (masks[n] == AT_NONE)
            return ED_PS_TOO_MANY_ARGS;
        if ((err = parse_arg(&args[n], str + start, i - start, dt, masks[n])))
            return err;
        n++;
    }

    if (n < 3 && masks[n] != AT_NONE && !(masks[n] & AT_OPTIONAL))
        return ED_PS_TOO_FEW_ARGS;
    *nargs = n;
    return ED_NONE;
}

#include "instructions.inc.c"

/*
    Return the relevant ASMInstParser function for a given mnemonic.

    NULL is returned if the mnemonic is not known.
*/
ASMInstParser get_inst_parser(char mstr[MAX_MNEMONIC_SIZE])
{
    // Exploit the fact that we can store the entire mnemonic string as a
    // single 32-bit value to do fast lookups:
    uint32_t key = JOIN(mstr[0], mstr[1], mstr[2], mstr[3]);
    return lookup_parser(key);
}
