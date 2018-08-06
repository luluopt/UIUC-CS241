/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#pragma once

#include <ncurses.h>

#include "document.h"
#include "editor.h"

#define TAB_WIDTH 8

/**
 * This display is written using ncurses as a backend.
 * Using ncurses allows us to safely abstract the mess of
 * moving the cursor around to a 3rd party library, and we
 * don't need to use ANSI escape codes to accomplish this.
*/

/**
 * Struct representing the display
 * Note that the location stored here is with respect
 * to the lines printed by the display and not the document.
 * The getter and setter functions take this into account.
 *
 * display requires a pointer to document, just to keep the functions
 * neat and not having to take in a document pointer for everything.
 *
 * Note that the location stored in display is NOT the location of the
 * pointer with respect to the document. It is with respect to the TERMINAL.
 * To get the location with respect to the document, use the display_get_loc
 * function.
 */
typedef struct display {
    const char *_header;  // Header to be display on top
    const char *_footer;  // Footer (status bar)
    location loc;         // location w.r.t. terminal
    document *document;   // document to be displayed
    WINDOW *window;       // ncurses window
    size_t row, col;      // size of the window
    size_t top_line;      // First line to be displayed
    size_t start_col_idx; // First collumn to be displayed
} display;

/**
 *  Updates display width so that terminal can be resized
 */
void update_display_param(display *display);

/**
 *  Creates and initialized a new display for a given document
 */
display *display_create(document *document);

/**
 * Sets header to be displayed above the document
 * Currently only being used to display welcome message and filename
 */
void display_set_header(display *d, const char *h);

/**
 * Displays text at the bottom under the document
 * Used for row/col no, and interactive commands
 */
void display_set_footer(display *d, const char *f);

/**
 * Get the real location of the cursor in the document
 */
location display_get_loc(display *display);

/**
 * Set the location of the cursor with respect to the document
 */
void display_set_loc(display *display, size_t lineno, size_t idx);

/**
 * Refreshes the display.
 * This functions should not be called directly, use one of the wrappers below
 */
void display_screen(display *d, int interact, const char *prompt,
                    char **result);

/**
 * Wrapper function around display_screen
 * No interactivity
 */
void display_refresh_text(display *d);

/**
 * Runs an interactive command and gets the result
 */
void display_interact(display *d, const char *prompt, char **result);

/**
 * Helper function for display_cursor_up/down().
 * Moves the cursor position in a line to min(line_len, current_position)
 */
void display_cursor_idx_check(display *display);

/**
 * Move cursor up
 */
void display_cursor_up(display *display);

/**
 * Move cursor down
 */
void display_cursor_down(display *display);

/**
 * Move cursor left
 */
void display_cursor_left(display *display);

/**
 * Move cursor right
 */
void display_cursor_right(display *display);

/**
 * Get one character from the window
 */
int display_get_char(display *display);

/**
 * Cleanup display
 */
void display_destroy(display *display);
