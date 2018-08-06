/**
 * Splendid Synchronization Lab
 * CS 241 - Fall 2017
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node
{
    void *data;
    struct queue_node *next;
} queue_node;

struct queue
{
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size)
{
    struct queue *newQueue = malloc (sizeof(queue));
    newQueue->max_size= max_size;
    newQueue->size = 0;
    pthread_mutex_init( &(newQueue->m), NULL );
    pthread_cond_init( &(newQueue->cv), NULL );
    newQueue->head = NULL;
    newQueue->tail = NULL;
    return newQueue;
}

void queue_destroy(queue *this)
{
    queue_node *ptr = this->head;
    queue_node *prePtr = NULL;
    while (ptr)
    {
        prePtr = ptr;
        ptr = prePtr->next;
        free(prePtr);
    }
    pthread_mutex_destroy( &(this->m) );
    pthread_cond_destroy( &(this->cv) );
    free(this);
}

void queue_push(queue *this, void *data)
{
    queue_node *newNode =NULL;
    if (this->max_size > 0)
    {
        pthread_mutex_lock(&(this->m));
        while (this->size == this->max_size)
        {
            pthread_cond_wait( &(this->cv), &(this->m) );
        }
    	newNode = malloc(sizeof(queue_node));
    	newNode -> data = data;
    	newNode -> next = NULL;
    	(this -> tail)->next = newNode;
    	this -> tail = newNode;
    	++(this->size);
        pthread_mutex_unlock(&(this->m));
    } 
    else
    {
    	newNode = malloc(sizeof(queue_node));
    	newNode -> data = data;
    	newNode -> next = NULL;
    	(this -> tail)->next = newNode;
    	this -> tail = newNode;
    	++(this->size);
     }

}

void *queue_pull(queue *this)
{
    void *data=NULL;
    queue_node *secondNode=NULL;
    if (this->max_size > 0)
    {
        pthread_mutex_lock(&(this->m));
        while (this->size == 0)
        {
            pthread_cond_wait( & (this->cv), &(this->m) );
        }
        secondNode = (this->head)->next;
        data = (this -> head)->data;
        free(this -> head);
        this -> head = secondNode;
        --(this->size);
        pthread_mutex_unlock(&(this->m));
    } 
    else
    {
        secondNode = (this->head)->next;
        data = (this -> head)->data;
        free(this -> head);
        this -> head = secondNode;
        --(this->size);
     }
     return data;
}
