/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libscheduler.h"
#include "vector.h"

typedef struct _simulator_job_list_t {
    int job_id;
    int arrival_time, run_time, priority;
    bool is_running;
    bool arrived;
} simulator_job_list_t;

void print_usage(char *program_name) {
    fprintf(stderr, "Usage: %s -s <scheme> <input file>\n", program_name);
    fprintf(stderr, "       %s -s fcfs examples/proc1.in\n", program_name);
    fprintf(stderr, "\n");
    fprintf(stderr,
            "Acceptable schemes are: fcfs, sjf, psrtf, pri, ppri, rr#\n");
}

bool set_active_job(const int job_id, vector *const jobs) {
    for (void **it = vector_begin(jobs); it != vector_end(jobs); ++it) {
        simulator_job_list_t *const job = *(simulator_job_list_t **)it;
        if (job->job_id == job_id && job->arrived) {
            job->is_running = true;
            return true;
        }
    }

    return false;
}

void print_available_jobs(vector *const jobs) {
    printf("Active jobs are: ");

    bool first = true;
    for (void **it = vector_begin(jobs); it != vector_end(jobs); ++it) {
        simulator_job_list_t *const job = *(simulator_job_list_t **)it;
        if (job->arrived) {
            if (first) {
                printf("%d", job->job_id);
                first = false;
            } else
                printf(", %d", job->job_id);
        }
    }

    if (!first) {
        printf("\n");
    }
}

void print_core_timing_diagram(vector *const core_timing_diagram) {
    printf("  History: ");
    VEC_FOR_EACH(core_timing_diagram, { printf("%s", elem); });
    printf("\n");
}

void *simulator_job_copy_constructor(void *elem) {
    simulator_job_list_t *copy = malloc(sizeof(*copy));
    *copy = *((simulator_job_list_t *)elem);
    return copy;
}

vector *parse_file(const char *const file_name) {
    /*
     * Open the file, read the file, and populate the jobs data structure.
     */
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file \"%s\".\n", file_name);
        return NULL;
    }

    size_t job_unique_id = 0;
    vector *jobs = vector_create(simulator_job_copy_constructor, free, NULL);

    char *line = NULL;
    size_t len = 0;
    getline(&line, &len, file); // Ignore the first (header) line
    while (getline(&line, &len, file) != -1) {
        char *arrival_time = strtok(line, ",");
        char *run_time = strtok(NULL, ",");
        char *priority = strtok(NULL, ",");

        if (arrival_time == NULL || run_time == NULL || priority == NULL) {
            fprintf(stderr,
                    "Illegal file format. Are you using a *.in file?\n");
            return NULL;
        }

        simulator_job_list_t elem;
        elem.job_id = job_unique_id++;
        elem.arrival_time = atoi(arrival_time);
        elem.run_time = atoi(run_time);
        elem.priority = atoi(priority);
        elem.is_running = false;
        elem.arrived = false;

        vector_push_back(jobs, &elem);
    }
    free(line);
    fclose(file);
    return jobs;
}

int main(int argc, char **argv) {
    int c;
    int scheme = -1;
    unsigned long quantum = 0;
    char *file_name;

    /*
     * Parse command line options.
     */
    while ((c = getopt(argc, argv, "s:")) != -1) {
        switch (c) {
        case 's':
            if (strcasecmp(optarg, "FCFS") == 0) {
                scheme = FCFS;
            } else if (strcasecmp(optarg, "PPRI") == 0) {
                scheme = PPRI;
            } else if (strcasecmp(optarg, "PRI") == 0) {
                scheme = PRI;
            } else if (strcasecmp(optarg, "PSRTF") == 0) {
                scheme = PSRTF;
            } else if (strncasecmp(optarg, "RR", 2) == 0) {
                scheme = RR;
                const long signed_quantum = strtol(optarg + 2, NULL, 10);

                if (signed_quantum <= 0) {
                    fprintf(stderr,
                            "Option -s RR# requires a positive number for the "
                            "quantum of RR. (Eg: -s RR2)\n");
                    print_usage(argv[0]);
                    return 1;
                }
                quantum = signed_quantum;
            } else if (strcasecmp(optarg, "SJF") == 0) {
                scheme = SJF;
            } else {
                // do nothing
            }
            break;

        case '?':
            print_usage(argv[0]);
            return 1;

        default:
            printf("...\n");
            break;
        }
    }

    if (scheme == -1) {
        fprintf(stderr,
                "Required option -s <scheme> is not present or invalid.\n");
        print_usage(argv[0]);
        return 1;
    }

    if (optind == argc - 1) {
        file_name = argv[optind];
    } else {
        fprintf(stderr, "A single input file is required.\n");
        print_usage(argv[0]);
        return 1;
    }

    /*
     * Parse the input file.
     */
    vector *const jobs = parse_file(file_name);
    assert(jobs);

    /*
     * Run the simulation.
     */

    printf("Loaded %zu job(s) using ", vector_size(jobs));
    if (scheme == FCFS) {
        printf("First Come First Served (FCFS)");
    } else if (scheme == PPRI) {
        printf("Preemptive Priority (PPRI)");
    } else if (scheme == PRI) {
        printf("Non-preemptive Priority (PRI)");
    } else if (scheme == PSRTF) {
        printf("Preemptive Shortest Remaining Time First (PSRTF)");
    } else if (scheme == RR) {
        printf("Round Robin (RR) with a quantum of %lu", quantum);
    } else if (scheme == SJF) {
        printf("Non-preemptive Shortest Job First (SJF)");
    }
    printf(" scheduling...\n\n");

    scheduler_start_up(scheme);

    /**
     * A counter that keeps track of how many ticks the simulator has seen.
     * Functions much like a clock.
     */
    unsigned int time = 0;

    /**
     * A counter that tracks how many jobs have arrived and have not yet been
     * completed. Once a job arrives for the first time, this counter
     * increments.
     * Once a job completes, this counter decrements.
     */
    int jobs_alive = 0;

    /**
     * A counter that tracks how far along the current time quantum the
     * simulator
     * has progressed. -1 for scheduling schemes that are not RR. If the scheme
     * is RR, the value is always in the interval [0, quantum] (where quantum is
     * provided as a command-line argument)/
     */
    ssize_t quantum_clock = -1;

    /**
     * A string that displays which job the core was running at each time unit.
     * That is, the first entry of this string will be the job ID that was
     * running
     * during the first tick. The second entry of this string will be the job ID
     * that was running during the second tick, and so on.
     */
    vector *core_timing_diagram =
        vector_create((void *(*)(void *))strdup, free, NULL);

    while (!vector_empty(jobs)) {
        printf("=== [TIME %d] ===\n", time);

        /*
         * 1. Check if any jobs finished in the last time unit.
         */
        for (ssize_t i = 0; i < (unsigned)vector_size(jobs); i++) {
            // .run_time is >0 if job hasn't been completed yet
            // ==0 if it JUST finished
            // ==-1 if it is already done
            const simulator_job_list_t *const job = vector_get(jobs, i);
            if (job->run_time == 0) {
                // Notify the scheduler has finished
                const int old_job_id = job->job_id;
                const int new_job_id =
                    scheduler_job_finished(job->job_id, time);

                if (scheme == RR)
                    quantum_clock = quantum;

                // Delete the finished jobs, decrease the number of active jobs
                vector_erase(jobs, i);
                jobs_alive--;
                i--;

                // Set the new job
                if (new_job_id != -1 && !set_active_job(new_job_id, jobs)) {
                    printf("The scheduler_job_finished() selected an invalid "
                           "job (job_id "
                           "== %d).\n",
                           new_job_id);
                    print_available_jobs(jobs);
                    return 3;
                } else {
                    printf(
                        "Job %d finished. Processor is now running job %d.\n",
                        old_job_id, new_job_id);
                    printf("  Queue: ");
                    scheduler_show_queue();
                    printf("\n\n");
                }
            }
        }

        /*
         * Check to see if we finished our last job.  (If we don't check here,
         * we
         * would run an extra time unit that will be totally idle.)
         */
        if (vector_empty(jobs))
            break;

        /*
         * 2. Check of any quantums expired in the last time unit.
         */
        if (scheme == RR && quantum_clock == 0) {
            for (void **it = vector_begin(jobs); it != vector_end(jobs); ++it) {
                simulator_job_list_t *const job = *(simulator_job_list_t **)it;
                if (job->is_running) {
                    // Notify the scheduler the quantum has expired
                    const int old_job_id = job->job_id;
                    const int new_job_id = scheduler_quantum_expired(time);

                    job->is_running = false;

                    quantum_clock = quantum;

                    // Set the new job
                    if (new_job_id != -1 && !set_active_job(new_job_id, jobs)) {
                        printf("The scheduler_quantum_expired() selected an "
                               "invalid job "
                               "(job_id == %d).\n",
                               new_job_id);
                        print_available_jobs(jobs);
                        return 3;
                    } else {
                        printf("Job %d had its quantum expire. "
                               "Processor is now running job %d.\n",
                               old_job_id, new_job_id);
                        printf("  Queue: ");
                        scheduler_show_queue();
                        printf("\n\n");
                    }

                    break;
                }
            }
        }

        /*
         * 3. Check for any new jobs that arrive in this time unit
         */
        for (void **it = vector_begin(jobs); it != vector_end(jobs); ++it) {
            simulator_job_list_t *const job = *(simulator_job_list_t **)it;
            if ((unsigned)job->arrival_time == time) {
                const bool schedulable = scheduler_new_job(
                    job->job_id, time, job->run_time, job->priority);

                job->arrived = true;
                jobs_alive++;

                if (schedulable) {
                    printf("A new job, job %d (running time=%d, priority=%d), "
                           "arrived. "
                           "Job %d is now running on processor.\n",
                           job->job_id, job->run_time, job->priority,
                           job->job_id);
                    printf("  Queue: ");
                    scheduler_show_queue();
                    printf("\n\n");

                    // Remove the job (if any) that is using the core
                    for (void **it = vector_begin(jobs); it != vector_end(jobs);
                         ++it) {
                        simulator_job_list_t *const job =
                            *(simulator_job_list_t **)it;
                        job->is_running = false;
                    }

                    // Assign the processor to the new job
                    job->is_running = true;

                    if (scheme == RR) {
                        quantum_clock = quantum;
                    }
                } else {
                    printf("A new job, job %d (running time=%d, priority=%d), "
                           "arrived. "
                           "Job %d is set to idle (-1).\n",
                           job->job_id, job->run_time, job->priority,
                           job->job_id);
                    printf("  Queue: ");
                    scheduler_show_queue();
                    printf("\n\n");
                }
            }
        }

        /*
         * 4. Run the time unit.
         */
        char *time_string = NULL;
        unsigned jobs_currently_running = 0;

        for (void **it = vector_begin(jobs); it != vector_end(jobs); ++it) {
            simulator_job_list_t *const job = *(simulator_job_list_t **)it;
            if (job->is_running) {
                jobs_currently_running++;
                job->run_time--;
                quantum_clock--;

                // There should only be one running job at a time. If there are
                // two,
                // the second running job will fail this assertion.
                assert(time_string == NULL);

                const char BASE_62[] = "0123456789"
                                       "abcdefghijklmnopqrstuvwxyz"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                const unsigned long MAX_REPRESENTATION =
                    sizeof(BASE_62) / sizeof(*BASE_62);

                if ((unsigned)job->job_id < MAX_REPRESENTATION) {
                    asprintf(&time_string, "%c", BASE_62[job->job_id]);
                } else {
                    asprintf(&time_string, "(%d)", job->job_id);
                }
            }
        }

        // If the core is idle, print a '-'
        if (time_string == NULL) {
            asprintf(&time_string, "-");
        }

        vector_push_back(core_timing_diagram, time_string);
        free(time_string);

        /*
         * 5. Print data!
         */
        printf("At the end of time unit %d...\n", time);
        print_core_timing_diagram(core_timing_diagram);
        printf("\n");

        printf("  Queue: ");
        scheduler_show_queue();
        printf("\n\n");

        /*
         * 6. Sanity Checking
         *
         * If there's a job alive (needing to be ran) and all CPUs are idle, the
         * scheduler failed to schedule properly.
         *
         * If there are ever more than two jobs running during one time instant,
         * the
         * scheduler failed to schedule properly because there is only one
         * processor.
         */
        if (jobs_alive > 0 && jobs_currently_running != 1) {
            printf("All cores are idle and at least one job remains "
                   "unscheduled.\n");
            print_available_jobs(jobs);
            return 3;
        } else if (jobs_currently_running > 1) {
            printf("Two jobs are running concurrently.\n");
            return 3;
        }

        /*
         * 7. Increase time
         */
        time++;
    }

    printf("FINAL TIMING DIAGRAM:\n");
    print_core_timing_diagram(core_timing_diagram);

    printf("\n");
    printf("Average Waiting Time: %.2f\n", scheduler_average_waiting_time());
    printf("Average Turnaround Time: %.2f\n",
           scheduler_average_turnaround_time());
    printf("Average Response Time: %.2f\n", scheduler_average_response_time());

    scheduler_clean_up();

    vector_destroy(core_timing_diagram);
    vector_destroy(jobs);

    return 0;
}
