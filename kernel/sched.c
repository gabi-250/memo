#include <task.h>
#include <sched.h>
#include <kmalloc.h>
#include <mm/vmm.h>
#include <mm/meminfo.h>
#include <stddef.h>

extern page_table_t ACTIVE_PAGE_DIRECTORY;

typedef struct task_list {
    task_control_block_t *task;
    task_control_block_t *next;
} task_list_t;

static task_list_t tasks;

task_control_block_t *current_task;

extern kernel_meminfo_t KERNEL_MEMINFO;

void
init_sched() {
    // Create the first kernel task
    current_task = (task_control_block_t *)kmalloc(sizeof(task_control_block_t));
    current_task->pid = 0;
    current_task->kernel_stack_top = KERNEL_MEMINFO.stack_top; // XXX
    current_task->virtual_addr_space = (uint32_t)&ACTIVE_PAGE_DIRECTORY;
    current_task->esp0 = KERNEL_MEMINFO.stack_top; // XXX
    current_task->vmm_context = NULL; // XXX

    tasks = (task_list_t) {
        .task = current_task,
        .next = NULL,
    };
}
