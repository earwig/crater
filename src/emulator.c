/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <signal.h>

#include "emulator.h"
#include "logging.h"

static GameGear *global_gg;

/*
    Signal handler for SIGINT.
*/
static void handle_sigint(int sig)
{
    (void) sig;
    if (global_gg)
        gamegear_power_off(global_gg);
}

/*
    GameGear callback: handle SDL logic at the end of a frame.
*/
static void draw_frame(GameGear *gg)
{
    (void) gg;
    // TODO: SDL draw / switch buffers here
}

/*
    Emulate a Game Gear. Handle I/O with the host computer.

    Block until emulation is finished.
*/
void emulate(GameGear *gg)
{
    global_gg = gg;
    signal(SIGINT, handle_sigint);
    gamegear_set_callback(gg, draw_frame);

    gamegear_simulate(gg);

    if (gamegear_get_exception(gg))
        ERROR("caught exception: %s", gamegear_get_exception(gg))
    else
        WARN("caught signal, stopping...")
    if (DEBUG_LEVEL)
        gamegear_print_state(gg);

    gamegear_clear_callback(gg);
    signal(SIGINT, SIG_DFL);
    global_gg = NULL;
}
