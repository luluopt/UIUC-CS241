/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#pragma once
#include <stdlib.h>

typedef struct {
    const char *command;
    int status;
    const char *error_message;
} failure_information;

/**
 * Used to exec a command specified by 'str'
 */
int exec_command(const char *str);

/**
 * Sets the file pointer to point to the very
 * beginning of the file
 */
void reset_file(int fd);

/**
 * Free the memory allocated by the failure struct
 */
void destroy_failure(failure_information *info);

// All your prints

/**
 * Print out a failure report based on the given
 * failure info struct
 */
void print_failure_report(failure_information *info, size_t num);

/**
 * Used to print when an invalid input file or
 * command has been passed in to pied_piper
 */
void print_invalid_input(void);

/**
 * Used to print when the specified output file
 * cannot be written to by pied_piper
 */
void print_invalid_output(void);