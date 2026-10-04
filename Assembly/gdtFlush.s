.global gdtFlushSegments
gdtFlushSegments:
    lgdt (%rdi)

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    movw $0x00, %ax
    movw %ax, %fs
    movw %ax, %gs

    pushq $0x08
    leaq .reload_cs(%rip), %rax
    pushq %rax
    lretq

.reload_cs:
    ret 

.global tssFlushSelector
tssFlushSelector:
    movw %di, %ax 
    ltr %ax 
    ret
