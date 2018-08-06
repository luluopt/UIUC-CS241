/**
 * MapReduce
 * CS 241 - Fall 2017
 * hxie13 collab with chu55
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char **argv) {
    char *input_fn;
    char *output_fn;
    char *mapper;
    char *reducer;
    char *splitter = "./splitter";
    int  num_mappers = 0;
    const char *num_mappers_str;
    int i = 0;
    pid_t pid = 0;

    int **mapper_fd_pairs;

    if (argc != 6) {
        print_usage();
        exit(1);
    }

    input_fn  = argv[1];
    output_fn = argv[2];
    mapper    = argv[3];
    reducer   = argv[4];
    num_mappers_str = argv[5];

    sscanf(num_mappers_str, "%d", &num_mappers);

    pid_t procs[2*num_mappers+1];
    char *proc_names[2*num_mappers];

    // Create an input pipe for each mapper.
    mapper_fd_pairs = malloc(num_mappers * sizeof(int*));   // int*[num_mappers]
    for (i=0; i<num_mappers; i++) {
        mapper_fd_pairs[i] = malloc(2 * sizeof(int));       // int[2]
        pipe(mapper_fd_pairs[i]);
    } 

    // Create one input pipe for the reducer.
    int reducer_fd[2];
    pipe(reducer_fd);

    // Open the output file.
    int out_fd = 1;
    if (strcmp("stdout", output_fn) == 0) {
        out_fd = 1;
    } else { 
        FILE *f = fopen(output_fn, "w");
        out_fd  = fileno(f); 
    }

    // Start a splitter process for each mapper.
    for (i=0; i<num_mappers; i++) {
        pid = fork();
        if (pid != 0) {
            // parent process
            close(mapper_fd_pairs[i][1]);  // close input of splitter
            procs[i] = pid;
            proc_names[i] = splitter;
        } else {
            // Code for child process
            close(mapper_fd_pairs[i][0]);
            
            char index[10];
            sprintf(index, "%d", i);
            dup2(mapper_fd_pairs[i][1], 1); // set the pipe output as stdout
            execlp(splitter, splitter, input_fn, num_mappers_str, index, NULL);
            // Only get here if there is an error
            exit(errno);
            break;
        }
    }

    // Start all the mapper processes.
    for (i=0; i<num_mappers; i++) {
        pid = fork();
        if (pid != 0) {
            // parent process
            close(mapper_fd_pairs[i][0]);  // close output of splitter 
            procs[num_mappers+i] = pid;
            proc_names[num_mappers+i] = mapper;
        } else {
            // child process
            close(mapper_fd_pairs[i][1]);
            close(reducer_fd[0]);
            close(0);
            close(1);
            
            dup2(mapper_fd_pairs[i][0], 0);  // set the pipe input as stdin
            dup2(reducer_fd[1], 1);          
            execlp(mapper, mapper, NULL);
            // Only get here if there is an error
            exit(errno);
            break;
        }
    }

    close(reducer_fd[1]); 

    // Start the reducer process.
    pid = fork();
    if (pid != 0) {
        close(reducer_fd[0]);
        procs[2*num_mappers] = pid;
    } else {
        close(reducer_fd[1]);
        close(0);

        dup2(reducer_fd[0], 0);
        dup2(out_fd, 1);
        execlp(reducer, reducer, NULL);
        // Only get here if there is an error
        exit(errno); 
    }

    // Wait for the reducer to finish.
    int status;
    int rc_reducer = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        rc_reducer = WEXITSTATUS(status);
    }

    // Print nonzero subprocess exit codes.
    int es = 0;
    for (i=0; i<2*num_mappers; i++) {
        waitpid(procs[i], &status, 0);
        if (WIFEXITED(status)) {
            if ((es = WEXITSTATUS(status)) != 0) { 
                print_nonzero_exit_status(proc_names[i], es);
            }
        }
    }
    if (rc_reducer != 0) {
        print_nonzero_exit_status(reducer, rc_reducer);
    }

    // Count the number of lines in the output file.
    print_num_lines(output_fn);

    // close the output file
    close(out_fd);

    // memory cleanup
    for (i=0; i<num_mappers; i++) {
        free(mapper_fd_pairs[i]);
    }
    free(mapper_fd_pairs);
    return 0;
}
