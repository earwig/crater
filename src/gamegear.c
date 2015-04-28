/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "gamegear.h"
#include "logging.h"
#include "util.h"

/* Clock speed in Hz was taken from the official Sega GG documentation */
#define CPU_CLOCK_SPEED 3579545

/*
    Create and return a pointer to a new GameGear object.

    If memory could not be allocated, OUT_OF_MEMORY() is triggered.
*/
GameGear* gamegear_create()
{
    GameGear *gg = malloc(sizeof(GameGear));
    if (!gg)
        OUT_OF_MEMORY()

    if (!mmu_init(&gg->mmu))
        OUT_OF_MEMORY()
    z80_init(&gg->cpu, &gg->mmu);
    gg->powered = false;
    gg->exc_buffer[0] = '\0';
    return gg;
}

/*
    Destroy a previously-allocated GameGear object.

    Does *not* destroy any loaded ROM objects.
*/
void gamegear_destroy(GameGear *gg)
{
    mmu_free(&gg->mmu);
    free(gg);
}

/*
    Load a ROM image into the GameGear object.

    Does *not* steal a reference to the ROM object, so it must be kept alive
    until another ROM is loaded or the GameGear is destroyed. Calling this
    function while the GameGear is powered on has no effect.
*/
void gamegear_load(GameGear *gg, const ROM *rom)
{
    if (gg->powered)
        return;

    mmu_load_rom(&gg->mmu, rom->data, rom->size);
}

/*
    Set the GameGear object's power state (true = on; false = off).

    Powering on the GameGear executes boot code (e.g. clearing memory and
    setting initial register values) and starts the clock. Powering it off
    stops the clock and clears any exception data.

    Setting the power state to its current value has no effect.
*/
void gamegear_power(GameGear *gg, bool state)
{
    if (gg->powered == state)
        return;

    if (state) {
        mmu_power(&gg->mmu);
        z80_power(&gg->cpu);
        gg->last_tick = get_time_ns();
    } else {
        gg->exc_buffer[0] = '\0';
    }
    gg->powered = state;
}

/*
    Update the simulation of the GameGear.

    This function simulates the number of clock cycles corresponding to the
    time since the last call to gamegear_simulate() or gamegear_power() if the
    system was just powered on. If the system is powered off, this function
    does nothing.

    The return value indicates whether an exception flag has been set
    somewhere. If true, emulation must be stopped. gamegear_get_exception() can
    be used to fetch exception information. Power-cycling the GameGear with
    gamegear_power(gg, false) followed by gamegear_power(gg, true) will reset
    the exception flag and allow emulation to restart normally.
*/
bool gamegear_simulate(GameGear *gg)
{
    if (!gg->powered)
        return false;

    uint64_t last = gg->last_tick, tick;
    tick = gg->last_tick = get_time_ns();
    return z80_do_cycles(&gg->cpu, (tick - last) * CPU_CLOCK_SPEED / 1e9);
}

/*
    If an exception flag has been set in the GameGear, return the reason.

    This function returns a const pointer to a buffer holding a human-readable
    exception string (although it may be cryptic to an end-user). The buffer is
    owned by the GameGear object and should not be freed - its contents last
    until the GameGear's power state is changed. If no exception flag is set,
    this function returns NULL.
*/
const char* gamegear_get_exception(GameGear *gg)
{
#define SET_EXC(...) snprintf(gg->exc_buffer, GG_EXC_BUFF_SIZE, __VA_ARGS__);
    if (!gg->powered)
        return NULL;

    if (!gg->exc_buffer[0]) {
        if (gg->cpu.except) {
            switch (gg->cpu.exc_code) {
                case Z80_EXC_NOT_POWERED:
                    SET_EXC("CPU not powered")
                    break;
                case Z80_EXC_UNIMPLEMENTED_OPCODE:
                    SET_EXC("unimplemented opcode: 0x%02X", gg->cpu.exc_data)
                    break;
                default:
                    SET_EXC("unknown exception")
                    break;
            }
        } else {
            return NULL;
        }
    }
    return gg->exc_buffer;
#undef SET_EXC
}
