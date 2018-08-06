/**
 * CS 241 - Systems Programming
 *
 * Pied Piper - Spring 2017
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unistd.h>

void print_usage(char *executable) {
    fprintf(stderr, "Usage: %s [file] [num_times]\n", executable);
    fprintf(stderr, "This program acts as a failure simulator for pipes\n");
    fprintf(stderr, "It will fail the first num_times-1 times\n");
    fprintf(stderr,
            "On the num_times'th call, it will succeed and act like cat\n");
    fprintf(stderr,
            "The file is needed to keep track of the number of failures\n");

    exit(2);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage(argv[0]);
    }

    FILE *log_file = NULL;
    char *fname = argv[1];
    if (access(fname, F_OK) != -1) {
        log_file = fopen(argv[1], "r+");
    } else {
        log_file = fopen(argv[1], "w+");
    }
    if (log_file == NULL) {
        puts("fopen");
        print_usage(argv[0]);
    }

    int num_failures = strtol(argv[2], NULL, 10);
    if (num_failures == 0) {
        fprintf(stderr, "%s is not a valid number of failures\n", argv[2]);
        print_usage(argv[0]);
    }

    int scanned = 0;
    int args = fscanf(log_file, "%d", &scanned);
    if (args != 1) {
        // This file was created for the first time
        // That is alright, that means this is the first time that
        // We are failing.
        printf("%d\n", args);
        scanned = 0;
        rewind(log_file);
        fprintf(log_file, "%d\n", ++scanned);
    } else if (scanned == 0) {
        // We mever write zero as a precaution
        // So if it is never scanned this is an error
        fprintf(stderr, "Corrupted log file\n");
        print_usage(argv[0]);

    } else {
        rewind(log_file);
        fprintf(log_file, "%d\n", ++scanned);
    }

    fprintf(stderr, "%d", scanned);
    if (scanned == num_failures) {
        fclose(log_file);
        unlink(argv[1]); // Remove file for the next run
        execl("/bin/cat", "/bin/cat", (char *)NULL);

        /*
        * Dead code, unless you don't have cat
        * In that case you probably don't have a functioning
        * Linux system lol (you should fix that)
        */
    }
    fflush(log_file);
    fclose(log_file);
    fprintf(stderr, "Simulated failure!\n");
    return 2;
}
