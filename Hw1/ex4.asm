.global _start

.section .text
_start:
	lea (head), %rcx
	lea (head), %rdi
	lea (head), %r14
	lea (head), %r13
	movq Source, %rsi
	movl Value(%rip), %r9d
	
	testq %rdi, %rdi
	je end_HW1
	cmpq %rdi, %rsi
	je end_HW1
	movq %rdi, %r15
	movq (%rdi), %rdi

loop_find_node_HW1:
	testq %rdi, %rdi
	je end_HW1
	movl (%rdi), %eax
	cmpl %r9d, %eax
	je find_source_HW1
	movq %rdi, %r15
	movq 4(%rdi), %rdi
	jmp loop_find_node_HW1
	
find_source_HW1:
	movq (%r14), %r14

loop_find_source_HW1:
	testq %r14, %r14
	je end_HW1
	cmpq %r14, %rsi
	je found_source_and_node_HW1
	movq %r14, %r13
	movq 4(%r14), %r14
	movl (%r14), %eax       #for me
	jmp loop_find_source_HW1
	
found_source_and_node_HW1:
	cmpq %rdi, %rsi
	je end_HW1
	
	cmpq %r15, %rsi
	je source_before_node_HW1
	
	cmpq %r13, %rdi
	je node_before_source_HW1
	
	movq 4(%rsi), %rax
	movq 4(%rdi), %rbx
	movq %rbx, 4(%rsi)
	movq %rax, 4(%rdi)
	cmpq %rcx, %r15
	je node_head_HW1
	movq %rsi, 4(%r15)
	cmpq %rcx, %r13
	je source_head_before_node_HW1
	movq %rdi, 4(%r13)
	jmp end_HW1
	
source_before_node_HW1:
	movq 4(%rdi), %rbx
	movq %rbx, 4(%rsi)
	movq %rsi, 4(%rdi)
	cmpq %rcx, %r13
	je source_head_before_node_HW1
	movq %rdi, 4(%r13)
	jmp end_HW1
	
node_before_source_HW1:
	movq 4(%rsi), %rbx
	movq %rbx, 4(%rdi)
	movq %rdi, 4(%rsi)
	cmpq %rcx, %r15
	je node_head_before_source_HW1
	movq %rsi, 4(%r15)
	jmp end_HW1

source_head_before_node_HW1:
	movq %rdi, (%r13)
	jmp end_HW1
node_head_before_source_HW1:
	movq %rsi, (%r15)
	jmp end_HW1
node_head_HW1:
	movq %rsi, (%r15)
	movq %rdi, 4(%r13)
	jmp end_HW1

end_HW1:
	

#your code here
