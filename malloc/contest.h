/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#pragma once

typedef struct _alloc_stats_t {
    unsigned long long max_heap_used;
    unsigned long memory_uses;
    unsigned long long memory_heap_sum;
} alloc_stats_t;

#ifdef CONTEST_MODE
// timeout in contest modes in seconds
#define TIMER_TIMEOUT 30
// Memory Limit for contest mode
#define MEMORY_LIMIT ((1024L * 1024L * 1024L * 2L) + (1024L * 1024L * 512L))
#else
// timeout in replace mode in seconds
#define TIMER_TIMEOUT 120000
// Memory Limit for contest mode
#define MEMORY_LIMIT ((1024L * 1024L * 1024L * 12L) + (1024L * 1024L * 512L))
#endif
