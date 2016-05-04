/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <signal.h>
#include <stdint.h>
#include <SDL.h>

#include "emulator.h"
#include "gamegear.h"
#include "logging.h"
#include "util.h"

typedef struct {
    GameGear *gg;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *pixels;
} Emulator;

static Emulator emu;

/*
    Signal handler for SIGINT. Tells the GameGear to power off, if it exists.
*/
static void handle_sigint(int sig)
{
    (void) sig;
    if (emu.gg)
        gamegear_power_off(emu.gg);  // Safe!
}

/*
    Set up SDL for drawing the game.
*/
static void setup_graphics(bool fullscreen, unsigned scale)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        FATAL("SDL failed to initialize: %s", SDL_GetError());

    uint32_t flags;
    if (fullscreen)
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        flags = SDL_WINDOW_RESIZABLE;

    SDL_CreateWindowAndRenderer(
        scale * GG_SCREEN_WIDTH, scale * GG_SCREEN_HEIGHT,
        flags, &emu.window, &emu.renderer);

    if (!emu.window)
        FATAL("SDL failed to create a window: %s", SDL_GetError());
    if (!emu.renderer)
        FATAL("SDL failed to create a renderer: %s", SDL_GetError());

    emu.texture = SDL_CreateTexture(emu.renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT);

    if (!emu.texture)
        FATAL("SDL failed to create a texture: %s", SDL_GetError());

    emu.pixels = cr_malloc(
        sizeof(uint32_t) * GG_SCREEN_WIDTH * GG_SCREEN_HEIGHT);

    SDL_RenderSetLogicalSize(emu.renderer, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetWindowTitle(emu.window, "crater");
    SDL_ShowCursor(SDL_DISABLE);
    SDL_GL_SetSwapInterval(1);  // Vsync

    SDL_SetRenderDrawColor(emu.renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(emu.renderer);
    SDL_RenderPresent(emu.renderer);
}

/*
    Actually send the pixel data to the screen.
*/
static void draw_frame()
{
    SDL_UpdateTexture(emu.texture, NULL, emu.pixels,
        GG_SCREEN_WIDTH * sizeof(uint32_t));
    SDL_SetRenderDrawColor(emu.renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(emu.renderer);
    SDL_RenderCopy(emu.renderer, emu.texture, NULL, NULL);
    SDL_RenderPresent(emu.renderer);
}

/*
    Handle a keyboard press; translate it into a Game Gear button press.
*/
static void handle_keypress(GameGear *gg, SDL_Keycode key, bool state)
{
    GGButton button;
    switch (key) {
        case SDLK_UP:
        case SDLK_w:
            button = BUTTON_UP;        break;
        case SDLK_DOWN:
        case SDLK_s:
            button = BUTTON_DOWN;      break;
        case SDLK_LEFT:
        case SDLK_a:
            button = BUTTON_LEFT;      break;
        case SDLK_RIGHT:
        case SDLK_d:
            button = BUTTON_RIGHT;     break;
        case SDLK_j:
        case SDLK_z:
        case SDLK_PERIOD:
            button = BUTTON_TRIGGER_1; break;
        case SDLK_k:
        case SDLK_x:
        case SDLK_SLASH:
            button = BUTTON_TRIGGER_2; break;
        case SDLK_RETURN:
        case SDLK_RETURN2:
        case SDLK_ESCAPE:
            button = BUTTON_START;     break;
        default:
            return;
    }
    gamegear_input(gg, button, state);
}

/*
    Handle SDL events, mainly quit events and button presses.
*/
static void handle_events(GameGear *gg)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                gamegear_power_off(gg);
                return;
            case SDL_KEYDOWN:
                handle_keypress(gg, event.key.keysym.sym, true);
                break;
            case SDL_KEYUP:
                handle_keypress(gg, event.key.keysym.sym, false);
                break;
        }
    }
}

/*
    GameGear callback: Draw the current frame and handle SDL event logic.
*/
static void frame_callback(GameGear *gg)
{
    draw_frame();
    handle_events(gg);
}

/*
    Clean up SDL stuff allocated in setup_graphics().
*/
static void cleanup_graphics()
{
    free(emu.pixels);
    SDL_DestroyTexture(emu.texture);
    SDL_DestroyRenderer(emu.renderer);
    SDL_DestroyWindow(emu.window);
    SDL_Quit();

    emu.window = NULL;
    emu.renderer = NULL;
    emu.texture = NULL;
}

/*
    Emulate a ROM in a Game Gear while handling I/O with the host computer.

    Block until emulation is finished.
*/
void emulate(ROM *rom, Config *config)
{
    emu.gg = gamegear_create();
    signal(SIGINT, handle_sigint);
    setup_graphics(config->fullscreen, config->scale);

    gamegear_attach_callback(emu.gg, frame_callback);
    gamegear_attach_display(emu.gg, emu.pixels);
    gamegear_load(emu.gg, rom);

    gamegear_simulate(emu.gg);

    if (gamegear_get_exception(emu.gg))
        ERROR("caught exception: %s", gamegear_get_exception(emu.gg))
    else
        WARN("caught signal, stopping...")
    if (DEBUG_LEVEL)
        gamegear_print_state(emu.gg);

    cleanup_graphics();
    signal(SIGINT, SIG_DFL);
    gamegear_destroy(emu.gg);
    emu.gg = NULL;
}
