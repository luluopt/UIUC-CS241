/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once

#include "mmu.h"
#include <stddef.h>
#include <stdio.h>

#define DEFAULT_PID 42

typedef struct {
  mmu *mmu;     // mmu object that will do our translations into physical memory
  FILE *stream; // file that we want to map into memory
  size_t length;           // length of the file
  uintptr_t start_address; // Starting address of the mapping (virtual address)
} mmap;

/**
 * Allocates and initializes a mmap struct with the file pointed to by
 * path_to_file
 * This will also create an mmu and add pages to the right segment.
 * Don't add more pages than required.
 *
 * Note: We will not be testing invalid paths.
*/
mmap *mmap_create(char *path_to_file);

/**
 * Read from the mmap'd file into a buffer provided by the user.
 * You will need to use the mmu to perform translations from virtual
 * memory into physical addresses.
 * Return the number of bytes successfully read.
*/
size_t mmap_read(mmap *this, void *buff, size_t offset, size_t num_bytes);

/**
 * Write to mapped memory from a buffer provided by the user.
 * You should not be writing the data back to the disk until
 * munmap is called, only to and from physical memory.
*/
size_t mmap_write(mmap *this, void *buff, size_t offset, size_t num_bytes);

/**
 * Cleanup the struct initialized by mmap_create. Make sure to write back dirty
 * pages
 * to the file that we mapped. Make sure to destroy all resources including the
 * mmu,
 * the file stream, and the pointer passed in.
*/
void munmap(mmap *this);
