/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reducers.h"

int add_base_case = 0;
int mult_base_case = 1;
int slow_base_case = 56;

reducer get_reducer(char *reducer_name) {
    if (strcmp(reducer_name, "add") == 0) {
        return add;
    } else if (strcmp(reducer_name, "mult") == 0) {
        return mult;
    } else if (strcmp(reducer_name, "slow") == 0) {
        return slow;
    }
    /* more else if clauses */
    else /* default: */ {
        fprintf(stderr, "Could not recognize [%s] as a reducer!\n",
                reducer_name);
        exit(4);
    }
}

int get_reducer_base_case(char *reducer_name) {
    if (strcmp(reducer_name, "add") == 0) {
        return add_base_case;
    } else if (strcmp(reducer_name, "mult") == 0) {
        return mult_base_case;
    } else if (strcmp(reducer_name, "slow") == 0) {
        return slow_base_case;
    }
    /* more else if clauses */
    else /* default: */ {
        fprintf(stderr, "Could not recognize [%s] as a reducer!\n",
                reducer_name);
        exit(4);
    }
}

int add(int elem1, int elem2) {
    return elem1 + elem2;
}

int mult(int elem1, int elem2) {
    return elem1 * elem2;
}

int slow(int elem1, int elem2) {
    (void)elem1;
    (void)elem2;
    usleep(1000);
    return 56;
}
