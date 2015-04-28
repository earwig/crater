/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <string.h>

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
    Convert a decimal integer to BCD-encoded form.
*/
uint8_t bcd_encode(uint8_t num)
{
    return ((num / 10) << 4) | (num % 10);
}

/*
    Convert a BCD-encoded integer to decimal.
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

/*
    Return the name of the region encoded by the given region code.

    The given code should not be larger than one nibble. NULL is returned if
    the region code is invalid.

    Region code information is taken from:
    http://www.smspower.org/Development/ROMHeader
*/
const char* region_code_to_string(uint8_t code)
{
    switch (code) {
        case 3:  return "SMS Japan";
        case 4:  return "SMS Export";
        case 5:  return "GG Japan";
        case 6:  return "GG Export";
        case 7:  return "GG International";
        default: return NULL;
    }
}

/*
    Return the region code that encodes the given region name.

    0 is returned if the name is not known. This is not a valid region code.
*/
uint8_t region_string_to_code(const char *name)
{
    if (!strncmp(name, "SMS ", 4)) {
        name += 4;
        if (!strcmp(name, "Japan"))
            return 3;
        if (!strcmp(name, "Export"))
            return 4;
    } else if (!strncmp(name, "GG ", 3)) {
        name += 3;
        if (!strcmp(name, "Japan"))
            return 5;
        if (!strcmp(name, "Export"))
            return 6;
        if (!strcmp(name, "International"))
            return 7;
    }
    return 0;
}

/*
    Return the number of bytes in a ROM image based on its header size code.

    0 is returned if the code is invalid.
*/
size_t size_code_to_bytes(uint8_t code)
{
    #define KB << 10
    #define MB << 20

    switch (code) {
        case 0xA: return   8 KB;
        case 0xB: return  16 KB;
        case 0xC: return  32 KB;
        case 0xD: return  48 KB;
        case 0xE: return  64 KB;
        case 0xF: return 128 KB;
        case 0x0: return 256 KB;
        case 0x1: return 512 KB;
        case 0x2: return   1 MB;
        default:  return   0;
    }

    #undef KB
    #undef MB
}

/*
    Given the number of bytes in a ROM image, return the size code.

    INVALID_SIZE_CODE is returned if the size is invalid.
*/
uint8_t size_bytes_to_code(size_t bytes)
{
    if (bytes & ((1 << 10) - 1))
        return INVALID_SIZE_CODE;  // Not an even number of KB

    switch (bytes >> 10) {
        case    8: return 0xA;
        case   16: return 0xB;
        case   32: return 0xC;
        case   48: return 0xD;
        case   64: return 0xE;
        case  128: return 0xF;
        case  256: return 0x0;
        case  512: return 0x1;
        case 1024: return 0x2;
        default:   return INVALID_SIZE_CODE;
    }
}

/*
    Compute a ROM data checksum.

    If the summable region (as specified by the range parameter) is too large
    (as specified by the size parameter), we'll compute the checksum over the
    default range (0x0000-0x7FF0), or the largest possible range. If size is
    zero, we will compute it over the given range regardless.
*/
uint16_t compute_checksum(const uint8_t *data, size_t size, uint8_t range)
{
    size_t low_region, high_region;
    switch (range & 0xF) {
        case 0xA: low_region = 0x1FEF; high_region = 0;       break;
        case 0xB: low_region = 0x3FEF; high_region = 0;       break;
        case 0xC: low_region = 0x7FEF; high_region = 0;       break;
        case 0xD: low_region = 0xBFEF; high_region = 0;       break;
        case 0xE: low_region = 0x7FEF; high_region = 0x0FFFF; break;
        case 0xF: low_region = 0x7FEF; high_region = 0x1FFFF; break;
        case 0x0: low_region = 0x7FEF; high_region = 0x3FFFF; break;
        case 0x1: low_region = 0x7FEF; high_region = 0x7FFFF; break;
        case 0x2: low_region = 0x7FEF; high_region = 0xFFFFF; break;
        default:  low_region = 0x7FEF; high_region = 0;       break;
    }

    if (size && low_region >= size)
        low_region = (size >= 0x4000) ? 0x3FEF : 0x1FEF;
    if (size && high_region >= size)
        high_region = 0;

    uint16_t sum = 0;
    for (size_t index = 0; index <= low_region; index++)
        sum += data[index];
    if (high_region) {
        for (size_t index = 0x08000; index <= high_region; index++)
            sum += data[index];
    }
    return sum;
}
