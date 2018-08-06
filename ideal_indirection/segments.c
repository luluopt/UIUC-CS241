/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */
#include "page_table.h"
#include "segments.h"
void grow_segment(vm_segmentations *segmentations, segments segment,
                  size_t num_pages) {
  vm_segmentation *segmentation = &(segmentations->segments[segment]);
  if (segmentation->grows_down) {
    segmentation->end -= num_pages * PAGE_SIZE;
  } else {
    segmentation->end += num_pages * PAGE_SIZE;
  }
}

void shrink_segment(vm_segmentations *segmentations, segments segment,
                    size_t num_pages) {
  vm_segmentation *segmentation = &(segmentations->segments[segment]);
  if (segmentation->grows_down) {
    segmentation->end += num_pages * PAGE_SIZE;
  } else {
    segmentation->end -= num_pages * PAGE_SIZE;
  }
}

bool address_in_segmentations(vm_segmentations *segmentations,
                              uint32_t address) {
  return find_segment(segmentations, address);
}

vm_segmentation *find_segment(vm_segmentations *segmentations,
                              uint32_t address) {
  for (int i = 0; i < NUM_SEGMENTS; i++) {
    vm_segmentation *segmentation = &(segmentations->segments[i]);
    if (segmentation->grows_down) {
      if (address >= segmentation->end && address <= segmentation->start) {
        return segmentation;
      }
    } else {
      if (address >= segmentation->start && address <= segmentation->end) {
        return segmentation;
      }
    }
  }

  return NULL;
}
