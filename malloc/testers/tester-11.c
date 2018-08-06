/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#include "tester-utils.h"

#define SIZE (2L * 1024L * M)
#define ITERS 10000

int main() {
    malloc(1);
    int i;
    for (i = 0; i < ITERS; i++) {
        // Write to end
        char *a = malloc(SIZE);
        if (!a)
            return 1;

        verify_write(a, SIZE);
        if (!verify_read(a, SIZE))
            return 1;

        free(a);

        char *b = malloc(SIZE / 2);
        verify_write(b, SIZE / 2);

        char *c = malloc(SIZE / 4);
        verify_write(c, SIZE / 4);

        if (!b || !c)
            return 1;

        if (!verify_read(b, SIZE / 2) || !verify_read(c, SIZE / 4) ||
            overlap(b, SIZE / 2, c, SIZE / 4))
            return 1;

        free(b);
        free(c);
    }

    fprintf(stderr, "Memory was allocated, used, and freed!\n");
    return 0;
}
