.global _start

_start:
    call subroutine
    xor %eax, %eax
    mov $0x11223344, %eax

subroutine:
    ret

