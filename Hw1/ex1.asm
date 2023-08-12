.global _start

.section .text
_start:
#initialize
	movq num(%rip), %rax
	movb $0, Bool(%rip)
	movl $0, %ebx
loop_body_HW1:
	add $1, %ebx
	ror %rax
	jb add_one_HW1
continue_loop_HW1:
	cmp $64, %ebx
	je end_HW1
	jne loop_body_HW1
add_one_HW1:
	add $1, Bool(%rip)
	jmp continue_loop_HW1
end_HW1:
#your code here
