.section .bss
.align 16

# Reserve a stack for the kernel
__kernel_stack_bottom:
.skip 1 << 14
__kernel_stack_top:

.section .data.boot, "awx"
.extern kernel_meminfo_size

# A temporary GDT to replace the one set up by GRUB (only needed until paging is
# enabled, because once we have paging, the GDT setup can more easily be handled in
# C land).
.include "./arch/i386/include/gdt.inc"
# The boot page tables
.include "./arch/i386/include/page_directory.inc"
# Some helper macros
.include "./arch/i386/include/macros.inc"

.section .text.boot, "awx"

.global __kernel_stack_top

# The kernel entry point
.global _start
_start:
    # Disable interrupts for now
    cli
    lea __kernel_stack_top - 0xC0000000, %esp
    # Save EBX: GRUB puts the physical address of the multiboot2 information
    # structure into EBX before invoking the OS (we add 0xC0000000 to get its
    # virtual address).
    add $0xC0000000, %ebx
    push %ebx
    # Save EAX, which contains a magic value that tells the kernel the bootloader
    # is multiboot2-compliant.
    push %eax
    # Set up a temporary GDT
    # Remember: at this point paging isn't enabled yet, so we need to subtract
    # 0xC0000000 from every memory access.
    lgdt tmp_gdt_descriptor
    jmp $TMP_GDT_CODE_SEGMENT, $.Ltmp_gdt_done
.Ltmp_gdt_done:
    mov $TMP_GDT_DATA_SEGMENT, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    # Read current CR4 and set the PSE bit to enable 4MB pages
    mov %cr4, %ebx
    or $0x00000010, %ebx
    # Update CR4
    mov %ebx, %cr4
    lea boot_page_directory, %eax
    mov %eax, %cr3
_enable_paging:
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0
    PUSH_KERN_MEMINFO
    call paging_bootstrap
    # Save the address of the PD returned by paging_bootstrap
    mov %eax, %ebx
    call kernel_meminfo_size
    add %eax, %esp
    # Update EIP to point to a higher-half address from now on.
    jmp _higher_half

# The higher-half kernel entry point
.section .text.boot.higher_half, "awx"
_higher_half:
    # update the stack pointer, keeping in mind that we already pushed two
    # values to the stack
    mov $__kernel_stack_top, %esp
    sub $8, %esp
    # Move the address of the "real" paging directory into CR3
    mov %ebx, %cr3
    # Set up the IDT and enable interrupts
    call idt_init
    # Now that we've enabled paging, call some C code to sets up the real GDT
    call gdt_init
    # Call the global constructors
    call _init
    PUSH_KERN_MEMINFO
    call kernel_main
    # Call the global destructors
    call _fini
    # kernel_main should not return
    cli
halt:
    hlt
    jmp halt
