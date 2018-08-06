/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#pragma once
#include "extdefs.h"
#define EXTENSIONS(c)                                                          \
    ({                                                                         \
        case 'g': {                                                            \
            ext_0(document, display, buffer, 'g');                             \
        } break;                                                               \
        case 'q': {                                                            \
            ext_1(document, display, buffer, 'q');                             \
        } break;                                                               \
        case 'w': {                                                            \
            ext_1(document, display, buffer, 'w');                             \
        } break;                                                               \
    })