/**
 * Networking
 * CS 241 - Fall 2017
 */
#pragma once
#include <stddef.h>
#include <sys/types.h>

#define LOG(...)                                                               \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    } while (0);

#define S_READING         0
#define S_HEADER_COMPLETE 1


typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;

ssize_t read_from_socket_eol(int socket, char *buffer, size_t max);
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);
size_t get_payload_size(int socket);
void write_payload_size(int socket, size_t size);
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);
void handle_rc(ssize_t rc, size_t read, size_t target);

ssize_t server_read_from_socket_eol(int socket, char *buffer, size_t max, int *status);
ssize_t server_read_all_from_socket(int socket, char *buffer, size_t count, int *conn_closed);
ssize_t server_write_all_to_socket(int socket, const char *buffer, size_t count);
