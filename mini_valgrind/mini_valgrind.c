/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2017
 */

#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    return NULL;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    return NULL;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    return NULL;
}

void mini_free(void *payload) {
    // your code here
}
