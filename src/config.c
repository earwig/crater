/* Copyright (C) 2014-2017 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "config.h"
#include "logging.h"
#include "util.h"
#include "version.h"

typedef struct {
    int argc;
    char **argv;
    int i, j;
    int paths_read;
} Arguments;

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
"    <rom_path>        path to the rom file to execute; if not given, will look\n"
"                      in the roms/ directory and prompt the user\n"
"\n"
"advanced options:\n"
"    -g, --debug       show logging information while running; add twice (-gg)\n"
"                      to show more detailed logs, including an emulator trace\n"
"    -s, --scale <n>   scale the game screen by an integer factor\n"
"                      (applies to windowed mode only; defaults to 4)\n"
"    -b, --save <path> save cartridge RAM (\"battery save\") to the given file\n"
"                      (defaults to <rom_path>.sav)\n"
"    -n, --no-save     disable saving cartridge RAM entirely\n"
"    -a, --assemble <in> [<out>]\n"
"                      convert z80 assembly source code into a binary file that\n"
"                      can be run by crater\n"
"    -d, --disassemble <in> [<out>]\n"
"                      convert a binary file into z80 assembly source code\n"
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
        paths = cr_malloc(sizeof(char*) * psize);
        while ((entry = readdir(dirp))) {
            path = entry->d_name;
            if (ends_with(path, ".gg") || ends_with(path, ".bin")) {
                if (npaths >= psize) {
                    paths = cr_realloc(paths, sizeof(char*) * (psize *= 2));
                }
                paths[npaths] = cr_malloc(sizeof(char*) *
                        (strlen(path) + strlen(ROMS_DIR) + 1));
                strcpy(paths[npaths], ROMS_DIR "/");
                strcat(paths[npaths], path);
                npaths++;
            }
        }
        closedir(dirp);
    } else {
        WARN_ERRNO("couldn't open '" ROMS_DIR "/'")
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
    free(paths);
    return path;
}

/*
    Consume and return the next argument, or NULL if it is at the end.
*/
static const char* consume_next(Arguments *args)
{
    static char partial[3] = {'-', '\0', '\0'};
    char *curr = args->argv[args->i];
    if (!curr)
        return NULL;

    if (curr[0] == '-' && curr[1] != '-' && curr[1] != '\0') {
        // Need to handle single dash arg clusters like -asdf <=> -a -s -d -f
        if (curr[args->j]) {
            partial[1] = curr[args->j];
            args->j++;
            return partial;
        }
        args->i++;
        args->j = 1;
        return consume_next(args);
    }

    args->i++;
    return curr;
}

/*
    Parse a single positional (typically a path) command-line argument.
*/
static int parse_pos_arg(Config *config, Arguments *args, const char *arg)
{
    if (args->paths_read >= 2) {
        ERROR("too many arguments given: emulator mode accepts one ROM file,\n"
              "and assembler mode accepts one input file and one output file")
        return CONFIG_EXIT_FAILURE;
    }

    char *path = cr_strdup(arg);
    if (args->paths_read == 1) {
        /* If this is the second path given, it can only be an output file for
           the assembler. If the assembler is not enabled by subsequent
           arguments, we'll throw an error. */
        config->dst_path = path;
    } else {
        /* Otherwise, put the argument in the expected place. If we put it in
           rom_path and the assembler is enabled by later arguments, we'll
           move it. */
        if (config->assemble || config->disassemble)
            config->src_path = path;
        else
            config->rom_path = path;
    }

    args->paths_read++;
    return CONFIG_OK;
}

/*
    Check if the given argument matches the given short or long form.
*/
static bool arg_check(const char *arg, const char *t1, const char *t2)
{
    return !strcmp(arg, t1) || !strcmp(arg, t2);
}

/*
    Parse a single optional ("flag") command-line argument.
*/
static int parse_opt_arg(Config *config, Arguments *args, const char *arg)
{
    do
        arg++;
    while (*arg == '-');

    if (arg_check(arg, "h", "help")) {
        print_help(args->argv[0]);
        return CONFIG_EXIT_SUCCESS;
    }
    else if (arg_check(arg, "v", "version")) {
        print_version();
        return CONFIG_EXIT_SUCCESS;
    }
    else if (arg_check(arg, "f", "fullscreen")) {
        config->fullscreen = true;
    }
    else if (arg_check(arg, "s", "scale")) {
        const char *next = consume_next(args);
        if (!next) {
            ERROR("the scale option requires an argument")
            return CONFIG_EXIT_FAILURE;
        }
        long scale = strtol(next, NULL, 10);
        if (scale <= 0 || scale > SCALE_MAX) {
            ERROR("scale factor of %s is not an integer or is out of range", next)
            return CONFIG_EXIT_FAILURE;
        }
        config->scale = scale;
    }
    else if (arg_check(arg, "b", "save")) {
        const char *next = consume_next(args);
        if (!next) {
            ERROR("the save option requires an argument")
            return CONFIG_EXIT_FAILURE;
        }
        free(config->sav_path);
        config->sav_path = cr_strdup(next);
    }
    else if (arg_check(arg, "n", "no-save")) {
        config->no_saving = true;
    }
    else if (arg_check(arg, "g", "debug")) {
        config->debug++;
    }
    else if (arg_check(arg, "a", "assemble")) {
        if (args->paths_read >= 1) {
            config->src_path = config->rom_path;
            config->rom_path = NULL;
        }
        config->assemble = true;
    }
    else if (arg_check(arg, "d", "disassemble")) {
        if (args->paths_read >= 1) {
            config->src_path = config->rom_path;
            config->rom_path = NULL;
        }
        config->disassemble = true;
    }
    else if (arg_check(arg, "r", "overwrite")) {
        config->overwrite = true;
    }
    else {
        ERROR("unknown argument: %s", arg)
        return CONFIG_EXIT_FAILURE;
    }
    return CONFIG_OK;
}

/*
    Parse the command-line arguments for any special flags.
*/
static int parse_args(Config *config, int argc, char *argv[])
{
    Arguments args = {argc, argv, 1, 1, 0};
    const char *arg;
    int retval;

    while ((arg = consume_next(&args))) {
        if (arg[0] != '-')
            retval = parse_pos_arg(config, &args, arg);
        else
            retval = parse_opt_arg(config, &args, arg);
        if (retval != CONFIG_OK)
            return retval;
    }

    if (!config->assemble && !config->disassemble) {
        if (args.paths_read >= 2) {
            ERROR("too many arguments given - emulator mode accepts one ROM file")
            return CONFIG_EXIT_FAILURE;
        }
        if (args.paths_read == 0) {
            char *path = get_rom_path_from_user();
            if (path[0] == '\0') {
                ERROR("no ROM image given")
                return CONFIG_EXIT_FAILURE;
            }
            config->rom_path = path;
        }
    }

    return CONFIG_OK;
}

/*
    If no output file is specified for the assembler, this function picks a
    filename based on the input file, replacing its extension with ".gg" or
    ".asm" (or adding it, if none is present).
*/
static void guess_assembler_output_file(Config *config)
{
    char *src = config->src_path, *ptr = src + strlen(src) - 1,
         *ext = config->assemble ? ".gg" : ".asm";
    size_t until_ext = ptr - src + 1;

    do {
        if (*ptr == '.') {
            until_ext = ptr - src;
            break;
        }
    } while (ptr-- >= src);

    config->dst_path = cr_malloc(sizeof(char) * (until_ext + 5));
    strcpy(stpncpy(config->dst_path, src, until_ext), ext);
}

/*
    Ensure that the combination of arguments in a config object are valid.
*/
static bool sanity_check(Config *config)
{
    bool assembler = config->assemble || config->disassemble;

    if (config->fullscreen && config->scale) {
        ERROR("cannot specify a scale in fullscreen mode")
        return false;
    } else if (config->assemble && config->disassemble) {
        ERROR("cannot assemble and disassemble at the same time")
        return false;
    } else if (assembler && (config->fullscreen || config->scale)) {
        ERROR("cannot specify emulator options in assembler mode")
        return false;
    } else if (assembler && !config->src_path) {
        ERROR("assembler mode requires an input file")
        return false;
    }
    return true;
}

/*
    Set some default values for missing arguments.

    Some additional sanity checking is done.
*/
static bool set_defaults(Config *config)
{
    bool assembler = config->assemble || config->disassemble;

    if (!config->scale) {
        config->scale = 4;
    }
    if (assembler && !config->dst_path) {
        guess_assembler_output_file(config);
    }
    if (assembler && !config->overwrite && !strcmp(config->src_path, config->dst_path)) {
        ERROR("refusing to overwrite the assembler input file; pass -r to override")
        return false;
    }
    if (!assembler && !config->sav_path && !config->no_saving) {
        config->sav_path = cr_malloc(sizeof(char) *
            (strlen(config->rom_path) + 4));
        strcpy(config->sav_path, config->rom_path);
        strcat(config->sav_path, ".sav");
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
    Config *config = cr_malloc(sizeof(Config));
    int retval;

    config->debug = 0;
    config->assemble = false;
    config->disassemble = false;
    config->fullscreen = false;
    config->scale = 0;
    config->rom_path = NULL;
    config->sav_path = NULL;
    config->src_path = NULL;
    config->dst_path = NULL;
    config->overwrite = false;
    config->no_saving = false;

    retval = parse_args(config, argc, argv);
    if (retval == CONFIG_OK && !(sanity_check(config) && set_defaults(config)))
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
    free(config->rom_path);
    free(config->sav_path);
    free(config->src_path);
    free(config->dst_path);
    free(config);
}

/*
    @DEBUG_LEVEL
    Print out all config arguments to stdout.
*/
void config_dump_args(const Config* config)
{
    DEBUG("Dumping arguments:")
    DEBUG("- debug:       %d", config->debug)
    DEBUG("- assemble:    %s", config->assemble ? "true" : "false")
    DEBUG("- disassemble: %s", config->disassemble ? "true" : "false")
    DEBUG("- fullscreen:  %s", config->fullscreen ? "true" : "false")
    DEBUG("- scale:       %d", config->scale)
    DEBUG("- rom_path:    %s", config->rom_path ? config->rom_path : "(null)")
    DEBUG("- sav_path:    %s", config->sav_path ? config->sav_path : "(null)")
    DEBUG("- src_path:    %s", config->src_path ? config->src_path : "(null)")
    DEBUG("- dst_path:    %s", config->dst_path ? config->dst_path : "(null)")
    DEBUG("- overwrite:   %s", config->overwrite ? "true" : "false")
    DEBUG("- no_saving:   %s", config->no_saving ? "true" : "false")
}
