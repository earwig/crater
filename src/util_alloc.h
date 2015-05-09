/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

#define OUT_OF_MEMORY() FATAL("couldn't allocate enough memory")

#define OOM_GUARD_(type, call)                                                \
    type ptr = call;                                                          \
    if (!ptr)                                                                 \
        OUT_OF_MEMORY()                                                       \
    return ptr;

#define OOM_GUARD_FUNC_1_(ret_type, func, arg_type)                           \
    static inline ret_type cr_##func(arg_type arg1) {                         \
        OOM_GUARD_(ret_type, func(arg1))                                      \
    }

#define OOM_GUARD_FUNC_2_(ret_type, func, arg1_type, arg2_type)               \
    static inline ret_type cr_##func(arg1_type arg1, arg2_type arg2) {        \
        OOM_GUARD_(ret_type, func(arg1, arg2))                                \
    }

/* Functions */

OOM_GUARD_FUNC_1_(void*, malloc, size_t)                // cr_malloc
OOM_GUARD_FUNC_2_(void*, calloc, size_t, size_t)        // cr_calloc
OOM_GUARD_FUNC_2_(void*, realloc, void*, size_t)        // cr_realloc
OOM_GUARD_FUNC_1_(char*, strdup, const char*)           // cr_strdup
OOM_GUARD_FUNC_2_(char*, strndup, const char*, size_t)  // cr_strndup

#undef OOM_GUARD_FUNC2_
#undef OOM_GUARD_FUNC1_
#undef OOM_GUARD_
