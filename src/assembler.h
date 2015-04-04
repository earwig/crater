/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Structs */

typedef struct {
    char *data;
    size_t length;
} Line;

typedef struct {
    Line *lines;
    size_t length;
} LineBuffer;

typedef struct {
    //
} ErrorInfo;

/* Functions */

void error_info_print(const ErrorInfo*, FILE*, const LineBuffer*);
void error_info_destroy(ErrorInfo*);
size_t assemble(const LineBuffer*, uint8_t**, ErrorInfo**);
bool assemble_file(const char*, const char*);
