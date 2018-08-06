/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#include "document.h"
#include "editor.h"
#include "format.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * You can programatically test your text editor.
*/
int main() {
    // Setting up a docment based on the file named 'filename'.
    char *filename = "test.txt";
    document *doc = document_create_from_file(filename);

    // print the first 5 lines
    handle_display_command(doc, 1, 5, 0, -1);
}
