/**
 * Deadlocked Diners Lab
 * CS 241 - Fall 2017
 */
#include "company.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern pthread_mutex_t arbitrator;

void *work_interns(void *p) {
    Company *company = (Company *)p;
    pthread_mutex_t *left_intern, *right_intern;
    int right_avail = 1;
    int left_avail  = 1;
    // int arb_locked  = 1;
    
    int arb_cycles  = 0;

    while (running) {
        left_intern = Company_get_left_intern(company);
        right_intern = Company_get_right_intern(company);

        // if left and right is the same intern, means we only have 1 intern
        if (left_intern == right_intern) {
            break;
        }

        pthread_mutex_lock(&arbitrator);

        arb_cycles = 0;
        while (1) {
            left_avail = pthread_mutex_trylock(left_intern);
            if (left_avail != 0) {
                // do nothing if left isn't available
            } else {
                right_avail = pthread_mutex_trylock(right_intern);
                
                if (right_avail == 0) {
                    // unlock arbitrator since both interns are available, get to work
                    pthread_mutex_unlock(&arbitrator);

                    Company_hire_interns(company);
                    pthread_mutex_unlock(right_intern);
                    pthread_mutex_unlock(left_intern);
                    usleep(100); // put thread to sleep after the company has worked so it doesn't hire interns right away
                    break;
                } else {
                    // unlcok left if right is unavailable
                    pthread_mutex_unlock(left_intern);
                }
            }

            arb_cycles++;
            usleep(100);  // act as thread_yield() but also allows time passing

            // set a limit to arbitration lock so that we dont wait forever incase there are interns available for another
            // company
            if (arb_cycles >= 300) {
                pthread_mutex_unlock(&arbitrator);
                break;
            }
        }

        usleep(100);  // act as thread_yield() but also make sure the thread does not immediate get arbitrated again
    }

    return NULL;
}
