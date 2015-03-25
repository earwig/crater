/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

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
void gamegear_destroy(GameGear* gg)
{
    free(gg);
}

/*
    Load a ROM image into the GameGear object.

    Does *not* steal the reference to the ROM object.
*/
void gamegear_load(GameGear* gg, ROM* rom)
{
    gg->rom = rom;
}

/*
    Powers a GameGear object, beginning emulation.

    This function call blocks until the GameGear is powered off.
*/
void gamegear_power(GameGear* gg)
{
    // TODO
}
