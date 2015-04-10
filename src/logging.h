/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Internal usage only */

#define LOG_MSG(dest, level, extra, after, ...) { \
        fprintf(dest, level ": " __VA_ARGS__);    \
        extra;                                    \
        fprintf(dest, "\n");                      \
        after;                                    \
    }
#define PRINT_ERRNO() fprintf(stderr, ": %s", strerror(errno))

/* Public logging macros */

#define FATAL(...)       LOG_MSG(stderr, "fatal",   {},            exit(EXIT_FAILURE), __VA_ARGS__)
#define FATAL_ERRNO(...) LOG_MSG(stderr, "fatal",   PRINT_ERRNO(), exit(EXIT_FAILURE), __VA_ARGS__)
#define ERROR(...)       LOG_MSG(stderr, "error",   {},            {},                 __VA_ARGS__)
#define ERROR_ERRNO(...) LOG_MSG(stderr, "error",   PRINT_ERRNO(), {},                 __VA_ARGS__)
#define WARN(...)        LOG_MSG(stderr, "warning", {},            {},                 __VA_ARGS__)
#define WARN_ERRNO(...)  LOG_MSG(stderr, "warning", PRINT_ERRNO(), {},                 __VA_ARGS__)

#ifdef DEBUG_MODE
#define DEBUG(...)       LOG_MSG(stdout, "[DEBUG]", {},            {},                 __VA_ARGS__)
#else
#define DEBUG(...)       {}
#endif

#define OUT_OF_MEMORY() FATAL("couldn't allocate enough memory")
