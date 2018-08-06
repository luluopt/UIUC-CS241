/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include "../libpriqueue/libpriqueue.h"
#include "../libscheduler.h"
#include "gthreads.h"

#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Global variables for maintaining state
static job gtmain;       // Main thread
static int counter = 1;  // Counter to switch to main
static job *suspended;   // Job suspended because of switch to main
static job *currjob;     // Current running job
static int gtidx;        // UID of newest thread
static void *leak_stack; // Unused stack to be deleted

/* ------------------------------------------------ */
/* Helper functions */

/* This function gets the current time (wall time) as a double */
static double currtime() {
    struct timespec now;
    double ret = -1;
    ret = clock_gettime(CLOCK_REALTIME, &now);
    if (ret < 0)
        return ret;
    ret = (double)now.tv_sec;
    ret += now.tv_nsec / (double)10000000000;
    return ret;
}

/* This function blocks the signal specified by sig
 *
 * If the signal is SIGALRM all pending alarms will be cleard
 */
static void setSig(int sig) {
    if (sig == SIGALRM) {
        alarm(0);
    }
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig);

    sigprocmask(SIG_BLOCK, &set, NULL);
}

/* This function unblocks a signal specified by sig
 *
 * If the signal is SIGALRM then all pending signals will
 * be unblocked and alarms will be then scheduled to run at 500ms intervals
 */
static void resetSig(int sig) {
    if (sig == SIGALRM) {
        // Clear pending alarms
        alarm(0);
    }
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, sig);

    sigprocmask(SIG_UNBLOCK, &set, NULL);

    if (sig == SIGALRM) {
        // Generate alarms
        ualarm(500, 500);
    }
}
/* ------------------------------------------------ */
/* Internal functions */

/* function from gtswtch.s that actually does the context switch
 * takes in context structs of the old job and the new job
 * that will be run after the switch. This function will only return
 * on the next context switch back to this thread.
 */
void gtswtch(struct jobctx *old, struct jobctx *new);

/* Takes in a job pointer and frees it's memory */
static void gtdestroy(job *job_done) {
    if (leak_stack) {
        free(leak_stack);
        leak_stack = NULL;
    }
    leak_stack = job_done->stack_start;
    free(job_done);
}

/* This function takes in a signal that signifies how and where
 * this function was called.
 *
 * This function has most of the scheduling "magic"
 */
static bool gtyield(int sig) {
    // sig == -1 implies gtstop was called
    struct jobctx *old, *new;

    if (leak_stack) {
        free(leak_stack);
        leak_stack = NULL;
    }

    if ((currjob == &gtmain) && suspended) {
        // switch back to suspended from main
        resetSig(SIGALRM);
        old = &currjob->ctx;
        job *newjob = suspended;
        new = &newjob->ctx;
        currjob = suspended;
        suspended = NULL;
        gtswtch(old, new);
        return true;
    }

    if (sig == SIGALRM || sig == GTSLEEP_SIG) {
        counter++;
        counter %= 10;
        if (!counter) {
            // ctx switch to main
            resetSig(SIGALRM);
            alarm(0);
            ualarm(500, 0);
            suspended = currjob;
            old = &currjob->ctx;
            job *newjob = &gtmain;
            new = &newjob->ctx;
            currjob = &gtmain;
            gtswtch(old, new);
            // return true;
        }
    }
    double time = currtime();
    job *quantum_job = (sig == -1) ? NULL : currjob;
    quantum_job = (quantum_job == &gtmain) ? NULL : quantum_job;

    job *newjob = NULL;
    newjob = scheduler_quantum_expired(quantum_job, time);
    newjob = newjob ? newjob : &gtmain;

    old = &currjob->ctx;
    new = &newjob->ctx;
    resetSig(SIGALRM);
    if (old == new) {
        return false;
    }
    if (sig == -1) {
        // need to destroy currjob, but save old
        if (currjob != &gtmain)
            scheduler_job_finished(currjob, currtime());

        struct jobctx saved;
        memcpy(&saved, old, sizeof(struct jobctx));
        old = &saved;
        gtdestroy(currjob);
    }

    currjob = newjob;

    gtswtch(old, new);
    return true;
}

/* Signal handler that is a wrapper around gtyield */
static void gthandle(int sig) {
    gtyield(sig);
}

/* Automatically called at the end of thread execution */
static void gtstop(void) {
    gtret(-1);
}

/* ------------------------------------------------ */
/* Initialization and setup functions */

/* Initializes the scheduler
 * Takes in a scheme_t determining what scheduling algorithm to use.
 * Also calls signal to setup the handler for SIGALRM defined above and
 * populates gtmain
 */
void gtinit(scheme_t s) {
    scheduler_start_up(s);
    gtidx = 0;
    currjob = &gtmain;
    currjob->state = Running;
    signal(SIGALRM, gthandle);
}

/* Start the actual scheduling process by having some alarms */
void gtstart() {
    ualarm(500, 500);
}

/* This function takes in a function to run and it's scheduler attributes
 * It also sets up the various threading details such as the stack pointer
 * and the context struct
 */
int gtgo(void (*f)(void), scheduler_info *sched_data) {
    char *stack;
    job *p = malloc(sizeof(job));
    scheduler_new_job(p, ++gtidx, currtime(), sched_data);

    stack = malloc(StackSize);
    if (!stack)
        return -1;

    // Set the return address to gtstop so that the thread
    // can be clean up
    *(uint64_t *)&stack[StackSize - 8] = (uint64_t)gtstop;
    *(uint64_t *)&stack[StackSize - 16] = (uint64_t)f;
    p->stack_start = stack;
    p->ctx.rsp = (uint64_t)&stack[StackSize - 16];
    p->state = Ready;
    return 0;
}

/* ------------------------------------------------ */
/* User controlled scheduling */

/* This function returns the current running thread */
void *gtcurrjob() {
    return (void *)currjob;
}

/* This function takes in an integer and passes it on to gtyield.
 * It is an error to call this function with an input of -1 or SIGALRM
 * probably best to call it with 0, but we've left this open to allow for
 * possible extra features to gtyield.
 */
bool gtdoyield(int sig) {
    setSig(SIGALRM);
    bool y = gtyield(sig);
    return y;
}

/* This function acts like sleep(3) but takes in a double and puts itself
 * back onto the scheduler queue
 */
void gtsleep(double sleep_time) {
    double time = currtime();
    while (currtime() - time < sleep_time)
        gtdoyield(GTSLEEP_SIG);
}

/* This function will cause the running thread to stop execution and will
 * effectively
 * take itself off of the queue.
 */
void __attribute__((noreturn)) gtret(int ret) {

    if (!currjob) {
        gtdoyield(ret);
    }
    if (currjob != &gtmain) {
        currjob->state = Unused;
        gtdoyield(ret);
        assert(!"reachable");
    }
    while (gtdoyield(0) || !counter)
        ;

    scheduler_clean_up();
    exit(ret);
}
/* ------------------------------------------------ */
