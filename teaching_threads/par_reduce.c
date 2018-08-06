/**
 * Teaching Threads
 * CS 241 - Fall 2017
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct
{
    int m_result;
    int *m_list;
    int m_len;
    reducer m_func;
    int m_base_case;
} param;


/* my_thread function */
void *my_thread(void *ptr) // the function that achieve the function of a thread
{
    param* param_ptr = (param*)ptr;
    int *list = param_ptr->m_list;
    int length = param_ptr->m_len;
    reducer reduce_func = param_ptr->m_func;
    int base_case = param_ptr->m_base_case;
    param_ptr->m_result = reduce(list, length, reduce_func, base_case);     //printf("result = %d\n",param_ptr->m_result);
    return NULL;
}

/* You should create a start routine for your threads. */

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads)
{
    /* Your implementation goes here */
    int result=0;
    unsigned i=0;

    pthread_t* thread_index=(pthread_t*) malloc(num_threads * sizeof(pthread_t));
    param* arry_param = (param*) malloc(num_threads * sizeof(param));

    unsigned divider;
    if(list_len%num_threads==0)
    {
        divider = list_len/num_threads;       //printf("divider = %d\n",divider);
    }
    else
    {
        divider = (list_len-list_len%num_threads)/(num_threads-1);         //printf("divider = %d\n",divider);
    }


    // partition of parameters
    for (i=0; i<num_threads-1; i++)
    {
        arry_param[i].m_list = list+divider*i;
        arry_param[i].m_len = divider;
        arry_param[i].m_func=reduce_func;
        arry_param[i].m_base_case=base_case;
    }
    i=num_threads-1;
    arry_param[i].m_list = list+divider*i;
    arry_param[i].m_len = list_len-(num_threads-1)*divider;
    arry_param[i].m_func=reduce_func;
    arry_param[i].m_base_case=base_case;

    /* create num_threads number of threads */
    for (i=0; i<num_threads; i++)
    {
        pthread_create(thread_index+i,NULL, my_thread, arry_param+i);
    }

    /* wait until all threads finish */
    for (i=0; i<num_threads; i++)
    {
        pthread_join(thread_index[i],NULL);
    }

    //combine the result
    int *final_list = malloc(num_threads * sizeof(int));
    for (i=0; i<num_threads; i++)
    {
        final_list[i]=arry_param[i].m_result;
    }

    result = reduce(final_list, num_threads, reduce_func, base_case);


    //int cmp = reduce(list, list_len, reduce_func, base_case);
    //printf("cmp = %d\n",cmp);


    return result;
}
