/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "src/errors.h"
#include "src/rom.h"
#include "src/version.h"

#define ROMS_DIR "roms"

/* Print command-line help/usage. */
static void print_help(const char *arg1)
{
    printf("%s [--help|-h] [--version|-v] [rom_path]\n", arg1);
}

/* Print crater's version. */
static void print_version()
{
    printf("crater %s\n", CRATER_VERSION);
}

/* Parse the command-line arguments for any special flags. */
static void parse_args(int argc, char *argv[])
{
    char *arg;
    int i;

    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (arg[0] != '-')
            continue;
        do
            arg++;
        while (arg[0] == '-');

        if (!strcmp(arg, "h") || !strcmp(arg, "help")) {
            print_help(argv[0]);
            exit(0);
        } else if (!strcmp(arg, "v") || !strcmp(arg, "version")) {
            print_version();
            exit(0);
        } else {
            FATAL("unknown argument: %s", argv[i])
        }
    }
}

/* Return whether the given string ends with the given suffix. */
static bool ends_with(const char *input, const char *suffix)
{
    size_t ilen = strlen(input), slen = strlen(suffix);

    if (ilen < slen)
        return false;
    return strcmp(input + (ilen - slen), suffix) == 0;
}

/* Load all potential ROM files in roms/ into a data structure. */
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

/* Find all potential ROM files in the roms/ directory, then ask the user which
   one they want to run. */
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

int main(int argc, char *argv[])
{
    char *rom_path;
    rom_type *rom;

    parse_args(argc, argv);
    printf("crater: a Sega Game Gear emulator\n\n");
    rom_path = argc > 1 ? argv[1] : get_rom_path_from_user();
    if (rom_path[0] == '\0')
        FATAL("no image given")

    if (!(rom = rom_open(rom_path))) {
        if (errno == ENOMEM)
            OUT_OF_MEMORY()
        else
            FATAL_ERRNO("couldn't load ROM image '%s'", rom_path)
    }
    if (argc <= 1)
        free(rom_path);
    printf("Loaded ROM image: %s.\n", rom->name);

    // TODO: start from here

    rom_close(rom);
    return 0;
}
