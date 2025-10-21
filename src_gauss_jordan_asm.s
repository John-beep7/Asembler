.global gaussJordanASM
.extern printf

gaussJordanASM:
    // Save link register & frame pointer
    stp x29, x30, [sp, #-16]!
    mov x29, sp

    // Load address of message into x0
    adrp x0, msg@PAGE
    add  x0, x0, msg@PAGEOFF
    bl   _printf      // call printf

    // Restore stack & return
    ldp x29, x30, [sp], #16
    ret

msg:
    .asciz "Hello from Assembly!\n"