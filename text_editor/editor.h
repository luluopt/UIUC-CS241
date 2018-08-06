/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#pragma once
#include "document.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

typedef struct {
    /* line_no can be any number >= 1 */
    size_t line_no;
    /**
     * if line_no <= document_size(document)
     * then 0 <= idx <= strlen(document_get_line(document, line_no))
     * else idx = 0
     */
    size_t idx;
} location;

/**
 * Note On Index:
 *
 * For all text editor commands line numbers are 1 indexed while characters
 * within a line are 0 indexed. This means that if you are editing the following
 * 1 line document:
 *
 * I love CS 241.
 *
 * And want to insert "really " into line 1 at character 2 then the result will
 * be:
 *
 * I really love CS 241.
 *
 * The user may only write to the beginning, middle or end of a line.
 * Note that a user may write to a non existing line in which case you should
 * create the line. If the user attempts to write to a non existing line, you
 * may assume the user will always insert at index 0.
 */

/**
 * Gets the path_to_file from argc & argv (it's a one liner).
 */
char *get_filename(int argc, char *argv[]);

/**
 * Opens a new document from a given path_to_file.
 * Use a function from document.h! Returns a pointer to the created document
 * This is also a one liner.
 */
document *handle_create_document(const char *path_to_file);

/**
 * Destroys the document. This is also a one liner.
 */
void handle_cleanup(document *document);

/**
 * Prints out 'max_lines' lines of the document starting from line 'start_line'
 * or until the end of the document is reached.
 * If 'max_lines' is -1, then print the entire document.
 *
 * For each line, 'max_cols' characters will be printed starting from
 * 'start_col_index'. If 'max_cols' is -1, then prints all characters starting
 * from 'start_col_index'.
 */
void handle_display_command(document *document, size_t start_line,
                            ssize_t max_lines, size_t start_col_index,
                            ssize_t max_cols);

/**
 * Inserts 'line' into 'document' at 'loc'. See limits on locations in the
 * struct.
 */
void handle_insert_command(document *document, location loc, const char *line);

/**
 * Appends 'line' to 'document' on line 'line_no'
 * If there is no line at 'line_no' this is the same as write.
 *
 * Also handles escaping characters in 'line', using '\' as the escape
 * character.
 * so if the user passing a string that is ['\', 'n'], this is a newline
 * ['\', a] where a is any character besides 'n' is a
 * if '\' is the last character, ignore it (['\'] -> [])
 */
void handle_append_command(document *document, size_t line_no,
                           const char *line);

/**
 * Writes 'line' into 'document' on line 'line_no'. This will overwrite any
 * existing text at 'line_no'.
 *
 * Also handles escaping characters in 'line', using '\' as the escape
 * character.
 * so if the user passing a string that is ['\', 'n'], this is a newline
 * ['\', a] where a is any character besides 'n' is a
 * if '\' is the last character, ignore it (['\'] -> [])
 */
void handle_write_command(document *document, size_t line_no, const char *line);

/**
 * Deletes 'num_chars' characters from 'document' at 'loc' onwards
 * Note: if strlen(line) - idx <= num_chars
 * then delete all the characters from 'loc' to the end of the line.
 */
void handle_delete_command(document *document, location loc, size_t num_chars);

/**
 * Deletes the 'line_no'th line from 'document'.
 */
void handle_delete_line(document *document, size_t line_no);

/**
 * Returns the location of 'search_str' of 'document' starting at 'loc'.
 * If 'search_str' is not found then line number of the return location is 0.
 * Make sure that the returned location contains the found line number
 * as well as the character index at which the search string starts
 *
 * If `search_str` is "", return a location with `line_no` = 0
 */
location handle_search_command(document *document, location loc,
                               const char *search_str);

/**
 * Merges 'line_no'th line in document with it's following line.
 *
 * Note that this function may not be called on the last line of the document.
 **/
void handle_merge_line(document *document, size_t line_no);

/**
 * Splits the 'loc.line_no'th line in 'document' at the 'loc.idx'th character
 * into two parts. The first part becomes the 'loc.line_no'th line in the
 * document while the second part gets inserted after the 'loc.line_no'th line
 * shifting all the subsequent lines down.
 */
void handle_split_line(document *document, location loc);

/**
 * Writes the document to a file
 * with the specified 'path_to_file'
 */
void handle_save_command(document *document, const char *path_to_file);
