/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>
#include <stdio.h>

#include "src/assembler.h"
#include "src/config.h"
#include "src/disassembler.h"
#include "src/gamegear.h"
#include "src/iomanager.h"
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
            retval = assemble_file(config->src_path, config->dst_path);
        else
            retval = disassemble_file(config->src_path, config->dst_path);
        retval = retval ? EXIT_SUCCESS : EXIT_FAILURE;
    } else {
        ROM *rom;
        const char* errmsg;

        if ((errmsg = rom_open(&rom, config->rom_path))) {
            ERROR("couldn't load ROM image '%s': %s", config->rom_path, errmsg)
            retval = EXIT_FAILURE;
        } else {
            GameGear *gg = gamegear_create();

            printf("crater: emulating: %s\n", rom->name);
            gamegear_load(gg, rom);
            iomanager_emulate(gg);

            gamegear_destroy(gg);
            rom_close(rom);
        }
    }

    config_destroy(config);
    return retval;
}
