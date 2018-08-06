/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include "../libscheduler.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define GTSLEEP_SIG 10

enum {
    StackSize = 0x400000,
};

void gtinit(scheme_t s);
void gtstart(void);
int gtgo(void (*f)(void), scheduler_info *sched_data);
void *gtcurrjob(void);
bool gtdoyield(int sig);
void gtsleep(double sleep_time);
void gtret(int ret);
