/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */

#include "mmu.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

mmu *mmu_create() {
  mmu *my_mmu = calloc(1, sizeof(mmu));
  my_mmu->tlb = tlb_create();
  return my_mmu;
}


uintptr_t get_virtual_base(uintptr_t virtual_address)
{
    uintptr_t result = virtual_address >> NUM_OFFSET_BITS;
    result = result << NUM_OFFSET_BITS;

    return result;
}


uintptr_t translate_to_physical(uintptr_t virtual_address, page_table_entry *pte)
{
    uintptr_t offset = virtual_address & 0xFFF;  // Use AND mask on the last 12 bits, so we keep the offset
    uintptr_t phys_address = (pte->base_addr << NUM_OFFSET_BITS) | offset;

    return phys_address;
}

uintptr_t get_pde_index(uintptr_t virtual_address)
{
    return virtual_address >> (NUM_VPN_BITS + NUM_OFFSET_BITS);
}

uintptr_t get_pte_index(uintptr_t virtual_address)
{
    uintptr_t result = virtual_address >> NUM_OFFSET_BITS;
    result &= 0x3FF;  // keep only the last 10 bits

    return result;
}


page_table_entry *mmu_get_pte_self(mmu *this, uintptr_t virtual_address, size_t pid)
{
    // If the mmu were not servicing this PID before, we need to flush
    // TLB, then update curr_pid
    if (this->curr_pid != pid) {
        tlb_flush(&(this->tlb));
        this->curr_pid = pid;
    }
    
    uintptr_t         virt_base = get_virtual_base(virtual_address);
    page_table_entry *pte       = tlb_get_pte(&(this->tlb), virt_base);

    if (pte != NULL) {
        // Check if the page is loaded to memory
        if (pte->present != 1) {  // Only do something when there is a page fault
            mmu_raise_page_fault(this);
            // Assign a new physical memory address to the virtual page
            pte->base_addr = ((uintptr_t)ask_kernel_for_frame(pte) >> NUM_OFFSET_BITS);
            // Load page from swap
            read_page_from_disk(pte);

            pte->present = 1;
        }

        return pte; 
    } else {
        // Mark a TLB miss
        mmu_tlb_miss(this);

        // Walk the page directory
        uintptr_t             pde_idx = 0;
        page_directory_entry *pde     = 0;
        page_table           *pt      = NULL;

        pde_idx = get_pde_index(virtual_address);

        pde     = &(this->page_directories[pid]->entries[pde_idx]);

        // See if page directory is allocated in physical memory
        if (pde->present != 1) {
            mmu_raise_page_fault(this);
            pde->base_addr = ((uintptr_t)ask_kernel_for_frame((page_table_entry*)NULL) >> NUM_OFFSET_BITS);
            pde->present = true;
            
            // initialize the page table to be 0s
            pt = (page_table*)((uintptr_t)pde->base_addr << NUM_OFFSET_BITS);
            memset(pt, 0, sizeof(page_table));
  
            // need to set the pte to allow writes 
            for (unsigned long i = 1; i < NUM_ENTRIES; i++) {
                pt->entries[i].read_write = true;
                pt->entries[i].user_supervisor = true;
            }
        }

        // Get the page table by de-referencing pointer from PDE
        pt = (page_table*)((uintptr_t)pde->base_addr << NUM_OFFSET_BITS);

        // Walk the page table
        uintptr_t         pte_idx = 0;
    
        pte_idx = get_pte_index(virtual_address);
        pte     = &(pt->entries[pte_idx]); 

        // Check if the page is loaded to memory
        if (pte->present != 1) {  // Only do something when there is a page fault
            mmu_raise_page_fault(this);
            // Assign a new physical memory address to the virtual page
            pte->base_addr = ((uintptr_t)ask_kernel_for_frame(pte) >> NUM_OFFSET_BITS);
            // Load page from swap
            read_page_from_disk(pte);

            pte->present = 1;
        }

        tlb_add_pte(&(this->tlb), virt_base, pte);
        return pte;
    }

    return pte;
}

void mmu_read_from_virtual_address(mmu *this, uintptr_t virtual_address,
                                   size_t pid, void *buffer, size_t num_bytes)
{
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);

    // Check for segfault first before we even do vm translations
    vm_segmentations *segmentations = this->segmentations[pid];
    if (!address_in_segmentations(segmentations, virtual_address)) {
        // Do nothing when there is a segfault
        mmu_raise_segmentation_fault(this);
        return;
    }

    page_table_entry *pte = mmu_get_pte_self(this, virtual_address, pid); 

    if (pte) {
        // obtain the physical address, with the offset
        uintptr_t phys_address = translate_to_physical(virtual_address, pte); 
        memcpy(buffer, (void*)phys_address, num_bytes); 
        
        // set the accessed flag
        pte->accessed = true;
    }
    
}


void mmu_write_to_virtual_address(mmu *this, uintptr_t virtual_address,
                                  size_t pid, const void *buffer,
                                  size_t num_bytes)
{
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);

    vm_segmentations *segmentations = this->segmentations[pid];
    vm_segmentation  *seg           = find_segment(segmentations, virtual_address);
    if (seg == NULL) {  // address not in segments
        // Do nothing when there is a segfault
        mmu_raise_segmentation_fault(this);
        return;
    } else if ((seg->permissions & WRITE) <= 0) {  // use the WRITE mask to check if we can write to this segment
        // segment is READ or EXEC only 
        mmu_raise_segmentation_fault(this);
        return;
    }

    page_table_entry *pte = mmu_get_pte_self(this, virtual_address, pid); 

    if (pte) {
        // check for segfaults
        if (pte->read_write == 0) {
            // page is READ ONLY 
            mmu_raise_segmentation_fault(this);
            return;
        } else {
            // obtain the physical address, with the offset
            uintptr_t phys_address = translate_to_physical(virtual_address, pte); 
            memcpy((void*)phys_address, buffer, num_bytes); 
            
            // set the accessed and dirty flag
            pte->accessed = true;
            pte->dirty    = true;
        }
    }
}

void mmu_tlb_miss(mmu *this) { this->num_tlb_misses++; }


void mmu_raise_page_fault(mmu *this) { this->num_page_faults++; }


void mmu_raise_segmentation_fault(mmu *this) {
  this->num_segmentation_faults++;
}


void mmu_add_process(mmu *this, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  this->page_directories[pid] =
      (page_directory *)ask_kernel_for_frame((page_table_entry *)NULL);
  page_directory *pd = this->page_directories[pid];
  this->segmentations[pid] = calloc(1, sizeof(vm_segmentations));
  vm_segmentations *segmentations = this->segmentations[pid];

  // Note you can see this information in a memory map by using
  // cat /proc/self/maps
  segmentations->segments[STACK] =
      (vm_segmentation){.start = 0xBFFFE000,
                        .end = 0xC07FE000, // 8mb stack
                        .permissions = READ | WRITE,
                        .grows_down = true};

  segmentations->segments[MMAP] = (vm_segmentation){.start = 0xC07FE000,
                                                    .end = 0xC07FE000,
                                                    .permissions = READ | EXEC,
                                                    .grows_down = true};

  segmentations->segments[HEAP] = (vm_segmentation){.start = 0x08072000,
                                                    .end = 0x08072000,
                                                    .permissions = READ | WRITE,
                                                    .grows_down = false};

  segmentations->segments[BSS] = (vm_segmentation){.start = 0x0805A000,
                                                   .end = 0x08072000,
                                                   .permissions = READ | WRITE,
                                                   .grows_down = false};

  segmentations->segments[DATA] = (vm_segmentation){.start = 0x08052000,
                                                    .end = 0x0805A000,
                                                    .permissions = READ | WRITE,
                                                    .grows_down = false};

  segmentations->segments[TEXT] = (vm_segmentation){.start = 0x08048000,
                                                    .end = 0x08052000,
                                                    .permissions = READ | EXEC,
                                                    .grows_down = false};

  // creating a few mappings so we have something to play with (made up)
  // this segment is made up for testing purposes
  segmentations->segments[TESTING] =
      (vm_segmentation){.start = PAGE_SIZE,
                        .end = 3 * PAGE_SIZE,
                        .permissions = READ | WRITE,
                        .grows_down = false};
  // first 4 mb is bookkept by the first page directory entry
  page_directory_entry *pde = &(pd->entries[0]);
  // assigning it a page table and some basic permissions
  pde->base_addr = ((uintptr_t)ask_kernel_for_frame((page_table_entry *)NULL) >>
                    NUM_OFFSET_BITS);
  pde->present = true;
  pde->read_write = true;
  pde->user_supervisor = true;

  // setting entries 1 and 2 (since each entry points to a 4kb page)
  // of the page table to point to our 8kb of testing memory defined earlier
  for (int i = 1; i < 3; i++) {
    page_table *pt =
        (page_table *)((uintptr_t)pde->base_addr << NUM_OFFSET_BITS);
    page_table_entry *pte = &(pt->entries[i]);
    pte->base_addr = ((uintptr_t)ask_kernel_for_frame(pte) >> NUM_OFFSET_BITS);
    pte->present = true;
    pte->read_write = true;
    pte->user_supervisor = true;
  }
}


void mmu_remove_process(mmu *this, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  // example of how to BFS through page table tree for those to read code.
  page_directory *pd = this->page_directories[pid];
  if (pd) {
    for (size_t vpn1 = 0; vpn1 < NUM_ENTRIES; vpn1++) {
      page_directory_entry *pde = &(pd->entries[vpn1]);
      if (pde->present) {
        page_table *pt =
            (page_table *)((uintptr_t)(pde->base_addr << NUM_OFFSET_BITS));
        for (size_t vpn2 = 0; vpn2 < NUM_ENTRIES; vpn2++) {
          page_table_entry *pte = &(pt->entries[vpn2]);
          if (pte->present) {
            void *frame =
                (void *)((uintptr_t)(pte->base_addr << NUM_OFFSET_BITS));
            return_frame_to_kernel(frame);
          }
          remove_swap_file(pte);
        }
        return_frame_to_kernel(pt);
      }
    }
    return_frame_to_kernel(pd);
  }

  this->page_directories[pid] = NULL;
  free(this->segmentations[pid]);
  this->segmentations[pid] = NULL;

  if (this->curr_pid == pid) {
    tlb_flush(&(this->tlb));
  }
}


void mmu_delete(mmu *this) {
  for (size_t pid = 0; pid < MAX_PROCESS_ID; pid++) {
    mmu_remove_process(this, pid);
  }

  tlb_delete(this->tlb);
  free(this);
  remove_swap_files();
}
