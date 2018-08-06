/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#include "reduce.h"
#include <stdlib.h>

int reduce(int *list, size_t length, reducer reduce_func, int base_case) {

    int result = base_case;

    for (size_t i = 0; i < length; ++i) {
        result = reduce_func(result, list[i]);
    }

    return result;
}
