/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdlib.h>

#include "errors.h"
#include "state.h"
#include "../assembler.h"
#include "../logging.h"

/* Error strings */

static const char *error_types[] = {
    [ET_INCLUDE] = "include directive",
    [ET_PREPROC] = "preprocessor",
    [ET_LAYOUT]  = "memory layout",
    [ET_SYMBOL]  = "symbol table",
    [ET_PARSER]  = "instruction parser"
};

static const char *error_descs[] = {
    [ED_NONE]             = "undefined error",

    [ED_INC_BAD_ARG]      = "missing or invalid argument",
    [ED_INC_DEPTH]        = "maximum include depth exceeded",
    [ED_INC_FILE_READ]    = "couldn't read included file",

    [ED_PP_UNKNOWN]       = "unknown directive",
    [ED_PP_DUPLICATE]     = "multiple values for directive",
    [ED_PP_NO_ARG]        = "missing argument for directive",
    [ED_PP_BAD_ARG]       = "invalid argument for directive",
    [ED_PP_ARG_RANGE]     = "directive argument out of range",

    [ED_LYT_HEADER_RANGE] = "header offset exceeds given ROM size",
    [ED_LYT_DECL_RANGE]   = "declared ROM size in header exceeds actual size",
    [ED_LYT_BOUNDS]       = "location is out of bounds for the ROM size",
    [ED_LYT_BLOCK0]       = "block zero cannot be mapped into a nonzero slot",
    [ED_LYT_SLOTS]        = "multiple slot declarations for block directive",
    [ED_LYT_BLOCK_CROSS]  = "instruction or data extends past block boundary",
    [ED_LYT_OVERLAP]      = "multiple instructions/data occupy same location",
    [ED_LYT_OVERLAP_HEAD] = "location overlaps with ROM header",

    [ED_SYM_DUPE_LABELS]  = "duplicate definitions for label",
    [ED_SYM_NO_LABEL]     = "undefined reference to label",
    [ED_SYM_IS_REGISTER]  = "labels cannot share names with registers",
    [ED_SYM_IS_CONDITION] = "labels cannot share names with condition codes",

    [ED_PS_OP_TOO_LONG]   = "opcode mnemonic is too long (2-4 characters)",
    [ED_PS_OP_TOO_SHORT]  = "opcode mnemonic is too short (2-4 characters)",
    [ED_PS_OP_INVALID]    = "invalid characters in opcode mnemonic",
    [ED_PS_OP_UNKNOWN]    = "unknown opcode mnemonic",
    [ED_PS_TOO_FEW_ARGS]  = "too few arguments for opcode",
    [ED_PS_TOO_MANY_ARGS] = "too many arguments for opcode",
    [ED_PS_ARG_SYNTAX]    = "invalid syntax in argument(s)",
    [ED_PS_ARG0_TYPE]     = "invalid type for first argument",
    [ED_PS_ARG0_BAD_REG]  = "unsupported register as first argument",
    [ED_PS_ARG1_TYPE]     = "invalid type for second argument",
    [ED_PS_ARG1_BAD_REG]  = "unsupported register as second argument",
    [ED_PS_ARG1_RANGE]    = "second argument out of range"
};

/* Internal structs */

struct ASMErrorLine {
    char *data;
    size_t length;
    size_t lineno;
    char *filename;
    struct ASMErrorLine *next;
};
typedef struct ASMErrorLine ASMErrorLine;

struct ErrorInfo {
    ASMErrorType type;
    ASMErrorDesc desc;
    ASMErrorLine *line;
};

/*
    Create an ASMErrorLine object from an ASMLine.
*/
static ASMErrorLine* create_error_line(const ASMLine *line)
{
    ASMErrorLine *el = malloc(sizeof(ASMErrorLine));
    if (!el)
        OUT_OF_MEMORY()

    const char *source = line->original->data;
    size_t length = line->original->length;
    if (!(el->data = malloc(sizeof(char) * length)))
        OUT_OF_MEMORY()

    // Ignore spaces at beginning:
    while (length > 0 && (*source == ' ' || *source == '\t'))
        source++, length--;
    memcpy(el->data, source, length);

    el->length = length;
    el->lineno = line->original->lineno;

    el->filename = strdup(line->filename);
    if (!el->filename)
        OUT_OF_MEMORY()

    el->next = NULL;
    return el;
}

/*
    Create an ErrorInfo object describing a particular error.

    The ErrorInfo object can be printed with error_info_print(), and must be
    freed when done with error_info_destroy().

    This function never fails (OOM triggers an exit()); the caller can be
    confident the returned object is valid.
*/
ErrorInfo* error_info_create(
    const ASMLine *line, ASMErrorType err_type, ASMErrorDesc err_desc)
{
    ErrorInfo *einfo = malloc(sizeof(ErrorInfo));
    if (!einfo)
        OUT_OF_MEMORY()

    einfo->type = err_type;
    einfo->desc = err_desc;
    einfo->line = line ? create_error_line(line) : NULL;
    return einfo;
}

/*
    Add an ASMLine to an ErrorInfo object, as part of a file trace.
*/
void error_info_append(ErrorInfo *einfo, const ASMLine *line)
{
    ASMErrorLine* el = create_error_line(line);
    el->next = einfo->line;
    einfo->line = el;
}

/*
    Print an ErrorInfo object returned by assemble() to the given stream.
*/
void error_info_print(const ErrorInfo *einfo, FILE *file)
{
    ASMErrorLine *line = einfo->line;

    fprintf(file, "error: %s: %s\n", error_types[einfo->type],
            error_descs[einfo->desc]);
    while (line) {
        fprintf(file, "%s:%zu:\n", line->filename, line->lineno);
        fprintf(file, "    %.*s\n", (int) line->length, line->data);
        line = line->next;
    }
}

/*
    Destroy an ErrorInfo object created by assemble().
*/
void error_info_destroy(ErrorInfo *error_info)
{
    if (!error_info)
        return;

    ASMErrorLine *line = error_info->line, *temp;
    while (line) {
        temp = line->next;
        free(line->data);
        free(line->filename);
        free(line);
        line = temp;
    }
    free(error_info);
}
