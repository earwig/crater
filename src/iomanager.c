/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include "iomanager.h"
#include "logging.h"

/*
    Emulate a Game Gear. Handle I/O with the host computer.

    Block until emulation is finished.
*/
void iomanager_emulate(GameGear *gg)
{
    DEBUG("IOManager powering GameGear")
    gamegear_power(gg, true);

    // TODO

    DEBUG("IOManager unpowering GameGear")
    gamegear_power(gg, false);
}
