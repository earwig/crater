/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "util.h"

#if defined __APPLE__
    #include <mach/mach_time.h>
#elif defined __linux__
    #include <time.h>
    #ifndef CLOCK_MONOTONIC
        #error "Unsupported operating system or compiler."
    #endif
#else
    #error "Unsupported operating system or compiler."
#endif

#define NS_PER_SEC 1000000000

/*
    Convert a BCD-encoded hexadecimal number to decimal.
*/
uint8_t bcd_decode(uint8_t num)
{
    return ((num >> 4) * 10) + (num & 0x0F);
}

/*
    Return monotonic time, in nanoseconds.
*/
uint64_t get_time_ns()
{
#if defined __APPLE__
    static mach_timebase_info_data_t tb_info;
    if (tb_info.denom == 0)
        mach_timebase_info(&tb_info);
    // Apple's docs pretty much say "screw overflow" here...
    return mach_absolute_time() * tb_info.numer / tb_info.denom;
#elif defined __linux__
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec.tv_sec * NS_PER_SEC + spec.tv_nsec;
#endif
}
