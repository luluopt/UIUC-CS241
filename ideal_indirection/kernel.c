/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */
#include "kernel.h"
#include <assert.h>
#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SWAP_FILE_DIRECTORY "swap_files"

#define RESERVED ((page_table_entry *)42)

/*
 * Note: the paging to disk scheme is not robust.
 *
 * If you page a page directory or page table to disk, then you need to update
 * the appropriate pointers in the mmu and page directory.
 * Also using a paging_entry pointer is not enough, since if you page tables to
 * disk and back then they might get relocated to different frames.
 * Even if you avoid paging tables and directories to disk then you will
 * eventually run out of physical memory if the user keeps spawning processes.
 */
void *ask_kernel_for_frame(page_table_entry *entry) {
  for (int i = 0; i < NUM_PHYSICAL_PAGES; i++) {
    if (entry_mapping[i] == NULL) {
      entry_mapping[i] = entry == NULL ? RESERVED : entry;
      char *frame_address = physical_memory + (PAGE_SIZE * i);
      // Zeroing out the page, since an os does not want processes reading other
      // processes' pages
      memset(frame_address, 0x00, PAGE_SIZE);
      return frame_address;
    }
  }

  printf("RAN OUT OF SPACE! NEED TO PAGE SOMETHING TO DISK\n");

  // No available frames so we are going to have to page one to disk :(
  // Going to kick out the first frame that doesn't belong to a processes paging
  // structures, since I don't want to write an eviction scheme
  for (int i = 0; i < NUM_PHYSICAL_PAGES; i++) {
    if (entry_mapping[i] != RESERVED && entry_mapping[i] != entry) {
      page_table_entry *evictee = entry_mapping[i];
      write_page_to_disk(evictee);
      evictee->present = false;
      evictee->base_addr = (uint32_t)NULL;
      entry_mapping[i] = NULL;
      // Now that a page has been freed up we can rerun the algorithm
      // (equivalent to a goto)
      return ask_kernel_for_frame(entry);
    }
  }

  return NULL;
}

void return_frame_to_kernel(void *base_addr) {
  assert((uint32_t)base_addr % PAGE_SIZE == 0);
  int index = ((char *)base_addr - physical_memory) / PAGE_SIZE;
  entry_mapping[index] = NULL;
}

void remove_swap_file(page_table_entry *entry) {
  char filepath[PATH_MAX];
  sprintf(filepath, "%s/%p", SWAP_FILE_DIRECTORY, entry);
  remove(filepath);
}

void remove_swap_files() {
  // remove everything in tempory directory
  DIR *theFolder = opendir(SWAP_FILE_DIRECTORY);
  if (theFolder) {
    struct dirent *next_file;
    char filepath[PATH_MAX];
    while ((next_file = readdir(theFolder)) != NULL) {
      // build the path for each file in the folder
      sprintf(filepath, "%s/%s", SWAP_FILE_DIRECTORY, next_file->d_name);
      remove(filepath);
    }

    closedir(theFolder);
    rmdir(SWAP_FILE_DIRECTORY);
  }
}

void write_page_to_disk(page_table_entry *entry) {
  // Check to see if the directory exists
  struct stat st;

  if (stat(SWAP_FILE_DIRECTORY, &st) == -1) {
    mkdir(SWAP_FILE_DIRECTORY, 0700);
  }

  void *frame = (void *)(uintptr_t)(entry->base_addr << NUM_OFFSET_BITS);
  char *file_location = NULL;
  // using entry address as the key to the frame
  asprintf(&file_location, "%s/%p.bin", SWAP_FILE_DIRECTORY, entry);
  FILE *swap_file = fopen(file_location, "w");
  if (swap_file) {
    fwrite(frame, FRAME_SIZE, 1, swap_file);
    fclose(swap_file);
  }

  free(file_location);
}

void read_page_from_disk(page_table_entry *entry) {
  void *frame = (void *)(uintptr_t)(entry->base_addr << NUM_OFFSET_BITS);
  char *file_location = NULL;
  // using entry address as the key to the frame
  asprintf(&file_location, "%s/%p.bin", SWAP_FILE_DIRECTORY, entry);
  FILE *swap_file = fopen(file_location, "r");
  if (swap_file) {
    fread(frame, FRAME_SIZE, 1, swap_file);
    fclose(swap_file);
  }

  free(file_location);
}
