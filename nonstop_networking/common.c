/**
 * Networking
 * CS 241 - Fall 2017
 */
#include "common.h"
#include "format.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>

// Read from the socket until we reach newline, in case something goes wrong, a max count is
// provided so we don't exceed the buffer size
ssize_t read_from_socket_eol(int socket, char *buffer, size_t max)
{
    size_t  count = 0;
    ssize_t rc    = 0;

    while (1) {
        rc = read(socket, (void*)buffer, 1);
        // Error checking
        if (rc == 0) {
            // Nothing to read anymore
            print_invalid_response();
            exit(1);
        } else if (rc == -1 && errno == EINTR) {
            continue;
        } else if (rc == -1 && errno != EINTR) {
            print_invalid_response();
            exit(1);
        }

        // If the char is newline
        if (buffer[0] == 0xa) {
            buffer[0] = 0x0;  // replace the \n with \0
            break;
        }
        
        // increment the counter, and move the buffer point along
        count++;
        buffer++;

        // reach the max counter
        if (count >= max) {
            print_invalid_response();
            exit(1);
        }
    }

    return 0;
}

ssize_t server_read_from_socket_eol(int socket, char *buffer, size_t max, int *status)
{
    size_t  count = 0;
    ssize_t rc    = 0;

    while (1) {
        rc = read(socket, (void*)buffer, 1);
        // Error checking
        if (rc == 0) {
            return -1;
        } else if (rc == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        } else if (rc == -1 && errno == EINTR) {
            continue;
        } else if (rc == -1 && errno != EINTR) {
            return -1;
        }

        // If the char is newline
        if (buffer[0] == 0xa) {
            buffer[0] = 0x0;  // replace the \n with \0
            
            *status = S_HEADER_COMPLETE;
            count  += rc;
            buffer += rc;
            break;
        }

        // increment the counter, and move the buffer point along
        count  += rc;
        buffer += rc;

        // reach the max counter
        if (count >= max) {
            return -1;
        }
    }

    return count;
}

// DISCLAIMER: Taken from the implementation in the Chatroom Lab
ssize_t read_all_from_socket(int socket, char *buffer, size_t count)
{
    ssize_t curr_size  = 0;
    ssize_t total_size = 0;

    // Keep reading until we write all, or the write results in an error
    while (1) {
        curr_size = read(socket, (void*)buffer, count);
        if (curr_size == 0) {
            // nothing to read anymore
            break;
        } else if (curr_size == -1 && errno == EINTR) {
            // retry
            continue;
        } else if (curr_size == -1 && errno != EINTR) {
            // read failed
            return -1;
        }

        // Add the to the total number of bytes read 
        total_size += curr_size;
        count      -= curr_size;

        // exit loop if we wrote everything
        if (count <= 0) {
            break;
        } else {
            // move the buffer pointer forward
            buffer = buffer + curr_size;
        }
    }
    
    return total_size;
}

ssize_t server_read_all_from_socket(int socket, char *buffer, size_t count, int *conn_closed)
{
    ssize_t curr_size  = 0;
    ssize_t total_size = 0;

    // Keep reading until we write all, or the write results in an error
    while (1) {
        curr_size = read(socket, (void*)buffer, count);
        if (curr_size == 0) {
            *conn_closed = 1;
            // nothing to read anymore
            break;
        } else if (curr_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        } else if (curr_size == -1 && errno == EINTR) {
            // retry
            continue;
        } else if (curr_size == -1 && errno != EINTR) {
            // read failed
            return -1;
        }

        // Add the to the total number of bytes read 
        total_size += curr_size;
        count      -= curr_size;

        // exit loop if we wrote everything
        if (count <= 0) {
            break;
        } else {
            // move the buffer pointer forward
            buffer = buffer + curr_size;
        }
    }
    
    return total_size;
}

size_t get_payload_size(int socket) {
    size_t  size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char*)&size, sizeof(size_t));
    if (read_bytes == 0 || read_bytes == -1) {
        print_invalid_response();
        exit(1);
    }

    return size; 
}

void write_payload_size(int socket, size_t size)
{
    ssize_t rc = write_all_to_socket(socket, (char*)&size, sizeof(size_t));
    if (rc == 0 || rc == -1) {
        print_invalid_response();
        exit(1);
    }
}

// DISCLAIMER: Taken from the implementation in the Chatroom Lab
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) 
{
    ssize_t curr_size  = 0;
    ssize_t total_size = 0;

    // Keep writing until we write all, or the write results in an error
    while (1) {
        curr_size = write(socket, (void*)buffer, count);
        if (curr_size == 0) {
            // nothing to write anymore
            break;
        } else if (curr_size == -1 && errno == EINTR) {
            // retry
            continue;
        } else if (curr_size == -1 && errno != EINTR) {
            // write failed
            return -1;
        }

        // Add the to the total number of bytes written
        total_size += curr_size;
        count      -= curr_size;

        // exit loop if we wrote everything
        if (count <= 0) {
            break;
        } else {
            // move the buffer pointer forward
            buffer = buffer + curr_size;
        }
    }
    
    return total_size;
}

ssize_t server_write_all_to_socket(int socket, const char *buffer, size_t count) 
{
    ssize_t curr_size  = 0;
    ssize_t total_size = 0;

    // Keep writing until we write all, or the write results in an error
    while (1) {
        curr_size = write(socket, (void*)buffer, count);
        if (curr_size == 0) {
            // nothing to write anymore
            break;
        } else if (curr_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
        } else if (curr_size == -1 && errno == EINTR) {
            // retry
            continue;
        } else if (curr_size == -1 && errno != EINTR) {
            // write failed
            return -1;
        }

        // Add the to the total number of bytes written
        total_size += curr_size;
        count      -= curr_size;

        // exit loop if we wrote everything
        if (count <= 0) {
            break;
        } else {
            // move the buffer pointer forward
            buffer = buffer + curr_size;
        }
    }
    
    return total_size;
}

void handle_rc(ssize_t rc, size_t read, size_t target)
{
    if (rc == -1) {
        print_invalid_response();
        exit(1);
    }
    
    if (rc == 0) {
        print_connection_closed();
        if (read < target) {
            print_too_little_data();
        }
        exit(1);
    }     
}

