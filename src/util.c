/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "util.h"

/*
    Convert a BCD-encoded hexadecimal number to decimal.
*/
uint8_t bcd_decode(uint8_t num)
{
    return ((num >> 4) * 10) + (num & 0x0F);
}
