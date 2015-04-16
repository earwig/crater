/* Copyright (C) 2014-2015 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "io.h"
#include "../logging.h"

/*
    Deallocate a LineBuffer previously created with read_source_file().
*/
void line_buffer_free(LineBuffer *buffer)
{
    Line *line = buffer->lines, *temp;
    while (line) {
        temp = line->next;
        free(line->data);
        free(line);
        line = temp;
    }

    free(buffer->filename);
    free(buffer);
}

/*
    Read the contents of the source file at the given path into a line buffer.

    Return the buffer if reading was successful; it must be freed with
    line_buffer_free() when done. Return NULL if an error occurred while
    reading. If print_errors is true, a message will also be printed to stderr.
*/
LineBuffer* read_source_file(const char *path, bool print_errors)
{
    FILE *fp;
    struct stat st;

    if (!(fp = fopen(path, "r"))) {
        if (print_errors)
            ERROR_ERRNO("couldn't open source file")
        return NULL;
    }

    if (fstat(fileno(fp), &st)) {
        fclose(fp);
        if (print_errors)
            ERROR_ERRNO("couldn't open source file")
        return NULL;
    }
    if (!(st.st_mode & S_IFREG)) {
        fclose(fp);
        if (print_errors)
            ERROR("couldn't open source file: %s", st.st_mode & S_IFDIR ?
                  "Is a directory" : "Is not a regular file")
        return NULL;
    }

    LineBuffer *source = malloc(sizeof(LineBuffer));
    if (!source)
        OUT_OF_MEMORY()

    source->lines = NULL;
    source->filename = strdup(path);
    if (!source->filename)
        OUT_OF_MEMORY()

    Line dummy = {.next = NULL};
    Line *line, *prev = &dummy;
    size_t lineno = 1;

    while (1) {
        char *data = NULL;
        size_t cap = 0;
        ssize_t len;

        if ((len = getline(&data, &cap, fp)) < 0) {
            if (feof(fp))
                break;
            if (errno == ENOMEM)
                OUT_OF_MEMORY()
            if (print_errors)
                ERROR_ERRNO("couldn't read source file")
            fclose(fp);
            source->lines = dummy.next;
            line_buffer_free(source);
            return NULL;
        }

        line = malloc(sizeof(Line));
        if (!line)
            OUT_OF_MEMORY()

        line->data = data;
        line->length = feof(fp) ? len : (len - 1);
        line->lineno = lineno++;
        line->next = NULL;

        prev->next = line;
        prev = line;
    }

    fclose(fp);
    source->lines = dummy.next;
    return source;
}

/*
    Write an assembled binary file to the given path.

    Return whether the file was written successfully. On error, a message is
    printed to stderr.
*/
bool write_binary_file(const char *path, const uint8_t *data, size_t size)
{
    FILE *fp;
    if (!(fp = fopen(path, "wb"))) {
        ERROR_ERRNO("couldn't open destination file")
        return false;
    }

    if (!fwrite(data, size, 1, fp)) {
        fclose(fp);
        ERROR_ERRNO("couldn't write to destination file")
        return false;
    }

    fclose(fp);
    return true;
}
