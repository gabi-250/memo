#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>

typedef struct task_control_block {
    uint32_t pid;
    uint32_t kernel_stack_top;
    uint32_t virtual_addr_space;
    uint32_t esp0;
} task_control_block_t;

#endif /* __TASK_H__ */
