/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "assembler/errors.h"
#include "assembler/io.h"
#include "assembler/state.h"
#include "logging.h"

#define DIRECTIVE_MARKER    '.'
#define DIR_INCLUDE         ".include"
#define DIR_ORIGIN          ".org"
#define DIR_OPTIMIZER       ".optimizer"
#define DIR_ROM_SIZE        ".rom_size"
#define DIR_ROM_HEADER      ".rom_header"
#define DIR_ROM_CHECKSUM    ".rom_checksum"
#define DIR_ROM_PRODUCT     ".rom_product"
#define DIR_ROM_VERSION     ".rom_version"
#define DIR_ROM_REGION      ".rom_region"
#define DIR_ROM_DECLSIZE    ".rom_declsize"

#define DIRECTIVE_HAS_ARG(line, d) ((line)->length > strlen(d))

#define IS_DIRECTIVE(line, d)                                                 \
    (((line)->length >= strlen(d)) &&                                         \
    !strncmp((line)->data, d, strlen(d)) &&                                   \
    (!DIRECTIVE_HAS_ARG(line, d) || (line)->data[strlen(d)] == ' '))

#define DIRECTIVE_OFFSET(line, d)                                             \
    (DIRECTIVE_HAS_ARG(line, d) ? strlen(d) : 0)

/*
    Preprocess a single source line (source, length) into a normalized ASMLine.

    *Only* the data and length fields in the ASMLine object are populated. The
    normalization process converts tabs to spaces, lowercases all alphabetical
    characters, and removes runs of multiple spaces (outside of string
    literals), strips comments, and other things.

    Return NULL if an ASM line was not generated from the source, i.e. if it is
    blank after being stripped.
*/
static ASMLine* normalize_line(const char *source, size_t length)
{
    char *data = malloc(sizeof(char) * length);
    if (!data)
        OUT_OF_MEMORY()

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
        return NULL;
    }

    ASMLine *line = malloc(sizeof(ASMLine));
    if (!line)
        OUT_OF_MEMORY()

    data = realloc(data, sizeof(char) * di);
    if (!data)
        OUT_OF_MEMORY()

    line->data = data;
    line->length = di;
    return line;
}

/*
    Read and return the target path from an include directive.

    This function allocates a buffer to store the filename; it must be free()'d
    after calling read_source_file(). If a syntax error occurs while trying to
    read the path, it returns NULL.
*/
char* read_include_path(const ASMLine *line)
{
    size_t maxlen = strlen(line->filename) + line->length, i, start, slashes;
    if (maxlen >= INT_MAX)  // Allows us to safely downcast to int later
        return NULL;

    char *path = malloc(sizeof(char) * maxlen);
    if (!path)
        OUT_OF_MEMORY()

    if (!(i = DIRECTIVE_OFFSET(line, DIR_INCLUDE)))
        goto error;
    if (line->length - i <= 4)  // Not long enough to hold a non-zero argument
        goto error;
    if (line->data[i++] != ' ' || line->data[i++] != '"')
        goto error;

    // TODO: parse escaped characters properly
    for (start = i, slashes = 0; i < line->length; i++) {
        if (line->data[i] == '"' && (slashes % 2) == 0)
            break;
        if (line->data[i] == '\\')
            slashes++;
        else
            slashes = 0;
    }

    if (i != line->length - 1)  // Junk present after closing quote
        goto error;

    char *dup = strdup(line->filename);
    if (!dup)
        OUT_OF_MEMORY()

    // TODO: should normalize filenames in some way to prevent accidental dupes
    snprintf(path, maxlen, "%s/%.*s", dirname(dup), (int) (i - start),
             line->data + start);
    free(dup);
    return path;

    error:
    free(path);
    return NULL;
}

/*
    Return whether the given path has already been loaded.
*/
static bool path_has_been_loaded(
    const char *path, const LineBuffer *root, const ASMInclude *include)
{
    if (!strcmp(path, root->filename))
        return true;

    while (include) {
        if (!strcmp(path, include->lines->filename))
            return true;
        include = include->next;
    }
    return false;
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
    const LineBuffer *root, const LineBuffer *source, ASMLine **head,
    ASMLine **tail, ASMInclude **includes)
{
    ASMLine dummy = {.next = NULL};
    ASMLine *line, *prev = &dummy;
    const Line *orig, *next_orig = source->lines;

    while ((orig = next_orig)) {
        line = normalize_line(orig->data, orig->length);
        next_orig = orig->next;
        if (!line)
            continue;

        // Populate ASMLine fields not set by normalize_line():
        line->original = orig;
        line->filename = source->filename;
        line->next = NULL;

        if (IS_DIRECTIVE(line, DIR_INCLUDE)) {
            ErrorInfo *ei;
            char *path = read_include_path(line);
            if (!path) {
                ei = error_info_create(line, ET_INCLUDE, ED_INC_BAD_ARG);
                asm_lines_free(line);
                asm_lines_free(dummy.next);
                return ei;
            }

            if (path_has_been_loaded(path, root, *includes)) {
                ei = error_info_create(line, ET_INCLUDE, ED_INC_RECURSION);
                asm_lines_free(line);
                asm_lines_free(dummy.next);
                free(path);
                return ei;
            }

            DEBUG("- reading included file: %s", path)
            LineBuffer *incbuffer = read_source_file(path, false);
            free(path);
            if (!incbuffer) {
                ei = error_info_create(line, ET_INCLUDE, ED_INC_FILE_READ);
                asm_lines_free(line);
                asm_lines_free(dummy.next);
                return ei;
            }

            ASMInclude *include = malloc(sizeof(ASMInclude));
            if (!include)
                OUT_OF_MEMORY()

            include->lines = incbuffer;
            include->next = *includes;
            *includes = include;

            ASMLine *inchead, *inctail;
            if ((ei = build_asm_lines(root, incbuffer, &inchead, &inctail,
                                      includes))) {
                error_info_append(ei, line);
                asm_lines_free(line);
                asm_lines_free(dummy.next);
                return ei;
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
}

/*
    Read in a boolean argument from the given line and store it in *result.

    auto_val is used if the argument's value is "auto". Return true on success
    and false on failure; in the latter case, *result is not modified.
*/
static inline bool read_bool_argument(
    bool *result, const ASMLine *line, const char *directive, bool auto_val)
{
    const char *arg = line->data + (DIRECTIVE_OFFSET(line, directive) + 1);
    ssize_t len = line->length - (DIRECTIVE_OFFSET(line, directive) + 1);

    if (len <= 0 || len > 5)
        return false;

    switch (len) {
        case 1:  // 0, 1
            if (*arg == '0' || *arg == '1')
                return (*result = *arg - '0'), true;
            return false;
        case 2:  // on
            if (!strncmp(arg, "on", 2))
                return (*result = true), true;
            return false;
        case 3:  // off
            if (!strncmp(arg, "off", 3))
                return (*result = false), true;
            return false;
        case 4:  // true, auto
            if (!strncmp(arg, "true", 4))
                return (*result = true), true;
            if (!strncmp(arg, "auto", 4))
                return (*result = auto_val), true;
            return false;
        case 5:  // false
            if (!strncmp(arg, "false", 5))
                return (*result = false), true;
            return false;
    }
    return false;
}

/*
    Preprocess the LineBuffer into ASMLines. Change some state along the way.

    This function processes include directives, so read_source_file() may be
    called multiple times (along with the implications that has), and
    state->includes may be modified.

    On success, NULL is returned. On error, an ErrorInfo object is returned.
    state->lines and state->includes may still be modified.
*/
static ErrorInfo* preprocess(AssemblerState *state, const LineBuffer *source)
{
    // state->header.offset             <-- check in list of acceptable values
    // state->header.product_code       <-- range check
    // state->header.version            <-- range check
    // state->header.region             <-- string conversion, check
    // state->header.rom_size           <-- value/range check
    // state->rom_size                  <-- value check

    // if giving rom size, check header offset is in rom size range
    // if giving reported and actual rom size, check reported is <= actual

#define CATCH_DUPES(line, first, oldval, newval)                              \
    if (first && oldval != newval) {                                          \
        ei = error_info_create(line, ET_PREPROC, ED_PP_DUPLICATE);            \
        error_info_append(ei, first);                                         \
        asm_lines_free(condemned);                                            \
        return ei;                                                            \
    }                                                                         \
    oldval = newval;                                                          \
    first = line;

#define REQUIRE_ARG(line, d)                                                  \
    if (!DIRECTIVE_HAS_ARG(line, d)) {                                        \
        asm_lines_free(condemned);                                            \
        return error_info_create(line, ET_PREPROC, ED_PP_NO_ARG);             \
    }

#define VALIDATE(retval)                                                      \
    if (!(retval)) {                                                          \
        asm_lines_free(condemned);                                            \
        return error_info_create(line, ET_PREPROC, ED_PP_BAD_ARG);            \
    }

    DEBUG("Running preprocessor:")

    ErrorInfo* ei;
    if ((ei = build_asm_lines(source, source, &state->lines, NULL,
                              &state->includes)))
        return ei;

    ASMLine dummy = {.next = state->lines};
    ASMLine *prev, *line = &dummy, *next = state->lines, *condemned = NULL;
    const ASMLine *first_optimizer = NULL, *first_checksum = NULL;

    while ((prev = line, line = next)) {
        next = line->next;
        if (line->data[0] == DIRECTIVE_MARKER) {
            if (IS_DIRECTIVE(line, DIR_ORIGIN))
                continue;  // Origins are handled by tokenizer

            DEBUG("- handling directive: %.*s", (int) line->length, line->data)

            if (IS_DIRECTIVE(line, DIR_OPTIMIZER)) {
                REQUIRE_ARG(line, DIR_OPTIMIZER)
                bool arg;
                VALIDATE(read_bool_argument(&arg, line, DIR_OPTIMIZER, false))
                CATCH_DUPES(line, first_optimizer, state->optimizer, arg)
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_SIZE)) {
                // TODO
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_HEADER)) {
                // TODO
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_CHECKSUM)) {
                REQUIRE_ARG(line, DIR_ROM_CHECKSUM)
                bool arg;
                VALIDATE(read_bool_argument(&arg, line, DIR_ROM_CHECKSUM, true))
                CATCH_DUPES(line, first_checksum, state->header.checksum, arg)
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_PRODUCT)) {
                // TODO
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_VERSION)) {
                // TODO
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_REGION)) {
                // TODO
            }
            else if (IS_DIRECTIVE(line, DIR_ROM_DECLSIZE)) {
                // TODO
            }
            else {
                asm_lines_free(condemned);
                return error_info_create(line, ET_PREPROC, ED_PP_UNKNOWN);
            }

            // Remove directive from lines, and schedule it for deletion:
            line->next = condemned;
            condemned = line;
            prev->next = next;
            line = prev;
        }
    }

    state->rom_size = 8;  // TODO

    asm_lines_free(condemned);
    state->lines = dummy.next;  // Fix list head if first line was a directive

#ifdef DEBUG_MODE
    DEBUG("Dumping ASMLines:")
    const ASMLine *temp = state->lines;
    while (temp) {
        DEBUG("- %-40.*s [%s:%02zu]", (int) temp->length, temp->data,
              temp->filename, temp->original->lineno)
        temp = temp->next;
    }
#endif

    return NULL;

#undef VALIDATE
#undef REQUIRE_ARG
#undef CATCH_DUPES
}

/*
    Tokenize ASMLines into ASMInstructions.

    On success, state->instructions is modified and NULL is returned. On error,
    an ErrorInfo object is returned and state->instructions is not modified.
    state->symtable may or may not be modified regardless of success.
*/
static ErrorInfo* tokenize(AssemblerState *state)
{
    // TODO

    // verify no instructions clash with header offset
    // if rom size is set, verify nothing overflows

    return NULL;
}

/*
    Resolve default placeholder values in assembler state, such as ROM size.

    On success, no new heap objects are allocated. On error, an ErrorInfo
    object is returned.
*/
static ErrorInfo* resolve_defaults(AssemblerState *state)
{
    // TODO

    // if (!state.rom_size)
            // set to max possible >= 32 KB, or error if too many instructions
            // if (state.header.rom_size)
                    // check reported rom size is <= actual rom size

    // if (!state.header.rom_size)
            // set to actual rom size

    return NULL;
}

/*
    Resolve symbol placeholders in instructions such as jumps and branches.

    On success, no new heap objects are allocated. On error, an ErrorInfo
    object is returned.
*/
static ErrorInfo* resolve_symbols(AssemblerState *state)
{
    // TODO

    return NULL;
}

/*
    Convert finalized ASMInstructions into a binary data block.

    This function should never fail.
*/
static void serialize_binary(AssemblerState *state, uint8_t *binary)
{
    // TODO

    for (size_t i = 0; i < state->rom_size; i++)
        binary[i] = 'X';
}

/*
    Assemble the z80 source code in the source code buffer into binary data.

    If successful, return the size of the assembled binary data and change
    *binary_ptr to point to the assembled ROM data buffer. *binary_ptr must be
    free()'d when finished.

    If an error occurred, return 0 and update *ei_ptr to point to an ErrorInfo
    object which can be shown to the user with error_info_print(). The
    ErrorInfo object must be destroyed with error_info_destroy() when finished.

    In either case, only one of *binary_ptr and *ei_ptr is modified.
*/
size_t assemble(const LineBuffer *source, uint8_t **binary_ptr, ErrorInfo **ei_ptr)
{
    AssemblerState state;
    ErrorInfo *error_info;
    size_t retval = 0;

    state_init(&state);

    if ((error_info = preprocess(&state, source)))
        goto error;

    if (!(state.symtable = malloc(sizeof(ASMSymbolTable))))
        OUT_OF_MEMORY()
    for (size_t bucket = 0; bucket < SYMBOL_TABLE_BUCKETS; bucket++)
        state.symtable->buckets[bucket] = NULL;

    if ((error_info = tokenize(&state)))
        goto error;

    if ((error_info = resolve_defaults(&state)))
        goto error;

    if ((error_info = resolve_symbols(&state)))
        goto error;

    uint8_t *binary = malloc(sizeof(uint8_t) * state.rom_size);
    if (!binary)
        OUT_OF_MEMORY()

    serialize_binary(&state, binary);
    *binary_ptr = binary;
    retval = state.rom_size;
    goto cleanup;

    error:
    *ei_ptr = error_info;

    cleanup:
    asm_lines_free(state.lines);
    asm_includes_free(state.includes);
    asm_instructions_free(state.instructions);
    asm_symtable_free(state.symtable);
    return retval;
}

/*
    Assemble the z80 source code at the input path into a binary file.

    Return true if the operation was a success and false if it was a failure.
    Errors are printed to STDOUT; if the operation was successful then nothing
    is printed.
*/
bool assemble_file(const char *src_path, const char *dst_path)
{
    LineBuffer *source = read_source_file(src_path, true);
    if (!source)
        return false;

    uint8_t *binary;
    ErrorInfo *error_info;
    size_t size = assemble(source, &binary, &error_info);
    line_buffer_free(source);

    if (!size) {
        error_info_print(error_info, stderr);
        error_info_destroy(error_info);
        return false;
    }

    bool success = write_binary_file(dst_path, binary, size);
    free(binary);
    return success;
}
