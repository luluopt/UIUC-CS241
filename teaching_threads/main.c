/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "par_reduce.h"
#include "reduce.h"
#include "reducers.h"

#define SEED 241

int *gen_random_list(size_t num_elems) {
    int *list = (int *)malloc(sizeof(int) * num_elems);

    for (size_t i = 0; i < num_elems; ++i) {
        list[i] = (rand() % 5) + 1; // elements are going to be between 1 and 5
    }

    return list;
}

bool verify(int output, int *input_list, reducer reduce_func, int base_case,
            size_t list_len) {
    // call 'reduce_func' then verify with 'output'.
    int soln = reduce(input_list, list_len, reduce_func, base_case);
    // printf("solution is: %d, output is: %d\n", soln, output);
    return (soln == output);
}

void validate_args(int argc, char **argv) {
    // Verify correct number of arguments
    if (argc != 4) {
        fprintf(stderr, "usage: %s <reducer_name> <list_len> <num_threads>\n",
                argv[0]);
        exit(1);
    }

    // Verify that list_len and num_threads are integers greater than 1
    char *endptr;
    for (int i = 2; i <= 3; i++) {
        char *str_value = argv[i];
        long value = strtol(str_value, &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "Failed to convert an [%s] to a long!\n",
                    str_value);
            exit(3);
        }

        if (value < 1) {
            fprintf(stderr, "[%s] needs to be greater than or equal to 1!\n",
                    str_value);
            exit(4);
        }
    }
}

int main(int argc, char *argv[]) {
    validate_args(argc, argv);

    // Seeding random number generator
    srand(SEED);

    char *reducer_name = argv[1];

    size_t list_len = strtol(argv[2], NULL, 10);
    size_t num_threads = strtol(argv[3], NULL, 10);

    int *list = gen_random_list(list_len);
    reducer reducer_func = get_reducer(reducer_name);
    int base_case = get_reducer_base_case(reducer_name);

    int *list_copy = (int *)malloc(sizeof(int) * list_len);
    memcpy(list_copy, list, sizeof(int) * list_len);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    int ret_value =
        par_reduce(list_copy, list_len, reducer_func, base_case, num_threads);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double diff =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1E9;

    printf("par_reduce ran in %f seconds\n", diff);

    bool passed =
        verify(ret_value, list_copy, reducer_func, base_case, list_len);

    if (passed) {
        printf("Congratulations you have succesfully ran par_reduce with %s on "
               "a list with %zu elements, and %zu threads\n",
               reducer_name, list_len, num_threads);
    } else {
        printf("Your par_reduce result is incorrect.");
    }

    free(list);
    free(list_copy);
    return passed;
}
