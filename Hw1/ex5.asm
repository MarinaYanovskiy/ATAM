.global _start

.section .text
_start:
mov root, %r8
mov new_node, %rbx
cmp $0, %r8
je .addNode_HW1

.findPlaceForNode_HW1:
mov (%r8), %rax
cmp %rax, %rbx
je .finish_HW1
cmp %rax, %rbx
jb .goLeft_HW1
jmp .goRight_HW1

.goLeft_HW1:
cmp $0, 8(%r8)
je .addLeftNode_HW1
mov 8(%r8), %r8
jmp .findPlaceForNode_HW1

.goRight_HW1:
cmp $0, 16(%r8)
je .addRightNode_HW1
mov 16(%r8), %r8
jmp .findPlaceForNode_HW1

.addRightNode_HW1:
movq $new_node, 16(%r8)
jmp .finish_HW1

.addLeftNode_HW1:
movq $new_node, 8(%r8)
jmp .finish_HW1

.addNode_HW1:
movq $new_node, root
.finish_HW1:
