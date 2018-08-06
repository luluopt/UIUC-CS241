/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "part3-functions.h"
#include "part3-utils.h"
#include "vector.h"

void dragon_destructor(void *elem) {
    if (elem) {
        dragon *d = (dragon *)elem;
        free(d->name);
        free(d);
    }
}

/**
 * Create a vector with 'count' unique dragons.
 * Be sure to check that each dragon has a different name!
 */
vector *one(size_t count) {
    vector *v = vector_create()；
    dragon *new_dragon；
    new_dragon = malloc(count * sizeof(dragon));
    size_t i;
    for (i = 0; i < count; ) {
        string temp_name= gen_new_dragon_name(NAME_LENGTH);
        size_t j;
        j = 0;
        VEC_FOR_EACH(v, each_vec) {
            if (each_vec.name != temp_name) {
                j++;
            }
        }
        if (i < j) {
            new_dragon[i].name = temp_name;
            vector_push_back(v, new_dragon[i]);
            i++;
        }
    }
    return v;
}

/**
 * Join a vector of strings together and return the result.
 * The resulting string should be allocated on the heap.
 *
 * Pseudocode example:
 * two(["foo", "bar", "baz"]) == "foobarbaz"
 */
char *two(vector *vec) {
    // TODO implement me
    string str ='';
    VEC_FOR_EACH(vec, each_vec) {
        str += each_vec;
    }
    char* stringresult = (char*)malloc((strlen(str)+1))*sizeof(char));
    strcpy(stringresult, str);
    return stringresult;
}

/**
 * Encodes a string into a vector of chunks of no more than 256 bytes.
 * Make sure to use dragon->talon to perform the encoding.
*/
vector *dragon_encode(dragon *dragon, const char *str) {
    // TODO implement me
    vector *v1 = vector_create()；
    char *p = str;
    int num = 0;
    for (p = str; *p != '\0'; p++) {
        if (num == 256) {
            num = num % 256;
        }
        char temp_str;
        temp_str = id_talon(dragon->talon);
        num++;
    }

    dragon->talon
    return NULL;
}

/**
 * Performs the inverse operation of dragon_encode.
 * The resulting string should be allocated on the heap.
 *
 * Pseudocode: dragon_decode(dragon, dragon_encode(dragon, foo)) == foo
 * Hint: You are allowed to modify the input vector if you wish.
 */
char *dragon_decode(dragon *dragon, vector *encoded_data) {
    // TODO implement me
    return NULL;
}

char encoder_provided(char c) {
    c = c + 1;
    c = c ^ 3;
    return c;
}

char decoder_provided(char c) {
    c = c ^ 3;
    c = c - 1;
    return c;
}

/**
 * Creates and returns an array of two dragons.
 * The second dragon should perform the inverse operation of the first.
 */
dragon *create_dragons() {
    dragon *dragons = malloc(2 * sizeof(dragon));
    dragons[0].name = "Dragon1";
    dragons[1].name = "Dragon2";

    // TODO set the 'talons' of dragons 1 and 2
    // dragon 1 should have the encoder, and 2 should have the decoder
    // To do this, you are going to need
    // to create a function that is the
    // opposite of encoder_provided.
    dragons[0].talons = encoder_provided('a');
    dragons[1].talons = decoder_provided(tragons[0].talons);

    // This will probably make sense if you've implemented encode
    // and decode
    return dragons;
}
