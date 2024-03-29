#ifndef __PAGING_H__
#define __PAGING_H__

// ======================================================================
// Paging constants
// ======================================================================
// 4 KB pages
#define PAGE_SIZE            (1 << 12)
#define PAGE_TABLE_SIZE      1024
#define PAGE_DIRECTORY_START 22
#define PAGE_TABLE_START     12

// ======================================================================
// Page table entry flags
// ======================================================================
// Present flag.
#define PAGE_FLAG_PRESENT         1
// Read/write flag. If 0, writes aren't allowed.
#define PAGE_FLAG_WRITE          (1 << 1)
// User/supervisor flag. If 0, user-mode accesses aren't allowed.
#define PAGE_FLAG_USER           (1 << 2)
// Page-level write-through flag. It (indirectly) determines the memory type
// used to access the page referenced by this entry.
#define PAGE_FLAG_WRITE_THROUGH  (1 << 3)
// The cache disable flag. It (indirectly) determines the memory type used to
// access this entry.
#define PAGE_FLAG_CACHE_DISABLE  (1 << 4)
// The accessed flag. If 1, the page referenced by this entry has been accessed.
#define PAGE_FLAG_ACCESSED       (1 << 5)
// The dirty flag. If 1, the page referenced by this entry has been written to.
#define PAGE_FLAG_DIRTY          (1 << 6)
// The page size flag. Must be 1 if 4M pages are in use.
#define PAGE_FLAG_PAGE_SIZE      (1 << 7)
// The global flag. If CR4.PGE = 1, determines whether the translation is
// global.
#define PAGE_FLAG_GLOBAL         (1 << 8)

#define PAGE_DIRECTORY_INDEX(vaddr)   ((vaddr) >> PAGE_DIRECTORY_START)
#define PAGE_TABLE_INDEX(vaddr)       (((vaddr) >> PAGE_TABLE_START) & ((1 << 10) - 1))

#ifndef __ASSEMBLY__
#include <stdbool.h>
#include <stdint.h>

typedef struct page_table {
    uint32_t entries[PAGE_TABLE_SIZE];
} __attribute__ ((aligned(4096))) page_table_t;

typedef struct paging_context {
    page_table_t *page_directory;
    page_table_t *page_tables;
} paging_context_t;


paging_context_t init_paging();
void paging_set_page_directory(uint32_t);

// The format of a page directory entry (with 4KB pages) is:
// | 31                   12| 11     8 | 7 | 6   | 5 | 4   | 3   | 2   | 1   | 0 |
// |-----------------------------------------------------------------------------|
// | address of page table  | ignored  | 0 | ign | A | PCD | PWT | U/S | R/W | P |
//
// The format of a page table entry (with 4KB pages) is:
// | 31                   12| 11     9 | 8 | 7   | 6 | 5  | 4   | 3   | 2   | 1   | 0 |
// |----------------------------------------------------------------------------------|
// | address of 4KB page    | ignored  | G | PAT | D | A  | PCT | PWT | U/S | R/W | P |
void paging_map_virtual_to_physical(paging_context_t, uint32_t, uint32_t, uint32_t);

// Unamp the specified address.
void paging_unmap_addr(paging_context_t paging_ctx, uint32_t virtual_addr);

// Check whether the specified address is page-aligned.
bool paging_is_aligned(uint32_t);

// Return the page the specified address belongs to.
uint32_t paging_align_addr(uint32_t addr);

// Invalidate the TLB entries of the page that corresponds to the specified
// address.
void paging_invlpg(uint32_t addr);

/// Calculate how many pages are required for an allocation of `size` bytes.
uint32_t paging_page_count(uint32_t size);
#endif

#endif /* __PAGING_H__ */
