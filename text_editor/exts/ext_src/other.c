/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
// Author: Aneesh Durg
// Key_codes:q,w
// F_name:my_ext
// Description:"Set text to insert"
//---
#include <string.h>
void my_ext(document *document, display *display, char **buffer, char k) {
    if (k == 'q') {
        char *input = NULL;
        display_interact(display, "Text to insert:", &input);
        buffer[EXT_NO] = input;
    } else {
        char *input = buffer[EXT_NO];
        if (!input)
            return;
        if (input[strlen(input) - 1] == '\n')
            input[strlen(input) - 1] = 0;
        location loc = display_get_loc(display);
        handle_insert_command(document, loc, input);
        buffer[EXT_NO] = NULL;
        free(input);
    }
}
