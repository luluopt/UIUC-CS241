/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#include "tester-utils.h"

#define SIZE (1024L * M)
#define ITERS 10

int main() {
    malloc(1);
    int i;
    for (i = 0; i < ITERS; i++) {
        // Write to end
        char *a = calloc(SIZE, sizeof(char));
        if (!a)
            return 1;

        verify_clean(a, SIZE);
        verify_write(a, SIZE);
        if (!verify_read(a, SIZE))
            return 1;

        free(a);

        char *b = calloc(SIZE / 2, sizeof(char));
        verify_clean(b, SIZE / 2);
        verify_write(b, SIZE / 2);

        char *c = calloc(SIZE / 4, sizeof(char));
        verify_clean(c, SIZE / 4);
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
