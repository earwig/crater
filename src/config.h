/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>

#define ROMS_DIR "roms"
#define CONTROLLER_DB_PATH "gamecontrollerdb.txt"

#define CONFIG_OK 0
#define CONFIG_EXIT_SUCCESS 1
#define CONFIG_EXIT_FAILURE 2

/*
    We need some sort of maximum scale - with a native resolution of 160 x 144,
    a scale factor of 128 will let us go up to 20,480 x 18,432 pixels.
*/
#define SCALE_MAX 128

/* Structs */

typedef struct {
    int debug;
    bool assemble;
    bool disassemble;
    bool fullscreen;
    bool no_saving;
    unsigned scale;
    bool square_par;
    char *rom_path;
    char *sav_path;
    char *bios_path;
    char *src_path;
    char *dst_path;
    bool overwrite;
} Config;

/* Functions */

int config_create(Config**, int, char*[]);
void config_destroy(Config*);
void config_dump_args(const Config*);
