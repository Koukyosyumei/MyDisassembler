	.globl main

main:
	mov $0x200, %rsi
	mov $0x300, %rdi
    mov $0x11, %rcx
    cld
    rep movsb
    ret
