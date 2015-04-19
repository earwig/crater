/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdio.h>

#include "state.h"

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
    ED_PP_BAD_ARG,
    ED_PP_ARG_RANGE,
    ED_PP_HEADER_RANGE,
    ED_PP_DECLARE_RANGE
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
    "invalid argument for directive",
    "directive argument out of range",
    "header offset exceeds given ROM size",
    "declared ROM size in header exceeds actual size"
};

/* Structs */

typedef struct ErrorInfo ErrorInfo;

/* Functions */

ErrorInfo* error_info_create(const ASMLine*, ASMErrorType, ASMErrorDesc);
void error_info_append(ErrorInfo*, const ASMLine*);
void error_info_print(const ErrorInfo*, FILE*);
void error_info_destroy(ErrorInfo*);
