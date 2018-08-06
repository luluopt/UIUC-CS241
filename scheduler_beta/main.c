/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include "gthreads/gthreads.h"
#include "libscheduler.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef void (*runner_t)(void);

void timer(int t, int id) {
    printf("Timer start!\n");
    for (int i = 0; i < t * 10000; i++) {
        usleep(100);
        if (i % 1000 == 0) {
            printf("Thread %d says hi!\n", t);
        }
    }
}

int globalid;

void f1() {
    int id = ++globalid;
    timer(1, id);
}
void f2() {
    int id = ++globalid;
    timer(2, id);
}
void f3() {
    int id = ++globalid;
    timer(3, id);
}
void f4() {
    int id = ++globalid;
    timer(4, id);
}
void f5() {
    int id = ++globalid;
    timer(5, id);
}

void rr_fcfs_sjf_test(scheme_t s) {
    gtinit(s);
    runner_t functions[5];
    functions[0] = f1;
    functions[1] = f2;
    functions[2] = f3;
    functions[3] = f4;
    functions[4] = f5;
    scheduler_info data;
    for (int i = 0; i < 5; i++) {
        data.running_time = 5 - i;
        data.priority = 1;

        gtgo(functions[5 - i - 1], &data);
    }
    gtstart();
    gtret(0);
};

void psrtf_test() {
    gtinit(PSRTF);
    gtstart();
    scheduler_info data;

    // p2 at 0
    data.running_time = 2;
    gtgo(f2, &data);
    gtsleep(1);
    // p1 at 1000
    data.running_time = 1;
    gtgo(f1, &data);
    gtsleep(2);
    // p5 at 3000
    data.running_time = 5;
    gtgo(f5, &data);
    gtsleep(1);
    // p4 at 4000
    data.running_time = 4;
    gtgo(f4, &data);
    gtsleep(1);
    // p3 at 5000
    data.running_time = 3;
    gtgo(f3, &data);
    // done!
    gtret(0);
}

void ppri_pri_test(scheme_t s) {
    gtinit(s);
    gtstart();
    scheduler_info data;

    data.priority = 3;
    gtgo(f3, &data);
    //
    data.priority = 4;
    gtgo(f4, &data);
    gtsleep(4);
    //
    data.priority = 5;
    gtgo(f5, &data);
    gtsleep(1);
    //
    data.priority = 2;
    gtgo(f2, &data);
    //
    data.priority = 1;
    gtgo(f1, &data);

    gtret(0);
}

void usage() {
    fprintf(stderr, "Usage ./main <scheme>\n");
    fprintf(stderr,
            "Acceptable schemes are: fcfs, sjf, psrtf, pri, ppri, rr\n");
    exit(1);
}

int main(int argc, char **argv) {
    scheme_t s = RR;
    if (argc > 1) {
        // Which scheme are we using?
        if (strcasecmp(argv[1], "FCFS") == 0) {
            s = FCFS;
        } else if (strcasecmp(argv[1], "SJF") == 0) {
            s = SJF;
        } else if (strcasecmp(argv[1], "PSRTF") == 0) {
            s = PSRTF;
        } else if (strcasecmp(argv[1], "PRI") == 0) {
            s = PRI;
        } else if (strcasecmp(argv[1], "PPRI") == 0) {
            s = PPRI;
        } else if (strcasecmp(argv[1], "RR") == 0) {
            s = RR;
        } else {
            usage();
        }
    } else
        usage();

    // We have a valid scheme, let's run the test!
    if (s == FCFS || s == SJF || s == RR)
        rr_fcfs_sjf_test(s);
    else if (s == PSRTF)
        psrtf_test();
    else
        ppri_pri_test(s);

    return 0;
}
