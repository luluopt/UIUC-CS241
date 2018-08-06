/**
 * Networking
 * CS 241 - Fall 2017
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>

// Error message to send in response to a malformed request
extern const char *err_bad_request;

// Error message to send if the client sends too little or too much data
extern const char *err_bad_file_size;

// Error message if a client tries to GET or DELETE a non existent file
extern const char *err_no_such_file;

/**
 * Used in client.c in the event that command line arguments are missing or
 * trivially wrong; prints basic usage information.
 */
void print_client_usage(void);

/**
 * Used in client.c in the event that the command line arguments to the client
 * program are invalid; prints more detailed usage information.
 */
void print_client_help(void);

/**
 * Use this function in client.c in the event that the client's connection
 * with the server is closed prematurely, and then exit the program.
 */
void print_connection_closed(void);

/**
 * Use this function in client.c to print out the error message provided by the
 * server in the event that the server's status message is prefaced with
 * "ERROR\n".
 */
void print_error_message(char *err);

/**
 * Use this function in client.c and server.c in the event that the
 * client/server does not send a valid response or header as per the protocol
 * described in the docs.
 */
void print_invalid_response(void);

/**
 * Use this function in client.c and server.c in the event that the
 * client/server is able to receive more bytes of file data than the
 * server/client had specified in its header.
 */
void print_too_little_data(void);

/**
 * Use this function in client.c and server.c in the event that the
 * client/server is able to receive more bytes of file data than the
 * server/client had specified in its header.
 */
void print_received_too_much_data(void);

/**
 * Use this function in client.c if the server successfully fulfilled a PUT or
 * DELETE request.
 */
void print_success(void);

/**
 * Use this function in server.c after creating a temporary directory in order
 * to print out the name of the temporary directory to standard output.
 */
void print_temp_directory(char *temp_directory);
