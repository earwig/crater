/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "assembler.h"
#include "logging.h"

/*
    Deallocate a LineBuffer previously created with read_source_file().
*/
static void free_line_buffer(LineBuffer *buffer)
{
    for (size_t i = 0; i < buffer->length; i++)
        free(buffer->lines[i].data);
    free(buffer->lines);
    free(buffer);
}

/*
    Read the contents of the source file at the given path into a line buffer.

    Return the buffer if reading was successful; it must be freed with
    free_line_buffer() when done. Return NULL if an error occurred while
    reading. A message will be printed to stderr in this case.
*/
static LineBuffer* read_source_file(const char *path)
{
    FILE* fp;
    struct stat st;

    if (!(fp = fopen(path, "r"))) {
        ERROR_ERRNO("couldn't open source file")
        return NULL;
    }

    if (fstat(fileno(fp), &st)) {
        fclose(fp);
        ERROR_ERRNO("couldn't open source file")
        return NULL;
    }
    if (!(st.st_mode & S_IFREG)) {
        fclose(fp);
        ERROR("couldn't open source file: %s", st.st_mode & S_IFDIR ?
              "Is a directory" : "Is not a regular file")
        return NULL;
    }

    size_t capacity = 16;
    LineBuffer *source = malloc(sizeof(LineBuffer));
    if (!source)
        OUT_OF_MEMORY()

    source->length = 0;
    source->lines = malloc(sizeof(Line) * capacity);
    if (!source->lines)
        OUT_OF_MEMORY()

    while (1) {
        char *line = NULL;
        size_t lcap = 0;
        ssize_t len;

        if ((len = getline(&line, &lcap, fp)) < 0) {
            if (feof(fp))
                break;
            if (errno == ENOMEM)
                OUT_OF_MEMORY()
            ERROR_ERRNO("couldn't read source file")
            free_line_buffer(source);
            source = NULL;
            break;
        }

        if (capacity <= source->length + 1) {
            capacity <<= 2;
            source->lines = realloc(source->lines, sizeof(Line) * capacity);
            if (!source->lines)
                OUT_OF_MEMORY()
        }

        source->lines[source->length++] = (Line) {line, len};
        if (feof(fp)) {
            source->lines[source->length].length--;
            break;
        }
    }

    fclose(fp);
    return source;
}

/*
    Write an assembled binary file to the given path.

    Return whether the file was written successfully. On error, a message is
    printed to stderr.
*/
static bool write_binary_file(const char *path, const uint8_t *data, size_t size)
{
    // TODO
    return false;
}

/*
    Print an ErrorInfo object returned by assemble() to the given file.

    The same LineBuffer passed to assemble() should be passed to this function.
    Passing NULL if it is unavailable will still work, but source code snippets
    where errors were noted will not be printed.
*/
void error_info_print(const ErrorInfo *error_info, FILE *file, const LineBuffer *source)
{
    // TODO
}

/*
    Destroy an ErrorInfo object created by assemble().
*/
void error_info_destroy(ErrorInfo *error_info)
{
    if (!error_info)
        return;

    // TODO
    free(error_info);
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
    // TODO
    return 0;
}

/*
    Assemble the z80 source code at the input path into a binary file.

    Return true if the operation was a success and false if it was a failure.
    Errors are printed to STDOUT; if the operation was successful then nothing
    is printed.
*/
bool assemble_file(const char *src_path, const char *dst_path)
{
    LineBuffer *source = read_source_file(src_path);
    if (!source)
        return false;

    uint8_t *binary;
    ErrorInfo *error_info;
    size_t size = assemble(source, &binary, &error_info);

    if (!size) {
        error_info_print(error_info, stderr, source);
        error_info_destroy(error_info);
        free_line_buffer(source);
        return false;
    }

    bool success = write_binary_file(dst_path, binary, size);
    free(binary);
    free_line_buffer(source);
    return success;
}
