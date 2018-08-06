/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#pragma once
#include "vector.h"

/* Forward declare document structure. */
struct document;
typedef struct document document;

/** Allocate and return a new document structure. */
document *document_create();

/**
 * Writes the content of 'document' to a file located at 'path_to_file'.
 *
 * For each line in 'document' write it to the file located at 'path_to_file'
 * with the format
 * ("%s\n", line), where line is a null terminated string.
 *
 * The file may or may not exist already. If it does exist, it should be
 * overwritten.
 * You should set the permissions so that anyone can read or write to the
 * document.
 */
void document_write_to_file(document *this, const char *path_to_file);

/**
 * Creates a document and loads it with the information from
 * the file located at 'path_to_file'.
 *
 * Do NOT store new lines in your underlying vector!
 *
 * Note if for any reason you can not open the file,
 * then you should treat it as an empty file and move on.
 */
document *document_create_from_file(const char *path_to_file);

/**
 * Returns the number of lines in use in 'document'.
 */
size_t document_size(document *this);

/**
 * Frees all memory used to represent 'document'
 */
void document_destroy(document *this);

/**
 * Sets the 'line_number'-th line in 'document' to a copy of 'str'.
 * Note: lines in document are 1-indexed.
 */
void document_set_line(document *this, size_t line_number, const char *str);

/**
 * Returns the string stored in the 'line_number'-th line of 'document'.
 * Note: lines in document are 1-indexed.
 */
const char *document_get_line(document *this, size_t line_number);

/**
 * Inserts a copy of 'str' into the 'line_number'-th line of 'document'.
 *
 * Note: Inserting into the middle of the document shifts all subsequent lines
 * down and inserting past the end fills the gap with empty strings.
 *
 * Note: lines in document are 1-indexed.
 *
 * Note: Inserting NULL is invalid. The document can only contain non-NULL
 * entries.
 */
void document_insert_line(document *this, size_t line_number, const char *str);

/**
 * Deletes the 'line_number'th line line in 'document'.
 *
 * Note: Deleting a line in the middle of the document shifts all subsequent
 * lines up.
 *
 * Note: lines in document are 1-indexed.
 */
void document_delete_line(document *this, size_t line_number);
