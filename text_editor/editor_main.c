/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#include "display.h"
#include "document.h"
#include "editor.h"
#include "exts/extensions.h"
#include "format.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <regex.h>

#include <ncurses.h>

short load_extensions = 0;

/**
 * This is the will be the entry point to your text editor.
*/
int entry_point(int argc, char *argv[]) {

    // Checking to see if the editor is being used correctly.
    if (argc != 2) {
        print_usage_error();
        return 1;
    }

    // Set up variables to hold regex
    regex_t writeOrAppend;
    regex_t printOrDelete;
    regex_t searchInText;
    regex_t oneCharCommand;

    // Compile regex
    // Pattern for writes and appends
    int regerr = regcomp(&writeOrAppend, "(w|a) [0-9]+ .+$", REG_EXTENDED);
    if (regerr) {
        fprintf(stderr, "Failed to compile regex!\n");
        return 1;
    }
    // Patterns in the form p 30 or d 10
    regerr = regcomp(&printOrDelete, "(p|d) [0-9]+$", REG_EXTENDED);
    if (regerr) {
        fprintf(stderr, "Failed to compile regex!\n");
        return 1;
    }
    // Search pattern, a backslash prepends the argument
    regerr = regcomp(&searchInText, "f .+$", REG_EXTENDED);
    if (regerr) {
        fprintf(stderr, "Failed to compile regex!\n");
        return 1;
    }
    // One character patterns for save, print and quit
    regerr = regcomp(&oneCharCommand, "(s|p|q)$", REG_EXTENDED);
    if (regerr) {
        fprintf(stderr, "Failed to compile regex!\n");
        return 1;
    }

    // Setting up a document based on the file named 'filename'.
    char *filename = get_filename(argc, argv);
    document *document = handle_create_document(filename);

    if (document == NULL) {
        fprintf(stderr, "Failed to create document!\n");
        return 1;
    }

    location res = {1, 0};

    while (1) {
        char *inputBuffer = NULL;
        size_t n = 0;
        ssize_t s = getline(&inputBuffer, &n, stdin);
        char *freeMe = inputBuffer;
        if (s < 0) {
            handle_cleanup(document);
            regfree(&writeOrAppend);
            regfree(&printOrDelete);
            regfree(&searchInText);
            regfree(&oneCharCommand);
            exit(0);
        }

        // Remove trailing newline
        if (inputBuffer[strlen(inputBuffer) - 1] == '\n') {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }

        if (!regexec(&writeOrAppend, inputBuffer, 0, NULL, 0)) {
            char wOrA = inputBuffer[0];
            char *newString = NULL;
            int lineno = (int)strtol(inputBuffer + 2, &newString, 10);
            newString++;
            if (wOrA == 'w') {
                handle_write_command(document, lineno, newString);
            } else {
                handle_append_command(document, lineno, newString);
            }
        } else if (!regexec(&printOrDelete, inputBuffer, 0, NULL, 0)) {
            char *end = NULL;
            int lineno = (int)strtol(inputBuffer + 2, &end, 10);
            if (*end != 0) {
                fprintf(stderr, "'%s' is not a valid command!\n", inputBuffer);
                free(freeMe);
                continue;
            }
            if (inputBuffer[0] == 'p')
                handle_display_command(document, MAX(3, lineno) - 2, 5, 0, -1);
            else
                handle_delete_line(document, lineno);
        } else if (!regexec(&searchInText, inputBuffer, 0, NULL, 0)) {
            location new_res =
                handle_search_command(document, res, inputBuffer + 2);
            if (new_res.line_no != 0) {
                res = new_res;
                res.idx++;
                handle_display_command(document, MAX(3, res.line_no) - 2, 5, 0,
                                       -1);
            } else {
                printf("No results found for '%s'!\n", inputBuffer + 1);
            }
        } else if (!regexec(&oneCharCommand, inputBuffer, 0, NULL, 0)) {
            if (inputBuffer[0] == 's') {
                handle_save_command(document, filename);
            } else if (inputBuffer[0] == 'q') {
                free(freeMe);
                regfree(&writeOrAppend);
                regfree(&printOrDelete);
                regfree(&searchInText);
                regfree(&oneCharCommand);
                handle_cleanup(document);
                exit(0);
            } else {
                handle_display_command(document, 1, -1, 0, -1);
            }
        } else {
            fprintf(stderr, "'%s' is not a valid command!\n", inputBuffer);
            free(freeMe);
            continue;
        }

        free(freeMe);
    }
}

int ncurses_entry_point(int argc, char *argv[]) {
    // Validate number of positional arguments (assume -n occurs once)
    if (argc != 2) {
        print_usage_error();
        return 1;
    }

    // Extensions :D
    char *buffer[100];

    // Setting up a document based on the file named 'filename'.
    char *filename = get_filename(argc, argv);
    document *document = handle_create_document(filename);

    if (document == NULL) {
        fprintf(stderr, "Failed to create document!\n");
        return 1;
    }

    display *display = display_create(document);

    char *header = NULL;
    asprintf(&header, "[%s]\n", filename);
    display_set_header(display, header);

    int done = 0;
    char *footer = NULL;
    char input_str[2];
    input_str[1] = 0;
    while (!done) {
        display_refresh_text(display);
        if (footer) {
            free(footer);
            footer = NULL;
        }

        int c = display_get_char(display);
        if (c < 0) {
            continue;
        }

        location real_loc = display_get_loc(display);
        switch (c) {
        case KEY_UP: {
            display_cursor_up(display);
        } break;
        case KEY_DOWN: {
            display_cursor_down(display);
        } break;
        case KEY_LEFT: {
            display_cursor_left(display);
        } break;
        case KEY_RIGHT: {
            display_cursor_right(display);
        } break;
        case KEY_BACKSPACE: {
            if (real_loc.idx >= 1) {
                handle_delete_command(
                    document, (location){real_loc.line_no, real_loc.idx - 1},
                    1);
                display_cursor_left(display);
            } else if (real_loc.line_no != 1) {
                // cursor is on first column, so merge this line with the
                // previous one
                size_t newidx =
                    strlen(document_get_line(document, real_loc.line_no - 1));
                handle_merge_line(document, real_loc.line_no - 1);
                display_cursor_up(display);
                // Update cursor index
                if (newidx - display->start_col_idx >
                    display->col - TAB_WIDTH - 2) {
                    display->start_col_idx =
                        newidx - (display->col - TAB_WIDTH - 2);
                    (display->loc).idx = display->col - TAB_WIDTH - 2;
                } else {
                    (display->loc).idx = newidx - display->start_col_idx;
                }

                asprintf(&footer, "merged: %zu", real_loc.line_no);
                display_set_footer(display, footer);
            }
        } break;
        case KEY_DC: {
            if (real_loc.idx !=
                strlen(document_get_line(document, real_loc.line_no))) {
                handle_delete_command(
                    document, (location){real_loc.line_no, real_loc.idx}, 1);
            } else if (real_loc.line_no != document_size(document)) {
                // cursor is on last column, so merge this line with the next
                // one
                handle_merge_line(document, real_loc.line_no);
                asprintf(&footer, "merged: %zu", real_loc.line_no + 1);
                display_set_footer(display, footer);
            }
        } break;
        case KEY_HOME: {
            (display->loc).idx = 0;
            display->start_col_idx = 0;
        } break;
        case KEY_END: {
            size_t line_len =
                strlen(document_get_line(document, real_loc.line_no));
            if (line_len - display->start_col_idx > display->col - TAB_WIDTH) {
                // end of line is on the right side of display
                display->start_col_idx =
                    line_len - (display->col - TAB_WIDTH - 2);
                (display->loc).idx = display->col - TAB_WIDTH - 2;
            } else {
                (display->loc).idx = line_len - display->start_col_idx;
            }
        } break;
        case 10: { // KEY_ENTER
            if (document_size(document)) {
                handle_split_line(document,
                                  (location){real_loc.line_no, real_loc.idx});
                (display->loc).idx = 0;
                display->start_col_idx = 0;
            } else {
                handle_insert_command(document,
                                      (location){real_loc.line_no + 1, 0}, "");
            }
            display_cursor_down(display);
        } break;
        case 27: {
            c = display_get_char(display);
            switch (c) {
                /**
                 *  OPTIONAL: Extending the TUI's functionality
                 *
                 *  You can add your own features here! Just read the file
                 * exts/Instructions.md
                 *  to find out how.
                */
                EXTENSIONS(c);
            }
        } break;
        default: {
            if ((c >= 32) && (c <= 126)) {
                input_str[0] = c;
                handle_insert_command(
                    document, (location){real_loc.line_no, real_loc.idx},
                    input_str);
                display_cursor_right(display);
                break;
            }
            const char *key_name = keyname(c);
            if (key_name[0] == '^') {
                // CTRL was pressed!
                switch (key_name[1]) {
                case 'A': {
                    display_destroy(display);
                    handle_cleanup(document);
                    done = 1;
                } break;
                case 'X': {
                    handle_save_command(document, filename);
                    asprintf(&footer, "Wrote file to: %s", filename);
                    display_set_footer(display, footer);
                } break;
                case 'W': {
                    // delete line with ctrl + w
                    handle_delete_line(document, real_loc.line_no);
                    if (real_loc.line_no > document_size(document)) {
                        if (document_size(document) != 0) {
                            display_cursor_up(display);
                        } else {
                            (display->loc).idx = 0;
                            display->start_col_idx = 0;
                        }
                    } else {
                        display_cursor_idx_check(display);
                    }
                    asprintf(&footer, "Deleted line: %zu", real_loc.line_no);
                    display_set_footer(display, footer);
                } break;
                case 'F': {
                    // ctrl+f to search
                    char *search_str = NULL;
                    display_interact(display, "Enter Search String:",
                                     &search_str);
                    if (search_str) {
                        // offsetting idx by 1 to not return the same location
                        // incase the user is already
                        // on top of a search result
                        location _search_line_buffer = handle_search_command(
                            document,
                            (location){real_loc.line_no, real_loc.idx + 1},
                            search_str);
                        if (_search_line_buffer.line_no) {
                            display_set_loc(display,
                                            _search_line_buffer.line_no,
                                            _search_line_buffer.idx);
                            asprintf(&footer, "Found %s at line: %zu char: %zu",
                                     search_str, _search_line_buffer.line_no,
                                     _search_line_buffer.idx);

                            display_set_footer(display, footer);
                        } else {
                            asprintf(&footer, "%s not found!", search_str);
                            display_set_footer(display, footer);
                        }
                        free(search_str);
                    }
                } break;
                }
            }
        }
        }
    }
    free(header);
    return 0;
}
// the code below allows the editor to run in a
// seperate process so that the tty mode can be restored

size_t _lines = 20;
int usenc = 0;

void cleanupAndExit(int sig) {
    (void)sig;
    // Just in case ncurses is acting up
    if (usenc) {
        endwin();
        system("stty sane");
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    // Catch ctrl+c
    if (argc > 1) {
        // n for ncurses
        if (!strcmp(argv[1], "-n")) {
            usenc++;
            for (int i = 2; i < argc; i++) {
                argv[i - 1] = argv[i];
            }
            argc--;
        }
    }

    pid_t pid;
    if (usenc) {
        pid = fork();

        if (pid < 0) {
            // fail
            perror("Can't fork.");
        } else if (pid == 0) {
            // child
            ncurses_entry_point(argc, argv);
        }
    } else {
        pid = fork();

        if (pid < 0) {
            // fail
            perror("Can't fork.");
        } else if (pid == 0) {
            // child
            entry_point(argc, argv);
            exit(0);
        }
    }
    signal(SIGINT, cleanupAndExit);
    int status = 0;
    waitpid(pid, &status, 0);

    cleanupAndExit(0);
}
