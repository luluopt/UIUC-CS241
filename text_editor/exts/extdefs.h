/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#pragma once
#include "../display.h"
#include "../document.h"
#include "../editor.h"
// Author: Aneesh Durg
// Key_codes:g
// F_name:ext_0
// Description:"Interactively inserts text"
//---
#include <string.h>
void ext_0(document *document, display *display, char **buffer, char k) {
    (void)buffer;
    (void)k;
    char *input = NULL;
    display_interact(display, "Text to insert:", &input);
    if (!input) {
        return;
    }
    if (input[strlen(input) - 1] == '\n')
        input[strlen(input) - 1] = 0;
    location loc = display_get_loc(display);
    handle_insert_command(document, loc, input);
    free(input);
}

// Author: Aneesh Durg
// Key_codes:q,w
// F_name:ext_1
// Description:"Set text to insert"
//---
#include <string.h>
void ext_1(document *document, display *display, char **buffer, char k) {
    if (k == 'q') {
        char *input = NULL;
        display_interact(display, "Text to insert:", &input);
        buffer[1] = input;
    } else {
        char *input = buffer[1];
        if (!input)
            return;
        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = 0;
        location loc = display_get_loc(display);
        handle_insert_command(document, loc, input);
        buffer[1] = NULL;
        free(input);
    }
}
