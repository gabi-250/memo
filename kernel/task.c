#include <stdint.h>

#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/paging.h>
#include <mm/addr_space.h>
#include <task.h>
#include <sched.h>
#include <kmalloc.h>
#include <panic.h>

static void
push_uint32(uint32_t *kernel_stack_top, uint32_t value) {
    *(uint32_t *)(*kernel_stack_top) = value;
    *kernel_stack_top -= sizeof(uint32_t);
}

task_control_block_t *
task_create(paging_context_t paging_ctx, vmm_context_t vmm_ctx, void (*task_fn)(void),
            void *ret_addr, bool is_userspace) {
    static uint32_t last_pid = 0;
    uint32_t kernel_stack_top = (uint32_t)alloc_kernel_stack(paging_ctx, &vmm_ctx);

    paging_context_t task_paging_ctx = is_userspace ?
                                       vmm_clone_paging_context(&vmm_ctx, paging_ctx) : paging_ctx;

    task_control_block_t *task = (task_control_block_t *)kmalloc(sizeof(task_control_block_t));
    uint32_t pid = last_pid++;

    // pid
    push_uint32(&kernel_stack_top, pid);
    // the return address of the sched_remove frame
    push_uint32(&kernel_stack_top, (uint32_t)sched_halt_or_crash);
    // task cleanup function
    push_uint32(&kernel_stack_top, (uint32_t)sched_remove);
    // push an address for task_fn to use
    if (ret_addr) {
        push_uint32(&kernel_stack_top, (uint32_t)ret_addr);
    }
    // EIP
    push_uint32(&kernel_stack_top, (uint32_t)task_fn);
    // EBP
    push_uint32(&kernel_stack_top, 0);
    // EBX
    push_uint32(&kernel_stack_top, 0);
    // ESI
    push_uint32(&kernel_stack_top, 0);
    // EDI
    *(uint32_t *)kernel_stack_top = 0;

    uint32_t cr3 = 0;

    // TODO: both cases should be handled by vmm_find_allocation.
    if (is_userspace) {
        vmm_allocation_t alloc = vmm_find_allocation(&vmm_ctx, (uint32_t)task_paging_ctx.page_directory);

        ASSERT(alloc.page_count, "invalid VMM state");

        cr3 = alloc.physical_addr;
    } else {
        cr3 = vmm_virtual_to_physical((uint32_t)task_paging_ctx.page_directory);
    }


    *task = (task_control_block_t) {
        .pid = pid,
        .kernel_stack_top = kernel_stack_top,
        .virtual_addr_space = cr3,
        .esp0 = kernel_stack_top,
        .vmm_context = vmm_ctx,
        .paging_ctx = task_paging_ctx,
        .parent = NULL,
    };

    return task;
}
