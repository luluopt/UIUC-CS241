/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#include "tester-utils.h"

#define SIZE (128 * M)
#define ITERS 100000

int main() {
    malloc(1);

    int i;
    for (i = 0; i < ITERS; i++) {
        char *a = malloc(SIZE + i);
        if (!a)
            return 1;

        verify_write(a, SIZE);

        int *b = malloc(SIZE + i);
        if (!b)
            return 1;

        verify_write(a, SIZE + i);

        if (!verify_read(a, SIZE))
            return 1;
        if (!verify_read(a, SIZE + i))
            return 1;

        free(a);
        free(b);

        a = malloc(2 * (SIZE + i));
        if (!a)
            return 1;

        verify_write(a, SIZE);
        if (!verify_read(a, SIZE))
            return 1;

        free(a);
    }

    fprintf(stderr, "Memory was allocated, used, and freed!\n");
    return 0;
}
