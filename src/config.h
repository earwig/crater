/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#pragma once

#include <stdbool.h>

#define ROMS_DIR "roms"

#define CONFIG_OK 0
#define CONFIG_EXIT_SUCCESS 1
#define CONFIG_EXIT_FAILURE 2

/*
    We need some sort of maximum scale - with a native resolution of 160 x 144,
    a scale factor of 1024 will let us go up to 163,840 x 147,456 pixels.
    No one has a screen this large.
*/
#define SCALE_MAX 1024

/* Structs */

typedef struct {
    int debug;
    bool assemble;
    bool disassemble;
    bool fullscreen;
    bool no_saving;
    unsigned scale;
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
