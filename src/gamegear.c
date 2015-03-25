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

    gg->rom = NULL;
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

    Does *not* steal the reference to the ROM object.
*/
void gamegear_load(GameGear *gg, ROM *rom)
{
    gg->rom = rom;
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
    if (gg->state == state)
        return;

    // TODO
    gg->state = state;
}
