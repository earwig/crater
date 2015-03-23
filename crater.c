/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <stdio.h>

#include "src/assembler.h"
#include "src/config.h"
#include "src/disassembler.h"
#include "src/logging.h"
#include "src/rom.h"

/*
    Main function.
*/
int main(int argc, char *argv[])
{
    Config *config;
    int retval = EXIT_SUCCESS;

    retval = config_create(&config, argc, argv);
    if (retval != CONFIG_OK)
        return retval == CONFIG_EXIT_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;

#ifdef DEBUG_MODE
    config_dump_args(config);
#endif

    if (config->assemble || config->disassemble) {
        printf("crater: running %s: %s -> %s\n",
               config->assemble ? "assembler" : "disassembler",
               config->src_path, config->dst_path);
        if (config->assemble)
            retval = assemble(config->src_path, config->dst_path);
        else
            retval = disassemble(config->src_path, config->dst_path);
        retval = retval ? EXIT_SUCCESS : EXIT_FAILURE;
    } else {
        ROM *rom;

        printf("crater: a Sega Game Gear emulator\n\n");
        if (!(rom = rom_open(config->rom_path))) {
            if (errno == ENOMEM)
                OUT_OF_MEMORY()
            else
                FATAL_ERRNO("couldn't load ROM image '%s'", config->rom_path)
        }
        printf("Loaded ROM image: %s\n", rom->name);

        // TODO: emulate game here...

        rom_close(rom);
    }

    config_destroy(config);
    return retval;
}
