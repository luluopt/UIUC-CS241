/**
 * Machine Problem: Malloc
 * CS 241 - Fall 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/**
 * A block like below
 *
 *         32-bits                       31-bits + 1-bit used-flag
 * +--------------------------------+--------------------------------+
 * |   required-size                |  block-size                   U| header
 * |--------------------------------+--------------------------------+
 * |  Content if used                                                | body
 * |  Prev-block and Next-block pointer if not used                  |
 * +--------------------------------+--------------------------------+
 * |   required-size                |  block-size                   U| footer
 * +--------------------------------+--------------------------------+
 *
 * A block has a header, a body and a footer
 * The header and footer have the same content:
 *   - required-size: 32-bits, so we can require 32GB at most once.
 *   - block-size: 31-bits, The block-size will be divide by 2, so the last
 *       bit will always zero, we can store it's value using 31-bits
 *   - used-flag: 1-bit, indicate if the block used of free
 * The body:
 *   - For used block: It is the memory for the user-space
 *   - For free block: It contains two 64-bits points, prev and next. So we
 *       can linked the free blocks using a double-linked list.
 * The body has min-size 16-bytes, so the block has min-size 32-bytes
 */

/**
 * For efficiency, I used macros to get or set block's header, body and footer
 */

/**
 * For a pointer to header or footer, get the required-size, block-size and used
 */
#define RSIZE(hp) ((*(uint64_t*)(hp)) >> 32)
#define BSIZE(hp) ((*(uint64_t*)(hp)) & 0xfffffffe)
#define USED(hp) ((*(uint64_t*)(hp)) & 1)

/**
 * For a pointer to header or footer, set the required-size, block-size and used
 */
#define SET_ALL(hp,rs,bs,u) (*(uint64_t*)(hp)=(((uint64_t)(rs)<<32)|(bs)|(u)))

/**
 * For a pointer to header or footer, set the used flag to used or unused
 */
#define SET_USED(hp) (*(uint64_t*)(hp) |= 1)
#define SET_UNUSED(hp) (*(uint64_t*)(hp) &= 0xfffffffe)

/**
 * For the pointer to the body, return the pointer to the header or footer
 */
#define HEADER(ptr) ((void*)(ptr) - 8)
#define FOOTER(ptr) ((void*)(ptr) + BSIZE((void*)(ptr)-8) - 16)

/**
 * For the pointer to the body, return the previous block just before it
 */
#define PREV_BLOCK(ptr) ((void *)(ptr) - BSIZE((void *)(ptr)-16))
/**
 * For the pointer to the body, return the next block just after it
 */
#define NEXT_BLOCK(ptr) ((void *)(ptr) + BSIZE((void *)(ptr)-8))

/**
 * For the free block, return the previous block just before it
 */
#define FREE_PREV_BLOCK(ptr) (*(void**)(ptr))
/**
 * For the free block, return the next block just after it
 */
#define FREE_NEXT_BLOCK(ptr) (*(void**)((void*)(ptr)+8))

/**
 * For the free block, set the previous block or next block
 */
#define FREE_SET_PREV(ptr,p) (*(void**)(ptr)=(p))
#define FREE_SET_NEXT(ptr,p) (*(void**)((void*)(ptr)+8)=(p))

/**
 * Fix a size to 16*n
 */
#define FIX_16N(s) (((s)+15)/16*16)

#define MIN(a,b) ((a)<(b)?(a):(b))

// First block will be initialize only once
void *first_block = NULL;
// Free block pointer the free list
void *free_block = NULL;

/**
 * Init the heap with a empty block and a epilogue header
 */
void* init_heap() {
    void *p = sbrk(32);
    if (p == (void*)-1) {
        return NULL;
    }
    SET_ALL(p, 0, 0, 1); // Unused
    SET_ALL(p + 8, 0, 16, 1); // Set the header of first-block
    SET_ALL(p + 16, 0, 16, 1); // Set the footer of first-block
    SET_ALL(p + 24, 0, 0, 1); // A epilogue header
    first_block = p + 16;
    return first_block;
}

/**
 * Remove a block from free_list
 */
void remove_block_from_free_list(void *ptr) {
    // Get the prev and next block
    void *prev = FREE_PREV_BLOCK(ptr);
    void *next = FREE_NEXT_BLOCK(ptr);
    if (prev) {
        // Update prev's next
        FREE_SET_NEXT(prev, next);
    }
    if (next) {
        // Update next's prev
        FREE_SET_PREV(next, prev);
    }

    // If the block is the header of free_list, update free_list
    if (ptr == free_block) {
        free_block = next;
    }
}

/**
 * Merge free block with its next block
 */
void merge_free_block_with_next(void *ptr) {
    if (USED(HEADER(ptr))) {
        // If current block in-used, can't merge
        return;
    }
    void *next = NEXT_BLOCK(ptr);
    if (USED(HEADER(next))) {
        // If the next block is in-used, can't merge
        return;
    }

    // Remove next from free_list
    remove_block_from_free_list(next);
    // Add next's bsize to current ptr
    size_t bsize = BSIZE(HEADER(ptr)) + BSIZE(HEADER(next));
    SET_ALL(HEADER(ptr), 0, bsize, 0);
    SET_ALL(FOOTER(ptr), 0, bsize, 0);
}
/**
 * Merge free block with its prev block and next block
 */
void merge_free_block(void *ptr) {
    merge_free_block_with_next(ptr);
    merge_free_block_with_next(PREV_BLOCK(ptr));
}

/**
 * Add a block to the free_list
 */
void add_block_to_free_list(void *ptr) {
    // Set the block unused
    SET_UNUSED(HEADER(ptr));
    SET_UNUSED(FOOTER(ptr));

    FREE_SET_PREV(ptr, NULL);
    FREE_SET_NEXT(ptr, NULL);
    if (free_block == NULL) {
        // Just set free_block
        free_block = ptr;
    } else {
        // Insert block before free_block
        FREE_SET_PREV(free_block, ptr);
        FREE_SET_NEXT(ptr, free_block);
        free_block = ptr;
    }

    merge_free_block(ptr);
}

/**
 * Search the free_list and find a block whose bsize >= bsize
 */
void* get_block_from_free_list(size_t bsize) {
    void *ptr = free_block;
    while (ptr) {
        if (BSIZE(HEADER(ptr)) >= bsize) {
            // If found a match block, remove it from free_list and return it
            remove_block_from_free_list(ptr);
            return ptr;
        }
        ptr = FREE_NEXT_BLOCK(ptr);
    }
    return NULL;
}

/**
 * Increase the heap and return a block with size
 */
void* increase_heap(size_t bsize) {
    if (bsize < 4096) {
        // If size less than 4096, fix to it
        bsize = 4096;
    } else {
        // size must be 16*n
        bsize = FIX_16N(bsize);
    }
    void* ptr = sbrk(bsize);
    if (ptr == (void*)-1) {
        return NULL;
    }
    // Last epilogue header will be the block's header
    // Init the Header and Footer
    SET_ALL(HEADER(ptr), 0, bsize, 0);
    SET_ALL(FOOTER(ptr), 0, bsize, 0);

    // Set new epilogue header 
    SET_ALL(HEADER(NEXT_BLOCK(ptr)), 0, 0, 1);

    // If the prev block if free, merge it
    void *prev = PREV_BLOCK(ptr);
    if (!USED(HEADER(prev))) {
        bsize = BSIZE(HEADER(prev)) + BSIZE(HEADER(ptr));
        remove_block_from_free_list(prev);
        SET_ALL(HEADER(prev), 0, bsize, 1);
        SET_ALL(FOOTER(prev), 0, bsize, 1);
        return prev;
    }

    return ptr;
}

/**
 * Set a block as used, if the remain room is to large, split it
 */
void use_block_and_split(void *ptr, size_t mbsize, size_t rsize) {
    size_t bsize = BSIZE(HEADER(ptr)); // The real block-size
    size_t remain = bsize - mbsize;
    if (remain > 32) {
        // The block remain too much, we can split it
        SET_ALL(HEADER(ptr), rsize, mbsize, 1);
        SET_ALL(FOOTER(ptr), rsize, mbsize, 1);

        // Set the remain part as a free block
        ptr = NEXT_BLOCK(ptr);
        SET_ALL(HEADER(ptr), 0, remain, 0);
        SET_ALL(FOOTER(ptr), 0, remain, 0);
        add_block_to_free_list(ptr);
    } else {
        SET_ALL(HEADER(ptr), rsize, bsize, 1);
        SET_ALL(FOOTER(ptr), rsize, bsize, 1);
    }
}


/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void *p = malloc(num * size);
    if (p != NULL) {
        memset(p, 0, num * size);
    }
    return p;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */

void *malloc(size_t size) {
    if (first_block == NULL) {
        // If first block not initialize, init the heap
        if (init_heap() == NULL) {
            return NULL;
        }
    }

    // Calculate the min block-size, need add 16-bytes header and footer
    size_t mbsize = 16; // 16 is the size of header + footer
    if (size <= 16) {
        // If size < 16, fix to 16
        mbsize += 16;
    } else {
        // Fix size to 16*n
        mbsize += FIX_16N(size);
    }

    // Get a block from free_list
    void *ptr = get_block_from_free_list(mbsize);
    if (ptr == NULL) {
        // Increase the heap
        ptr = increase_heap(mbsize);
        if (ptr == NULL) {
            return NULL;
        }
    }

    // Used the block
    use_block_and_split(ptr, mbsize, size);

    return ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (ptr == NULL) {
        return;
    }
    add_block_to_free_list(ptr);
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // Calculate the min block-size, need add 16-bytes header and footer
    size_t mbsize = 16; // 16 is the size of header + footer
    if (size <= 16) {
        // If size < 16, fix to 16
        mbsize += 16;
    } else {
        // Fix size to 16*n
        mbsize += FIX_16N(size);
    }

    // Get current block size
    size_t bsize = BSIZE(HEADER(ptr)); 
    size_t rsize = RSIZE(HEADER(ptr)); 
    if (bsize >= mbsize) {
        // Just reset rsize
        use_block_and_split(ptr, mbsize, size);
        return ptr;
    }

    // If not enough, but the next block if free
    void *next = NEXT_BLOCK(ptr);
    if (!USED(HEADER(next))) {
        bsize = BSIZE(HEADER(ptr)) + BSIZE(HEADER(next));
        remove_block_from_free_list(next);
        SET_ALL(HEADER(ptr), 0, bsize, 1);
        SET_ALL(FOOTER(ptr), 0, bsize, 1);
        if (bsize >= mbsize) {
            use_block_and_split(ptr, mbsize, size);
            return ptr;
        }
    }

    // If not enough, but the prev block if free
    void *prev = PREV_BLOCK(ptr);
    if (!USED(HEADER(prev))) {
        bsize = BSIZE(HEADER(prev)) + BSIZE(HEADER(ptr));
        if (bsize >= mbsize) {
            remove_block_from_free_list(prev);
            SET_ALL(HEADER(prev), 0, bsize, 1);
            SET_ALL(FOOTER(prev), 0, bsize, 1);
            memcpy(prev, ptr, MIN(rsize, size));
            use_block_and_split(prev, mbsize, size);
            return prev;
        }
    }
    void *p = malloc(size);
    if (p == NULL) {
        return NULL;
    }
    memcpy(p, ptr, MIN(rsize, size));
    free(ptr);
    return p;
}

/**
 * The function used to debug
 * Attention please! Not use printf here because the printf will call malloc
 */
void print_block(void *ptr) {
    char buf[128];
    void *h = HEADER(ptr);
    int len = snprintf(buf, sizeof(buf), "%p:\t%lu\t%lu\t%lu\n",
            h, RSIZE(h), BSIZE(h), USED(h));
    write(1, buf, len);
}
void print_free_block(void *ptr) {
    char buf[128];
    void *h = HEADER(ptr);
    void *prev = FREE_PREV_BLOCK(ptr);
    void *next = FREE_NEXT_BLOCK(ptr);
    if (prev) {
        prev = HEADER(prev);
    }
    if (next) {
        next = HEADER(next);
    }
    int len = snprintf(buf, sizeof(buf), "%p:%10lu %10lu %10lu %10p %10p\n",
            h, RSIZE(h), BSIZE(h), USED(h), prev, next);
    write(1, buf, len);
}
void debug() {
    void *ptr = NEXT_BLOCK(first_block);
    while (ptr && ptr < sbrk(0)) {
        print_block(ptr);
        ptr = NEXT_BLOCK(ptr);
    }
    write(1, "Free:\n", 6);
    ptr = free_block;
    int i = 0;
    while (ptr) {
        i++;
        print_free_block(ptr);
        ptr = FREE_NEXT_BLOCK(ptr);
        if (i > 10) {
            break;
                }
    }
    write(1, "\n", 1);
}
