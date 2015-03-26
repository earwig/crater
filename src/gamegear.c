/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "gamegear.h"
#include "logging.h"

/*
    Create and return a pointer to a new GameGear object.

    If memory could not be allocated, OUT_OF_MEMORY() is triggered.
*/
GameGear* gamegear_create()
{
    GameGear *gg = malloc(sizeof(GameGear));
    if (!gg)
        OUT_OF_MEMORY()

    // mmu_init(&gg->mmu, ...);
    z80_init(&gg->cpu, CPU_CLOCK_SPEED);
    gg->powered = false;
    return gg;
}

/*
    Destroy a previously-allocated GameGear object.

    Does *not* destroy any loaded ROM objects.
*/
void gamegear_destroy(GameGear *gg)
{
    free(gg);
}

/*
    Load a ROM image into the GameGear object.

    Does *not* steal a reference to the ROM object. Calling this function while
    the GameGear is powered on has no effect.
*/
void gamegear_load(GameGear *gg, ROM *rom)
{
    if (gg->powered)
        return;

    // mmu_hard_map(&gg->mmu, rom->data, ..., ...);
}

/*
    Set the GameGear object's power state (true = on; false = off).

    Powering on the GameGear executes boot code (e.g. clearing memory and
    setting initial register values) and starts the clock. Powering it off
    stops the clock.

    Setting the power state to its current value has no effect.
*/
void gamegear_power(GameGear *gg, bool state)
{
    if (gg->powered == state)
        return;

    if (state) {
        // mmu_power(&gg->mmu);
        z80_power(&gg->cpu);
    }
    gg->powered = state;
}

/*
    Update the simulation of the GameGear.

    This function simulates the number of clock cycles corresponding to the
    time since the last call to gamegear_simulate() or gamegear_power() if the
    system was just powered on. If the system is powered off, this function
    does nothing.
*/
void gamegear_simulate(GameGear *gg)
{
    if (!gg->powered)
        return;

    // TODO
    // z80_do_cycles(&gg->cpu, ...);
}
