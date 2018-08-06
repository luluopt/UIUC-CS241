/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct _job_info {
    int id;
    int priority;

    /* Add whatever other bookkeeping you need into this struct. */
} job_info;

static priqueue_t pqueue;
static scheme_t pqueue_scheme;
comparer_t comparision_func;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    return NULL;
}

int comparer_fcfs(const void *a, const void *b) {
    return 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    return 0;
}

int comparer_psrtf(const void *a, const void *b) {
    return 0;
}

int comparer_rr(const void *a, const void *b) {
    return 0;
}

int comparer_sjf(const void *a, const void *b) {
    return 0;
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}
void scheduler_job_finished(job *job_done, double time) {
}

double scheduler_average_waiting_time() {
    return 9001;
}

double scheduler_average_turnaround_time() {
    return 9001;
}

double scheduler_average_response_time() {
    return 9001;
}

void scheduler_show_queue() {
    // Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
