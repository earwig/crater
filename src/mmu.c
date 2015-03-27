/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "z80.h"

/*
    Initialize a MMU object. This must be called before using the MMU.

    Return true if initialization was successful, or false if the required
    amount of memory could not be allocated.
*/
bool mmu_init(MMU *mmu)
{
    mmu->system_ram = malloc(sizeof(uint8_t) * MMU_SYSTEM_RAM_SIZE);
    if (!mmu->system_ram)
        return false;

    for (size_t slot = 0; slot < MMU_NUM_SLOTS; slot++)
        mmu->map_slots[slot] = NULL;

    for (size_t bank = 0; bank < MMU_NUM_ROM_BANKS; bank++)
        mmu->rom_banks[bank] = NULL;

    return true;
}

/*
    Free memory previously allocated by the MMU.
*/
void mmu_free(MMU *mmu)
{
    free(mmu->system_ram);
}

/*
    Load a block of ROM into the MMU.

    size must be a multiple of MMU_ROM_BANK_SIZE (16 KB), the load will fail
    silently. It should also be a power of two, or problems might occur with
    ROM mirroring logic. It should not be larger than
    MMU_ROM_BANK_SIZE * MMU_NUM_ROM_BANKS, or the extra banks will be ignored.

    This function will still work if called while the system is running, but it
    will likely cause unexpected behavior.
*/
void mmu_load_rom(MMU *mmu, const uint8_t *data, size_t size)
{
    if (size % MMU_ROM_BANK_SIZE)
        return;

    size_t banks = size / MMU_ROM_BANK_SIZE;
    if (banks > MMU_NUM_ROM_BANKS)
        banks = MMU_NUM_ROM_BANKS;

    for (size_t bank = 0; bank < banks; bank++) {
        for (size_t mirror = bank; mirror < banks; mirror += bank + 1) {
            mmu->rom_banks[mirror] = data + (bank * MMU_ROM_BANK_SIZE);
        }
    }
}

/*
    Map the given RAM slot to the given ROM bank.
*/
static inline void map_slot(MMU *mmu, size_t slot, size_t bank)
{
    DEBUG("MMU mapping memory slot %zu to bank %zu", slot, bank)
    mmu->map_slots[slot] = mmu->rom_banks[bank];
}

/*
    Power on the MMU, setting initial memory values.

    This must be called before memory is read from or written to. If no ROM has
    been loaded, those regions will be read as 0xFF and will not accept writes.
*/
void mmu_power(MMU *mmu)
{
    for (size_t slot = 0; slot < MMU_NUM_SLOTS; slot++)
        map_slot(mmu, slot, slot);

    memset(mmu->system_ram, 0xFF, MMU_SYSTEM_RAM_SIZE);
    // TODO: set system_ram to correct values at 0xFFFC-0xFFFF
}

/*
    Read a byte of memory from the given address.
*/
uint8_t mmu_read_byte(MMU *mmu, uint16_t addr)
{
    // TODO
    return 0x00;
}

/*
    Write a byte of memory to the given address.

    Return true if the byte was written, and false if it wasn't. Writes will
    fail when attempting to write to read-only memory.
*/
bool mmu_write_byte(MMU *mmu, uint16_t addr, uint8_t value)
{
    // TODO
    return true;
}
