/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#pragma once
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "vector.h"

#define NAME_LENGTH 5

// Type of function pointers that dragons use to
// encode or decode a byte of innocent data
typedef char (*talon_data_type)(char c);

char xor_talon(char c);
char id_talon(char c);

typedef void (*printer_type)(void *data);

void vector_print(printer_type printer, vector *v);

// Struct representing a dragon
typedef struct {
    char *name;
    talon_data_type talon;
} dragon;

void dragon_printer(void *elem);
void string_printer(void *elem);

char *gen_new_dragon_name(int length);

char *readfile(char *filename);
