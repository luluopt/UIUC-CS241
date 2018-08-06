/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#pragma once
#define EXIT_BAD_INPUT 3
#define EXIT_BAD_OUTPUT 4
#define EXIT_OUT_OF_RETRIES 5

int pied_piper(int input_fd, int output_fd, char **executables);
