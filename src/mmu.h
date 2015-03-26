/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Structs */

typedef struct {
    //
} MMU;

/* Functions */

bool mmu_init(MMU*);
void mmu_free(MMU*);
void mmu_load_rom(MMU*, const uint8_t*, size_t);
void mmu_power(MMU*);
uint8_t mmu_read_byte(MMU*, uint16_t);
void mmu_write_byte(MMU*, uint16_t, uint8_t);
