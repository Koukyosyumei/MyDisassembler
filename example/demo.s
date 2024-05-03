.global main
    .text
main:
    movsx %ebx, %rax
    jmp end_program
result_is_zero:
    ret
end_program:
    ret
