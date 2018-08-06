/**
 * Machine Problem: Shell
 * CS 241 - Fall 2017
 */
#include "format.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_usage() {
    printf("./shell -f <filename>\n");
}
void print_command(const char *command) {
    printf("%s\n", command);
}
void print_script_file_error() {
    printf("Unable to open script file!\n");
}
void print_history_file_error() {
    printf("Unable to open history file!\n");
}
void print_prompt(const char *directory, pid_t pid) {
    printf("(pid=%d)%s$ ", pid, directory);
}
void print_no_directory(const char *path) {
    printf("%s: No such file or directory!\n", path);
}
void print_command_executed(pid_t pid) {
    printf("Command executed by pid=%d\n", pid);
}
void print_fork_failed() {
    printf("Fork Failed!\n");
}
void print_exec_failed(const char *command) {
    printf("%s: not found\n", command);
}
void print_wait_failed() {
    printf("Failed to wait on child!\n");
}
void print_setpgid_failed() {
    printf("Failed to start new process group!\n");
}
void print_invalid_command(const char *command) {
    printf("Invalid command: %s\n", command);
}
void print_process_info(const char *status, int pid, const char *name) {
    printf("%s %d\t%s\n", status, pid, name);
}
void print_no_process_found(int pid) {
    printf("%d: no such process\n", pid);
}
void print_stopped_process(int pid, char *command) {
    printf("%d suspended\t%s\n", pid, command);
}
void print_killed_process(int pid, char *command) {
    printf("%d killed\t%s\n", pid, command);
}

void print_history_line(size_t index, const char *command) {
    printf("%zu\t%s\n", index, command);
}
void print_invalid_index() {
    printf("Invalid Index!\n");
}
void print_no_history_match() {
    printf("No Match!\n");
}

char **strsplit(const char *str, const char *delim, size_t *numtokens) {
    // copy the original string so that we don't overwrite parts of it
    // (don't do this if you don't need to keep the old line,
    // as this is less efficient)
    char *s = strdup(str);
    // these three variables are part of a very common idiom to
    // implement a dynamically-growing array
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char *));
    char *token, *strtok_ctx;
    for (token = strtok_r(s, delim, &strtok_ctx); token != NULL;
         token = strtok_r(NULL, delim, &strtok_ctx)) {
        // check if we need to allocate more space for tokens
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char *));
        }
        tokens[tokens_used++] = strdup(token);
    }
    // cleanup
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char *));
    }
    *numtokens = tokens_used;
    free(s);
    // Adding a null terminator
    tokens = realloc(tokens, sizeof(char *) * (tokens_used + 1));
    tokens[tokens_used] = NULL;
    return tokens;
}

void free_args(char **args) {
    char **ptr = args;
    while (*ptr) {
        free(*ptr);
        ptr++;
    }
    free(args);
}

char *get_full_path(char *filename) {
    char *full_path = malloc(sizeof(char *) * (PATH_MAX + 1));
    realpath(filename, full_path);
    return full_path;
}
