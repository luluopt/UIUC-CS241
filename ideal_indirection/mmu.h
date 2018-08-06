/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "kernel.h"
#include "page_table.h"
#include "segments.h"
#include "tlb.h"

/**
 * The file is responsible to excuting the logic of a Memory Management Unit
 * (MMU).
 */

// 32768 is the largest process id on most linux systems
#define MAX_PROCESS_ID 32768

/*
 * Struct used to represent an MMU
 */
typedef struct {
  /* An array of pointers to top level page directories (one per process)
   * details in page_table.h*/
  page_directory *page_directories[MAX_PROCESS_ID];
  /* An array of pointers to segmentations (one per process) details in
   * segments.h*/
  vm_segmentations *segmentations[MAX_PROCESS_ID];
  /* Pointer to a translation lookaside buffer; details in tlb.h */
  tlb *tlb;
  /* Statistics to check if MMU is correctly translating addresses */
  size_t num_page_faults;
  size_t num_segmentation_faults;
  size_t num_tlb_misses;
  /*
   * Current pid that is reading or writing to memory.
   * Changes whenever there is a context switch
   */
  size_t curr_pid;
} mmu;

/*
 * Creates a default MMU with all fields intialized on the heap.
 * Call on mmu_delete() to free all memory used.
 */
mmu *mmu_create();

/*
 * Get the page_table_entry that corresponds to the 'virtual_address' for 'pid'.
 * Note that this function does interact with the tlb, raises a
 * pagefault/segfault.
 * This function assumes that all paging structures (page directory and page
 * table)
 * are paged in memory and does no error checking.
 */
page_table_entry *mmu_get_pte(mmu *this, uintptr_t virtual_address, size_t pid);

/**
  The following applies to both mmu_read_from_virtual address and
  mmu_write_to_virtual_address:

  Reads from or writes 'num_bytes' to a 'virtual_address' for the process with
  'pid' if it has already been assigned.
  Whenever possible you should see if the page_table entry corresponding to a
  `virtual_address`
  is stored in the tlb before going through multiple page tables.
  This is because page tables are orders of magnitude slower than the cache of a
  tlb.
  Make sure to keep your cache valid by checking for context switches.

  If a pointer trys to access a page of address space that's currently not
  mapped onto physical memory,
  then you should raise a page_fault(mmu_raise_page_fault) (hint there is a
  useful bit in the page table entry)
  and load that page into memory (ask_kernel_for_frame() and
  read_page_from_disk()).

  If a program tries to access an invalid or illegal memory address,
  then you should raise a segmentation fault(mmu_raise_segmentation_fault).
  A memory address is invalid if the address is not in any segmentation or
  if the user does not have the correct permissions for it (hint there are
  useful bit in the page table entry).

  Set the appropriate flags (see page_table.h).

  Note: Your tlb needs to be updated whenever this method is called.
  Note: All process pagetables when been set up at this point.
  Note: All read and writes will be contained within a single page.
 */

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

/**
  Fress all memory used by the mmu
*/
void mmu_delete(mmu *this);
