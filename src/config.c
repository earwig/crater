/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "config.h"
#include "logging.h"
#include "version.h"

#define ROMS_DIR "roms"

/*
    Print command-line help/usage.
*/
static void print_help(const char *arg1)
{
    printf("%s [-h] [-v] [-f] [-s <n>] [<rom_path>] ...\n"
"\n"
"basic options:\n"
"    -h, --help        show this help message and exit\n"
"    -v, --version     show crater's version number and exit\n"
"    -f, --fullscreen  enable fullscreen mode\n"
"    -s, --scale <n>   scale the game screen by an integer factor\n"
"                      (applies to windowed mode only)\n"
"    <rom_path>        path to the rom file to execute; if not given, will look\n"
"                      in the roms/ directory and prompt the user\n"
"\n"
"advanced options:\n"
"    -g, --debug       display information about emulation state while running,\n"
"                      including register and memory values; can also pause\n"
"                      emulation, set breakpoints, and change state\n"
"    -a, --assemble <in> [<out>]     convert z80 assembly source code into a\n"
"                                    binary file that can be run by crater\n"
"    -d, --disassemble <in> [<out>]  convert a binary file into z80 assembly\n"
"                                    source code\n"
"    -r, --overwrite   allow crater to write assembler output to the same\n"
"                      filename as the input\n",
    arg1);
}

/*
    Print crater's version.
*/
static void print_version()
{
    printf("crater %s\n", CRATER_VERSION);
}

/*
    Return whether the given string ends with the given suffix.
*/
static bool ends_with(const char *input, const char *suffix)
{
    size_t ilen = strlen(input), slen = strlen(suffix);

    if (ilen < slen)
        return false;
    return strcmp(input + (ilen - slen), suffix) == 0;
}

/*
    Load all potential ROM files in roms/ into a data structure.
*/
static int get_rom_paths(char ***path_ptr)
{
    DIR *dirp;
    struct dirent *entry;
    char **paths = NULL, *path;
    int psize = 8, npaths = 0;

    dirp = opendir(ROMS_DIR);
    if (dirp) {
        paths = malloc(sizeof(char*) * psize);
        if (!paths)
            OUT_OF_MEMORY()
        while ((entry = readdir(dirp))) {
            path = entry->d_name;
            if (ends_with(path, ".gg") || ends_with(path, ".bin")) {
                if (npaths >= psize) {
                    paths = realloc(paths, sizeof(char*) * (psize *= 2));
                    if (!paths)
                        OUT_OF_MEMORY()
                }
                paths[npaths] = malloc(sizeof(char*) *
                        (strlen(path) + strlen(ROMS_DIR) + 1));
                if (!paths[npaths])
                    OUT_OF_MEMORY()
                strcpy(paths[npaths], ROMS_DIR "/");
                strcat(paths[npaths], path);
                npaths++;
            }
        }
        closedir(dirp);
    } else {
        WARN_ERRNO("couldn't open 'roms/'")
    }
    *path_ptr = paths;
    return npaths;
}

/*
    Find all potential ROM files in the roms/ directory, then ask the user
    which one they want to run.
*/
static char* get_rom_path_from_user()
{
    char **paths, *path, *input = NULL;
    int npaths, i;
    long int index;
    size_t size = 0;
    ssize_t len;

    npaths = get_rom_paths(&paths);
    for (i = 0; i < npaths; i++)
        printf("[%2d] %s\n", i + 1, paths[i]);
    if (npaths)
        printf("Enter a ROM number from above, or the path to a ROM image: ");
    else
        printf("Enter the path to a ROM image: ");

    len = getline(&input, &size, stdin);
    if (!input)
        OUT_OF_MEMORY()
    if (len > 0 && input[len - 1] == '\n')
        input[len - 1] = '\0';
    index = strtol(input, NULL, 10);
    if (index < 1 || index > npaths)
        path = input;
    else
        path = paths[index - 1];

    for (i = 0; i < npaths; i++) {
        if (paths[i] != path)
            free(paths[i]);
    }
    if (paths)
        free(paths);
    return path;
}

/*
    Parse the command-line arguments for any special flags.
*/
static int parse_args(Config *config, int argc, char *argv[])
{
    char *arg, *path;
    int i, paths_read = 0;

    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (arg[0] != '-') {
            // Parsing a path or other variable:
            if (paths_read >= 2) {
                ERROR("too many arguments given - emulator mode accepts one "
                      "ROM file,\nand assembler mode accepts one input file "
                      "and one output file")
                return CONFIG_EXIT_FAILURE;
            }

            path = malloc(sizeof(char) * (strlen(arg) + 1));
            if (!path)
                OUT_OF_MEMORY()
            strcpy(path, arg);

            if (paths_read == 1) {
                /* If this is the second path given, it can only be an output
                   file for the assembler. If the assembler is not enabled by
                   subsequent arguments, we'll throw an error. */
                config->dst_path = path;
            } else {
                /* Otherwise, put the argument in the expected place. If we put
                   it in rom_path and the assembler is enabled by later
                   arguments, we'll move it. */
                if (config->assemble || config->disassemble)
                    config->src_path = path;
                else
                    config->rom_path = path;
            }

            paths_read++;
            continue;
        }

        // Parsing an option:
        do
            arg++;
        while (arg[0] == '-');

        if (!strcmp(arg, "h") || !strcmp(arg, "help")) {
            print_help(argv[0]);
            return CONFIG_EXIT_SUCCESS;
        } else if (!strcmp(arg, "v") || !strcmp(arg, "version")) {
            print_version();
            return CONFIG_EXIT_SUCCESS;
        } else if (!strcmp(arg, "f") || !strcmp(arg, "fullscreen")) {
            config->fullscreen = true;
        } else if (!strcmp(arg, "s") || !strcmp(arg, "scale")) {
            if (i++ >= argc) {
                ERROR("the scale option requires an argument")
                return CONFIG_EXIT_FAILURE;
            }
            arg = argv[i];
            long scale = strtol(arg, NULL, 10);
            if (scale <= 0 || scale > SCALE_MAX) {
                ERROR("scale factor of %s is not an integer or is out of range", arg)
                return CONFIG_EXIT_FAILURE;
            }
            config->scale = scale;
        } else if (!strcmp(arg, "g") || !strcmp(arg, "debug")) {
            config->debug = true;
        } else if (!strcmp(arg, "a") || !strcmp(arg, "assemble")) {
            if (paths_read >= 1) {
                config->src_path = config->rom_path;
                config->rom_path = NULL;
            }
            config->assemble = true;
        } else if (!strcmp(arg, "d") || !strcmp(arg, "disassemble")) {
            if (paths_read >= 1) {
                config->src_path = config->rom_path;
                config->rom_path = NULL;
            }
            config->disassemble = true;
        } else if (!strcmp(arg, "r") || !strcmp(arg, "overwrite")) {
            config->overwrite = true;
        } else {
            ERROR("unknown argument: %s", argv[i])
            return CONFIG_EXIT_FAILURE;
        }
    }

    if (!config->assemble && !config->disassemble && paths_read >= 2) {
        ERROR("too many arguments given - emulator mode accepts one ROM file")
        return CONFIG_EXIT_FAILURE;
    }
    if (!config->assemble && !config->disassemble && paths_read == 0) {
        path = get_rom_path_from_user();
        if (path[0] == '\0') {
            ERROR("no ROM image given")
            return CONFIG_EXIT_FAILURE;
        }
        config->rom_path = path;
    }

    return CONFIG_OK;
}

/*
    If no output file is specified for the assembler, this function picks a
    filename based on the input file, replacing its extension with '.gg' or
    '.s' (or adding it, if none is present).
*/
static void guess_assembler_output_file(Config* config)
{
    char *src = config->src_path, *ptr = src + strlen(src) - 1,
         *ext = config->assemble ? ".gg" : ".s";
    size_t until_ext = ptr - src + 1;

    do {
        if (*ptr == '.') {
            until_ext = ptr - src;
            break;
        }
    } while (ptr-- >= src);

    config->dst_path = malloc(sizeof(char) * (until_ext + 4));
    if (!config->dst_path)
        OUT_OF_MEMORY()
    strcpy(stpncpy(config->dst_path, src, until_ext), ext);
}

/*
    Ensure that the combination of arguments in a config object are valid.

    Some modifications are made in the case of missing arguments, like guessing
    the filenames for assembler output files.
*/
static bool sanity_check(Config* config)
{
    bool assembler = config->assemble || config->disassemble;

    if (config->fullscreen && config->scale > 1) {
        ERROR("cannot specify a scale in fullscreen mode")
        return false;
    } else if (config->assemble && config->disassemble) {
        ERROR("cannot assemble and disassemble at the same time")
        return false;
    } else if (assembler && (config->debug || config->fullscreen || config->scale > 1)) {
        ERROR("cannot specify emulator options in assembler mode")
        return false;
    } else if (assembler && !config->src_path) {
        ERROR("assembler mode requires an input file")
        return false;
    }

    if (assembler && !config->dst_path) {
        guess_assembler_output_file(config);
    }
    if (assembler && !config->overwrite && !strcmp(config->src_path, config->dst_path)) {
        ERROR("refusing to overwrite the assembler input file; pass -r to override")
        return false;
    }

    return true;
}

/*
    Create a new config object and load default values into it.

    Return value is one of CONFIG_OK, CONFIG_EXIT_SUCCESS, CONFIG_EXIT_FAILURE
    and indicates how the caller should proceed. If the caller should exit,
    then the config object should *not* be freed; otherwise it should be freed
    with config_destroy() when the caller is ready.
*/
int config_create(Config** config_ptr, int argc, char* argv[])
{
    Config *config;
    int retval;

    if (!(config = malloc(sizeof(Config)))) {
        OUT_OF_MEMORY()
        return CONFIG_EXIT_FAILURE;
    }

    config->debug = false;
    config->assemble = false;
    config->disassemble = false;
    config->fullscreen = false;
    config->scale = 1;
    config->rom_path = NULL;
    config->src_path = NULL;
    config->dst_path = NULL;
    config->overwrite = false;

    retval = parse_args(config, argc, argv);
    if (retval == CONFIG_OK && !sanity_check(config))
        retval = CONFIG_EXIT_FAILURE;
    if (retval != CONFIG_OK) {
        config_destroy(config);
        return retval;
    }

    *config_ptr = config;
    return CONFIG_OK;
}

/*
    Destroy a config object previously created with config_create().
*/
void config_destroy(Config *config)
{
    if (config->rom_path)
        free(config->rom_path);
    if (config->src_path)
        free(config->src_path);
    if (config->dst_path)
        free(config->dst_path);
    free(config);
}

#ifdef DEBUG_MODE
/*
    DEBUG FUNCTION: Print out all config arguments to stdout.
*/
void config_dump_args(Config* config)
{
    DEBUG("Dumping arguments:")
    DEBUG("- fullscreen:  %s", config->fullscreen ? "true" : "false")
    DEBUG("- scale:       %d", config->scale)
    DEBUG("- debug:       %s", config->debug ? "true" : "false")
    DEBUG("- assemble:    %s", config->assemble ? "true" : "false")
    DEBUG("- disassemble: %s", config->disassemble ? "true" : "false")
    if (config->rom_path)
        DEBUG("- rom_path:    %s", config->rom_path)
    else
        DEBUG("- rom_path:    (null)")
    if (config->src_path)
        DEBUG("- src_path:    %s", config->src_path)
    else
        DEBUG("- src_path:    (null)")
    if (config->dst_path)
        DEBUG("- dst_path:    %s", config->dst_path)
    else
        DEBUG("- dst_path:    (null)")
    DEBUG("- overwrite:   %s", config->overwrite ? "true" : "false")
}
#endif
