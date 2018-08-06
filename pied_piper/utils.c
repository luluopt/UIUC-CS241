/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int exec_command(const char *str) {
    char *sh_executable = "/bin/sh";
    return execl(sh_executable, sh_executable, "-c", str, (char *)NULL);
}

void reset_file(int fd) {
    if (fd > 0) {
        lseek(fd, 0, SEEK_SET);
    }
}

void destroy_failure(failure_information *info) {
    free((void *)info->error_message);
    return;
}

static int qsort_selector(const void *lhs, const void *rhs) {
    failure_information *lhs_info = (failure_information *)lhs;
    failure_information *rhs_info = (failure_information *)rhs;
    return strcmp(lhs_info->command, rhs_info->command);
}

void print_failure_report(failure_information *info, size_t num) {
    // Do a shallow copy to sort
    failure_information *sorted_info = malloc(sizeof(*info) * num);
    memcpy(sorted_info, info, sizeof(*info) * num);
    qsort(sorted_info, num, sizeof(*sorted_info), qsort_selector);

    for (size_t i = 0; i < num; ++i) {
        const char *command = sorted_info[i].command;
        int status = sorted_info[i].status;
        const char *error_message = sorted_info[i].error_message;
        if (WIFEXITED(status)) {
            printf("Command \"%s\" exited with %d.", command,
                   WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Command \"%s\" signaled with signum %d.", command,
                   WTERMSIG(status));
        }
        if (*error_message != '\0') {
            printf(" Stderror: %s\n", error_message);
        } else {
            printf(" No Standard Error Output.\n");
        }
    }
    free(sorted_info);
}

void print_invalid_input(void) {
    fprintf(stderr, "Error: Invalid input file\n");
}

void print_invalid_output(void) {
    fprintf(stderr, "Error: Invalid output file\n");
}
