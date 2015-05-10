/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdarg.h>
#include <stdlib.h>

#include "errors.h"
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

#define INST_IX_PREFIX_ 0xDD
#define INST_IY_PREFIX_ 0xFD

#define INST_PREFIX_(reg)                                                     \
    (((reg) == REG_IX || (reg) == REG_IXH || (reg) == REG_IXL) ?              \
     INST_IX_PREFIX_ : INST_IY_PREFIX_)

#define INST_RETURN_WITH_SYMBOL_(len, label, ...) {                           \
        *symbol = cr_strdup(label.text);                                      \
        INST_ALLOC_(len)                                                      \
        INST_FILL_BYTES_(len - 2, __VA_ARGS__)                                \
        return ED_NONE;                                                       \
    }

/* Essential/basic helper macros */

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

#define INST_RETURN(len, ...) {                                               \
        (void) symbol;                                                        \
        INST_ALLOC_(len)                                                      \
        INST_FILL_BYTES_(len, __VA_ARGS__)                                    \
        return ED_NONE;                                                       \
    }

/* Convenience macros */

#define INST_FORCE_TYPE(n, t) {                                               \
        if (INST_TYPE(n) != t)                                                \
            INST_ERROR(ARG##n##_TYPE)                                         \
    }

#define INST_CHECK_IMM(n, m) {                                                \
        if (!(INST_IMM(n).mask & (m)))                                        \
            INST_ERROR(ARG##n##_RANGE)                                        \
    }

#define INST_REG_ONLY(n, reg) {                                               \
        if (INST_TYPE(n) != AT_REGISTER)                                      \
            INST_ERROR(ARG##n##_TYPE)                                         \
        if (INST_REG(n) != reg)                                               \
            INST_ERROR(ARG##n##_BAD_REG)                                      \
    }

#define INST_INDIRECT_HL_ONLY(n) {                                            \
        if (INST_INDIRECT(n).type != AT_REGISTER)                             \
            INST_ERROR(ARG##n##_TYPE)                                         \
        if (INST_INDIRECT(n).addr.reg != REG_HL)                              \
            INST_ERROR(ARG##n##_BAD_REG)                                      \
    }

#define INST_INDEX_PREFIX(n) INST_PREFIX_(INST_INDEX(n).reg)

#define INST_INDEX_BYTES(n, b)                                                \
    INST_INDEX_PREFIX(n), b, INST_INDEX(n).offset

#define INST_INDIRECT_IMM(n)                                                  \
    INST_INDIRECT(n).addr.imm.uval >> 8,                                      \
    INST_INDIRECT(n).addr.imm.uval & 0xFF

#define INST_RETURN_INDIRECT_LABEL(n, len, ...)                               \
    INST_RETURN_WITH_SYMBOL_(len, INST_INDIRECT(n).addr.label, __VA_ARGS__)

#define INST_HANDLE_MAIN_8_BIT_REGS(base)                                     \
    case REG_A:   INST_RETURN(1, base + 7)                                    \
    case REG_B:   INST_RETURN(1, base)                                        \
    case REG_C:   INST_RETURN(1, base + 1)                                    \
    case REG_D:   INST_RETURN(1, base + 2)                                    \
    case REG_E:   INST_RETURN(1, base + 3)                                    \
    case REG_H:   INST_RETURN(1, base + 4)                                    \
    case REG_L:   INST_RETURN(1, base + 5)                                    \
    case REG_IXH: INST_RETURN(2, INST_IX_PREFIX_, base + 4)                   \
    case REG_IXL: INST_RETURN(2, INST_IX_PREFIX_, base + 5)                   \
    case REG_IYH: INST_RETURN(2, INST_IY_PREFIX_, base + 4)                   \
    case REG_IYL: INST_RETURN(2, INST_IY_PREFIX_, base + 5)                   \

/* Internal functions */

uint8_t fill_bytes_variadic(uint8_t*, size_t, ...);
ASMErrorDesc parse_args(ASMInstArg args[3], size_t*, ASMArgParseInfo);
