/**
* Know Your Tools
* CS 241 - Fall 2017
*/
#include "utils.h"
#include <stdio.h>
#include <unistd.h>

/*
* You don't need to worry about functions in this file
* You're boss already tested them
*/

ssize_t file_size(int fd) {
    struct stat buf;
    if (fstat(fd, &buf) == -1) {
        perror("fstat");
        return -1;
    }
    return (ssize_t)buf.st_size;
}

void reset_file(int fd) { lseek(fd, 0, SEEK_SET); }

ssize_t min(ssize_t a, ssize_t b) { return a < b ? a : b; }
