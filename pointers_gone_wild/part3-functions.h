/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#pragma once
#include <string.h>

#include "callbacks.h"
#include "part3-utils.h"
#include "vector.h"

vector *one(size_t count);
char *two(vector *vec);

vector *dragon_encode(dragon *d, const char *str);
char *dragon_decode(dragon *dragon, vector *encoded_data);
dragon *create_dragons();
