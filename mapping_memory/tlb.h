/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "page_table.h"
#include <stdlib.h>
// Max number of items that the tlb can hold
#define MAX_NODES 16

// The tlb struct is just a node of a linked list.
// We are using a linked list to implement an LRU cache.
typedef struct tlb {
  // The key is the virtual address with the offset removed.
  // This is because all virtual addresses with the same virtual page numbers
  // will map to the same frame in memory.
  uintptr_t base_virtual_addr;
  // The value is the page table entry that stores the physical address of the
  // beginning of the frame that corresponds to the 'base_virtual_addr'.
  page_table_entry *entry;
  struct tlb *next;
} tlb;

/**
 * Allocate and return a new tlb structure.
 */
tlb *tlb_create();

/**
  Checks to see if 'tlb' knows the value of 'base_virtual_addr'.

  If so, then this will return the pte that corresponds to 'base_virtual_addr'.
  If not, then this will return NULL.

  Notice that this function takes a pointer to a tlb pointer.
  The double pointer is so that this function can modify the pointer the user
  passes in (updating their head pointer).
*/
page_table_entry *tlb_get_pte(tlb **head, uintptr_t base_virtual_addr);

/**
  Adds what the corresponding 'base_virtual_addr': 'entry' pair to the tlb.

  If the tlb is at its capacity, then it will evict the least recently used
  (LRU) item.

  Notice that this function takes a pointer to a tlb pointer.
  The double pointer is so that this function can modify the pointer the user
  passes in (updating their head pointer).
 */
void tlb_add_pte(tlb **head, uintptr_t base_virtual_addr,
                 page_table_entry *entry);

/**
 * Clears the tlb's data
 */
void tlb_flush(tlb **head);

/**
 * Frees the tlb
 */
void tlb_delete(tlb *tlb);
