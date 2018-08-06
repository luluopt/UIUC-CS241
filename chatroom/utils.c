/**
 * Chatroom Lab
 * CS 241 - Fall 2017
 */
 
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    uint32_t write_size = htonl(size);
    return write_all_to_socket(socket, (char*)&write_size, MESSAGE_SIZE_DIGITS);
}

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
