/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/logging.h"

/*
    TODO
*/
static bool test_cpu()
{
    // TODO
    return true;
}

/*
    TODO
*/
static bool test_vdp()
{
    // TODO
    return true;
}

/*
    TODO
*/
static bool test_psg()
{
    // TODO
    return true;
}

/*
    TODO
*/
static bool test_asm()
{
    // TODO
    return true;
}

/*
    TODO
*/
static bool test_dis()
{
    // TODO
    return true;
}

/*
    TODO
*/
static bool test_integrate()
{
    // TODO
    return true;
}


/*
    Main function.
*/
int main(int argc, char *argv[])
{
    if (argc != 2)
        FATAL("a component name is required")

    const char *component = argv[1], *name;
    bool (*func)();

    if (!strcmp(component, "cpu")) {
        name = "Z80 CPU";
        func = test_cpu;
    } else if (!strcmp(component, "vdp")) {
        name = "VDP";
        func = test_vdp;
    } else if (!strcmp(component, "psg")) {
        name = "SN76489 PSG";
        func = test_psg;
    } else if (!strcmp(component, "asm")) {
        name = "assembler";
        func = test_asm;
    } else if (!strcmp(component, "dis")) {
        name = "disassembler";
        func = test_dis;
    } else if (!strcmp(component, "integrate")) {
        name = "integration";
        func = test_integrate;
    } else {
        FATAL("unknown component: %s", component)
    }

    printf("crater: running %s tests\n", name);
    return func() ? EXIT_SUCCESS : EXIT_FAILURE;
}
