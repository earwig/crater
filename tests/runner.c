/* Copyright (C) 2014-2016 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/logging.h"
#include "../src/util.h"

#define ASM_PREFIX "asm/"
#define ASM_OUTFILE ASM_PREFIX ".output.gg"

/* Helper macros for reporting test passings/failures */

#define PASS_TEST()        \
    do {                   \
        printf(".");       \
        fflush(stdout);    \
        passed_tests++;    \
        pending_nl = true; \
    } while(0);

#define FAIL_MSG "***** FAILURE *****"
#define FAIL_TEST(format, ...)                                   \
    do {                                                         \
        printf("F\n");                                           \
        fprintf(stderr, FAIL_MSG "\n" format "\n", __VA_ARGS__); \
        failed_tests++;                                          \
        pending_nl = false;                                      \
    } while(0);

#define READY_STDOUT()          \
    do {                        \
        if (pending_nl) {       \
            printf("\n");       \
            pending_nl = false; \
        }                       \
    } while(0);

static int passed_tests = 0, failed_tests = 0;
static bool pending_nl = false;

/*
    Prints out the test report. Called before exiting using atexit().
*/
static void finalize() {
    READY_STDOUT()
    if (failed_tests)
        printf("fail (%d/%d)\n", passed_tests, passed_tests + failed_tests);
    else
        printf("pass (%d/%d)\n", passed_tests, passed_tests);
}

/*
    Compare two files. If they are identical, then return true. Otherwise,
    return false and print an error message showing the difference.
*/
static bool diff_files(const char *expected_path, const char *actual_path)
{
    bool same = false;
    FILE *expected = NULL, *actual = NULL;

    if (!(actual = fopen(actual_path, "rb"))) {
        FAIL_TEST("missing output file: %s", actual_path)
        goto cleanup;
    }
    if (!(expected = fopen(expected_path, "rb"))) {
        FAIL_TEST("missing reference file: %s", expected_path)
        goto cleanup;
    }

    size_t len = 0;
    int e, a;
    while ((e = fgetc(expected)) != EOF) {
        a = fgetc(actual);
        if (a == EOF) {
            FAIL_TEST("files differ: output file too short (index %zu)", len)
            goto cleanup;
        }
        if (e != a) {
            FAIL_TEST("files differ: 0x%02X != 0x%02X (expected vs. actual at "
                  "index %zu)", e, a, len)
            goto cleanup;
        }
        len++;
    }

    if (fgetc(actual) != EOF) {
        FAIL_TEST("files differ: junk at end of output file (index %zu)", len)
        goto cleanup;
    }

    same = true;

    cleanup:
    if (expected)
        fclose(expected);
    if (actual)
        fclose(actual);
    return same;
}

/*
    Return whether the given character is valid within a filename.
*/
static bool is_valid_filename_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-';
}

/*
    Run a single ASM->ROM test, converting the given source file to a temporary
    output file, compared against the reference file.
*/
static bool run_asm_test(const char *src_file, const char *ref_file)
{
    char *cmd_prefix = "../crater --assemble " ASM_PREFIX;
    char *cmd = cr_malloc(sizeof(char) *
        (strlen(cmd_prefix) + strlen(ASM_OUTFILE) + strlen(src_file)) + 2);

    // Construct the command by concatenating:
    //   ../crater --assemble asm/<src_file> asm/.output.gg
    stpcpy(stpcpy(stpcpy(cmd, cmd_prefix), src_file), " " ASM_OUTFILE);
    unlink(ASM_OUTFILE);
    system(cmd);
    free(cmd);

    // Construct the full reference file path in a temporary variable and diff
    // it with the output file:
    char *ref_path = malloc(sizeof(char) *
        (strlen(ASM_PREFIX) + strlen(ref_file) + 1));
    stpcpy(stpcpy(ref_path, ASM_PREFIX), ref_file);
    bool diff = diff_files(ref_path, ASM_OUTFILE);
    free(ref_path);
    return diff;
}

/* --------------------------- Main test runners --------------------------- */

/*
    Run tests for the Z80 CPU.
*/
static bool test_cpu()
{
    // TODO
    return true;
}

/*
    Run tests for the VDP.
*/
static bool test_vdp()
{
    // TODO
    return true;
}

/*
    Run tests for the SN76489 PSG.
*/
static bool test_psg()
{
    // TODO
    return true;
}

/*
    Run tests for the assembler.
*/
static bool test_asm()
{
    FILE *fp = fopen(ASM_PREFIX "manifest", "r");
    if (!fp) {
        ERROR_ERRNO("couldn't open manifest file")
        return false;
    }

    char *line = NULL, *split, c;
    size_t cap = 0, lineno = 0, i;
    ssize_t len;

    while ((len = getline(&line, &cap, fp)) > 0) {
        lineno++;
        line[--len] = '\0';
        if (!len)
            continue;

        i = 0;
        while ((c = line[i++])) {
            if (!is_valid_filename_char(c) && c != ' ') {
                READY_STDOUT()
                ERROR("bad character in manifest file on line %zu", lineno)
                return false;
            }
        }

        split = strchr(line, ' ');
        if (!split || strchr(split + 1, ' ')) {
            READY_STDOUT()
            ERROR("bad format in manifest file on line %zu", lineno)
            return false;
        }

        *(split++) = '\0';
        if (!run_asm_test(line, split)) {
            fprintf(stderr, "in test: %s -> %s\n", line, split);
            return false;
        }
        PASS_TEST()
    }

    unlink(ASM_OUTFILE);
    free(line);
    return true;
}

/*
    Run tests for the disassembler.
*/
static bool test_dis()
{
    // TODO
    return true;
}

/*
    Run integration tests (i.e., multiple components working together).
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
        FATAL("a single component name is required")

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
    atexit(finalize);
    return func() ? EXIT_SUCCESS : EXIT_FAILURE;
}
