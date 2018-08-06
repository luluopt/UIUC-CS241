/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pied_piper.h"
#include "utils.h"
#include <fcntl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_usage(const char *exec_name) {
    fprintf(stderr, "Usage: %s [-i input file] [-o output file]", exec_name);
    fprintf(stderr, "\"command1 args*\" ...\n");
    fprintf(stderr,
            "This executable is a pipe resillence module. Meaning that it");
    fprintf(
        stderr,
        "tries to execute the chain of pipes and if there is a failure under");
    fprintf(stderr,
            "various conditions, it reports back the errors to the user");
    exit(2);
}

static int try_open_input(const char *input_path) {
    if (!input_path) {
        return -1;
    }
    int fd = open(input_path, O_RDONLY);
    if (fd < 0) {
        print_invalid_input();
        exit(EXIT_BAD_INPUT);
    }
    return fd;
}

static int try_open_output(const char *out_path) {
    if (!out_path) {
        return -1;
    }
    int fd = open(out_path, O_WRONLY | O_TRUNC | O_CREAT,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd < 0) {
        print_invalid_output();
        exit(EXIT_BAD_OUTPUT);
    }
    return fd;
}

char **parse_arguments(int argc, char *argv[], char **input, char **output) {

    *input = NULL;
    *output = NULL;

    int c;

    while ((c = getopt(argc, argv, "i:o:h")) != -1) {
        switch (c) {
        case 'i':
            *input = strdup(optarg);
            break;
        case 'o':
            *output = strdup(optarg);
            break;
        case '?':
        case 'h':
        default:
            print_usage(argv[0]);
        }
    }

    return argv + optind;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_usage(argv[0]);
    }
    char *input, *output;
    char **execs = parse_arguments(argc, argv, &input, &output);
    int in_fd = try_open_input(input);
    int out_fd = try_open_output(output);
    free(input);
    free(output);
    if (*execs == NULL) {
        print_usage(argv[0]);
    }
    return pied_piper(in_fd, out_fd, execs);
}
