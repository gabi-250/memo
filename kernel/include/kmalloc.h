#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stddef.h>

#define KERNEL_HEAP_START 0xE0000000
// 16M
#define KERNEL_HEAP_SIZE  0x01000000

typedef struct kmalloc_header {
    // The size of this free region
    size_t size;
    // The next free region
    struct kmalloc_header *next;
} kmalloc_header_t;

// Map all the heap pages. This should be called before any calls to kmalloc or
// kfree.
void kmalloc_init();
void *kmalloc(size_t);
void kfree(void *);

#endif /* __KMALLOC_H__ */
