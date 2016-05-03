/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "io.h"
#include "mmu.h"
#include "psg.h"
#include "rom.h"
#include "z80.h"

#define GG_SCREEN_WIDTH  160
#define GG_SCREEN_HEIGHT 144

#define GG_FPS 60
#define GG_EXC_BUFF_SIZE 128

/* Structs, etc. */

struct GameGear;
typedef void (*GGFrameCallback)(struct GameGear*);

typedef struct GameGear {
    Z80 cpu;
    MMU mmu;
    VDP vdp;
    PSG psg;
    IO io;
    bool powered;
    GGFrameCallback callback;
    char exc_buffer[GG_EXC_BUFF_SIZE];
} GameGear;

typedef enum {
    BUTTON_UP        = 0,
    BUTTON_DOWN      = 1,
    BUTTON_LEFT      = 2,
    BUTTON_RIGHT     = 3,
    BUTTON_TRIGGER_1 = 4,
    BUTTON_TRIGGER_2 = 5,
    BUTTON_START
} GGButton;

/* Functions */

GameGear* gamegear_create();
void gamegear_destroy(GameGear*);
void gamegear_load(GameGear*, const ROM*);
void gamegear_simulate(GameGear*);
void gamegear_input(GameGear*, GGButton, bool);
void gamegear_power_off(GameGear*);

void gamegear_attach_callback(GameGear*, GGFrameCallback);
void gamegear_attach_display(GameGear*, uint32_t*);
void gamegear_detach(GameGear*);

const char* gamegear_get_exception(GameGear*);
void gamegear_print_state(const GameGear*);
