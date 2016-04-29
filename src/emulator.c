/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <signal.h>
#include <SDL.h>

#include "emulator.h"
#include "logging.h"

#define SCREEN_WIDTH  3 * (160 + 96)  // TODO: define elsewhere; use scale
#define SCREEN_HEIGHT 3 * (144 + 48)

static GameGear *global_gg;

static SDL_Window *window;
SDL_Renderer* renderer;

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
    TODO: ...
*/
static void setup_graphics()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        FATAL("SDL failed to initialize: %s", SDL_GetError());

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_BORDERLESS/* |SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE ? */,
        &window, &renderer);

    if (!window)
        FATAL("SDL failed to create a window: %s", SDL_GetError());
    if (!renderer)
        FATAL("SDL failed to create a renderer: %s", SDL_GetError());

    SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

/*
    GameGear callback: handle SDL logic at the end of a frame.
*/
static void draw_frame(GameGear *gg)
{
    SDL_RenderPresent(renderer);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            gamegear_power_off(gg);
            return;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0x33, 0x33, 0x33, 0xFF);
    SDL_RenderClear(renderer);
}

/*
    TODO: ...
*/
static void cleanup_graphics()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    renderer = NULL;
    SDL_Quit();
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
    setup_graphics();

    gamegear_simulate(gg);

    if (gamegear_get_exception(gg))
        ERROR("caught exception: %s", gamegear_get_exception(gg))
    else
        WARN("caught signal, stopping...")
    if (DEBUG_LEVEL)
        gamegear_print_state(gg);

    cleanup_graphics();
    gamegear_clear_callback(gg);
    signal(SIGINT, SIG_DFL);
    global_gg = NULL;
}
