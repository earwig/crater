/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdio.h>

#include "state.h"

/* Enums */

typedef enum {
    ET_INCLUDE,
    ET_PREPROC,
    ET_LAYOUT,
    ET_SYMBOL,
    ET_PARSER
} ASMErrorType;

typedef enum {
    ED_NONE = 0,

    ED_INC_BAD_ARG,
    ED_INC_DEPTH,
    ED_INC_FILE_READ,

    ED_PP_UNKNOWN,
    ED_PP_DUPLICATE,
    ED_PP_NO_ARG,
    ED_PP_BAD_ARG,
    ED_PP_ARG_RANGE,

    ED_LYT_HEADER_RANGE,
    ED_LYT_DECL_RANGE,
    ED_LYT_BOUNDS,
    ED_LYT_BLOCK0,
    ED_LYT_SLOTS,
    ED_LYT_BLOCK_CROSS,
    ED_LYT_OVERLAP,
    ED_LYT_OVERLAP_HEAD,

    ED_SYM_DUPE_LABELS,
    ED_SYM_NO_LABEL,
    ED_SYM_TOO_LONG,
    ED_SYM_IS_REGISTER,
    ED_SYM_IS_CONDITION,

    ED_PS_OP_TOO_LONG,
    ED_PS_OP_TOO_SHORT,
    ED_PS_OP_INVALID,
    ED_PS_OP_UNKNOWN,
    ED_PS_TOO_FEW_ARGS,
    ED_PS_TOO_MANY_ARGS,
    ED_PS_ARG_SYNTAX,
    ED_PS_ARG_TYPE,
    ED_PS_ARG_VALUE
} ASMErrorDesc;

/* Structs */

typedef struct ErrorInfo ErrorInfo;

/* Functions */

ErrorInfo* error_info_create(const ASMLine*, ASMErrorType, ASMErrorDesc);
void error_info_append(ErrorInfo*, const ASMLine*);
void error_info_print(const ErrorInfo*, FILE*);
void error_info_destroy(ErrorInfo*);
