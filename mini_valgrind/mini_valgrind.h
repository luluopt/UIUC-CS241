/**
 * Mini Valgrind Lab
 * CS 241 - Fall 2017
 */

#pragma once
#include <stddef.h>
#include <stdlib.h>

typedef struct _meta_data {
    // Number of bytes of heap memory the user requested from malloc
    size_t request_size;
    // Name of the file where the memory request came from, as a string
    // (you should store the pointer we give to mini_malloc directly;
    //  you should NOT try to allocate a new buffer for it)
    const char *filename;
    // The address of the instruction that requested this memory;
    // this will be used later to find the function name and line number
    void *instruction;
    // Pointer to the next instance of meta_data in the list
    struct _meta_data *next;
} meta_data;

/*
 * Global head of the linked list of meta_data. This is used in
 * mini_valgrind.c as a global pointer to the head.
 *
 * Note: this is in static memory, so it is implicitly initialized to NULL.
 */
meta_data *head;

/*
 * The total memory requested (in bytes) by the program, not including
 * the overhead for "meta_data".
 *
 * Note: This number only increases throughout the lifetime of the program.
 * Note: This is static memory, so it is implicitly initialized to zero.
 */
size_t total_memory_requested;

/*
 * The total memory freed (in bytes) by the program, not including
 * the overhead for "meta_data".
 *
 * Note: This number only increases throughout the lifetime of the program.
 * Note: This is static memory, so it is implicitly initialized to zero.
 */
size_t total_memory_freed;

/*
 * The number of times the user tried to free or realloc an invalid address.
 *
 * Note: an invalid free or realloc happens when a user calls free() or
 * realloc() on an address that is not in your meta_data linked list, EXCEPT
 * for NULL (which is always a valid free).
 *
 * Note: This is static memory, so it is implicitly initialized to zero.
 */
size_t invalid_addresses;

/*
 * Wrap a call to malloc.
 *
 * This malloc creates a metadata object and inserts it into the head of the
 * list. You have to allocate enough to hold both the user's requested amount
 * of memory and the meta_data structure. You should only call malloc once in
 * this function.
 *
 * If the requested size is 0, malloc's behavior is undefined. In this case,
 * you may just return a NULL pointer.
 *
 * @param request_size
 *  Size of the requested memory block, in bytes.
 * @param filename
 *  Name of the file invoking this call to mini_malloc. You should store this
 *  directly in the metadata without modification.
 * @param instruction
 *  Address of the instruction invoking this call to mini_malloc.
 *
 * @return
 *  On success, return a pointer to the memory block allocated by the function.
 *  This should be the start of the user's memory, and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory, return a
 *  NULL pointer.
 */
void *mini_malloc(size_t request_size, const char *filename, void *instruction);

/*
 * Wrap a call to calloc.
 *
 * This works just like malloc, but zeros out the allocated memory.
 *
 * You may call calloc, malloc, or mini_malloc in this function,
 * but you should only do it once.
 *
 * If either the number of elements or the element size is 0, calloc's behavior
 * is undefined.
 *
 * @param num_elements
 *  Number of elements to allocate.
 * @param element_size
 *  Size of each element, in bytes.
 * @param filename
 *  Name of the file invoking this call to mini_calloc.
 * @param instruction
 *  Address of the instruction invoking this call to mini_calloc.
 *
 * @return
 *  On success, return a pointer to the memory block allocated by the function.
 *  This should be the start of the user's memory, and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory, return a
 *  NULL pointer.
 */
void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction);

/*
 * Wrap a call to realloc.
 *
 * If the given pointer is NULL, you should treat this like a call to
 * mini_malloc. If the requested size is 0, you should treat this like a call
 * to mini_free and return NULL. If the pointer is NULL and the size is 0,
 * the behavior is undefined.
 *
 * In all other cases, you should use realloc to resize an existing allocation,
 * and then update the existing metadata struct with new values. If the size of
 * the allocation has increased, you must increase total_memory_requested;
 * if it has decreased, you must increase total_memory_freed. In other words,
 * these values should never decrease.
 *
 * If the user tries to realloc an invalid pointer, increment invalid_addresses
 * and return NULL.
 *
 * As with the other functions, you should only call malloc or realloc once.
 *
 * @param ptr
 *  The pointer to realloc.
 * @param request_size
 *  Size of the requested memory block, in bytes.
 * @param filename
 *  Name of the file invoking this call to mini_realloc.
 * @param instruction
 *  Address of the instruction invoking this call to mini_realloc.
 *
 * @return
 *  On success, return a pointer to the memory block allocated by the function.
 *  This should be the start of the user's memory, and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory, return a
 *  NULL pointer.
 */
void *mini_realloc(void *ptr, size_t request_size, const char *filename,
                   void *instruction);

/*
 * Wrap a call to free.
 *
 * This free will also remove the metadata node from the list, assuming it is
 * a valid pointer.
 *
 * Unlike the regular free, you should not crash when given an invalid pointer.
 * Instead, increment invalid_addresses.
 *
 * Invalid pointers include pointers that you did not return from mini_malloc
 * (or mini_calloc, or mini_realloc), and double frees.
 *
 * @param ptr
 *  Pointer to a previously allocated memory block. If a NULL pointer is
 *  passed, no action occurs, and it does not count as a bad free.
 */
void mini_free(void *ptr);
