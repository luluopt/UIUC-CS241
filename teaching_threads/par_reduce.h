/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#ifndef __CS241_PAR_REDUCE_H__
#define __CS241_PAR_REDUCE_H__

#include "reducers.h"

/**
 * multi-threaded solution to reduce().
 *
 * This method takes in a `list` of ints and returns a int that is a "reduced"
 * version of the list, but does so with `num_threads` threads.
 *
 * Note: that this function DOES NOT modify the original list.
 *
 * `list`- is a pointer to the begining of an array of ints.
 * `length` - is how many ints are in the array of ints.
 * `reducer_func` - is the reducer used to transform a list of ints to an int.
 * `base_case` - is the base case that the reducer will use.
 * `num_threads` - is how many threads (in addition to the main thread) are used
 * in the parallelization.
 */
int par_reduce(int *list, size_t length, reducer reducer_func, int base_case,
               size_t num_threads);
#endif /* __CS241_PAR_REDUCE_H__ */
