/**
* Know Your Tools
* CS 241 - Fall 2017
*/
#include "secure_move.h"

#define NUM_LINES_HEAD 5
#define NUM_LINES_TAIL 5

void print_usage(char *filename) {
    fprintf(stderr, "Usage:\n\n");
    fprintf(stderr, "\t%s: <path> <destination>\n\n", filename);
    fprintf(stderr,
            "%s takes a file located at <path>, copies the file into memory\n",
            filename);
    fprintf(stderr, "then wraps the file to 80 characters maximum and\n");
    fprintf(stderr, "overwrites the file 3 times to prevent recovery.\n");
    fprintf(stderr,
            "Then it writes to the file at <desination> the following\n");
    fprintf(stderr, "\t- The first five lines (after wrapping)\n");
    fprintf(stderr, "\t- A newline\n");
    fprintf(stderr, "\t- The last five lines (after wrapping)\n");
    fprintf(stderr, "\t- A newline\n");
    fprintf(stderr, "\t- The rest of the file\n");
}

void print_head(file *input, int fd) {
    for (size_t i = 0; i < NUM_LINES_HEAD; ++i) {
        dprintf(fd, "%s\n", input->lines[i]);
    }
    dprintf(fd, "\n");
}

void print_tail(file *input, int fd) {
    for (size_t i = input->num_lines - NUM_LINES_TAIL; i < input->num_lines;
         ++i) {
        dprintf(fd, "%s\n", input->lines[i]);
    }
    dprintf(fd, "\n");
}

void print_entire_file(file *input, int fd) {
    for (size_t i = 0; i < input->num_lines; ++i) {
        dprintf(fd, "%s\n", input->lines[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }
    char *file_name = argv[1];
    char *dest = argv[2];
    file *input_file = read_wrap(file_name);
    shred(file_name);
    int fd = open(dest, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd <= -1) {
        perror("");
        exit(2);
    }
    if (input_file->num_lines > 10) {
        print_head(input_file, fd);
        print_tail(input_file, fd);
    }
    print_entire_file(input_file, fd);
    destroy_file(input_file);
    close(fd);
    return 0;
}
