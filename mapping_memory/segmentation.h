/**
 * Mapping Memory Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  uintptr_t start;
  uintptr_t end;
  uint8_t permissions;
  bool grows_down;
} vm_segmentation;

typedef enum segments {
  STACK,
  MMAP,
  HEAP,
  BSS,
  DATA,
  TEXT,
  TESTING, // this segment is made up for testing purposes
  NUM_SEGMENTS
} segments;

typedef struct { vm_segmentation segments[NUM_SEGMENTS]; } vm_segmentations;

typedef enum permissions { READ = 0x1, WRITE = 0x2, EXEC = 0x4 } permissions;

void grow_segment(vm_segmentations *segmentations, segments segment,
                  size_t num_pages);
void shrink_segment(vm_segmentations *segmentations, segments segment,
                    size_t num_pages);
bool address_in_segmentations(vm_segmentations *segmentations,
                              uint32_t address);
vm_segmentation *find_segment(vm_segmentations *segmentations,
                              uint32_t address);
