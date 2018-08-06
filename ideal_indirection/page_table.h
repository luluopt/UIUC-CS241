/**
 * Ideal Indirection Lab
 * CS 241 - Fall 2017
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define VIRTUAL_ADDR_SPACE 32
#define PHYSICAL_ADDR_SPACE 32
#define NUM_PAGE_TABLES 2
#define NUM_OFFSET_BITS 12
#define NUM_VPN_BITS ((VIRTUAL_ADDR_SPACE - NUM_OFFSET_BITS) / NUM_PAGE_TABLES)
#define PAGE_SIZE (1 << NUM_OFFSET_BITS)
#define FRAME_SIZE (PAGE_SIZE)
#define NUM_ENTRIES (PAGE_SIZE / sizeof(page_directory_entry))

/*
 * The following structs are bit for the standard 32bit virtual and physical
 * address,
 * 4kb offset, 2 level page table.
 */
typedef struct {
  uint32_t base_addr : 20;
  uint32_t available : 3;
  uint32_t global_page : 1;
  uint32_t page_size : 1;
  uint32_t reserved : 1;
  uint32_t accessed : 1;
  uint32_t cache_disabled : 1;
  uint32_t write_through : 1;
  uint32_t user_supervisor : 1;
  uint32_t read_write : 1;
  uint32_t present : 1;
} page_directory_entry;

typedef struct { page_directory_entry entries[NUM_ENTRIES]; } page_directory;

typedef struct {
  uint32_t base_addr : 20;
  uint32_t available : 3;
  uint32_t global_page : 1;
  uint32_t page_table_attribute_index : 1;
  uint32_t dirty : 1;
  uint32_t accessed : 1;
  uint32_t cache_disabled : 1;
  uint32_t write_through : 1;
  uint32_t user_supervisor : 1;
  uint32_t read_write : 1;
  uint32_t present : 1;
} page_table_entry;

typedef struct { page_table_entry entries[NUM_ENTRIES]; } page_table;

typedef union {
  page_directory_entry pde;
  page_table_entry pte;
} paging_entry;

/*
 * The following is documentation for all the fields for those that are curious
 * (IA-32 Intel® Architecture Software Developer’s Manual, Volume 3)
 * (http://flint.cs.yale.edu/cs422/doc/24547212.pdf#page=88)
 */

/*
 * Page base address, bits 12 through 32 -
 *
 * Specifies the physical address of the first byte of a 4-KByte page.
 * The bits in this field are interpreted as the 20 mostsignificant bits
 * of the physical address, which forces pages to be aligned on
 * 4-KByte boundaries.
 */

/*
 * Present (P) flag, bit 0 -
 *
 * Indicates whether the page or page table being pointed to by the entry is
 * currently loaded in physical memory.
 * When the flag is set, the page is in physical memory and address translation
 * is carried out
 * When the flag is clear, the page is not in memory and,
 * if the processor attempts to access the page, it generates a page-fault
 * exception.
 *
 * If the processor generates a page-fault exception,
 * the operating system generally needs to carry out the following operations:
 * 1. Copy the page from disk storage into physical memory.
 * 2. Load the page address into the page-table or page-directory entry and set
 * its present flag.
 * Other flags, such as the dirty and accessed flags, may also be set at this
 * time.
 */

/*
 * Read/write (R/W) flag, bit 1 -
 *
 * Specifies the read-write privileges for a page or group of pages
 * (in the case of a page-directory entry that points to a page table).
 * When this flag is clear, the page is read only;
 * when the flag is set, the page can be read and written into.
 */

/*
 * User/supervisor (U/S) flag, bit 2 -
 *
 * Specifies the user-supervisor privileges for a page or group of pages
 * (in the case of a page-directory entry that points to a page table).
 * When this flag is clear, the page is assigned the supervisor privilege level;
 * when the flag is set, the page is assigned the user privilege level.
 */

/*
 * Page-level write-through (PWT) flag, bit 3 -
 *
 * Controls the write-through or write-back caching policy of
 * individual pages or page tables.
 * When the PWT flag is set,
 * write-through caching is enabled for the associated page or page table;
 * when the flag is clear, write-back caching is
 * enabled for the associated page or page table.
 */

/*
 * Page-level cache disable (PCD) flag, bit 4 -
 *
 * Controls the caching of individual pages or page tables.
 * When the PCD flag is set, caching of the associated page
 * or page table is prevented;
 * when the flag is clear, the page or page table can be cached.
 * This flag permits caching to be disabled for pages that contain
 * memory-mapped I/O ports or that do not
 * provide a performance benefit when cached.
 */

/*
 * Accessed (A) flag, bit 5 -
 *
 * Indicates whether a page or page table has been accessed
 * (read from or written to) when set.
 * Memory management software typically clears this flag when a
 * page or page table is initially loaded into physical memory.
 * The processor then sets this flag the first time
 * a page or page table is accessed.
 * This flag is a “sticky” flag, meaning that once set,
 * the processor does not implicitly clear it.
 * Only software can clear this flag.
 * The accessed and dirty flags are provided for use
 * by memory management software to manage
 * the transfer of pages and page tables into and out of physical memory.
 */

/*
 * Dirty (D) flag, bit 6 -
 *
 * Indicates whether a page has been written to when set.
 * (This flag is not used in page-directory entries that point to page tables.)
 * Memory management software typically clears this flag when
 * a page is initially loaded into physical memory.
 * The processor then sets this flag the first time
 * a page is accessed for a write operation.
 * This flag is “sticky,” meaning that once set,
 * the processor does not implicitly clear it.
 * Only software can clear this flag.
 * The dirty and accessed flags are provided for use by memory management
 * software
 * to manage the transfer of pages and page tables into and out of physical
 * memory.
 */

/*
 * Page size (PS) flag, bit 7 page-directory entries for 4-KByte pages
 *
 * Determines the page size.
 * When this flag is clear, the page size is 4 KBytes and the page-directory
 * entry points to a page table.
 * When the flag is set, the page size is 4 MBytes for normal 32-bit addressing
 * (and 2 MBytes if extended physical addressing is enabled)
 * and the page-directory entry points to a page.
 * If the page-directory entry points to a page table,
 * all the pages associated with that page table will be 4-KByte pages.
 */

/*
 * Page attribute table index (PAT) flag, bit 7
 * in page-table entries for 4-KByte pages
 * and bit 12 in page-directory entries for 4-MByte pages
 * (Introduced in the Pentium III processor.)

 * Selects PAT entry.
 * For processors that support the page attribute table (PAT),
 * this flag is used along with the PCD and PWT flags
 * to select an entry in the PAT,
 * which in turn selects the memory type for the page
 * (see Section 10.12., “Page Attribute Table (PAT)”).
 * For processors that do not support the PAT, this bit is reserved and should
 be set to 0.
 */

/*
 * Global (G) flag, bit 8 -
 * (Introduced in the Pentium Pro processor.)
 *
 * Indicates a global page when set.
 * When a page is marked global and the page global enable (PGE) flag in
 * register CR4 is set,
 * the page-table or page-directory entry for the page is not invalidated
 * in the TLB when register CR3 is loaded or a task switch occurs.
 * This flag is provided to prevent frequently used pages
 * (such as pages that contain kernel or other operatiatng system or executive
 * code)
 * from being flushed from the TLB.
 * Only software can set or clear this flag.
 * For page-directory entries that point to page tables,
 * this flag is ignored and the global characteristics of a page are set in the
 * page-table entries.
 * See Section 3.11., “Translation Lookaside Buffers (TLBs)”,
 * for more information about the use of this flag.
 * (This bit is reserved in Pentium and earlier IA-32 processors.)
 */

/*
 * Reserved and available-to-software bits For all IA-32 processors.
 * Bits 9, 10, and 11 are available for use by software.
 * (When the present bit is clear, bits 1 through 31 are available to software
 * see Figure 3-16.)
 *
 * In a page-directory entry that points to a page table, bit 6 is reserved and
 * should be set to 0.
 * When the PSE and PAE flags in control register CR4 are set,
 * the processor generates a page fault if reserved bits are not set to 0.
 * For Pentium II and earlier processors.
 * Bit 7 in a page-table entry is reserved and should be set to 0.
 * For a page-directory entry for a 4-MByte page, bits 12 through 21 are
 * reserved and must be set to 0.
 * For Pentium III and later processors.
 * For a page-directory entry for a 4-MByte page, bits 13 through 21 are
 * reserved and must be set to 0.
 */
