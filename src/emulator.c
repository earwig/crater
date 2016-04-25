/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include "emulator.h"
#include "logging.h"

static volatile bool caught_signal;

/*
    Signal handler for SIGINT.
*/
static void handle_sigint(int sig)
{
    (void) sig;  // We don't care
    caught_signal = true;
}

/*
    Emulate a Game Gear. Handle I/O with the host computer.

    Block until emulation is finished.
*/
void emulate(GameGear *gg)
{
    caught_signal = false;
    signal(SIGINT, handle_sigint);

    DEBUG("Interface powering GameGear")
    gamegear_power(gg, true);

    // TODO: use SDL events
    while (!caught_signal) {
        if (gamegear_simulate(gg)) {
            ERROR("caught exception: %s", gamegear_get_exception(gg))
            if (DEBUG_LEVEL)
                z80_dump_registers(&gg->cpu);
            break;
        }
        usleep(1000 * 1000 / 60);
    }

    if (caught_signal) {
        WARN("caught signal, stopping...")
        if (DEBUG_LEVEL)
            z80_dump_registers(&gg->cpu);
    }

    DEBUG("Interface unpowering GameGear")
    gamegear_power(gg, false);

    signal(SIGINT, SIG_DFL);
}
