/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */
#include "tlb.h"

tlb *tlb_create() { return calloc(1, sizeof(tlb)); }

page_table_entry *tlb_get_pte(tlb **head, uintptr_t base_virtual_addr) {
  // If the first node has the base_physical_addr we want, then we can just
  // return the corresponding entry.
  if ((*head)->base_virtual_addr == base_virtual_addr) {
    return (*head)->entry;
  }
  // Using the 'slow pointer and fast pointer' trick for singly linked lists:

  // There are a lot of interesting interview questions that arise from this
  // trick:
  //  * Given a singly linked list and head pointer determine if the list has a
  //  cycle
  //  * Given a singly linked list and a head pointer return to me the 'kth'
  //  element from the end.
  //  * Given a singly linked list and a head pointer return to me the middle
  //  element.

  tlb *slow = *head;
  tlb *fast = (*head)->next;

  // Finds the correct node and promote it to the head.
  while (fast) {
    if (fast->base_virtual_addr == base_virtual_addr) {
      // We found the correct node!
      // Now we need to move it to remove it from the list and promote as the
      // new head of the linked list.
      slow->next = fast->next;
      fast->next = *head;
      *head = fast;
      return fast->entry;
    }

    fast = fast->next;
    slow = slow->next;
  }

  return NULL;
}

void tlb_add_pte(tlb **head, uintptr_t base_virtual_addr,
                 page_table_entry *entry) {
  size_t num_nodes = 2;
  tlb *slow = *head;
  tlb *fast = (*head)->next;

  // Figure out how many nodes are in the linked list.
  while (fast && fast->next) {
    num_nodes++;
    fast = fast->next;
    slow = slow->next;
  }

  if (num_nodes >= MAX_NODES) {
    // We need to evict the least recently used item.
    // At this point fast is pointing to the tail.
    free(fast);
    slow->next = NULL;
  }

  // Appends new node to the head of the linked list.
  tlb *new_head = tlb_create();
  new_head->base_virtual_addr = base_virtual_addr;
  new_head->entry = entry;
  new_head->next = *head;
  *head = new_head;
}

void tlb_flush(tlb **tlb) {
  tlb_delete(*tlb);
  (*tlb) = tlb_create();
}

void tlb_delete(tlb *tlb) {
  if (tlb) {
    tlb_delete(tlb->next);
    free(tlb);
  }
}
