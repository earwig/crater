/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdint.h>

#include "../assembler.h"

/* Functions */

void line_buffer_free(LineBuffer*);
LineBuffer* read_source_file(const char*, bool);
bool write_binary_file(const char*, const uint8_t*, size_t);
