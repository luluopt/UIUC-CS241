/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define K 1024L
#define M (1024L * 1024L)
#define G (1024 * 1024 * 1024)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define SECONDS_PER_DAY 86400

#define RAND_TODAY() time(NULL) - time(NULL) % SECONDS_PER_DAY

// Check if two regions overlap
int overlap(void *r1, size_t len1, void *r2, size_t len2) {
    return ((size_t)r1 <= (size_t)r2 && (size_t)r2 < (size_t)(r1 + len1)) ||
           ((size_t)r2 <= (size_t)r1 && (size_t)r1 < (size_t)(r2 + len2));
}

#define START_CHAR 'e'
#define END_CHAR 'l'

// Ensure that the start and end of region are writable
void verify_write(char *ptr, size_t len) {
    *(ptr) = START_CHAR;
    *(ptr + len - 1) = END_CHAR;
}

/*
 * Used with verify_write, make sure we get the same value back
 * return 1 if we get the same value back (good).
 * return 0 if we dont (bad).
 */
int verify_read(char *ptr, size_t len) {
    int ret = 1;

    if (*ptr != START_CHAR)
        ret = 0;

    if (*(ptr + len - 1) != END_CHAR)
        ret = 0;

    if (ret == 0)
        fprintf(stderr, "Failure to verify data.\n");

    return ret;
}

/*
 * Ensure that the whol region contain only integer c
 */
void verify(void *region, int c, size_t len) {
    char *r = region;
    while (len--)
        if (*(r++) != (char)c) {
            fprintf(stderr, "Memeory failed to contain correct value!\n");
            exit(1);
        }
}

/*
 * Ensure that the whole region contains only 0
 * Randomly choose a region
 */
void verify_clean(char *ptr, size_t len) {
    srand(time(NULL));
    size_t front45 = 4 * len / 5;
    size_t start = rand() % front45;
    size_t check_size = MIN(len / 5, len - start - 1);
    verify(ptr + start, 0x00, check_size);
}

/*
 * Ensure that two regions [r1, r1+len) and [r2, r2+len)
 * dont overlap
 */
void verify_overlap2(void *r1, void *r2, size_t len) {
    if (overlap(r1, len, r2, len)) {
        fprintf(stderr, "Memory regions overlap!\n");
        exit(1);
    }
}

/*
 * Ensure that three regions [r1, r1 + len), [r2, r2+len), and [r3, r3+len)
 * dont overlap
 */
void verify_overlap3(void *r1, void *r2, void *r3, size_t len) {
    if (overlap(r1, len, r2, len) || overlap(r1, len, r3, len) ||
        overlap(r2, len, r3, len)) {
        fprintf(stderr, "Memory regions overlap!\n");
        exit(1);
    }
}
