/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

/* Enums */

typedef enum {
    ET_INCLUDE,
    ET_PREPROC
} ASMErrorType;

typedef enum {
    ED_INC_BAD_ARG,
    ED_INC_RECURSION,
    ED_INC_FILE_READ,

    ED_PP_UNKNOWN,
    ED_PP_DUPLICATE,
    ED_PP_NO_ARG,
    ED_PP_BAD_ARG
} ASMErrorDesc;

/* Strings */

static const char *asm_error_types[] = {
    "include directive",
    "preprocessor"
};

static const char *asm_error_descs[] = {
    "missing or invalid argument",
    "infinite recursion detected",
    "couldn't read included file",

    "unknown directive",
    "multiple values for directive",
    "missing argument for directive",
    "invalid argument for directive"
};
