/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#ifndef __CS241_REDUCE_H__
#define __CS241_REDUCE_H__

#include "reducers.h"
#include <stdlib.h>

/**
 * Single threaded solution to reduce().
 *
 * This method takes in a `list` of ints and returns a int that is the
 * 'reduced' version of the list.
 *
 * Note: that this function DOES NOT modify the original list.
 *
 * `list`- is a pointer to the begining of an array of ints.
 * `length` - is how many ints are in the array of ints.
 * `reducer_func` - is the reducing function that is applied to the entire list.
 * `base_case` - is the base case for the reducer function.
 */
int reduce(int *list, size_t length, reducer reduce_func, int base_case);
#endif /* __CS241_REDUCE_H__ */
