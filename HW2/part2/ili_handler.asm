.globl my_ili_handler

.text
.align 4, 0x90
my_ili_handler:
	pushq   %rsi		/* pt_regs->si */
	pushq	%rdx		/* pt_regs->dx */
	pushq   %rcx		/* pt_regs->cx */
	pushq   %rax		/* pt_regs->ax */
	pushq   %r8		/* pt_regs->r8 */
	pushq   %r9		/* pt_regs->r9 */
	pushq   %r10		/* pt_regs->r10 */
	pushq   %r11		/* pt_regs->r11 */
	pushq	%rbx		/* pt_regs->rbx */
	pushq	%rbp		/* pt_regs->rbp */
	pushq	%r12		/* pt_regs->r12 */
	pushq	%r13		/* pt_regs->r13 */
	pushq	%r14		/* pt_regs->r14 */
	pushq	%r15

#first step- get the opcode
    movq 112(%rsp), %rax
   

#second step- pass the opcode to WHAT TO DO
    xor %rdi,%rdi
    xor %rbx, %rbx
   cmpb $0x0F,(%rax) 
    je passLowByte

   
   mov $1,%rbx
	movb (%rax),%dil
    jmp callFunction

passLowByte:
   
    mov $2, %rbx
	movb 1(%rax),%dil
    #mov $4, %rdi

callFunction:
    pushq %rbx
    call what_to_do
	popq %rbx

#third step- do things based on return value
    cmp $0, %rax
    je returnFromHandler

    mov %rax, %rdi
    add %rbx, 112(%rsp)

	popq %r15
    popq %r14
   	popq %r13
   	popq %r12
   	popq %rbp
   	popq %rbx
    popq %r11
   	popq %r10
   	popq %r9
   	popq %r8
   	popq %rax
    popq %rcx
    popq %rdx
   	popq %rsi

  iretq

returnFromHandler:

    popq %r15
    popq %r14
   	popq %r13
   	popq %r12
   	popq %rbp
   	popq %rbx
    popq %r11
   	popq %r10
   	popq %r9
   	popq %r8
   	popq %rax
    popq %rcx
    popq %rdx
   	popq %rsi

	jmp * (old_ili_handler)
	
	iretq
	
