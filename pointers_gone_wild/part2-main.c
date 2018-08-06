/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int * second = malloc(sizeof(int));
    *second = 132;
    second_step(second);
    free(second);

    int ** doublet = malloc(sizeof(int));
    doublet[0] = malloc(sizeof(int));
    *doublet[0] = 8942;
    double_step(doublet);
    free(doublet[0]);
    free(doublet);

    char * strange;
    int strange_val = 15;
    strange = (char*)(&strange_val) - 5 * sizeof(char);
    strange_step(strange);

    char* empty = malloc(4 * sizeof(char));
    empty[3] = 0;
    empty_step((void*) empty);

    char * two2 = malloc(4 * sizeof(char));
    two2[3] = 'u';
    void * two = two2;
    two_step(two, two2);
    free(two2);

    char *g = malloc(1);
    char *h = g+2;
    char *i = h+2;
    three_step(g, h, i);
    free(g);

    char * step1 = malloc(2 * sizeof(char));
    char * step2 = malloc(3 * sizeof(char));
    char * step3 = malloc(4 * sizeof(char));
    step1[1] = '0';
    step2[2] = step1[1]+8;
    step3[3] = step2[2]+8;
    step_step_step(step1, step2, step3);
    free(step1);
    free(step2);
    free(step3);

    char * odd = malloc(sizeof(char));
    *odd = '0';
    it_may_be_odd(odd, (int)'0');
    free(odd);

    char tok[10] = "zzz,CS241";
    tok_step(tok);

    char * end = malloc(4);
    end[0] = 1;
    end[1] = 0;
    end[2] = 0;
    end[3] = 2;
    void * end1 = end;
    void * end2 = end;
    the_end(end1, end2);
    free(end);

    return 0;
}
