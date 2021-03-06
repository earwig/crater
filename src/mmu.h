/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "save.h"

#define MMU_NUM_SLOTS       (3)
#define MMU_NUM_ROM_BANKS   (64)
#define MMU_ROM_BANK_SIZE   (16 * 1024)
#define MMU_SYSTEM_RAM_SIZE ( 8 * 1024)
#define MMU_CART_RAM_SIZE   (32 * 1024)

/* Structs */

typedef struct {
    uint8_t *system_ram;
    uint8_t *cart_ram;
    const uint8_t *rom_slots[MMU_NUM_SLOTS];
    const uint8_t *rom_banks[MMU_NUM_ROM_BANKS];
    uint8_t *cart_ram_slot;
    const uint8_t *bios_rom;
    bool cart_ram_mapped, cart_ram_external;
    bool bios_enabled;
    Save *save;
} MMU;

/* Functions */

void mmu_init(MMU*);
void mmu_free(MMU*);
void mmu_load_rom(MMU*, const uint8_t*, size_t);
void mmu_load_bios(MMU*, const uint8_t*);
void mmu_load_save(MMU*, Save*);
void mmu_power(MMU*);

uint8_t mmu_read_byte(const MMU*, uint16_t);
uint16_t mmu_read_double(const MMU*, uint16_t);
uint32_t mmu_read_quad(const MMU*, uint16_t);
bool mmu_write_byte(MMU*, uint16_t, uint8_t);
bool mmu_write_double(MMU*, uint16_t, uint16_t);
