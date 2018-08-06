/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#include "pied_piper.h"
#include "utils.h"
#include <fcntl.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TRIES 3


int pied_piper(int input_fd, int output_fd, char **executables)
{
    // Clean-up input and output, use stdin stdout if needed
    input_fd  = input_fd < 0  ? STDIN_FILENO : input_fd;
    output_fd = output_fd < 0 ? STDOUT_FILENO : output_fd; 

    // Find number of processes to execute
    char **temp     = executables;
    int    num_proc = 0;

    while (*temp != NULL) {
        temp++;
        num_proc++;
    }

    fflush(stdout);

    // Try executing the entire pipeline
    pid_t pid;
    pid_t child_pids[num_proc];
    int   io_fd[num_proc-1][2];
    int   err_fd[num_proc][2];
    failure_information info[num_proc];


    for (int k=0; k<TRIES; k++) {
        // initialize error pipe if it's the last attempt
        if (k == TRIES - 1) {
            for (int i=0; i<num_proc; i++) {
                pipe(err_fd[i]);
            }
        }
        

        // initialize io pipes
        for (int i=0; i<num_proc-1; i++) {
            pipe(io_fd[i]);
        }
        

        // exec
        for (int i=0; i<num_proc; i++) {
            pid = fork();

            if (pid != 0) {
                // parent process
                if (i != 0 ) {
                    close(io_fd[i-1][0]);  // close pipe input of the previous stage
                }
                if (i < num_proc - 1) {
                    close(io_fd[i][1]);    // close pipe output of the current stage
                }
                if (k == TRIES - 1) {
                    close(err_fd[i][1]);
                }


                // store the child pid's
                child_pids[i] = pid;
            } else {
                // child process
                if (i != 0) {
                    close(io_fd[i-1][1]);   // close the output end of the previous pipe
                    dup2(io_fd[i-1][0], 0);    
                } else {
                    dup2(input_fd, 0);
                }
                if (i < num_proc - 1) {
                    close(io_fd[i][0]);     // for the actual command close the input end of the pipe
                    dup2(io_fd[i][1], 1);
                } else {
                    dup2(output_fd, 1);
                }
                if (k == TRIES - 1) {
                    close(err_fd[i][0]);
                    dup2(err_fd[i][1], STDERR_FILENO);
                }
                

                // Run the actual command
                exec_command(executables[i]);


                // Close the pipes if exec() failed
                if (i != 0) {
                    close(io_fd[i-1][0]);
                } 
                if (i < num_proc - 1) {
                    close(io_fd[i][1]);
                }
                if (k == TRIES - 1) {
                    close(err_fd[i][1]);
                    close(input_fd);
                }
                close(output_fd);
                exit(1);
            }
        }



        bool success = true;
        // Parent process again, wait for all children to complete
        for (int i=0; i<num_proc; i++) {
            waitpid(child_pids[i], &(info[i].status), 0);
            
            // If any stage has a non-zero exit status
            if (WEXITSTATUS(info[i].status)) {
                success = false;
            }
        }
        

        // return from function if all stages are successful
        if (success) {
            close(input_fd);
            fflush(stdout);
            close(output_fd);

            return EXIT_SUCCESS;
        } else {
            // Try again unless it's the last attempt
            if (k == TRIES - 1) {
                // record error information for printing
                for (int i=0; i<num_proc; i++) {
                    // reserve 4KB worth of error message space for each stage
                    char *msg = malloc(4096);
                    memset(msg, 0, 4096);
                    read(err_fd[i][0], msg, 4095);
                    close(err_fd[i][0]);
                    info[i].error_message = msg;
                    info[i].command = executables[i];
                }
                

                // print out failure report
                print_failure_report(info, num_proc);

                close(input_fd);
                fflush(stdout);


                // memory cleanup
                for (int i=0; i<num_proc; i++) {
                    destroy_failure(&(info[i]));
                }

                close(output_fd);
                return EXIT_OUT_OF_RETRIES;
            }
        }
    } 
     
     
     
    return EXIT_OUT_OF_RETRIES;
}
