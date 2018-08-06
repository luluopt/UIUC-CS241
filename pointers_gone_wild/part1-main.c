/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#include "part1-functions.h"
#include <stdio.h>
#include <stdlib.h>

// Any input that is "blue" should finish in state 5
// Any input that is not "blue" should finish in state 4
int transition_function_blue(int state, char input) {
    int ret = 4;

    switch (state) {
    case 0:
        ret = input == 'b' ? 1 : 4;
        break;
    case 1:
        ret = input == 'l' ? 2 : 4;
        break;
    case 2:
        ret = input == 'u' ? 3 : 4;
        break;
    case 3:
        ret = input == 'e' ? 5 : 4;
        break;
    }
    // if we are in 4 or 5 and get any input, we go to 4

    return ret;
}

// Any input that is "orange" should finish in state 7
// Any input that is not "orange" should finish in state 6
int transition_function_orange(int state, char input) {
    int ret = 6;

    switch (state) {
    case 0:
        ret = input == 'o' ? 1 : 6;
        break;
    case 1:
        ret = input == 'r' ? 2 : 6;
        break;
    case 2:
        ret = input == 'a' ? 3 : 6;
        break;
    case 3:
        ret = input == 'n' ? 4 : 6;
        break;
    case 4:
        ret = input == 'g' ? 5 : 6;
        break;
    case 5:
        ret = input == 'e' ? 7 : 6;
        break;
    }

    return ret;
}

int main() {
    printf("== one() ==\n");
    const char *a = "20";
    one(a);
    const char *b = "100";
    one(b);

    printf("== two() ==\n");
    two();

    printf("== three() ==\n");
    const int num1 = 3;
    const int num2 = 3;
    three(&num1, &num2);

    const int num3 = 4;
    three(&num1, &num3);

    printf("== four() ==\n");
    float *p_four;
    int i4 = 4, i432 = 432;

    p_four = four(&i4);
    printf("%d == %f\n", i4, *p_four);
    free(p_four);

    p_four = four(&i432);
    printf("%d == %f\n", i432, *p_four);
    free(p_four);

    printf("== five() ==\n");
    const char s = 'S';
    five(&s);
    const char t = '_';
    five(&t);

    printf("== six() ==\n");
    six("World!");

    printf("== seven() ==\n");
    seven();

    printf("== eight() ==\n");
    eight(10);

    printf("== nine() ==\n");
    nine("red");
    nine("orange");
    nine("blue");
    nine("green");

    printf("== ten() ==\n");
    ten(35);
    ten(20);

    printf("== clear_bits() ==\n");
    long int result;

    result = clear_bits(0xFF, 0x55);
    printf("%ld\n", result);

    result = clear_bits(0x00, 0xF0);
    printf("%ld\n", result);

    result = clear_bits(0xAB, 0x00);
    printf("%ld\n", result);

    result = clear_bits(0xCA, 0xFE);
    printf("%ld\n", result);

    result = clear_bits(0x14, 0x00);
    printf("%ld\n", result);

    result = clear_bits(0xBB, 0xBB);
    printf("%ld\n", result);

    printf("== little finite automatons\n");
    printf("%d\n", little_automaton(transition_function_blue, "blue"));
    printf("%d\n", little_automaton(transition_function_blue, "orange"));

    printf("%d\n", little_automaton(transition_function_orange, "blue"));
    printf("%d\n", little_automaton(transition_function_orange, "orange"));

    return 0;
}
