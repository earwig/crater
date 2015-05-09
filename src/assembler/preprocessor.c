/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "preprocessor.h"
#include "directives.h"
#include "errors.h"
#include "io.h"
#include "parse_util.h"
#include "../logging.h"
#include "../rom.h"
#include "../util.h"

#define MAX_INCLUDE_DEPTH 16

/* Helper macros for preprocess() */

#define FAIL_ON_COND_(cond, err_desc)                                         \
    if ((cond)) {                                                             \
        ei = error_info_create(line, ET_PREPROC, err_desc);                   \
        goto cleanup;                                                         \
    }

#define CALL_GENERIC_PARSER_(arg_type)                                        \
    dparse_##arg_type((arg_type*) &arg, line, directive)

#define CALL_SPECIFIC_PARSER_(arg_type, parser)                               \
    dparse_##parser((arg_type*) &arg, line, directive)

#define DISPATCH_(first, second, target, ...) target

#define CALL_PARSER_(...)                                                     \
    DISPATCH_(__VA_ARGS__, CALL_SPECIFIC_PARSER_, CALL_GENERIC_PARSER_,       \
              __VA_ARGS__)(__VA_ARGS__)

#define VALIDATE(func)                                                        \
    FAIL_ON_COND_(!(func(arg)), ED_PP_BAD_ARG)

#define CHECK_RANGE(bound)                                                    \
    FAIL_ON_COND_(arg > bound, ED_PP_ARG_RANGE)

#define USE_PARSER(...)                                                       \
    FAIL_ON_COND_(!CALL_PARSER_(__VA_ARGS__), ED_PP_BAD_ARG)

#define PARSER_BRANCH(arg_type, true_part, false_part)                        \
    if (CALL_PARSER_(arg_type)) {true_part} else {false_part}

#define SAVE_LINE(target)                                                     \
    if (!dir_is_auto) target = line;

#define BEGIN_DIRECTIVE_BLOCK                                                 \
    ssize_t first_ctr = -1;                                                   \
    if (0) {}

#define BEGIN_DIRECTIVE(d, arg_type, dest_loc, auto_val)                      \
    else if (first_ctr++, IS_DIRECTIVE(line, d)) {                            \
        directive = d;                                                        \
        FAIL_ON_COND_(!DIRECTIVE_HAS_ARG(line, directive), ED_PP_NO_ARG)      \
        arg_type arg = 0;                                                     \
        arg_type* dest = &(dest_loc);                                         \
        bool dir_is_auto = DIRECTIVE_IS_AUTO(line, directive);                \
        if (dir_is_auto) {                                                    \
            arg = auto_val;                                                   \
        } else {

#define END_DIRECTIVE                                                         \
        }                                                                     \
        if (firsts[first_ctr] && *dest != arg) {                              \
            ei = error_info_create(line, ET_PREPROC, ED_PP_DUPLICATE);        \
            error_info_append(ei, firsts[first_ctr]);                         \
            goto cleanup;                                                     \
        }                                                                     \
        *dest = arg;                                                          \
        firsts[first_ctr] = line;                                             \
    }

#define END_DIRECTIVE_BLOCK                                                   \
    else FAIL_ON_COND_(true, ED_PP_UNKNOWN)

/*
    Functions similar memcpy, but lowercases the characters along the way.
*/
static void memcpy_lc(char *restrict dst, const char *restrict src, size_t n)
{
    while (n-- > 0) {
        char c = *(src++);
        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
        *(dst++) = c;
    }
}

/*
    Preprocess a single source line for labels.

    Return the index of first non-whitespace non-label character. *head_ptr is
    updated to the first label in sequence, and *tail_ptr to the last. Both
    will be set to NULL if the line doesn't contain labels.
*/
static size_t read_labels(
    const char *source, size_t length, ASMLine **head_ptr, ASMLine **tail_ptr)
{
    size_t start = 0, i, nexti;
    while (start < length && (source[start] == ' ' || source[start] == '\t'))
        start++;

    i = start;
    while (i < length && is_valid_symbol_char(source[i], i == start))
        i++;

    if (i == start || i == length || source[i] != ':') {
        *head_ptr = NULL;
        *tail_ptr = NULL;
        return 0;
    }

    ASMLine *line = cr_malloc(sizeof(ASMLine));
    line->data = cr_malloc(sizeof(char) * (i - start + 1));
    memcpy_lc(line->data, source + start, i - start + 1);
    line->length = i - start + 1;
    line->is_label = true;

    nexti = read_labels(source + i + 1, length - i - 1, &line->next, tail_ptr);
    *head_ptr = line;
    if (!nexti)
        *tail_ptr = line;
    return i + 1 + nexti;
}

/*
    Preprocess a single source line (source, length) into one or more ASMLines.

    Only the data, length, is_label, and next fields of the ASMLine objects are
    populated. The normalization process strips comments, makes various
    adjustments outside of string literals (converts tabs to spaces, lowercases
    all alphabetical characters, and removes runs of multiple spaces), among
    other things.

    Return NULL if an ASM line was not generated from the source, i.e. if it is
    blank after being stripped.
*/
static ASMLine* normalize_line(const char *source, size_t length)
{
    ASMLine *head, *tail;
    size_t offset = read_labels(source, length, &head, &tail);

    source += offset;
    length -= offset;

    char *data = cr_malloc(sizeof(char) * length);
    size_t si, di, slashes = 0;
    bool has_content = false, space_pending = false, in_string = false;
    for (si = di = 0; si < length; si++) {
        char c = source[si];

        if (c == '\\')
            slashes++;
        else
            slashes = 0;

        if (in_string) {
            if (c == '"' && (slashes % 2) == 0)
                in_string = false;

            data[di++] = c;
        } else {
            if (c == ';')
                break;
            if (c == '"' && (slashes % 2) == 0)
                in_string = true;
            if (c >= 'A' && c <= 'Z')
                c += 'a' - 'A';

            if (c == ' ' || c == '\t')
                space_pending = true;
            else {
                if (space_pending) {
                    if (has_content)
                        data[di++] = ' ';
                    space_pending = false;
                }
                has_content = true;
                data[di++] = c;
            }
        }
    }

    if (!has_content) {
        free(data);
        return head;
    }

    ASMLine *line = cr_malloc(sizeof(ASMLine));
    data = cr_realloc(data, sizeof(char) * di);
    line->data = data;
    line->length = di;
    line->is_label = false;
    line->next = NULL;

    if (head) {  // Line has labels, so link the main part up
        tail->next = line;
        return head;
    }
    return line;
}

/*
    Read and return the target path from an include directive.

    This function allocates a buffer to store the filename; it must be free()'d
    after calling read_source_file(). If a syntax error occurs while trying to
    read the path, it returns NULL.
*/
static char* read_include_path(const ASMLine *line)
{
    size_t maxlen = strlen(line->filename) + line->length, i, baselen;
    if (maxlen >= INT_MAX)  // Allows us to safely downcast to int later
        return NULL;

    char *path = cr_malloc(sizeof(char) * maxlen), *base, *dup;
    if (!(i = DIRECTIVE_OFFSET(line, DIR_INCLUDE)))
        goto error;
    if (line->length - i <= 3)  // Not long enough to hold a non-zero argument
        goto error;
    if (line->data[i++] != ' ')
        goto error;
    if (!parse_string(&base, &baselen, line->data + i, line->length - i))
        goto error;

    dup = cr_strdup(line->filename);

    // TODO: should normalize filenames in some way to prevent accidental dupes
    snprintf(path, maxlen, "%s/%.*s", dirname(dup), (int) baselen, base);
    free(dup);
    free(base);
    return path;

    error:
    free(path);
    return NULL;
}

/*
    Build a LineBuffer into a ASMLines, normalizing them along the way.

    This function operates recursively to handle includes, but handles no other
    preprocessor directives.

    On success, NULL is returned; *head points to the head of the new ASMLine
    list, and *tail to its tail (assuming it is non-NULL). On error, an
    ErrorInfo object is returned, and *head and *tail are not modified.
    *includes may be updated in either case.
*/
static ErrorInfo* build_asm_lines(
    const LineBuffer *source, ASMLine **head, ASMLine **tail,
    ASMInclude **includes, unsigned depth)
{
    ErrorInfo *ei;
    ASMLine dummy = {.next = NULL};
    ASMLine *line, *prev = &dummy, *temp;
    const Line *orig, *next_orig = source->lines;

    while ((orig = next_orig)) {
        line = temp = normalize_line(orig->data, orig->length);
        next_orig = orig->next;
        if (!line)
            continue;

        // Populate ASMLine fields not set by normalize_line():
        while (temp) {
            temp->original = orig;
            temp->filename = source->filename;
            temp = temp->next;
        }

        // If there are multiple ASMLines, all but the last must be labels:
        if (line->next) {
            while (line->next) {
                prev->next = line;
                prev = line;
                line = line->next;
            }
            prev->next = NULL;  // Disconnect in case the line is an .include
        }

        if (IS_DIRECTIVE(line, DIR_INCLUDE)) {
            char *path = read_include_path(line);
            if (!path) {
                ei = error_info_create(line, ET_INCLUDE, ED_INC_BAD_ARG);
                goto error;
            }

            if (depth >= MAX_INCLUDE_DEPTH) {
                free(path);
                ei = error_info_create(line, ET_INCLUDE, ED_INC_DEPTH);
                goto error;
            }

            DEBUG("- reading included file: %s", path)
            LineBuffer *incbuffer = read_source_file(path, false);
            free(path);
            if (!incbuffer) {
                ei = error_info_create(line, ET_INCLUDE, ED_INC_FILE_READ);
                goto error;
            }

            ASMInclude *include = cr_malloc(sizeof(ASMInclude));
            include->lines = incbuffer;
            include->next = *includes;
            *includes = include;

            ASMLine *inchead, *inctail;
            if ((ei = build_asm_lines(incbuffer, &inchead, &inctail, includes,
                                      depth + 1))) {
                error_info_append(ei, line);
                goto error;
            }

            prev->next = inchead;
            prev = inctail;
            asm_lines_free(line);  // Destroy only the .include line
        }
        else {
            prev->next = line;
            prev = line;
        }
    }

    *head = dummy.next;
    if (tail)
        *tail = prev;
    return NULL;

    error:
    asm_lines_free(line);
    asm_lines_free(dummy.next);
    return ei;
}

/*
    Return whether the given ROM size is valid.
*/
static inline bool is_rom_size_valid(size_t size)
{
    return size_bytes_to_code(size) != INVALID_SIZE_CODE;
}

/*
    Return whether the given header offset is a valid location.
*/
static inline bool is_header_offset_valid(uint16_t offset)
{
    return offset == 0x7FF0 || offset == 0x3FF0 || offset == 0x1FF0;
}

/*
    Preprocess the LineBuffer into ASMLines. Change some state along the way.

    This function processes include directives, so read_source_file() may be
    called multiple times (along with the implications that has), and
    state->includes may be modified.

    On success, NULL is returned. On error, an ErrorInfo object is returned.
    state->lines and state->includes may still be modified.
*/
ErrorInfo* preprocess(AssemblerState *state, const LineBuffer *source)
{
    ErrorInfo* ei = NULL;
    DEBUG("Running preprocessor:")

    if ((ei = build_asm_lines(source, &state->lines, NULL, &state->includes, 0)))
        return ei;

    const ASMLine *firsts[NUM_DIRECTIVES];
    for (size_t i = 0; i < NUM_DIRECTIVES; i++)
        firsts[i] = NULL;

    ASMLine dummy = {.next = state->lines};
    ASMLine *prev, *line = &dummy, *next = state->lines, *condemned = NULL;
    const ASMLine *rom_size_line = NULL, *rom_declsize_line = NULL;
    const char *directive;

    while ((prev = line, line = next)) {
        next = line->next;
        if (line->is_label || line->data[0] != DIRECTIVE_MARKER)
            continue;
        if (IS_LOCAL_DIRECTIVE(line))
            continue;  // "Local" directives are handled by the tokenizer

        DEBUG("- handling directive: %.*s", (int) line->length, line->data)

        BEGIN_DIRECTIVE_BLOCK

        BEGIN_DIRECTIVE(DIR_ROM_SIZE, size_t, state->rom_size, 0)
            PARSER_BRANCH(uint32_t, {}, {
                USE_PARSER(uint32_t, rom_size)
            })
            VALIDATE(is_rom_size_valid)
            SAVE_LINE(rom_size_line)
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_ROM_HEADER, size_t, state->header.offset, DEFAULT_HEADER_OFFSET)
            USE_PARSER(uint16_t)
            VALIDATE(is_header_offset_valid)
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_ROM_CHECKSUM, bool, state->header.checksum, true)
            USE_PARSER(bool)
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_ROM_PRODUCT, uint32_t, state->header.product_code, 0)
            USE_PARSER(uint32_t)
            CHECK_RANGE(160000)
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_ROM_VERSION, uint8_t, state->header.version, 0)
            USE_PARSER(uint8_t)
            CHECK_RANGE(0x10)
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_ROM_REGION, uint8_t, state->header.region, DEFAULT_REGION)
            PARSER_BRANCH(uint8_t, {
                CHECK_RANGE(0x10)
                VALIDATE(region_code_to_string)
            }, {
                USE_PARSER(uint8_t, region_string)
            })
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_ROM_DECLSIZE, uint8_t, state->header.rom_size, DEFAULT_DECLSIZE)
            PARSER_BRANCH(uint8_t, {
                CHECK_RANGE(0x10)
                VALIDATE(size_code_to_bytes)
            }, {
                USE_PARSER(uint8_t, size_code)
            })
            SAVE_LINE(rom_declsize_line)
        END_DIRECTIVE

        BEGIN_DIRECTIVE(DIR_CROSS_BLOCKS, bool, state->cross_blocks, false)
            USE_PARSER(bool)
        END_DIRECTIVE

        END_DIRECTIVE_BLOCK

        // Remove directive from lines, and schedule it for deletion:
        line->next = condemned;
        condemned = line;
        prev->next = next;
        line = prev;
    }

    if (rom_size_line && state->header.offset + HEADER_SIZE > state->rom_size) {
        // TODO: maybe should force offset to be explicit, otherwise autofix
        ei = error_info_create(rom_size_line, ET_LAYOUT, ED_LYT_HEADER_RANGE);
        goto cleanup;
    }

    if (rom_size_line && rom_declsize_line &&
            size_code_to_bytes(state->header.rom_size) > state->rom_size) {
        ei = error_info_create(rom_size_line, ET_LAYOUT, ED_LYT_DECL_RANGE);
        error_info_append(ei, rom_declsize_line);
        goto cleanup;
    }

    if (!rom_declsize_line)  // Mark as undefined, for resolve_defaults()
        state->header.rom_size = INVALID_SIZE_CODE;

    cleanup:
    asm_lines_free(condemned);
    state->lines = dummy.next;  // Fix list head if first line was a directive
    return ei;
}
