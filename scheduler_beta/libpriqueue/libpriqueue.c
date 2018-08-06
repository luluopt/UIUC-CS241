/**
 * Scheduler Lab
 * CS 241 - Fall 2017
 */
#include <stdio.h>
#include <stdlib.h>

#include "libpriqueue.h"

/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of
  priqueue_t
    - You may assume this function will be the first function called using an
  instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int (*comparer)(const void *, const void *)) {
    q->comparer = comparer;
    q->size = 0;
    q->head = NULL;
}

/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where
  0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr) {
    entry *newentry = malloc(sizeof(entry));
    newentry->value = ptr;
    newentry->next = NULL;
    q->size++;

    if (q->head == NULL) {
        q->head = newentry;
        return 0;
    }
    // before head
    if (q->comparer(newentry->value, q->head->value) < 0) {
        newentry->next = q->head;
        q->head = newentry;
        return 0;
    }
    int retval = 1;
    entry *leftentry = q->head;
    while (leftentry != NULL) {
        //---tail
        int cmpA = q->comparer(leftentry->value, newentry->value);
        if (leftentry->next == NULL) {
            leftentry->next = newentry;
            break;
        }
        //---inbetween
        int cmpB = q->comparer(newentry->value, leftentry->next->value);
        if (cmpA <= 0 && cmpB < 0) {
            newentry->next = leftentry->next;
            leftentry->next = newentry;
            break;
        }
        leftentry = leftentry->next;
        retval++;
    }
    return retval;
}

/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q) {
    entry *head = q->head;
    return (head == NULL) ? NULL : head->value;
}

/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q) {
    if (q->head == NULL)
        return NULL;
    entry *prevhead = q->head;
    void *retval = prevhead->value;
    q->head = prevhead->next;
    free(prevhead);
    q->size = q->size - 1;
    return retval;
}

/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q) {
    return q->size;
}

/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q) {
    while (priqueue_poll(q) != NULL)
        ;
}
