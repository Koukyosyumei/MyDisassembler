	.globl main

main:
	mov	$0x11223344, %eax
	mov	$0x11223344, %ecx
	mov	$0x11223344, %r8
	mov	$0x11223344, %r15
    
	add	$0x11223344, %eax
	sub	$0x11223344, %eax

	ret
