/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2017
 */

#include "callbacks.h"
#include <string.h>

void *shallow_copy_constructor(void *elem) { return elem; }

void shallow_destructor(void *elem) {
    (void)elem;
    return;
}

void *shallow_default_constructor(void) { return NULL; }

size_t shallow_hash_function(void *elem) {
    (void)elem;
    return 0;
}

size_t pointer_hash_function(void *elem) { return (size_t)elem; }

void *string_copy_constructor(void *elem) {
    if (!elem)
        return NULL;
    return strdup(elem);
}

void string_destructor(void *elem) { free(elem); }

void *string_default_constructor(void) { return calloc(1, 1); }

/**
 * Modified from implementation presented here:
 * http://www.cse.yorku.ca/~oz/hash.html
 */
size_t string_hash_function(void *elem) {
    char *str = (char *)elem;
    if (!str) {
        return 163;
    }

    size_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = (hash << 5) + hash + c;
    }

    return hash;
}

// The following is code generated
void *char_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    char *copy = malloc(sizeof(char));
    *copy = *((char *)elem);
    return copy;
}

void char_destructor(void *elem) { free(elem); }

void *char_default_constructor(void) { return calloc(1, sizeof(char)); }

size_t char_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((char *)elem);
}

void *double_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    double *copy = malloc(sizeof(double));
    *copy = *((double *)elem);
    return copy;
}

void double_destructor(void *elem) { free(elem); }

void *double_default_constructor(void) { return calloc(1, sizeof(double)); }

size_t double_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((double *)elem);
}

void *float_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    float *copy = malloc(sizeof(float));
    *copy = *((float *)elem);
    return copy;
}

void float_destructor(void *elem) { free(elem); }

void *float_default_constructor(void) { return calloc(1, sizeof(float)); }

size_t float_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((float *)elem);
}

void *int_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    int *copy = malloc(sizeof(int));
    *copy = *((int *)elem);
    return copy;
}

void int_destructor(void *elem) { free(elem); }

void *int_default_constructor(void) { return calloc(1, sizeof(int)); }

size_t int_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((int *)elem);
}

void *long_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    long *copy = malloc(sizeof(long));
    *copy = *((long *)elem);
    return copy;
}

void long_destructor(void *elem) { free(elem); }

void *long_default_constructor(void) { return calloc(1, sizeof(long)); }

size_t long_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((long *)elem);
}

void *short_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    short *copy = malloc(sizeof(short));
    *copy = *((short *)elem);
    return copy;
}

void short_destructor(void *elem) { free(elem); }

void *short_default_constructor(void) { return calloc(1, sizeof(short)); }

size_t short_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((short *)elem);
}

void *unsigned_char_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    unsigned char *copy = malloc(sizeof(unsigned char));
    *copy = *((unsigned char *)elem);
    return copy;
}

void unsigned_char_destructor(void *elem) { free(elem); }

void *unsigned_char_default_constructor(void) {
    return calloc(1, sizeof(unsigned char));
}

size_t unsigned_char_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((unsigned char *)elem);
}

void *unsigned_int_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    unsigned int *copy = malloc(sizeof(unsigned int));
    *copy = *((unsigned int *)elem);
    return copy;
}

void unsigned_int_destructor(void *elem) { free(elem); }

void *unsigned_int_default_constructor(void) {
    return calloc(1, sizeof(unsigned int));
}

size_t unsigned_int_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((unsigned int *)elem);
}

void *unsigned_long_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    unsigned long *copy = malloc(sizeof(unsigned long));
    *copy = *((unsigned long *)elem);
    return copy;
}

void unsigned_long_destructor(void *elem) { free(elem); }

void *unsigned_long_default_constructor(void) {
    return calloc(1, sizeof(unsigned long));
}

size_t unsigned_long_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((unsigned long *)elem);
}

void *unsigned_short_copy_constructor(void *elem) {
    if (!elem) {
        return elem;
    }
    unsigned short *copy = malloc(sizeof(unsigned short));
    *copy = *((unsigned short *)elem);
    return copy;
}

void unsigned_short_destructor(void *elem) { free(elem); }

void *unsigned_short_default_constructor(void) {
    return calloc(1, sizeof(unsigned short));
}

size_t unsigned_short_hash_function(void *elem) {
    if (!elem) {
        return 0;
    }
    return (size_t) * ((unsigned short *)elem);
}
