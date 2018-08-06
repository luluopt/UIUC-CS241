/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#ifndef __CS241_REDUCERS_H__
#define __CS241_REDUCERS_H__
/**
 * This callback function takes in an int and returns an int.
 */
typedef int (*reducer)(int elem1, int elem2);

/**
 * Returns the reducer that matches the name 'reducer_name'
 */
reducer get_reducer(char *reducer_name);

/**
 * Returns the base case of the reducer that matches the name 'reducer_name'
 */
int get_reducer_base_case(char *reducer_name);

extern int add_base_case;
extern int mult_base_case;
extern int slow_base_case;

// Callback functions
int add(int elem1, int elem2);
int mult(int elem1, int elem2);
int slow(int elem1, int elem2);

#endif /* __CS241_REDUCERS_H__ */
