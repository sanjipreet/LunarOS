# Assembly/idtStubs.s

.macro InterruptIgnore num
.global HandleInterruptIgnore\num 
HandleInterruptIgnore\num:
    movb $\num, (active_interrupt_vector) 
    jmp commonStubHandler
.endm

.macro HandleInterrupt num 
.global HandleInterrupt\num 
HandleInterrupt\num:
    movb $\num, (active_interrupt_vector)  
    jmp commonStubHandler
.endm 

InterruptIgnore 0x00
InterruptIgnore 0x01
InterruptIgnore 0x02
InterruptIgnore 0x03
InterruptIgnore 0x04
InterruptIgnore 0x05
InterruptIgnore 0x06
InterruptIgnore 0x07
HandleInterrupt 0x08
InterruptIgnore 0x09
HandleInterrupt 0x0A
HandleInterrupt 0x0B
HandleInterrupt 0x0C
HandleInterrupt 0x0D
HandleInterrupt 0x0E
InterruptIgnore 0x0F
InterruptIgnore 0x10
HandleInterrupt 0x11
InterruptIgnore 0x12
InterruptIgnore 0x13
InterruptIgnore 0x14
HandleInterrupt 0x1E

InterruptIgnore 0x20
InterruptIgnore 0x21

commonStubHandler:
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rax
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %rbx
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    .extern coreInterruptDispatcher
    call coreInterruptDispatcher

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rbx
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rax
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    
    iretq

.data
.global active_interrupt_vector
active_interrupt_vector: 
    .byte 0
