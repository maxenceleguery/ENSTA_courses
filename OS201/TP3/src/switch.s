.global switch_coroutine
switch_coroutine:
    push %rbp
    push %rbx
    push %r12
    push %r13
    push %r14
    push %r15
    mov %rsp, (%rdi)
    mov %rsi, %rdi
    jmp enter_coroutine
