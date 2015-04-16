/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include "state.h"
#include "../assembler.h"

/* Functions */

ErrorInfo* preprocess(AssemblerState*, const LineBuffer*);
