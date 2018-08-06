/**
 * MP: Extreme Edge Cases
 * CS 241 - Fall 2017
 */
#pragma once

#include "camelCaser.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **));
