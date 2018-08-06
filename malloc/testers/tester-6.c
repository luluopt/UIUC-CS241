/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#include "tester-utils.h"

#define MIN_ALLOC_SIZE (256 * K * K)
#define MAX_ALLOC_SIZE (K * K * K)

void *malloc_and_break(void *region, int c, size_t len) {
    if (len < MIN_ALLOC_SIZE) {
        return region;
    }

    void *sr1 = realloc(region, len / 3);
    void *sr2 = malloc(len / 3);
    void *sr3 = malloc(len / 3);

    verify_overlap3(sr1, sr2, sr3, len / 3);
    verify(sr1, c, len / 3);

    memset(sr1, 0xab, len / 3);
    memset(sr2, 0xcd, len / 3);
    memset(sr3, 0xef, len / 3);
    free(sr2);

    sr1 = malloc_and_break(sr1, 0xab, len / 3);
    sr3 = malloc_and_break(sr3, 0xef, len / 3);

    sr1 = realloc(sr1, len / 2);
    sr3 = realloc(sr3, len / 2);

    verify(sr1, 0xab, len / 3);
    verify(sr3, 0xef, len / 3);

    memset(sr1, 0xab, len / 2);
    memset(sr3, 0xef, len / 2);

    verify_overlap2(sr1, sr3, len / 2);
    free(sr3);

    sr1 = realloc(sr1, len);
    verify(sr1, 0xab, len / 2);

    memset(sr1, c, len);

    return sr1;
}

int main() {
    malloc(1);

    size_t len = MAX_ALLOC_SIZE;
    while (len > MIN_ALLOC_SIZE) {
        void *mem = malloc(len);
        memset(mem, 0xff, len);
        free(malloc_and_break(mem, 0xff, len));
        len /= 3;
    }

    fprintf(stderr, "Memory was allocated and freed!\n");
    return 0;
}
