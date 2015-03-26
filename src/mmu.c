/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "z80.h"

/*
    Initialize a MMU object.

    Return true if initialization was successful, or false if the required
    amount of memory could not be allocated.
*/
bool mmu_init(MMU *mmu)
{
    // TODO
    return true;
}

/*
    Free memory previously allocated by the MMU.
*/
void mmu_free(MMU *mmu)
{
    // TODO
}

/*
    Load a block ROM into the MMU.

    size should be a power of two.
*/
void mmu_load_rom(MMU *mmu, const uint8_t *data, size_t size)
{
    // TODO
}

/*
    Power on the MMU, setting initial memory values.
*/
void mmu_power(MMU *mmu)
{
    // TODO
}

/*
    Read a byte of memory.
*/
uint8_t mmu_read_byte(MMU *mmu, uint16_t addr)
{
    // TODO
    return 0x00;
}

/*
    ...
*/
void mmu_write_byte(MMU *mmu, uint16_t addr, uint8_t value)
{
    // TODO
}
