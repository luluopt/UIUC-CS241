/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#include "tester-utils.h"

#define TOTAL_ALLOCS 6000
#define ALLOC_SIZE 1024 * 1024

int main() {
    malloc(1);

    int i;
    void *ptr = NULL;

    for (i = 0; i < TOTAL_ALLOCS; i++) {
        ptr = malloc(ALLOC_SIZE);
        if (ptr == NULL) {
            fprintf(stderr, "Memory failed to allocate!\n");
            return 1;
        }

        memset(ptr, 0xab, ALLOC_SIZE);

        free(ptr);
    }

    fprintf(stderr, "Memory was allocated and freed!\n");
    return 0;
}
