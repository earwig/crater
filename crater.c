/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <stdio.h>

#include "src/assembler.h"
#include "src/config.h"
#include "src/disassembler.h"
#include "src/emulator.h"
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

    SET_LOG_LEVEL(config->debug)
    if (DEBUG_LEVEL)
        config_dump_args(config);

    if (config->assemble) {
        retval = assemble_file(config->src_path, config->dst_path);
        retval = retval ? EXIT_SUCCESS : EXIT_FAILURE;
    } else if (config->disassemble) {
        retval = disassemble_file(config->src_path, config->dst_path);
        retval = retval ? EXIT_SUCCESS : EXIT_FAILURE;
    } else {
        ROM rom;
        const char* errmsg;

        if ((errmsg = rom_open(&rom, config->rom_path))) {
            ERROR("couldn't load ROM image '%s': %s", config->rom_path, errmsg)
            retval = EXIT_FAILURE;
        } else {
            printf("crater: emulating: %s\n", rom.name);
            emulate(&rom, config->fullscreen, config->scale);
            rom_close(&rom);
        }
    }

    config_destroy(config);
    return retval;
}
