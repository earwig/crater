/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Structs */

struct Line {
    char *data;
    size_t length;
    size_t lineno;
    struct Line *next;
};
typedef struct Line Line;

typedef struct {
    Line *lines;
    char *filename;
} LineBuffer;

typedef struct ErrorInfo ErrorInfo;

/* Functions */

void error_info_print(const ErrorInfo*, FILE*);
void error_info_destroy(ErrorInfo*);
size_t assemble(const LineBuffer*, uint8_t**, ErrorInfo**);
bool assemble_file(const char*, const char*);
