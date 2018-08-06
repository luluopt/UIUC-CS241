/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "kernel.h"
#include "page_table.h"
#include "segmentation.h"
#include "tlb.h"

// 32768 is the largest process id on most linux systems
#define MAX_PROCESS_ID 32768

typedef struct {
  page_directory *page_directories[MAX_PROCESS_ID];
  vm_segmentations *segmentations[MAX_PROCESS_ID];
  tlb *tlb;
  size_t num_page_faults;
  size_t num_segmentation_faults;
  size_t num_tlb_misses;
  size_t curr_pid;
} mmu;

mmu *mmu_create();

page_table_entry *mmu_get_pte(mmu *this, uintptr_t virtual_address, size_t pid);

/**
  Gets the physical address from a virtual address for a pid if it has already
  been assigned.
  Whenever possible you should see if the physical address is stored in the tlb
  before going through multiple page tables.
  This is because page tables are orders of magnitude slower than the cache of a
  tlb.

  If a pointer trys to access a page of address space that's currently not
  mapped onto physical memory,
  then you should raise a page_fault(mmu_raise_page_fault) (hint there is a
  useful bit in the page table entry)
  and load that page into memory.

  If a program tries to access an invalid or illegal memory address,
  then you should raise a segmentation fault(mmu_raise_segmentation_fault).
  A memory address is invalid if there is no pagetable entry for it or
  if the user does not have the correct permissions for it (hint there are
  useful bit in the page table entry).

  Note: Your tlb needs to be updated whenever this method is called.
  Note: All process pagetables when been set up at this point.
 */
uintptr_t mmu_get_physical_address(mmu *this, uintptr_t virtual_address,
                                   size_t pid);

void mmu_read_from_virtual_address(mmu *this, uintptr_t virtual_address,
                                   size_t pid, void *buffer, size_t num_bytes);

void mmu_write_to_virtual_address(mmu *this, uintptr_t virtual_address,
                                  size_t pid, const void *buffer,
                                  size_t num_bytes);
/**
  You need to call this function every time the mmu cache misses with the tlb
  and needs to check the page tables.
*/
void mmu_tlb_miss(mmu *this);

/**
  Raises a page_fault.
*/
void mmu_raise_segmentation_fault(mmu *this);

/**
  Raises a segfault.
*/
void mmu_raise_page_fault(mmu *this);

/**
  Adds a process with 'pid' to the mmu by creating it's page_tables.
*/
void mmu_add_process(mmu *this, size_t pid);

/**
  Free all the physical memory used by a certain process given by 'pid', so that
  other process can use that space.
*/
void mmu_free_process_tables(mmu *this, size_t pid);

void mmu_delete(mmu *this);
