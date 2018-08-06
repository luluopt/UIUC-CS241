/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#include <stdio.h>
#include <string.h>

#include "part3-functions.h"
#include "part3-utils.h"
#include "vector.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ./part3 [inputfilename]\n");
        exit(1);
    }

    printf("== one() ==\n");
    vector *v = one(10);
    vector_print(dragon_printer, v);
    vector_destroy(v);

    printf("== two() ==\n");
    v = string_vector_create();
    vector_push_back(v, (void *)"Join ");
    vector_push_back(v, (void *)"all ");
    vector_push_back(v, (void *)"the ");
    vector_push_back(v, (void *)"strings!");
    char *joined = two(v);
    printf("%s\n", joined);
    free(joined);
    vector_destroy(v);

    printf("== dragons ==\n");

    // Read the given input file into a string
    char *buff = readfile(argv[1]);
    dragon jim;
    jim.name = "Jim";
    jim.talon = id_talon;

    dragon bob;
    bob.name = "Bob";
    bob.talon = xor_talon;

    // Use the dragon Jim to encode the input file and print it out
    vector *encoded = dragon_encode(&jim, buff);
    vector_print(string_printer, encoded);

    // Decode with Jim again (id is its own inverse)
    char *decoded = dragon_decode(&jim, encoded);
    vector_destroy(encoded);

    // Check that the decoded string matches the original input
    int diff = strcmp(decoded, buff);
    free(decoded);
    if (!diff) {
        printf("Successfully decoded text using id!\n");
    }

    // Same as above, with Bob
    encoded = dragon_encode(&bob, buff);
    vector_print(string_printer, encoded);
    decoded = dragon_decode(&bob, encoded);
    vector_destroy(encoded);
    diff = strcmp(decoded, buff);
    free(decoded);
    if (!diff) {
        printf("Successfully decoded text using xor!\n");
    }

    // Same as above, with the dragons from create_dragons()
    dragon *dragons_of_destiny = create_dragons();
    encoded = dragon_encode(dragons_of_destiny, buff);
    vector_print(string_printer, encoded);
    decoded = dragon_decode(dragons_of_destiny + 1, encoded);
    vector_destroy(encoded);
    diff = strcmp(decoded, buff);
    free(decoded);
    if (!diff) {
        printf("Successfully decoded text using created dragons!\n");
    }
    free(dragons_of_destiny);

    free(buff);
    return 0;
}
