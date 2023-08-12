.global	keyMixing, byteSubstitution, shiftRows, cipher

.section	.data
sbox:	.byte	99,124,119,123,-14,107,111,-59,48,1,103,43,-2,-41,-85,118
		.byte	-54,-126,-55,125,-6,89,71,-16,-83,-44,-94,-81,-100,-92,114,-64
		.byte	-73,-3,-109,38,54,63,-9,-52,52,-91,-27,-15,113,-40,49,21
		.byte	4,-57,35,-61,24,-106,5,-102,7,18,-128,-30,-21,39,-78,117
		.byte	9,-125,44,26,27,110,90,-96,82,59,-42,-77,41,-29,47,-124
		.byte	83,-47,0,-19,32,-4,-79,91,106,-53,-66,57,74,76,88,-49
		.byte	-48,-17,-86,-5,67,77,51,-123,69,-7,2,127,80,60,-97,-88
		.byte	81,-93,64,-113,-110,-99,56,-11,-68,-74,-38,33,16,-1,-13,-46
		.byte	-51,12,19,-20,95,-105,68,23,-60,-89,126,61,100,93,25,115
		.byte	96,-127,79,-36,34,42,-112,-120,70,-18,-72,20,-34,94,11,-37
		.byte	-32,50,58,10,73,6,36,92,-62,-45,-84,98,-111,-107,-28,121
		.byte	-25,-56,55,109,-115,-43,78,-87,108,86,-12,-22,101,122,-82,8
		.byte	-70,120,37,46,28,-90,-76,-58,-24,-35,116,31,75,-67,-117,-118
		.byte	112,62,-75,102,72,3,-10,14,97,53,87,-71,-122,-63,29,-98
		.byte	-31,-8,-104,17,105,-39,-114,-108,-101,30,-121,-23,-50,85,40,-33
		.byte	-116,-95,-119,13,-65,-26,66,104,65,-103,45,15,-80,84,-69,22

.section	.text
keyMixing:
	#prolog
	pushq %rbp
	movq %rsp, %rbp

	#body
	movq (%rsi), %r10
	xorq %r10, (%rdi)

	movq 8(%rsi), %r10
	xorq %r10, 8(%rdi)
	
	#epilog
	movq %rbp, %rsp
	popq %rbp
	ret


byteSubstitution:
	#prolog
	pushq %rbp
	movq %rsp, %rbp

	#body
	movq (%rdi), %r8
	movl $8, %ecx
	leaq sbox(%rip), %r11
	
loop_BS_1_HW2:
	movq $0, %r10
	movb %r8b, %r10b
	addq %r11, %r10
	movb (%r10), %r9b
	movb %r9b, %r8b
	
	ror $8, %r8
	
	sub $1, %ecx
	cmpl $0, %ecx
	jne loop_BS_1_HW2
	
	movq %r8, (%rdi)
	movq 8(%rdi), %r8
	movl $8, %ecx
	
loop_BS_2_HW2:
	movq $0, %r10
	movb %r8b, %r10b
	movb (%r11,%r10), %r9b
	movb %r9b, %r8b
	
	ror $8, %r8

	sub $1, %ecx
	cmpl $0, %ecx
	jne loop_BS_2_HW2

	#epilog
	movq %r8, 8(%rdi)
	movq %rbp, %rsp
	popq %rbp
	ret


shiftRows:
	#prolog
	pushq %rbp
	movq %rsp, %rbp

	#body
	movq (%rdi), %r8
	rorq $32, %r8
	rorl $8, %r8d
	
	movl %r8d, 4(%rdi)
	movq 8(%rdi), %r8

	rorl $16, %r8d
	movl %r8d, 8(%rdi)
	
	movl 12(%rdi), %r8d
	rorl $24, %r8d
	movl %r8d, 12(%rdi)
	
	#epilog
	#movq %r8, 8(%rdi)
	movq %rbp, %rsp
	popq %rbp
	ret
	

cipher:
	#prolog
	pushq %rbp
	movq %rsp, %rbp
	push %r8
	push %r9
	push %r10
	push %r11
	push %rcx
	push %rdi
	push %rsi
	push %rdx
	push %r12
	push %r13
	
	#body
	
	movq %rdi, %r12
	movq %rsi, %r9
	movb %dl, %r13b

loop_per_block_HW2:
	movl $9, %eax   #regular local parameter
	call keyMixing
loop_inside_HW2:
	movq %r12, %rdi
	call byteSubstitution
	movq %r12, %rdi
	call shiftRows
	movq %r8, %rdi
	movq %r12, %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %rcx
	push %rdi
	push %rsi
	push %rdx
	push %rax
	call mixColumns
	pop %rax
	pop %rdx
	pop %rsi
	pop %rdi
	pop %rcx
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	movq %r12, %rdi
	call keyMixing
	movq %r12, %rdi	
	
	
	sub $1, %eax
	
	cmpl $0, %eax
	jne loop_inside_HW2
	
	call byteSubstitution
	movq %r12, %rdi
	call shiftRows
	movq %r12, %rdi
	call keyMixing
	movq %r12, %rdi

	addq $16, %rdi 
	movq %rdi, %r12
	sub $1, %r13b
	cmpb $0, %r13b
	jne loop_per_block_HW2

	#epilog
	pop %r13
	pop %r12
	pop %rdx
	pop %rdi
	pop %rcx
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	
	movq %rbp, %rsp
	popq %rbp
	ret
