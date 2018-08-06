/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "tlb.h"
#define DBG 1

void print_tlb(tlb *test_tlb) {
  if (DBG) {
    while (test_tlb) {
      printf("base_virtual_addr: [%lu], value: [%p], next: [%p]\n",
             test_tlb->base_virtual_addr, test_tlb->entry, test_tlb->next);
      test_tlb = test_tlb->next;
    }

    printf("---------------------------------------------------\n");
  }
}

void check_node(tlb *head, uintptr_t base_virtual_addr, page_table_entry *value,
                size_t index) {
  tlb *runner = head;
  while (index) {
    runner = runner->next;
    index--;
  }

  assert(runner->base_virtual_addr == base_virtual_addr);
  assert(runner->entry == value);
  assert(index == 0);
}

int check_empty(tlb **head) {
  printf("Checking Empty tlb... \n");
  // Check that the first node is a sentinal node
  check_node(*head, (uintptr_t)NULL, NULL, 0);
  // And that its the only node in the list
  assert(!(*head)->next);
  print_tlb(*head);
  return 0;
}

int insert_1_node(tlb **head) {
  printf("Inserting 1 node into tlb... \n");
  uintptr_t base_virtual_addr = 0xDEADBEEF;
  page_table_entry *value = (void *)0xCAFEBABE;
  tlb_add_pte(head, base_virtual_addr, value);
  check_node(*head, base_virtual_addr, value, 0);
  print_tlb(*head);
  return 0;
}

int insert_capacity_nodes(tlb **head) {
  printf("Inserting capacity nodes into tlb... \n");
  for (size_t i = 0; i < MAX_NODES; i++) {
    tlb_add_pte(head, i + 1, (void *)i + 1);
  }

  for (size_t i = 0; i < MAX_NODES; i++) {
    check_node(*head, i + 1, (void *)i + 1, MAX_NODES - i - 1);
  }

  print_tlb(*head);
  return 0;
}

int insert_over_capacity_nodes(tlb **head) {
  printf("Inserting over capacity nodes into tlb... \n");
  insert_capacity_nodes(head);
  for (size_t i = 0; i < MAX_NODES; i++) {
    tlb_add_pte(head, i + 1, (void *)i);
  }

  for (size_t i = 0; i < MAX_NODES; i++) {
    check_node(*head, i + 1, (void *)i, MAX_NODES - i - 1);
  }

  print_tlb(*head);
  return 0;
}

int get_first_element_size_1_list(tlb **head) {
  printf("Getting first element from a tlb with 1 element... \n");
  insert_1_node(head);
  uintptr_t base_virtual_addr = 0xDEADBEEF;
  page_table_entry *value = (void *)0xCAFEBABE;
  page_table_entry *entry = tlb_get_pte(head, base_virtual_addr);
  assert(entry == value);
  check_node(*head, base_virtual_addr, value, 0);
  print_tlb(*head);
  return 0;
}

int get_first_element_max_capacity_list(tlb **head) {
  printf("Getting first element from a full tlb... \n");
  insert_capacity_nodes(head);
  page_table_entry *entry = tlb_get_pte(head, 0);
  assert(entry == 0);
  check_node(*head, MAX_NODES, (void *)MAX_NODES, 0);
  print_tlb(*head);
  return 0;
}

int get_last_element_max_capacity_list(tlb **head) {
  printf("Getting last element from a full tlb... \n");
  insert_capacity_nodes(head);
  page_table_entry *entry = tlb_get_pte(head, 1);
  assert(entry == (void *)1);
  check_node(*head, 1, (void *)1, 0);
  print_tlb(*head);
  return 0;
}

int check_tlb_flush(tlb **head) {
  printf("Check flushing a full tlb... \n");
  insert_capacity_nodes(head);
  tlb_flush(head);
  check_empty(head);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    fprintf(stderr, "%s\n", "Needs test number");
    return 13;
  }

  tlb *test_tlb = tlb_create();
  int test_number = atoi(argv[1]);
  switch (test_number) {
  default:
    fprintf(stderr, "%s\n", "Invalid test number");
    return 13;
  case 1:
    return check_empty(&test_tlb);
  case 2:
    return insert_1_node(&test_tlb);
  case 3:
    return insert_capacity_nodes(&test_tlb);
  case 4:
    return insert_over_capacity_nodes(&test_tlb);
  case 5:
    return get_first_element_size_1_list(&test_tlb);
  case 6:
    return get_first_element_max_capacity_list(&test_tlb);
  case 7:
    return get_last_element_max_capacity_list(&test_tlb);
  case 8:
    return check_tlb_flush(&test_tlb);
  }

  tlb_delete(test_tlb);
  return 0;
}
