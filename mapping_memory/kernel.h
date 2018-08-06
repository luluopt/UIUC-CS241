/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include "page_table.h"
#include <stdbool.h>
#include <stdlib.h>

#define NUM_PHYSICAL_PAGES 250
// 1MB = 250 4kb pages
#define PHYSICAL_MEMORY_SIZE (PAGE_SIZE * NUM_PHYSICAL_PAGES)

/* Memory pool that represents all of physical memory */
char physical_memory[PHYSICAL_MEMORY_SIZE] __attribute__((aligned(PAGE_SIZE)));

/* mapping to determine which page_table_entry owns which page. */
page_table_entry *entry_mapping[NUM_PHYSICAL_PAGES];

/*
 * Returns the physical address of the next chunk to allocate if possible,
 * these will be in PAGE_SIZE chunks aligned to PAGE_SIZE bytes.
 * Returns NULL if there are no more frames to give out.
 */
void *ask_kernel_for_frame(page_table_entry *entry);
void remove_swap_files();
void write_page_to_disk(page_table_entry *entry);
void read_page_from_disk(page_table_entry *entry);
void return_frame_to_kernel(void *frame_address);
