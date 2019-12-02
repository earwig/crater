/* Copyright (C) 2014-2019 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <SDL.h>

#include "emulator.h"
#include "config.h"
#include "gamegear.h"
#include "logging.h"
#include "save.h"
#include "util.h"

typedef struct {
    SDL_GameController **items;
    int num, capacity;
} Controllers;

typedef struct {
    GameGear *gg;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *pixels;
    Controllers controllers;
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
    Get the name of a SDL game controller.
*/
static const char *get_controller_name(SDL_GameController *controller)
{
    const char *name = SDL_GameControllerName(controller);
    if (name)
        return name;
    return "(UNKNOWN)";
}

/*
    Set up SDL for accepting controller input.
*/
static void setup_input()
{
    if (!access(CONTROLLER_DB_PATH, R_OK)) {
        if (SDL_GameControllerAddMappingsFromFile(CONTROLLER_DB_PATH) < 0)
            ERROR("SDL failed to load controller mappings: %s", SDL_GetError());
    }

    int i, c = 0, n = SDL_NumJoysticks();
    if (n <= 0)
        return;

    SDL_GameController **items = cr_calloc(n, sizeof(SDL_GameController*));
    for (i = 0; i < n; i++) {
        if (!SDL_IsGameController(i)) {
            DEBUG("Ignoring joystick %i: not a game controller", i);
            continue;
        }

        SDL_GameController *controller = SDL_GameControllerOpen(i);
        if (!controller) {
            ERROR("SDL failed to open controller %i: %s", i, SDL_GetError());
            continue;
        }
        DEBUG("Loaded controller %i: %s", i, get_controller_name(controller));
        items[c++] = controller;
    }

    emu.controllers.items = items;
    emu.controllers.num = c;
    emu.controllers.capacity = n;
}

/*
    Set up SDL for drawing the game.
*/
static void setup_graphics(Config *config)
{
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

    uint32_t flags;
    if (config->fullscreen)
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        flags = SDL_WINDOW_RESIZABLE;

    int width  = config->scale * GG_SCREEN_WIDTH;
    int height = config->scale * GG_SCREEN_HEIGHT;
    if (!config->square_par)
        height = height * GG_PIXEL_HEIGHT / GG_PIXEL_WIDTH;

    emu.window = SDL_CreateWindow("crater", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, width, height, flags);
    if (!emu.window)
        FATAL("SDL failed to create a window: %s", SDL_GetError());

    emu.renderer = SDL_CreateRenderer(emu.window, -1,
        SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!emu.renderer) {
        // Fall back to software renderer (or a hardware renderer
        // without vsync, if one exists?)
        emu.renderer = SDL_CreateRenderer(emu.window, -1, 0);
        if (!emu.renderer)
            FATAL("SDL failed to create a renderer: %s", SDL_GetError());
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(emu.renderer, &info);
    DEBUG("Using %s renderer", info.name);

    emu.texture = SDL_CreateTexture(emu.renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT);

    if (!emu.texture)
        FATAL("SDL failed to create a texture: %s", SDL_GetError());

    emu.pixels = cr_malloc(
        sizeof(uint32_t) * GG_SCREEN_WIDTH * GG_SCREEN_HEIGHT);

    SDL_RenderSetLogicalSize(emu.renderer,
        config->square_par ? GG_SCREEN_WIDTH  : GG_LOGICAL_WIDTH,
        config->square_par ? GG_SCREEN_HEIGHT : GG_LOGICAL_HEIGHT);
    SDL_SetTextureBlendMode(emu.texture, SDL_BLENDMODE_BLEND);
    SDL_ShowCursor(SDL_DISABLE);

    SDL_SetRenderDrawColor(emu.renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(emu.renderer);
    SDL_RenderPresent(emu.renderer);
}

/*
    Set up SDL.
*/
static void setup_sdl(Config *config)
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER) < 0)
        FATAL("SDL failed to initialize: %s", SDL_GetError());

    setup_input();
    setup_graphics(config);
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
    Handle controller input.
*/
static void handle_controller_input(
    GameGear *gg, SDL_GameControllerButton input, bool state)
{
    GGButton button;
    switch (input) {
        case SDL_CONTROLLER_BUTTON_A:
            button = BUTTON_TRIGGER_1; break;
        case SDL_CONTROLLER_BUTTON_B:
            button = BUTTON_TRIGGER_2; break;
        case SDL_CONTROLLER_BUTTON_START:
            button = BUTTON_START;     break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:
            button = BUTTON_UP;        break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            button = BUTTON_DOWN;      break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            button = BUTTON_LEFT;      break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            button = BUTTON_RIGHT;     break;
        default:
            return;
    }
    gamegear_input(gg, button, state);
}

/*
    Handle a controller being added.
*/
static void handle_controller_added(int i)
{
    SDL_Joystick *joy = SDL_JoystickOpen(i);
    SDL_JoystickID id = SDL_JoystickInstanceID(joy);

    for (int i = 0; i < emu.controllers.num; i++) {
        SDL_Joystick *other = SDL_GameControllerGetJoystick(
            emu.controllers.items[i]);
        if (SDL_JoystickInstanceID(other) == id) {
            TRACE("SDL added duplicate joystick %i", i);
            SDL_JoystickClose(joy);
            return;
        }
    }

    SDL_JoystickClose(joy);
    if (!SDL_IsGameController(i)) {
        DEBUG("SDL added joystick %i: not a game controller", i);
        return;
    }

    SDL_GameController *controller = SDL_GameControllerOpen(i);
    if (!controller) {
        ERROR("SDL failed to open controller %i: %s", i, SDL_GetError());
        return;
    }

    DEBUG("Added controller %i: %s", i, get_controller_name(controller));
    if (emu.controllers.num == 0) {
        int n = 4;
        emu.controllers.items = cr_calloc(n, sizeof(SDL_GameController*));
        emu.controllers.capacity = n;
    }
    else if (emu.controllers.num >= emu.controllers.capacity) {
        emu.controllers.capacity *= 2;
        emu.controllers.items = cr_realloc(emu.controllers.items,
            emu.controllers.capacity * sizeof(SDL_GameController*));
    }
    emu.controllers.items[emu.controllers.num++] = controller;
}

/*
    Handle a controller being removed.
*/
static void handle_controller_removed(int id)
{
    for (int i = 0; i < emu.controllers.num; i++) {
        SDL_GameController *controller = emu.controllers.items[i];
        SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);
        if (!joy)
            continue;

        if (SDL_JoystickInstanceID(joy) == id) {
            DEBUG("Removed controller %i: %s", i,
                get_controller_name(controller));
            SDL_GameControllerClose(controller);
            for (; i < emu.controllers.num; i++)
                emu.controllers.items[i] = emu.controllers.items[i + 1];
            emu.controllers.num--;
            return;
        }
    }
    DEBUG("SDL removed unknown controller: %i", id)
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
            case SDL_CONTROLLERBUTTONDOWN:
                handle_controller_input(gg, event.cbutton.button, true);
                break;
            case SDL_CONTROLLERBUTTONUP:
                handle_controller_input(gg, event.cbutton.button, false);
                break;
            case SDL_CONTROLLERDEVICEADDED:
                handle_controller_added(event.cdevice.which);
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                handle_controller_removed(event.cdevice.which);
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
    Clean up SDL stuff allocated in setup_sdl().
*/
static void cleanup_sdl()
{
    free(emu.pixels);
    SDL_DestroyTexture(emu.texture);
    SDL_DestroyRenderer(emu.renderer);
    SDL_DestroyWindow(emu.window);
    for (int i = 0; i < emu.controllers.num; i++)
        SDL_GameControllerClose(emu.controllers.items[i]);
    if (emu.controllers.items)
        free(emu.controllers.items);
    SDL_Quit();

    emu.window = NULL;
    emu.renderer = NULL;
    emu.texture = NULL;
    emu.controllers.items = NULL;
    emu.controllers.num = emu.controllers.capacity = 0;
}

/*
    Emulate a ROM in a Game Gear while handling I/O with the host computer.

    Block until emulation is finished.
*/
void emulate(ROM *rom, Config *config)
{
    Save save;
    if (!config->no_saving) {
        if (!save_init(&save, config->sav_path, rom))
            return;
    }

    BIOS *bios = NULL;
    if (config->bios_path) {
         if (!(bios = bios_open(config->bios_path)))
            return;
    }

    emu.gg = gamegear_create();
    signal(SIGINT, handle_sigint);
    setup_sdl(config);

    gamegear_attach_callback(emu.gg, frame_callback);
    gamegear_attach_display(emu.gg, emu.pixels);
    gamegear_load_rom(emu.gg, rom);
    if (bios)
        gamegear_load_bios(emu.gg, bios);
    if (!config->no_saving)
        gamegear_load_save(emu.gg, &save);

    gamegear_simulate(emu.gg);

    if (gamegear_get_exception(emu.gg))
        ERROR("caught exception: %s", gamegear_get_exception(emu.gg))
    else
        WARN("caught signal, stopping...")
    if (DEBUG_LEVEL)
        gamegear_print_state(emu.gg);

    cleanup_sdl();
    signal(SIGINT, SIG_DFL);
    gamegear_destroy(emu.gg);
    emu.gg = NULL;
    if (!config->no_saving)
        save_free(&save);
}
