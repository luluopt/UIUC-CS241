/**
* Know Your Tools
* CS 241 - Fall 2017
*/
#include "shred.h"
#include "utils.h"
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define PAGE_SIZE 4096

static const int iterations = 3;

char *random_string(ssize_t size) {
    static int random_fd = 0;
    if (random_fd == 0) {
        random_fd = open("/dev/urandom", O_RDONLY);
        if (random_fd < 0) {
            perror("open /dev/urandom");
            exit(1);
        }
    }
    char *buf = malloc(size + 1); // Remember the null byte!
    read(random_fd, buf, size);
    buf[size] = '\0';
    return buf;
}

void write_random(int fd, ssize_t num_bytes) {
    reset_file(fd);
    char *buffer;
    for (; num_bytes > 0; num_bytes -= PAGE_SIZE) {
        ssize_t string_size = min(PAGE_SIZE, num_bytes + 1);
        buffer = random_string(string_size);
        dprintf(fd, "%s", buffer);
	free(buffer); //
    }
    return;
}

int shred(const char *filename) {
    int fd = open(filename, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "open %s", filename);
        perror("");
        exit(2);
        return -1;
    }
    ssize_t size = file_size(fd);
    if (size <= 0) {
        return -1;
    }
    for (int i = 0; i < iterations; ++i) {
        write_random(fd, size);
    }
    close(fd);
    return 1;
}
