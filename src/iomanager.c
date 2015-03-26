/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <unistd.h>

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

#ifdef DEBUG_MODE
    z80_dump_registers(&gg->cpu);
#endif

    // TODO: use SDL events
    while (1) {
        gamegear_simulate(gg);
        usleep(1000 * 1000 / 60);
    }

    DEBUG("IOManager unpowering GameGear")
    gamegear_power(gg, false);
}
