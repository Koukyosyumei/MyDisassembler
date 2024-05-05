.global _start

_start:
    mov $0, %eax
    cmp $0, %eax
    jz zero_label
    jmp end_label
    jmp _start

zero_label:
    push %rsp
    xor %eax, %eax
    ret

end_label:
    push %rdi
    xor %ecx, %ecx
    ret

