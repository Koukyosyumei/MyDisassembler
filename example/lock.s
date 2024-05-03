.global main
    .text
main:
    mov $0x0, %eax
    mov $0x10, %ecx
    
repeat_add:
    lock inc dword [counter]
    loop repeat_add              
    
    jmp end_program            
    
end_program:
    ret

section .bss
    counter resd 1

