/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include "emulator.h"
#include "logging.h"
#include "util.h"

#define NS_PER_FRAME (1000 * 1000 * 1000 / 60)

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

    DEBUG("Emulator powering GameGear")
    gamegear_power(gg, true);

    while (!caught_signal) {
        uint64_t start = get_time_ns(), delta;
        if (gamegear_simulate_frame(gg)) {
            ERROR("caught exception: %s", gamegear_get_exception(gg))
            if (DEBUG_LEVEL)
                z80_dump_registers(&gg->cpu);
            break;
        }
        // TODO: SDL draw / switch buffers here
        delta = get_time_ns() - start;
        if (delta < NS_PER_FRAME)
            usleep((NS_PER_FRAME - delta) / 1000);
    }

    if (caught_signal) {
        WARN("caught signal, stopping...")
        if (DEBUG_LEVEL)
            z80_dump_registers(&gg->cpu);
    }

    DEBUG("Emulator unpowering GameGear")
    gamegear_power(gg, false);

    signal(SIGINT, SIG_DFL);
}
