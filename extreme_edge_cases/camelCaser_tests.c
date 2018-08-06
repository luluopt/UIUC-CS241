/**
 * MP: Extreme Edge Cases
 * CS 241 - Fall 2017
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **))
{
    
    char **outputs = 0;
    // Empty string
    outputs = (*camelCaser)("");
    if (outputs[0] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // No punctuation
    outputs = (*camelCaser)("Hello World");
    if (outputs[0] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // One sentence
    outputs = (*camelCaser)("Hello World.");
    if (strcmp(outputs[0], "helloWorld") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // Count the number of sentences correctly
    outputs = (*camelCaser)("..,\"");
    for (int i=0; i<4; i++) {
        if (strcmp(outputs[i], "") != 0) {
            destroy(outputs);
            return 0;
        }
    }
    if (outputs[4] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    
    // Multiple setences
    outputs = (*camelCaser)("Hello. World.");
    if (strcmp(outputs[0], "hello") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "world") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[2] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // Start with non-alphabetical value
    outputs = (*camelCaser)("1Hello world. What is UP?");
    if (strcmp(outputs[0], "1helloWorld") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "whatIsUp") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[2] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // Consecutive white spaces
    outputs = (*camelCaser)("1Hello      world. What \t   is \n UP?");
    if (strcmp(outputs[0], "1helloWorld") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "whatIsUp") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[2] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // ASCII chars 
    outputs = (*camelCaser)("Hello\x1fWorld.");
    if (strcmp(outputs[0], "hello\x1fworld") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    outputs = (*camelCaser)("DEAL WITH THIS.");
    if (strcmp(outputs[0], "dealWithThis") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    outputs = (*camelCaser)("deal with this.");
    if (strcmp(outputs[0], "dealWithThis") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    outputs = (*camelCaser)("dEAL WiTh THis.");
    if (strcmp(outputs[0], "dealWithThis") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    outputs = (*camelCaser)(".dEAL wITH tHIS.");
    if (strcmp(outputs[0], "") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "dealWithThis") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[2] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);
    
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.
    return 1;
}
