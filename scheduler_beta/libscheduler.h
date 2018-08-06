/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "libpriqueue/libpriqueue.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Struct that holds information for creating
 * a new job
*/
typedef struct {
    double running_time;
    double priority;
} scheduler_info;

/**
 * Struct that defines a thread.
 * You should NOT modify any field except metadata
*/
typedef struct {
    struct jobctx {
        uint64_t rsp;
        uint64_t r15;
        uint64_t r14;
        uint64_t r13;
        uint64_t r12;
        uint64_t rbx;
        uint64_t rbp;
    } ctx;
    enum {
        Unused,
        Running,
        Ready,
    } state;
    void *stack_start;
    void *metadata; // This is where job_info should go!
} job;

/**
  Constants which represent the different scheduling algorithms
*/
typedef enum {
    FCFS,  // First Come First Served
    PPRI,  // Preemptive Priority
    PRI,   // Priority
    PSRTF, // Preemptive Least Remaining Time First
    RR,    // Round Robin
    SJF    // Shortest Job First
} scheme_t;

/*
 * The following comparers can return -1, 0, or 1 based on the following:
 * -1 if 'a' comes before 'b' in the ordering.
 *  1 if 'b' comes before 'a' in the ordering.
 *  0 if break_tie() returns 0.
 *
 *  Note: if 'a' and 'b' have the same ordering,
 *  then return whatever break_tie() returns.
 */

// Comparater for First Come First Serve
int comparer_fcfs(const void *a, const void *b);
// Comparater for Premptive Priority
int comparer_ppri(const void *a, const void *b);
// Comparater for Priority
int comparer_pri(const void *a, const void *b);
// Comparater for Preemptive Shortest Job First
int comparer_psrtf(const void *a, const void *b);
// Comparater for Round Robin
int comparer_rr(const void *a, const void *b);
// Comparater for Shortest Job First
int comparer_sjf(const void *a, const void *b);

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that scheme is a valid scheduling scheme.

  @param s  the scheduling scheme that should be used. This value will be one of
  the six enum values of scheme_t
*/
void scheduler_start_up(scheme_t s);

/**
  Called when a new job arrives.

  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time.
  @param running_time the total number of time units this job will run before it
  will be finished.
  @param priority the priority of the job. (The lower the value, the higher the
  priority.)

 */
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *stats);

/**
  Called when a job has completed execution.

  This function should clean up the metadata and possible
  collect information from the thread. Do NOT free job_done.

  @param job_done pointer to the job that has recenty finished
  @param time the current time.
 */
void scheduler_job_finished(job *job_done, double time); //

/**

  This function is called when the quantum timer has expired. It will be called
  for every scheme.

  If the last running thread has finished or there were no threads previously
  running, job_evicted will be NULL.
  You should return the job* of the next thread to be run. If there are no
  waiting
  threads, return NULL.

  Note that if you are using a non-preemptive scheme this function should
  return job_evicted if it is not NULL.

  @param time the current time.
  @return pointer to job that should be scheduled
  @return NULL if there are no more jobs
 */
job *scheduler_quantum_expired(job *job_evicted, double time);

/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
  jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
double scheduler_average_waiting_time();

/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
  jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
double scheduler_average_turnaround_time();

/**
  Returns the average response time of all jobs scheduled by your scheduler. For
  preemptive scheduling algorithms, use the first time a job was started as the
  job's start time.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
  jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
double scheduler_average_response_time();

/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up();

/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.

  In our provided output, we have implemented this function to list the jobs in
  the order they are to be scheduled. Furthermore, we have also listed the
  current state of the job (either running on the processor or idle). For
  example, if we have a non-preemptive algorithm and job(id=4) has began
  running, job(id=2) arrives with a higher priority, and job(id=1) arrives with
  a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue();
