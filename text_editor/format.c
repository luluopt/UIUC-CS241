/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#include "document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage_error() { printf("\n  editor [-n] <filename>\n"); }

void print_line(document *document, size_t index, size_t start_col_index,
                ssize_t max_cols) {

    char *line = (char *)document_get_line(document, index);
    line = strdup(line);
    size_t line_len = strlen(line);
    for (size_t i = 0; i < line_len; i++) {
        if (line[i] == '\t' || line[i] == '\r') {
            line[i] = ' ';
        }
    }
    if (line_len == 0 || line_len < start_col_index) {
        printf("%zu\n", index);
    } else if (max_cols == -1) {
        printf("%zu\t%s\n", index, line + start_col_index);
    } else {
        printf("%zu\t%.*s\n", index, (int)max_cols, line + start_col_index);
    }
    free(line);
}

void print_document_empty_error() {
    fprintf(stderr, "This file has no lines to display!\n");
}
