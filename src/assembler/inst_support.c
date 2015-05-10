/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "inst_support.h"

/* Macro used by parse_arg() */

#define TRY_PARSER(func, argtype, field)                                      \
    if (argparse_##func(&arg->data.field, info)) {                            \
        arg->type = argtype;                                                  \
        return ED_NONE;                                                       \
    }

/*
    Fill an instruction's byte array with the given data.

    This internal function is only called for instructions longer than four
    bytes (of which there is only one: the fake emulator debugging/testing
    opcode with mnemonic "emu"), so it does not get used in normal situations.

    Return the value of the last byte inserted, for compatibility with the
    INST_SETn_ family of macros.
*/
uint8_t fill_bytes_variadic(uint8_t *bytes, size_t len, ...)
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
    ASMArgParseInfo info = {.arg = str, .size = size, .deftable = deftable};
    TRY_PARSER(register, AT_REGISTER, reg)
    TRY_PARSER(immediate, AT_IMMEDIATE, imm)
    TRY_PARSER(indirect, AT_INDIRECT, indirect)
    TRY_PARSER(indexed, AT_INDEXED, index)
    TRY_PARSER(condition, AT_CONDITION, cond)
    TRY_PARSER(label, AT_LABEL, label)
    return ED_PS_ARG_SYNTAX;
}

/*
    Parse an argument string into ASMInstArg objects.

    Return ED_NONE (0) on success or an error code on failure.
*/
ASMErrorDesc parse_args(
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
