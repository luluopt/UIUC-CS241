/**
 * Parallel Make
 * CS 241 - Fall 2017
 */

#include "format.h"
#include <stdio.h>

void print_cycle_failure(char *target) {
    printf("parmake: dropped build target '%s' due to circular dependencies\n",
           target);
    fflush(stdout);
}
