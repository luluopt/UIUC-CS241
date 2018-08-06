/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include "libpriqueue.h"
#include "libscheduler.h"

/**
 * Stores information making up a job to be scheduled including any statistics.
*/
typedef struct _job_t {
    int id;
    int priority;

    /* Add whatever other bookkeeping you need into this struct. */
    int arrival_time;
    int requeue_time; // used for RR
    int start_time;
    int end_time;
    int run_time;
    int remain_time;  // used for PSRTF
} job_t;

/**
 * Sores information on how one core is being used.
 */
typedef struct _core_t {
    job_t *job;
    int   slice_start_time;  // everytime the cpu takes a new job, timestamp it
} core_t;

typedef struct _stats_t {
    int turnaround_time;
    int response_time;
    int wait_time;
    int num_jobs;
} stats_t;

static priqueue_t pqueue;
static core_t core;
static scheme_t scheme;
static stats_t stats;
static int (*comparison_func)(const void *, const void *);

// compare arrival_time
int comparer_fcfs(const void *a, const void *b) {
    job_t *ja = (job_t*)a;
    job_t *jb = (job_t*)b;

    if (ja->arrival_time < jb->arrival_time) {
        return -1;
    } else if (ja->arrival_time > jb->arrival_time) {
        return 1;
    } else {
        return 0;
    }
}

int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

// compare priority
int comparer_pri(const void *a, const void *b) {
    job_t *ja = (job_t*)a;
    job_t *jb = (job_t*)b;

    if (ja->priority < jb->priority) {
        return -1;
    } else if (ja->priority > jb->priority) {
        return 1;
    } else {
        return break_tie(a, b);
    }
}

// compare remain_time
int comparer_psrtf(const void *a, const void *b) {
    job_t *ja = (job_t*)a;
    job_t *jb = (job_t*)b;

    if (ja->remain_time < jb->remain_time) {
        return -1;
    } else if (ja->remain_time > jb->remain_time) {
        return 1;
    } else {
        return break_tie(a, b);
    }
}

// compare requeue_time
int comparer_rr(const void *a, const void *b) {
    job_t *ja = (job_t*)a;
    job_t *jb = (job_t*)b;

    if (ja->requeue_time < jb->requeue_time) {
        return -1;
    } else if (ja->requeue_time > jb->requeue_time) {
        return 1;
    } else {
        return break_tie(a, b);
    }
}

// compare run_time
int comparer_sjf(const void *a, const void *b) {
    job_t *ja = (job_t*)a;
    job_t *jb = (job_t*)b;

    if (ja->run_time < jb->run_time) {
        return -1;
    } else if (ja->run_time > jb->run_time) {
        return 1;
    } else {
        return break_tie(a, b);
    }
}

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparison_func = comparer_fcfs;
        break;
    case PRI:
        comparison_func = comparer_pri;
        break;
    case PPRI:
        comparison_func = comparer_ppri;
        break;
    case PSRTF:
        comparison_func = comparer_psrtf;
        break;
    case RR:
        comparison_func = comparer_rr;
        break;
    case SJF:
        comparison_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparison_func);

    // set the global variable scheme
    scheme = s;

    // clear the statistics struct
    memset(&stats, 0, sizeof(stats_t));
}

bool scheduler_new_job(int job_number, unsigned time, unsigned running_time,
                       int priority) 
{
    // create a new job object 
    job_t *new_job = malloc(sizeof(job_t));
    memset(new_job, 0, sizeof(job_t));

    new_job->id           = job_number;
    new_job->priority     = priority;
    new_job->arrival_time = time;
    new_job->requeue_time = time;
    new_job->run_time     = running_time;
    new_job->remain_time  = running_time;
    new_job->start_time   = -1;
    
    // push the object into priority queue
    priqueue_offer(&pqueue, new_job);

    job_t *head_job = (job_t*)priqueue_peek(&pqueue);

    // if CPU is free load the job
    if (core.job == NULL) {
        head_job = (job_t*)priqueue_poll(&pqueue);
        if (head_job->start_time == -1) {
            head_job->start_time = time;
        }
        core.job = head_job;
        core.slice_start_time = time;
        return (head_job->id == new_job->id);
    } 

    // check if the job should be run
    if (scheme == FCFS || scheme == PRI || scheme == SJF || scheme == RR) {
        // no pre-emption for FCFS, PRI, SJF
        // RR will not pre-empt on a new job queue
        // do nothing if we reached here
        return false;
    } else if (scheme == PPRI || scheme == PSRTF) {
        // check if the job had any cycles, mark it as new if it is in the same cycle
        if (core.job->start_time == (int)time) {
            core.job->start_time = -1;
        }
        // update remain time of the job on CPU
        core.job->remain_time -= time - core.slice_start_time;

        // first push the current job back into the queue
        priqueue_offer(&pqueue, core.job);

        // then pull the highest pri job from queue
        head_job = (job_t*)priqueue_poll(&pqueue);
        if (head_job->start_time == -1) {
            head_job->start_time = time;
        }
        core.job = head_job;
        core.slice_start_time = time;
        return (head_job->id == new_job->id);
    } else {
        return false;
    }
    return false;
}

int scheduler_job_finished(int job_number, unsigned time)
{
    if (core.job->id != job_number) {
        printf("ERROR: wrong job finished, something went wrong, job %d finished\n", job_number);
        printf("ERROR: core is running %d\n", core.job->id);
    }
    
    job_t *job = core.job;
    job->end_time = time;

    int turnaround_time = job->end_time   - job->arrival_time;
    int response_time   = job->start_time - job->arrival_time;
    int wait_time       = (job->end_time  - job->arrival_time) - job->run_time;
    free(core.job);  
    core.job = NULL;

    // Record the statistics
    stats.turnaround_time += turnaround_time;
    stats.response_time   += response_time;
    stats.wait_time       += wait_time;
    stats.num_jobs        += 1;
    
    // see if there is anything to schedule
    job_t *head_job = (job_t*)priqueue_poll(&pqueue);

    if (head_job != NULL) {
        if (head_job->start_time == -1) {
            head_job->start_time = time;
        }
        core.job = head_job;
        core.slice_start_time = time;
        return head_job->id;
    } else {
        return -1;
    } 
}

int scheduler_quantum_expired(unsigned time) {
    // check if the job had any cycles, mark it as new if it is in the same cycle
    if (core.job->start_time == (int)time) {
        core.job->start_time = -1;
    }
    // update remain time of the job on CPU
    core.job->remain_time -= time - core.slice_start_time;

    // track when the entry make it back to the queue, this is the sort key
    core.job->requeue_time = time;

    // first push the current job back into the queue
    priqueue_offer(&pqueue, core.job);

    // then pull the highest pri job from queue
    job_t *head_job = (job_t*)priqueue_poll(&pqueue);
    if (head_job->start_time == -1) {
        head_job->start_time = time;
    }
    core.job = head_job;
    core.slice_start_time = time;
    return core.job->id;
}

float scheduler_average_waiting_time()
{
    return (float)(stats.wait_time) / stats.num_jobs;
}

float scheduler_average_turnaround_time()
{
    return (float)(stats.turnaround_time) / stats.num_jobs;
}

float scheduler_average_response_time()
{
    return (float)(stats.response_time) / stats.num_jobs;
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
}


void scheduler_show_queue() {
    // This function is left entirely to you! Totally optional.
}
