/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Internal usage only */

#define LOG_MSG_(dest, level, extra, after, ...) { \
        fprintf(dest, level ": " __VA_ARGS__);    \
        extra;                                    \
        fprintf(dest, "\n");                      \
        after;                                    \
    }

#define LOG_ERR_(...) LOG_MSG_(stderr, __VA_ARGS__)
#define LOG_OUT_(...) LOG_MSG_(stdout, __VA_ARGS__)

#define PRINT_ERRNO_() fprintf(stderr, ": %s", strerror(errno))

/* Public logging macros */

#define FATAL(...)       LOG_ERR_("fatal",   {},             exit(EXIT_FAILURE), __VA_ARGS__)
#define FATAL_ERRNO(...) LOG_ERR_("fatal",   PRINT_ERRNO_(), exit(EXIT_FAILURE), __VA_ARGS__)
#define ERROR(...)       LOG_ERR_("error",   {},             {},                 __VA_ARGS__)
#define ERROR_ERRNO(...) LOG_ERR_("error",   PRINT_ERRNO_(), {},                 __VA_ARGS__)
#define WARN(...)        LOG_ERR_("warning", {},             {},                 __VA_ARGS__)
#define WARN_ERRNO(...)  LOG_ERR_("warning", PRINT_ERRNO_(), {},                 __VA_ARGS__)

#ifdef DEBUG_MODE
#define DEBUG(...)       LOG_OUT_("[DEBUG]", {},             {},                 __VA_ARGS__)
#else
#define DEBUG(...)       {}
#endif
