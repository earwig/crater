/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Internal usage only */

#define LOG_MSG(level, extra, after, ...) {      \
        fprintf(stderr, level ": " __VA_ARGS__); \
        extra;                                   \
        fprintf(stderr, "\n");                   \
        after;                                   \
    }
#define PRINT_ERRNO() fprintf(stderr, ": %s", strerror(errno))

/* Public logging macros */

#define FATAL(...)       LOG_MSG("Error",   {},            exit(EXIT_FAILURE), __VA_ARGS__)
#define FATAL_ERRNO(...) LOG_MSG("Error",   PRINT_ERRNO(), exit(EXIT_FAILURE), __VA_ARGS__)
#define ERROR(...)       LOG_MSG("Error",   {},            {},                 __VA_ARGS__)
#define ERROR_ERRNO(...) LOG_MSG("Error",   PRINT_ERRNO(), {},                 __VA_ARGS__)
#define WARN(...)        LOG_MSG("Warning", {},            {},                 __VA_ARGS__)
#define WARN_ERRNO(...)  LOG_MSG("Warning", PRINT_ERRNO(), {},                 __VA_ARGS__)

#ifdef DEBUG_MODE
#define DEBUG(...)       LOG_MSG("[DEBUG]", {},            {},                 __VA_ARGS__)
#define DEBUG_ERRNO(...) LOG_MSG("[DEBUG]", PRINT_ERRNO(), {},                 __VA_ARGS__)
#else
#define DEBUG(...)       {}
#define DEBUG_ERRNO(...) {}
#endif

#define OUT_OF_MEMORY() FATAL("couldn't allocate enough memory")
