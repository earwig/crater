/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <string.h>

#include "mmu.h"
#include "logging.h"
#include "util.h"
#include "z80.h"

/*
    Initialize a MMU object. This must be called before using the MMU.
*/
void mmu_init(MMU *mmu)
{
    mmu->system_ram = cr_malloc(sizeof(uint8_t) * MMU_SYSTEM_RAM_SIZE);
    mmu->cart_ram = NULL;
    mmu->cart_ram_slot = NULL;
    mmu->cart_ram_mapped = false;
    mmu->cart_ram_external = false;
    mmu->save = NULL;

    for (size_t slot = 0; slot < MMU_NUM_SLOTS; slot++)
        mmu->rom_slots[slot] = NULL;

    for (size_t bank = 0; bank < MMU_NUM_ROM_BANKS; bank++)
        mmu->rom_banks[bank] = NULL;
}

/*
    Free memory previously allocated by the MMU.
*/
void mmu_free(MMU *mmu)
{
    free(mmu->system_ram);
    if (!mmu->cart_ram_external)
        free(mmu->cart_ram);
}

/*
    @DEBUG_LEVEL
    Print out the bank mapping.
*/
static void dump_bank_table(const MMU *mmu, const uint8_t *data)
{
    char buffer[49];
    size_t group, elem, bank;

    DEBUG("Dumping MMU bank table:")
    for (group = 0; group < MMU_NUM_ROM_BANKS / 8; group++) {
        for (elem = 0; elem < 8; elem++) {
            bank = 8 * group + elem;
            snprintf(buffer + 6 * elem, 7, "%02zX=%02zX ", bank,
                     (mmu->rom_banks[bank] - data) >> 14);
        }
        buffer[47] = '\0';
        DEBUG("- %s", buffer)
    }
}

/*
    Load a block of ROM into the MMU.

    size must be a multiple of MMU_ROM_BANK_SIZE (16 KB), or the load will fail
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

    size_t banks = size / MMU_ROM_BANK_SIZE, bank, mirror;
    if (banks > MMU_NUM_ROM_BANKS)
        banks = MMU_NUM_ROM_BANKS;

    for (bank = 0; bank < banks; bank++) {
        for (mirror = bank; mirror < MMU_NUM_ROM_BANKS; mirror += banks)
            mmu->rom_banks[mirror] = data + (bank * MMU_ROM_BANK_SIZE);
    }

    if (DEBUG_LEVEL)
        dump_bank_table(mmu, data);
}

/*
    Load a save into the MMU.

    If the save has valid cartridge RAM from a previous game, we will load that
    into the MMU. Otherwise, we will defer creating fresh cartridge RAM until
    it is requested by the system.

    This function can be called while the system is running, but it may have
    strange consequences. It will replace any existing cart RAM with the save
    RAM, which will likely confuse the program.
*/
void mmu_load_save(MMU *mmu, Save *save)
{
    mmu->save = save;
    if (save_has_cart_ram(save)) {
        if (mmu->cart_ram && !mmu->cart_ram_external)
            free(mmu->cart_ram);

        DEBUG("MMU loading cartridge RAM from external save")
        mmu->cart_ram = save_get_cart_ram(save);
        mmu->cart_ram_external = true;
    }
}

/*
    Map the given RAM slot to the given ROM bank.
*/
static inline void map_rom_slot(MMU *mmu, size_t slot, size_t bank)
{
    TRACE("MMU mapping memory slot %zu to ROM bank 0x%02zX", slot, bank)
    mmu->rom_slots[slot] = mmu->rom_banks[bank];
}

/*
    Power on the MMU, setting initial memory values.

    This must be called before memory is read from or written to. If no ROM has
    been loaded, those regions will be read as 0xFF and will not accept writes.
*/
void mmu_power(MMU *mmu)
{
    for (size_t slot = 0; slot < MMU_NUM_SLOTS; slot++)
        map_rom_slot(mmu, slot, slot);

    memset(mmu->system_ram, 0xFF, MMU_SYSTEM_RAM_SIZE);
}

/*
    Read a byte from a memory bank, or return 0xFF if the bank is not mapped.
*/
static inline uint8_t bank_byte_read(const uint8_t* bank, uint16_t addr)
{
    return bank ? bank[addr] : 0xFF;
}

/*
    Read a byte of memory from the given address.

    Memory region information is based on:
    - http://www.smspower.org/Development/MemoryMap
    - http://www.smspower.org/Development/Mappers
*/
uint8_t mmu_read_byte(const MMU *mmu, uint16_t addr)
{
    if (addr < 0x0400) {  // First kilobyte is unpaged, for interrupt handlers
        return bank_byte_read(mmu->rom_banks[0], addr);
    } else if (addr < 0x4000) {  // Slot 0 (0x0400 - 0x3FFF)
        return bank_byte_read(mmu->rom_slots[0], addr);
    } else if (addr < 0x8000) {  // Slot 1 (0x4000 - 0x7FFF)
        return bank_byte_read(mmu->rom_slots[1], addr - 0x4000);
    } else if (addr < 0xC000) {  // Slot 2 (0x8000 - 0xBFFF)
        if (mmu->cart_ram_mapped)
            return mmu->cart_ram_slot[addr - 0x8000];
        return bank_byte_read(mmu->rom_slots[2], addr - 0x8000);
    } else if (addr < 0xE000) {  // System RAM (0xC000 - 0xDFFF)
        return mmu->system_ram[addr - 0xC000];
    } else {  // System RAM, mirrored (0xE000 - 0xFFFF)
        return mmu->system_ram[addr - 0xE000];
    }
}

/*
    Read two bytes of memory from the given address.
*/
uint16_t mmu_read_double(const MMU *mmu, uint16_t addr)
{
    return mmu_read_byte(mmu, addr) + (mmu_read_byte(mmu, addr + 1) << 8);
}

/*
    Read four bytes of memory from the given address.
*/
uint32_t mmu_read_quad(const MMU *mmu, uint16_t addr)
{
    return (
         mmu_read_byte(mmu, addr) +
        (mmu_read_byte(mmu, addr + 1) <<  8) +
        (mmu_read_byte(mmu, addr + 2) << 16) +
        (mmu_read_byte(mmu, addr + 3) << 24));
}

/*
    Write to the cartridge RAM mapping control register at 0xFFFC.
*/
static void write_ram_control_register(MMU *mmu, uint8_t value)
{
    // TODO: 0x03 = bank shift, 0x10 = 0xC000-0xFFFF enable, 0x80 = ROM write
    bool bank_select  = value & 0x04;
    bool slot2_enable = value & 0x08;

    if (slot2_enable)
        TRACE("MMU enabling cart RAM bank %d in memory slot 2", bank_select)
    else if (!slot2_enable && mmu->cart_ram_mapped)
        TRACE("MMU disabling cart RAM in memory slot 2")

    if (slot2_enable && !mmu->cart_ram) {
        DEBUG("MMU initializing cartridge RAM (fresh battery save)")
        if (mmu->save && save_init_cart_ram(mmu->save)) {
            mmu->cart_ram = save_get_cart_ram(mmu->save);
            mmu->cart_ram_external = true;
        } else {
            mmu->cart_ram = cr_malloc(sizeof(uint8_t) * MMU_CART_RAM_SIZE);
            mmu->cart_ram_external = false;
        }
        memset(mmu->cart_ram, 0xFF, MMU_CART_RAM_SIZE);
    }

    mmu->cart_ram_slot =
        bank_select ? (mmu->cart_ram + 0x4000) : mmu->cart_ram;
    mmu->cart_ram_mapped = slot2_enable;
}

/*
    Write a byte of memory to the given address.

    Return true if the byte was written, and false if it wasn't. Writes will
    fail when attempting to write to read-only memory.
*/
bool mmu_write_byte(MMU *mmu, uint16_t addr, uint8_t value)
{
    if (addr < 0xC000) {
        if (addr >= 0x8000 && mmu->cart_ram_mapped) {
            mmu->cart_ram_slot[addr - 0x8000] = value;
            return true;
        }
        return false;
    } else if (addr < 0xE000) {  // System RAM (0xC000 - 0xDFFF)
        mmu->system_ram[addr - 0xC000] = value;
        return true;
    } else {  // System RAM, mirrored (0xE000 - 0xFFFF)
        if (addr == 0xFFFC)
            write_ram_control_register(mmu, value);
        else if (addr == 0xFFFD)
            map_rom_slot(mmu, 0, value & 0x3F);
        else if (addr == 0xFFFE)
            map_rom_slot(mmu, 1, value & 0x3F);
        else if (addr == 0xFFFF)
            map_rom_slot(mmu, 2, value & 0x3F);
        mmu->system_ram[addr - 0xE000] = value;
        return true;
    }
}

/*
    Write two bytes of memory to the given address.
*/
bool mmu_write_double(MMU *mmu, uint16_t addr, uint16_t value)
{
    bool b1 = mmu_write_byte(mmu, addr, value & 0xFF);
    bool b2 = mmu_write_byte(mmu, addr + 1, value >> 8);
    return b1 && b2;
}
