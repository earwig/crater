/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "src/config.h"
#include "src/logging.h"
#include "src/rom.h"

/*
    Main function.
*/
int main(int argc, char *argv[])
{
    Config *config;
    ROM *rom;
    int retval;

    retval = config_create(&config, argc, argv);
    if (retval != CONFIG_OK)
        return retval == CONFIG_EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;

    printf("crater: a Sega Game Gear emulator\n\n");

#ifdef DEBUG_MODE
    config_dump_args(config);
#endif

    if (!(rom = rom_open(config->rom_path))) {
        if (errno == ENOMEM)
            OUT_OF_MEMORY()
        else
            FATAL_ERRNO("couldn't load ROM image '%s'", config->rom_path)
    }
    printf("Loaded ROM image: %s.\n", rom->name);

    // TODO: start from here

    rom_close(rom);
    config_destroy(config);
    return EXIT_SUCCESS;
}
