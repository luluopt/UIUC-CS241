/**
 * Machine Problem: Password Cracker
 * CS 241 - Fall 2017
 */

#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "queue.h"

#include <stdio.h>
#include <string.h>
#include <crypt.h>

typedef struct task_st {
    char *username;
    char *digest;
    char *prefix;
    int  prefix_length;
    int  pw_length;
    int  unknown_length;
} task;

typedef struct thread_args_st {
    int    tid;
    queue *q;
    int    passes;
    int    fails;
} thread_args;

// Create a task object that contains all the information about cracking one password
task *create_task(const char *username, const char *digest, const char *hint)
{
    // Allocate and initialize an object
    task *t = malloc(sizeof(task));
    memset(t, 0, sizeof(task)); 
    
    int length = 0;
    // allocate space for username, and copy from temp buffer
    length = strlen(username);
    t->username = malloc(sizeof(char) * (length + 1));   // +1 is for null termination
    memcpy(t->username, username, length + 1);

    // allocate space for digest, and copy from temp buffer
    length = strlen(digest);
    t->digest = malloc(sizeof(char) * (length + 1));
    memcpy(t->digest, digest, length + 1);

    // allocate space to store the prefix, also record information about password
    t->prefix_length  = getPrefixLength(hint);
    t->pw_length      = strlen(hint);
    t->unknown_length = t->pw_length - t->prefix_length;
    t->prefix         = malloc(sizeof(char) * (t->prefix_length + 1));
    memset(t->prefix, 0, t->prefix_length + 1);
    memcpy(t->prefix, hint, t->prefix_length);

    return t;
}

void delete_task(task *t) 
{
    free(t->username);
    free(t->digest);
    free(t->prefix);
    free(t);
}

char *crack_pw(task *t, int tid, int thread_count, long *hash_count)
{
    long start_idx;
    long attempt_count;

    // create the initial password attempt string
    char *pw_attempt = malloc(sizeof(char) * (t->pw_length + 1));
    memset(pw_attempt,   0, t->pw_length+1);  // initialize to 0s
    memset(pw_attempt, 'a', t->pw_length);  // fill with 'a' except for null termination
    memcpy(pw_attempt, t->prefix, t->prefix_length);

    // determine the number of attempts on a thread
    getSubrange(t->unknown_length, thread_count, tid, &start_idx, &attempt_count);

    // set the attempt string to the appropriate start
    for (int i=0; i<start_idx; i++) {
        incrementString(pw_attempt);
    }

    // brute force password cracking
    long i = 0;
    const        char *hashed;
    struct crypt_data  cdata;
    cdata.initialized = 0;

    for (i=0; i<attempt_count; i++) {
        hashed = crypt_r(pw_attempt, "xx", &cdata);
        
        // stop when the generated hash matches the actual hash
        if (strcmp(hashed, t->digest) == 0) {
            char *result = malloc(sizeof(char) * (t->pw_length + 1));
            memset(result, 0, t->pw_length+1);
            memcpy(result, pw_attempt, t->pw_length);
            *hash_count = i+1; 

            free(pw_attempt);
            return result;
        } else {
            incrementString(pw_attempt);
        }
    }

    *hash_count = attempt_count;

    free(pw_attempt);
    return NULL;
}

// arg is of type thread_args
void *pw_cracker_start(void *arg)
{
    thread_args *t_args = (thread_args*)arg;
    int          tid    = t_args->tid;
    queue *task_queue   = t_args->q;

    task  *t  = NULL;
    long   hash_count    = 0;
    char  *pw = NULL;
    double start_time   = 0.0;
    double end_time     = 0.0;
    
    // The PW cracker sits idle until there is an new entry into the task queue
    while (1) {
        // Check if there is an entry to the queue
        t = (task*)queue_pull(task_queue);

        if (t == NULL) {
            break;
        }
      
        v1_print_thread_start(tid, t->username); 

        // Crack password, would not stop until we found a solution
        start_time = getThreadCPUTime();
        pw = crack_pw(t, 1, 1, &hash_count);
        end_time   = getThreadCPUTime();

        if (pw != NULL) {
            v1_print_thread_result(tid, t->username, pw, hash_count, end_time - start_time, 0);
            t_args->passes++;
        } else {
            v1_print_thread_result(tid, t->username, pw, hash_count, end_time - start_time, 1);
            t_args->fails++;
        }
        free(pw);
        delete_task(t);
    }

    return NULL;
}

int start(size_t thread_count) {
    // Remember to ONLY crack passwords in other threads
    char line_buffer[769];
    char username[257];
    char digest[257];
    char hint[257];
    
    // Task Queue
    task *temp_task;
    queue *task_queue;
    
    task_queue = queue_create(thread_count);
    pthread_t*   threads[thread_count];
    thread_args* t_args[thread_count];
    size_t       i;

    for (i=0; i<thread_count; i++) {
        // Start the thread
        threads[i] = malloc(sizeof(pthread_t));
        t_args[i]  = malloc(sizeof(thread_args));
        memset(t_args[i], 0, sizeof(thread_args));
        t_args[i]->tid = i + 1;
        t_args[i]->q   = task_queue;
        
        pthread_create(threads[i], NULL, &pw_cracker_start, (void*)t_args[i]); 
    }
    
    // The main thread will read the inputs and populate task queue
    while(fgets(line_buffer, 768, stdin) != NULL) {
        sscanf(line_buffer, "%s %s %s", username, digest, hint);
        temp_task = create_task(username, digest, hint);
        
        queue_push(task_queue, (void*)temp_task);
    }

    // push NULL elements to terminate threads, each element will terminate one thread
    for (i=0; i<thread_count; i++) {
        queue_push(task_queue, NULL);
    }

    // Join all threads
    for (i=0; i<thread_count; i++) {
        pthread_join(*(threads[i]), NULL);
    }

    // Print summary
    int num_recovered = 0;
    int num_failed    = 0;
    for (i=0; i<thread_count; i++) {
        num_recovered += t_args[i]->passes;
        num_failed    += t_args[i]->fails;
    }
    v1_print_summary(num_recovered, num_failed);

    for (i=0; i<thread_count; i++) {
        free(threads[i]);
        free(t_args[i]);
    }

    queue_destroy(task_queue);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
