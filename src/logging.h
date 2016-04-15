/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Internal usage only */

#define LOG_MSG_(dest, level, type, extra, after, ...) \
    do {                                               \
        if (logging_level_ >= level) {                 \
            fprintf(dest, type " " __VA_ARGS__);       \
            extra;                                     \
            fprintf(dest, "\n");                       \
            after;                                     \
        }                                              \
    } while (0);

#define LOG_ERR_(...) LOG_MSG_(stderr, __VA_ARGS__)
#define LOG_OUT_(...) LOG_MSG_(stdout, __VA_ARGS__)

#define PRINT_ERR_ fprintf(stderr, ": %s", strerror(errno))
#define EXIT_FAIL_ exit(EXIT_FAILURE)

#define DEBUG_TEXT_ "\x1b[0m\x1b[37m[DEBUG]\x1b[0m"
#define TRACE_TEXT_ "\x1b[1m\x1b[33m[TRACE]\x1b[0m"

unsigned logging_level_;

/* Public logging macros */

#define FATAL(...)       LOG_ERR_(0, "fatal:",    {},         EXIT_FAIL_, __VA_ARGS__)
#define FATAL_ERRNO(...) LOG_ERR_(0, "fatal:",    PRINT_ERR_, EXIT_FAIL_, __VA_ARGS__)
#define ERROR(...)       LOG_ERR_(0, "error:",    {},         {},         __VA_ARGS__)
#define ERROR_ERRNO(...) LOG_ERR_(0, "error:",    PRINT_ERR_, {},         __VA_ARGS__)
#define WARN(...)        LOG_ERR_(0, "warning:",  {},         {},         __VA_ARGS__)
#define WARN_ERRNO(...)  LOG_ERR_(0, "warning:",  PRINT_ERR_, {},         __VA_ARGS__)
#define DEBUG(...)       LOG_OUT_(1, DEBUG_TEXT_, {},         {},         __VA_ARGS__)
#define TRACE(...)       LOG_OUT_(2, TRACE_TEXT_, {},         {},         __VA_ARGS__)

#define SET_LOG_LEVEL(level) logging_level_ = (level);
#define DEBUG_LEVEL (logging_level_ >= 1)
#define TRACE_LEVEL (logging_level_ >= 2)
