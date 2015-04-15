/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

/* Enums */

typedef enum {
    ET_SYNTAX,
    ET_FILEIO
} ASMErrorType;

typedef enum {
    ED_INCLUDE_BAD_ARG,
    ED_FILE_READ_ERR
} ASMErrorDesc;

/* Strings */

static const char *asm_error_types[] = {
    "invalid syntax",
    "file I/O"
};

static const char *asm_error_descs[] = {
    "bad argument passed to include directive",
    "couldn't read from file"
};
