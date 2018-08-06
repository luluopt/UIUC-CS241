/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#include <fcntl.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

#include "display.h"
#include "document.h"
#include "editor.h"

/**
 *  Updates display width so that terminal can be resized
 */
void update_display_param(display *display) {
    getmaxyx(stdscr, (display->row), (display->col));
    display->row--;

    // Update cursor location if user resizes display window
    if ((display->loc).line_no > display->row - 1) { // update cursor row
        (display->loc).line_no = display->row - 1;
        display_cursor_idx_check(display);
    }
    if ((display->loc).idx > display->col - TAB_WIDTH - 2) // update cursor col
        (display->loc).idx = display->col - TAB_WIDTH - 2;

    // handle errors by reducing lineno and idx if neccessary
}

/**
 *  Creates and initialized a new display for a given document
 */
display *display_create(document *document) {
    display *new_display = malloc(sizeof(display));
    new_display->document = document;
    new_display->window = initscr();
    cbreak();
    keypad(stdscr, TRUE);

    (new_display->loc).line_no = 1;
    (new_display->loc).idx = 0;
    new_display->start_col_idx = 0;
    new_display->top_line = 1;
    new_display->_footer = NULL;
    update_display_param(new_display);

    return new_display;
}

/**
 * Sets header to be displayed above the document
 * Currently only being used to display welcome message and filename
 */
void display_set_header(display *d, const char *h) { d->_header = h; }

/**
 * Displays text at the bottom under the document
 * Used for row/col no, and interactive commands
 */
void display_set_footer(display *d, const char *f) { d->_footer = f; }

/**
 * Get the real location of the cursor in the document
 */
location display_get_loc(display *display) {
    size_t real_row = (display->loc).line_no + display->top_line - 1;
    size_t idx = (display->loc).idx + display->start_col_idx;
    return (location){real_row, idx};
}

/**
 * Set the location of the cursor with respect to the document
 */
void display_set_loc(display *display, size_t lineno, size_t idx) {
    if (idx > display->col - TAB_WIDTH - 1) {
        display->start_col_idx = idx;
        (display->loc).idx = 0;
    } else {
        display->start_col_idx = 0;
        (display->loc).idx = idx;
    }

    size_t lines = document_size(display->document);
    if (lines <= display->row) {
        display->top_line = 1;
        (display->loc).line_no = lineno;
    } else {
        display->top_line = lineno;
        (display->loc).line_no = 1;
    }
}

/**
 * Refreshes the display.
 * This functions should not be called directly, use one of the wrappers below
 */
void display_screen(display *d, int interact, const char *prompt,
                    char **result) {
    // Detect window resize
    update_display_param(d);

    document *document = d->document;

    // Erase display
    clear();

    // Print header
    printw(d->_header);

    // Creating pipes so that students still only print to stdout
    // This way, they only need to implement it with the line editor in mind
    int savedStdout = dup(1);
    int stdoutPipe[2];

    pipe2(stdoutPipe, 0);

    dup2(stdoutPipe[1], 1);

    handle_display_command(document, d->top_line, d->row - 1, d->start_col_idx,
                           d->col - TAB_WIDTH - 1);
    fflush(stdout);
    close(stdoutPipe[1]);

    dup2(savedStdout, 1);
    close(savedStdout);

    // Read from the pipe and call printw to print to ncurses
    FILE *reader = fdopen(stdoutPipe[0], "r");
    ssize_t n = 0;
    size_t lines_written = 0;
    while (1) {
        char *line = NULL;
        size_t c = 0;
        n = getline(&line, &c, reader);
        if (n >= 0) {
            printw("%s", line);
            lines_written++;
            free(line);
        } else {
            if (line)
                free(line);
            break;
        }
    }
    close(stdoutPipe[0]);

    while (lines_written != d->row - 1) {
        printw("-\n");
        lines_written += 1;
    }

    // If the command requires input from the user
    if (interact) {
        printw("%s", prompt);
        int c;
        if ((*result) == NULL) {
            *result = malloc(1024);
        }
        int i = 0;
        for (; i < 1023; i++) {
            c = getch();
            // printw("%d", c);
            if (c == 27) {
                // ESC was pressed!
                free(*result);
                *result = NULL;
                interact = 0;
                break;
            } else if (c == KEY_BACKSPACE) {
                int x = 0, y = 0;
                getyx(d->window, y, x);
                if (i < 1) {
                    i = -1;
                    move(y, x + 1);
                    continue;
                }
                move(y, x);
                printw(" ");
                (*result)[i - 1] = 0;
                move(y, x);
                i -= 2;
            } else if (c == 10) {
                // ENTER was pressed! Time to stop
                i += 1;
                break;
            } else if ((c >= 32) && (c <= 126)) {
                (*result)[i] = c;
            }
        }
        if (*result)
            (*result)[i] = '\0';
    }
    if ((!interact) && d->_footer) {
        // Command is not interactive, print footer instead
        printw("%s\n", d->_footer);
        d->_footer = NULL;
    } else {
        // There is no footer, print the current row/col instead
        location loc = display_get_loc(d);
        printw("row: %zu | col: %zu\n", loc.line_no, loc.idx);
    }
    move((d->loc).line_no, TAB_WIDTH + (d->loc).idx);
}

/**
 * Wrapper function around display_screen
 * No interactivity
 */
void display_refresh_text(display *d) { display_screen(d, 0, NULL, NULL); }

/**
 * Runs an interactive command and gets the result
 */
void display_interact(display *d, const char *prompt, char **result) {
    display_screen(d, 1, prompt, result);
}

/**
 * Helper function for display_cursor_up/down().
 * Moves the cursor position in a line to min(line_len, current_position)
 */
void display_cursor_idx_check(display *display) {
    location cursor_loc = display_get_loc(display);
    size_t line_len =
        strlen(document_get_line(display->document, cursor_loc.line_no));
    if (line_len < display->start_col_idx) {
        display->start_col_idx = line_len;
    }
    if (cursor_loc.idx > line_len - display->start_col_idx) {
        (display->loc).idx = line_len - display->start_col_idx;
    }
}

/**
 * Move cursor up
 */
void display_cursor_up(display *display) {
    if ((display->loc).line_no != 1)
        (display->loc).line_no--;
    else {
        if (display->top_line != 1)
            display->top_line--;
    }
    display_cursor_idx_check(display);
}

/**
 * Move cursor down
 */
void display_cursor_down(display *display) {
    size_t line_count = document_size(display->document);
    if (display_get_loc(display).line_no ==
        line_count) { // Reached the last line of the document
        return;
    }
    if ((display->loc).line_no < (display->row - 1))
        (display->loc).line_no++;
    else {
        if ((display->top_line + display->row - 1) <= line_count) {
            display->top_line++;
        }
    }
    display_cursor_idx_check(display);
}

/**
 * Move cursor left
 */
void display_cursor_left(display *display) {
    if ((display->loc).idx != 0)
        (display->loc).idx--;
    else {
        if (display->start_col_idx != 0)
            display->start_col_idx--;
    }
}

/**
 * Move cursor right
 */
void display_cursor_right(display *display) {
    size_t lineno = display_get_loc(display).line_no;
    size_t line_len = strlen(document_get_line(display->document, lineno));
    if ((display->loc).idx + display->start_col_idx < line_len) {
        if ((display->loc).idx + TAB_WIDTH < display->col - 2) {
            (display->loc).idx++;
        } else {
            display->start_col_idx++;
        }
    }
}

/**
 * Get one character from the window
 */
int display_get_char(display *display) {
    int ret = getch();
    if (!document_size(display->document)) {
        if ((ret != 10) && !((ret >= 32) && (ret <= 126)))
            return -1;
    }
    return ret;
}

/**
 * Cleanup display
 */
void display_destroy(display *display) {
    endwin();
    free(display);
    // clean up here!
}
