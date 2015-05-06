/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdarg.h>
#include <stdlib.h>

#include "instructions.h"
#include "inst_args.h"
#include "parse_util.h"
#include "../logging.h"

/* Helper macros for get_inst_parser() */

#define JOIN_(a, b, c, d) ((uint32_t) ((a << 24) + (b << 16) + (c << 8) + d))

#define DISPATCH_(s, z) (                                                     \
    (z) == 2 ? JOIN_(s[0], s[1], 0x00, 0x00) :                                \
    (z) == 3 ? JOIN_(s[0], s[1], s[2], 0x00) :                                \
               JOIN_(s[0], s[1], s[2], s[3]))                                 \

#define MAKE_CMP_(s) DISPATCH_(s, sizeof(s) / sizeof(char) - 1)

#define HANDLE(m) if (key == MAKE_CMP_(#m)) return parse_inst_##m;

/* Internal helper macros for instruction parsers */

#define INST_ALLOC_(len)                                                      \
    *length = len;                                                            \
    if (!(*bytes = malloc(sizeof(uint8_t) * (len))))                          \
        OUT_OF_MEMORY()

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
    (((reg) == REG_IX || (reg) == REG_IXH || (reg) == REG_IXL) ? 0xDD : 0xFD)

/* Helper macros for instruction parsers */

#define INST_FUNC(mnemonic)                                                   \
static ASMErrorDesc parse_inst_##mnemonic(                                    \
    uint8_t **bytes, size_t *length, char **symbol, ASMArgParseInfo ap_info)  \

#define INST_ERROR(desc) return ED_PS_##desc;

#define INST_TAKES_NO_ARGS                                                    \
    if (ap_info.arg)                                                          \
        INST_ERROR(TOO_MANY_ARGS)                                             \

#define INST_TAKES_ARGS(lo, hi)                                               \
    if (!ap_info.arg)                                                         \
        INST_ERROR(TOO_FEW_ARGS)                                              \
    ASMInstArg args[3];                                                       \
    size_t nargs = 0;                                                         \
    ASMErrorDesc err = parse_args(args, &nargs, ap_info);                     \
    if (err)                                                                  \
        return err;                                                           \
    if (nargs < lo)                                                           \
        INST_ERROR(TOO_FEW_ARGS)                                              \
    if (nargs > hi)                                                           \
        INST_ERROR(TOO_MANY_ARGS)

#define INST_NARGS nargs
#define INST_TYPE(n) args[n].type
#define INST_REG(n) args[n].data.reg
#define INST_IMM(n) args[n].data.imm
#define INST_INDIRECT(n) args[n].data.indirect
#define INST_INDEX(n) args[n].data.index
#define INST_LABEL(n) args[n].data.label
#define INST_COND(n) args[n].data.cond

#define INST_FORCE_TYPE(n, t) {                                               \
        if (INST_TYPE(n) != t)                                                \
            INST_ERROR(ARG##n##_TYPE)                                         \
    }

#define INST_CHECK_IMM(n, m) {                                                \
        if (!(INST_IMM(n).mask & (m)))                                        \
            INST_ERROR(ARG##n##_RANGE)                                        \
    }

#define INST_INDIRECT_HL_ONLY(n) {                                            \
        if (INST_INDIRECT(n).type != AT_REGISTER)                             \
            INST_ERROR(ARG##n##_TYPE)                                         \
        if (INST_INDIRECT(n).addr.reg != REG_HL)                              \
            INST_ERROR(ARG##n##_BAD_REG)                                      \
    }

#define INST_RETURN(len, ...) {                                               \
        (void) symbol;                                                        \
        INST_ALLOC_(len)                                                      \
        INST_FILL_BYTES_(len, __VA_ARGS__)                                    \
        return ED_NONE;                                                       \
    }

#define INST_RETURN_WITH_SYMBOL(len, label, ...) {                            \
        *symbol = strdup(label);                                              \
        if (!(*symbol))                                                       \
            OUT_OF_MEMORY()                                                   \
        INST_ALLOC_(len)                                                      \
        INST_FILL_BYTES_(len - 2, __VA_ARGS__)                                \
        return ED_NONE;                                                       \
    }

#define INST_INDEX_PREFIX(n) INST_PREFIX_(INST_INDEX(n).reg)

#define INST_INDEX_BYTES(n, b)                                                \
    INST_INDEX_PREFIX(n), b, INST_INDEX(n).offset

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
    ASMInstArg *arg, const char *str, size_t size, ASMDefineTable *deftable)
{
#define TRY_PARSER(func, argtype, field)                                      \
    if (argparse_##func(&arg->data.field, info)) {                            \
        arg->type = argtype;                                                  \
        return ED_NONE;                                                       \
    }

    ASMArgParseInfo info = {.arg = str, .size = size, .deftable = deftable};
    DEBUG("parse_arg(): -->%.*s<-- %zu", (int) size, str, size)
    TRY_PARSER(register, AT_REGISTER, reg)
    TRY_PARSER(immediate, AT_IMMEDIATE, imm)
    TRY_PARSER(indirect, AT_INDIRECT, indirect)
    TRY_PARSER(indexed, AT_INDEXED, index)
    TRY_PARSER(condition, AT_CONDITION, cond)
    TRY_PARSER(label, AT_LABEL, label)
    return ED_PS_ARG_SYNTAX;

#undef TRY_PARSER
}

/*
    Parse an argument string into ASMInstArg objects.

    Return ED_NONE (0) on success or an error code on failure.
*/
static ASMErrorDesc parse_args(
    ASMInstArg args[3], size_t *nargs, ASMArgParseInfo ap_info)
{
    ASMErrorDesc err;
    ASMDefineTable *dt = ap_info.deftable;
    const char *str = ap_info.arg;
    size_t size = ap_info.size, start = 0, i = 0;

    while (i < size) {
        char c = str[i];
        if (c == ',') {
            if (i == start)
                return ED_PS_ARG_SYNTAX;
            if ((err = parse_arg(&args[*nargs], str + start, i - start, dt)))
                return err;
            (*nargs)++;

            i++;
            if (i < size && str[i] == ' ')
                i++;
            start = i;
            if (i == size)
                return ED_PS_ARG_SYNTAX;
            if (*nargs >= 3)
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
        if ((err = parse_arg(&args[*nargs], str + start, i - start, dt)))
            return err;
        (*nargs)++;
    }
    return ED_NONE;
}

/* Instruction parser functions */

INST_FUNC(nop)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x00)
}

INST_FUNC(inc)
{
    INST_TAKES_ARGS(1, 1)
    switch (INST_TYPE(0)) {
        case AT_REGISTER:
            switch (INST_REG(0)) {
                case REG_A:   INST_RETURN(1, 0x3C)
                case REG_B:   INST_RETURN(1, 0x04)
                case REG_C:   INST_RETURN(1, 0x0C)
                case REG_D:   INST_RETURN(1, 0x14)
                case REG_E:   INST_RETURN(1, 0x1C)
                case REG_H:   INST_RETURN(1, 0x24)
                case REG_L:   INST_RETURN(1, 0x2C)
                case REG_BC:  INST_RETURN(1, 0x03)
                case REG_DE:  INST_RETURN(1, 0x13)
                case REG_HL:  INST_RETURN(1, 0x23)
                case REG_SP:  INST_RETURN(1, 0x33)
                case REG_IX:  INST_RETURN(2, 0xDD, 0x23)
                case REG_IY:  INST_RETURN(2, 0xFD, 0x23)
                case REG_IXH: INST_RETURN(2, 0xDD, 0x2C)
                case REG_IXL: INST_RETURN(2, 0xFD, 0x2C)
                case REG_IYH: INST_RETURN(2, 0xDD, 0x2C)
                case REG_IYL: INST_RETURN(2, 0xFD, 0x2C)
                default: INST_ERROR(ARG0_BAD_REG)
            }
        case AT_INDIRECT:
            INST_INDIRECT_HL_ONLY(0)
            INST_RETURN(1, 0x34)
        case AT_INDEXED:
            INST_RETURN(3, INST_INDEX_BYTES(0, 0x34))
        default:
            INST_ERROR(ARG0_TYPE)
    }
}

INST_FUNC(add)
{
    INST_TAKES_ARGS(2, 2)
    INST_FORCE_TYPE(0, AT_REGISTER)
    switch (INST_REG(0)) {
        case REG_A:
            switch (INST_TYPE(1)) {
                case AT_REGISTER:
                    switch (INST_REG(1)) {
                        case REG_A:   INST_RETURN(1, 0x87)
                        case REG_B:   INST_RETURN(1, 0x80)
                        case REG_C:   INST_RETURN(1, 0x81)
                        case REG_D:   INST_RETURN(1, 0x82)
                        case REG_E:   INST_RETURN(1, 0x83)
                        case REG_H:   INST_RETURN(1, 0x84)
                        case REG_L:   INST_RETURN(1, 0x85)
                        case REG_IXH: INST_RETURN(2, 0xDD, 0x84)
                        case REG_IXL: INST_RETURN(2, 0xDD, 0x85)
                        case REG_IYH: INST_RETURN(2, 0xFD, 0x84)
                        case REG_IYL: INST_RETURN(2, 0xFD, 0x85)
                        default: INST_ERROR(ARG1_BAD_REG)
                    }
                case AT_IMMEDIATE:
                    INST_CHECK_IMM(1, IMM_U8)
                    INST_RETURN(2, 0xC6, INST_IMM(1).uval)
                case AT_INDIRECT:
                    INST_INDIRECT_HL_ONLY(1)
                    INST_RETURN(1, 0x86)
                case AT_INDEXED:
                    INST_RETURN(3, INST_INDEX_BYTES(1, 0x86))
                default:
                    INST_ERROR(ARG1_TYPE)
            }
        case REG_HL:
            INST_FORCE_TYPE(1, AT_REGISTER)
            switch (INST_REG(1)) {
                case REG_BC: INST_RETURN(1, 0x09)
                case REG_DE: INST_RETURN(1, 0x19)
                case REG_HL: INST_RETURN(1, 0x29)
                case REG_SP: INST_RETURN(1, 0x39)
                default: INST_ERROR(ARG1_BAD_REG)
            }
        case REG_IX:
        case REG_IY:
            INST_FORCE_TYPE(1, AT_REGISTER)
            switch (INST_REG(1)) {
                case REG_BC: INST_RETURN(2, INST_INDEX_PREFIX(1), 0x09)
                case REG_DE: INST_RETURN(2, INST_INDEX_PREFIX(1), 0x19)
                case REG_IX:
                case REG_IY:
                    if (INST_REG(0) != INST_REG(1))
                        INST_ERROR(ARG1_BAD_REG)
                    INST_RETURN(2, INST_INDEX_PREFIX(1), 0x29)
                case REG_SP: INST_RETURN(2, INST_INDEX_PREFIX(1), 0x39)
                default: INST_ERROR(ARG1_BAD_REG)
            }
        default:
            INST_ERROR(ARG0_TYPE)
    }
}

/*
INST_FUNC(adc)
{
    DEBUG("dispatched to -> ADC")
    return ED_PS_TOO_FEW_ARGS;
}
*/

INST_FUNC(reti)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x4D)
}

INST_FUNC(retn)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x45)
}

/*
    Return the relevant ASMInstParser function for a given mnemonic.

    NULL is returned if the mnemonic is not known.
*/
ASMInstParser get_inst_parser(char mstr[MAX_MNEMONIC_SIZE])
{
    // Exploit the fact that we can store the entire mnemonic string as a
    // single 32-bit value to do fast lookups:
    uint32_t key = (mstr[0] << 24) + (mstr[1] << 16) + (mstr[2] << 8) + mstr[3];

    HANDLE(nop)
    HANDLE(inc)
    HANDLE(add)
    // HANDLE(adc)
    HANDLE(reti)
    HANDLE(retn)

    return NULL;
}
