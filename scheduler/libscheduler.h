/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"

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

  If the job arriving should be scheduled to run during the next time cycle,
  return whether or not the job should be scheduled. If another job is already
  running on the processor, this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it
  will be finished.
  @param priority the priority of the job. (The lower the value, the higher the
  priority.)
  @return True if the job is to be scheduled immediately. False otherwise.

 */
bool scheduler_new_job(int job_number, unsigned time, unsigned running_time,
                       int priority);

/**
  Called when a job has completed execution.

  The job_number and time parameters are provided for convenience. You may be
  able to calculate the values with your own data structure. If any job should
  be scheduled to run, return the job_number of the job that should be scheduled
  to run.

  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on the processor
  @return -1 if the processor should remain idle.
 */
int scheduler_job_finished(int job_number, unsigned time);

/**
  When the scheme is set to RR, called when the quantum timer has expired on the
  processor.

  If any job should be scheduled to run on the processor, return the job_number
  of the job that should be scheduled to run.

  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled
  @return -1 if processor should remain idle
 */
int scheduler_quantum_expired(unsigned time);

/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
  jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_turnaround_time();

/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
  jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_waiting_time();

/**
  Returns the average response time of all jobs scheduled by your scheduler. For
  preemptive scheduling algorithms, use the first time a job was started as the
  job's start time.

  Assumptions:
    - This function will only be called after all scheduling is complete (all
  jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time();

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
