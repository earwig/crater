/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdarg.h>
#include <stdlib.h>

#include "instructions.h"
#include "inst_args.h"
#include "parse_util.h"
#include "../util.h"

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
    (((reg) == REG_IX || (reg) == REG_IXH || (reg) == REG_IXL) ? 0xDD : 0xFD)

/* Helper macros for instruction parsers */

#define INST_FUNC(mnemonic)                                                   \
static ASMErrorDesc parse_inst_##mnemonic(                                    \
    uint8_t **bytes, size_t *length, char **symbol, ASMArgParseInfo ap_info)  \

#define INST_ERROR(desc) return ED_PS_##desc;

#define INST_TAKES_NO_ARGS                                                    \
    if (ap_info.arg)                                                          \
        INST_ERROR(TOO_MANY_ARGS)

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
        *symbol = cr_strdup(label);                                           \
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

INST_FUNC(adc)
{
    INST_TAKES_ARGS(2, 2)
    INST_FORCE_TYPE(0, AT_REGISTER)
    switch (INST_REG(0)) {
        case REG_A:
            switch (INST_TYPE(1)) {
                case AT_REGISTER:
                    switch (INST_REG(1)) {
                        case REG_A:   INST_RETURN(1, 0x8F)
                        case REG_B:   INST_RETURN(1, 0x88)
                        case REG_C:   INST_RETURN(1, 0x89)
                        case REG_D:   INST_RETURN(1, 0x8A)
                        case REG_E:   INST_RETURN(1, 0x8B)
                        case REG_H:   INST_RETURN(1, 0x8C)
                        case REG_L:   INST_RETURN(1, 0x8D)
                        case REG_IXH: INST_RETURN(2, 0xDD, 0x8C)
                        case REG_IXL: INST_RETURN(2, 0xDD, 0x8D)
                        case REG_IYH: INST_RETURN(2, 0xFD, 0x8C)
                        case REG_IYL: INST_RETURN(2, 0xFD, 0x8D)
                        default: INST_ERROR(ARG1_BAD_REG)
                    }
                case AT_IMMEDIATE:
                    INST_CHECK_IMM(1, IMM_U8)
                    INST_RETURN(2, 0xCE, INST_IMM(1).uval)
                case AT_INDIRECT:
                    INST_INDIRECT_HL_ONLY(1)
                    INST_RETURN(1, 0x8E)
                case AT_INDEXED:
                    INST_RETURN(3, INST_INDEX_BYTES(1, 0x8E))
                default:
                    INST_ERROR(ARG1_TYPE)
            }
        case REG_HL:
            INST_FORCE_TYPE(1, AT_REGISTER)
            switch (INST_REG(1)) {
                case REG_BC: INST_RETURN(2, 0xED, 0x4A)
                case REG_DE: INST_RETURN(2, 0xED, 0x5A)
                case REG_HL: INST_RETURN(2, 0xED, 0x6A)
                case REG_SP: INST_RETURN(2, 0xED, 0x7A)
                default: INST_ERROR(ARG1_BAD_REG)
            }
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

INST_FUNC(and)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(bit)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(call)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ccf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x3F)
}

INST_FUNC(cp)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(cpd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA9)
}

INST_FUNC(cpdr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB9)
}

INST_FUNC(cpi)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA1)
}

INST_FUNC(cpir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB1)
}

INST_FUNC(cpl)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x2F)
}

INST_FUNC(daa)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x27)
}

INST_FUNC(dec)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(di)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xF3)
}

INST_FUNC(djnz)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ei)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xFB)
}

INST_FUNC(ex)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(exx)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0xD9)
}

INST_FUNC(halt)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x76)
}

INST_FUNC(im)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(in)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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

INST_FUNC(ind)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xAA)
}

INST_FUNC(indr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xBA)
}

INST_FUNC(ini)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA2)
}

INST_FUNC(inir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB2)
}

INST_FUNC(jp)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(jr)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ld)
{
    INST_TAKES_ARGS(2, 2)
    switch (INST_TYPE(0)) {
        case AT_REGISTER:
            switch (INST_REG(0)) {
                case REG_A:   // TODO (20 cases)
                case REG_B:   // TODO (15 cases)
                case REG_C:   // TODO (15 cases)
                case REG_D:   // TODO (15 cases)
                case REG_E:   // TODO (15 cases)
                case REG_H:   // TODO (11 cases)
                case REG_L:   // TODO (11 cases)
                case REG_I:   // TODO ( 1 case )
                case REG_R:   // TODO ( 1 case )
                case REG_BC:  // TODO ( 2 cases)
                case REG_DE:  // TODO ( 2 cases)
                case REG_HL:  // TODO ( 3 cases)
                case REG_IX:  // TODO ( 2 cases)
                case REG_IY:  // TODO ( 2 cases)
                case REG_SP:  // TODO ( 5 cases)
                case REG_IXH: // TODO ( 8 cases)
                case REG_IXL: // TODO ( 8 cases)
                case REG_IYH: // TODO ( 8 cases)
                case REG_IYL: // TODO ( 8 cases)
                default: INST_ERROR(ARG0_BAD_REG)
            }
        case AT_INDIRECT:
            switch (INST_INDIRECT(0).type) {
                case AT_REGISTER:
                    switch (INST_INDIRECT(0).addr.reg) {
                        case REG_BC: // TODO (1 case )
                        case REG_DE: // TODO (1 case )
                        case REG_HL: // TODO (8 cases)
                        default: INST_ERROR(ARG0_BAD_REG)
                    }
                case AT_IMMEDIATE:
                    // TODO (8 cases)
                case AT_LABEL:
                    // TODO (same 8 cases)
                default:
                    INST_ERROR(ARG0_TYPE)
            }
        case AT_INDEXED:
            // TODO (16 cases)
        default:
            INST_ERROR(ARG0_TYPE)
    }
}

INST_FUNC(ldd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA8)
}

INST_FUNC(lddr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB8)
}

INST_FUNC(ldi)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA0)
}

INST_FUNC(ldir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB0)
}

INST_FUNC(neg)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(nop)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x00)
}

INST_FUNC(or)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(otdr)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xBB)
}

INST_FUNC(otir)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xB3)
}

INST_FUNC(out)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(outd)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xAB)
}

INST_FUNC(outi)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0xA3)
}

INST_FUNC(pop)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(push)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(res)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(ret)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

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

INST_FUNC(rl)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rla)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x17)
}

INST_FUNC(rlc)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rlca)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x07)
}

INST_FUNC(rld)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(2, 0xED, 0x6F)
}

INST_FUNC(rr)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rra)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x1F)
}

INST_FUNC(rrc)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rrca)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x0F)
}

INST_FUNC(rrd)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(rst)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sbc)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(scf)
{
    INST_TAKES_NO_ARGS
    INST_RETURN(1, 0x37)
}

INST_FUNC(set)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sl1)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sla)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sll)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sls)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sra)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(srl)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(sub)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
}

INST_FUNC(xor)
{
    // TODO
    INST_TAKES_NO_ARGS
    INST_ERROR(ARG_SYNTAX)
    INST_RETURN(1, 0xFF)
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

    HANDLE(adc)
    HANDLE(add)
    HANDLE(and)
    HANDLE(bit)
    HANDLE(call)
    HANDLE(ccf)
    HANDLE(cp)
    HANDLE(cpd)
    HANDLE(cpdr)
    HANDLE(cpi)
    HANDLE(cpir)
    HANDLE(cpl)
    HANDLE(daa)
    HANDLE(dec)
    HANDLE(di)
    HANDLE(djnz)
    HANDLE(ei)
    HANDLE(ex)
    HANDLE(exx)
    HANDLE(halt)
    HANDLE(im)
    HANDLE(in)
    HANDLE(inc)
    HANDLE(ind)
    HANDLE(indr)
    HANDLE(ini)
    HANDLE(inir)
    HANDLE(jp)
    HANDLE(jr)
    HANDLE(ld)
    HANDLE(ldd)
    HANDLE(lddr)
    HANDLE(ldi)
    HANDLE(ldir)
    HANDLE(neg)
    HANDLE(nop)
    HANDLE(or)
    HANDLE(otdr)
    HANDLE(otir)
    HANDLE(out)
    HANDLE(outd)
    HANDLE(outi)
    HANDLE(pop)
    HANDLE(push)
    HANDLE(res)
    HANDLE(ret)
    HANDLE(reti)
    HANDLE(retn)
    HANDLE(rl)
    HANDLE(rla)
    HANDLE(rlc)
    HANDLE(rlca)
    HANDLE(rld)
    HANDLE(rr)
    HANDLE(rra)
    HANDLE(rrc)
    HANDLE(rrca)
    HANDLE(rrd)
    HANDLE(rst)
    HANDLE(sbc)
    HANDLE(scf)
    HANDLE(set)
    HANDLE(sl1)
    HANDLE(sla)
    HANDLE(sll)
    HANDLE(sls)
    HANDLE(sra)
    HANDLE(srl)
    HANDLE(sub)
    HANDLE(xor)

    return NULL;
}
