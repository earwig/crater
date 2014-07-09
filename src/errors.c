/* Copyright (C) 2014 Ben Kurtovic <ben.kurtovic@gmail.com>
   Released under the terms of the MIT License. See LICENSE for details. */

#include <stdio.h>
#include <stdlib.h>

#include "errors.h"

/* Called after an out-of-memory error. Prints a message and dies. */
void out_of_memory() {
    printf("Error: couldn't allocate memory.\n");
    exit(1);
}
