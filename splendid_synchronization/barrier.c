/**
 * Splendid Synchronization Lab
 * CS 241 - Fall 2017
 */
#include "barrier.h"
#include <stdlib.h>

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
	if(barrier == NULL){
		return -1;
	}
	pthread_mutex_destroy( &(barrier->mtx ) );
	pthread_cond_destroy( &(barrier->cv) );
	free(barrier);
	
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
	barrier = malloc(sizeof(barrier_t));
	
	barrier->count = num_threads;
	barrier->n_threads = num_threads;
	barrier->times_used = 0;
	pthread_mutex_init( &(barrier->mtx), NULL );
	pthread_cond_init( &(barrier->cv), NULL  );
	
    return error;
}

int barrier_wait(barrier_t *barrier) {
	
	pthread_mutex_lock(&(barrier->mtx));
	--(barrier->count); 
	if ( barrier->count == 0 ) {
		pthread_cond_broadcast( &(barrier->cv) );
		barrier->count  = 	barrier->n_threads;
                ++(barrier->times_used	);
	} else {
		while(barrier->count != 0) {
			pthread_cond_wait(&(barrier->cv), &(barrier->mtx)); 
		}
	}
	pthread_mutex_unlock(&(barrier->mtx));

    return 0;
}
