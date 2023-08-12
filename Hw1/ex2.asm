.global _start

.section .text
_start:
	                  #initialize
	movl num(%rip), %eax
	leaq (source), %rsi 
	leaq (destination), %rdi
	
	testl %eax, %eax
	jz end_HW1
	js negative_num_state_HW1
	jmp positive_num_state_HW1
	
positive_num_state_HW1:
	cmpq %rdi, %rsi
	ja source_after_HW1
	je end_HW1         #in case source and destination are equal-we end!
	
	                   #in case => source < destination
	dec %eax					
	movq %rsi, %r10   
	addq %rax, %r10    #r10 = source + num
	movq %rdi, %r11
	addq %rax, %r11    #r11 = destination + num
	inc %eax
	jmp destination_after_HW1

source_after_HW1:
	movb (%rsi), %r8b
	movb %r8b, (%rdi)
	inc %rdi
	inc %rsi
	dec %eax
	cmpl $0, %eax
	je end_HW1
	jne source_after_HW1
	
destination_after_HW1:
	movb (%r10), %r8b
	movb %r8b, (%r11)
	dec %r10
	dec %r11
	dec %eax
	cmpl $0, %eax
	je end_HW1
	jne destination_after_HW1

negative_num_state_HW1:
	movl %eax, (%rdi)

end_HW1:

#your code here
