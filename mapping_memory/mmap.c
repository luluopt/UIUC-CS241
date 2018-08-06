/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 * Collab with chu55
 */
#include "mmap.h"
#include "mmu.h"
#include "page_table.h"

#include <string.h>

mmap *mmap_create(char *path_to_file)
{
    // Allocate mmap struct on the heap
    mmap *this = malloc(sizeof(mmap));    
    memset(this, 0, sizeof(mmap));

    // Create mmu
    this->mmu = mmu_create();
    mmu_add_process(this->mmu, DEFAULT_PID); 

    // Open file
    this->stream = fopen(path_to_file, "rb+");
    if (!(this->stream)) {
        fprintf(stderr, "Cannot open %s\n", path_to_file);
        exit(1);
    }

    // Find file size
    fseek(this->stream, 0, SEEK_END);
    this->length = ftell(this->stream);
    fseek(this->stream, 0, SEEK_SET);    // rewind back to the beginning

    // Size the MMAP segment approriately
    vm_segmentations *p_segmentations = this->mmu->segmentations[DEFAULT_PID];
    p_segmentations->segments[MMAP].grows_down = false;

    size_t num_pages = (this->length + PAGE_SIZE - 1) / PAGE_SIZE;  // ceiling operation
    // size_t mmap_num_pages = (p_segmentations->segments[MMAP].end - p_segmentations->segments[MMAP].start) / PAGE_SIZE;
    grow_segment(p_segmentations, MMAP, num_pages);

    this->start_address = p_segmentations->segments[MMAP].start;

    return this;
}

// Perform demand paging
void mmap_demand_paging(mmap *this, size_t offset, size_t length)
{
    uintptr_t va_start = (this->start_address + offset) >> NUM_OFFSET_BITS << NUM_OFFSET_BITS;
    uintptr_t va_end   = (this->start_address + offset + length - 1) >> NUM_OFFSET_BITS << NUM_OFFSET_BITS;

    size_t num_pages   = (va_end - va_start) / PAGE_SIZE + 1;

    for (size_t i=0; i < num_pages; i++) {
        uintptr_t va  = va_start + i * PAGE_SIZE;
        uintptr_t pde_idx   = va >> NUM_OFFSET_BITS >> NUM_VPN_BITS;
        page_directory *pdir = this->mmu->page_directories[DEFAULT_PID];
        page_directory_entry *pde = &(pdir->entries[pde_idx]);
        // If the pde has not been allocated yet
        if (!pde->present) {
            pde->base_addr = (uintptr_t)(ask_kernel_for_frame((page_table_entry*)NULL)) >> NUM_OFFSET_BITS;
            pde->present   = 1;
        }
        page_table_entry *pte = mmu_get_pte(this->mmu, va, DEFAULT_PID);
        // If the pte has not been allocated yet, allocate a physical page for it
        if (!pte->present) {
            pte->base_addr = (uintptr_t)(ask_kernel_for_frame((page_table_entry*)NULL)) >> NUM_OFFSET_BITS;
            pte->present = 1;
            pte->dirty = 0;
            // if page was not present before, we read in the page from file
            int fpos = i * PAGE_SIZE;
            fseek(this->stream, fpos, SEEK_SET);
            char *buffer = (char*)((uintptr_t)(pte->base_addr) << NUM_OFFSET_BITS);
            memset(buffer, 0, PAGE_SIZE);
            fread(buffer, 1, PAGE_SIZE, this->stream);
        }
    }
}

size_t mmap_read(mmap *this, void *buffer, size_t offset,
                 size_t bytes_to_read)
{
    if (offset + bytes_to_read > this->length) {
        return 0;
    }

    mmap_demand_paging(this, offset, bytes_to_read); 

    // the mmu_read_from_virtual_address() function was not implemented properly
    uintptr_t read_start  = this->start_address + offset; 
    size_t    remain = bytes_to_read;

    // read to be read across virtual page boundaries
    while (remain > 0) {
        size_t bytes_in_page = PAGE_SIZE - (read_start % PAGE_SIZE);
               bytes_in_page = bytes_in_page > remain ? remain : bytes_in_page;
        size_t offset_in_page = read_start % PAGE_SIZE;
        page_table_entry *pte = mmu_get_pte(this->mmu, read_start, DEFAULT_PID);
        char *phys = (char*)((uintptr_t)(pte->base_addr) << NUM_OFFSET_BITS);
        memcpy(buffer, phys + offset_in_page, bytes_in_page); 
        
        buffer     += bytes_in_page;
        read_start += bytes_in_page;
        remain     -= bytes_in_page;
    }
    
    return bytes_to_read;
}

size_t mmap_write(mmap *this, void *buffer, size_t offset,
                  size_t bytes_to_write)
{
    if (offset + bytes_to_write > this->length) {
        return 0;
    }

    mmap_demand_paging(this, offset, bytes_to_write);
    // the mmu_read_from_virtual_address() function was not implemented properly
    uintptr_t write_start  = this->start_address + offset; 
    size_t    remain = bytes_to_write;
    while (remain > 0) {
        size_t bytes_in_page = PAGE_SIZE - (write_start % PAGE_SIZE);
               bytes_in_page = bytes_in_page > remain ? remain : bytes_in_page;
        size_t offset_in_page = write_start % PAGE_SIZE;
        page_table_entry *pte = mmu_get_pte(this->mmu, write_start, DEFAULT_PID);
        pte->dirty = 1;
        char *phys = (char*)((uintptr_t)(pte->base_addr) << NUM_OFFSET_BITS);
        memcpy(phys + offset_in_page, buffer, bytes_in_page); 
        
        buffer      += bytes_in_page;
        write_start += bytes_in_page;
        remain      -= bytes_in_page;
    }
    return bytes_to_write;
}

void munmap(mmap *this)
{
    size_t num_pages = this->length / PAGE_SIZE;

    // write back dirty pages
    for (size_t i=0; i < num_pages; i++) {
        uintptr_t va = this->start_address + i * PAGE_SIZE;
        page_table_entry *pte = mmu_get_pte(this->mmu, va, DEFAULT_PID);
        if (pte->dirty) {
            char *phys = (char*)((uintptr_t)(pte->base_addr) << NUM_OFFSET_BITS);
            fseek(this->stream, PAGE_SIZE*i, SEEK_SET);
            fwrite(phys, 1, PAGE_SIZE, this->stream);
        }
    }
    mmu_free_process_tables(this->mmu, DEFAULT_PID);
    mmu_delete(this->mmu);
    fclose(this->stream);
    free(this);
}
