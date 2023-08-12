.global _start

.section .text
_start:
xor %r8, %r8      #r8 = i = 0
xor %r9, %r9      #r9 = j = 0
xor %r10, %r10  #r10 = k = 0

ifNotTheEndOfArrays_HW1:
mov array1(%r8), %eax
mov array2(%r9), %ebx 
test %eax, %eax                        #if array1[i]==0
je fromArray2toMerged_HW1
test %ebx, %ebx                        #if array2[j]==0
je fromArray1toMerged_HW1

ifThisValueEqualToNextValue1_HW1:
mov %r8, %r11
add $4, %r11    
cmp %eax, array1(%r11)          #if array1[i]==array1[i+1]
je EqualPlaceInArray1_HW1                    

ifThisValueEqualToNextValue2_HW1:
mov %r9, %r12
add $4, %r12
cmp %ebx,  array2(%r12)            #if array2[j]==array1[j+1]
je EqualPlaceInArray2_HW1

cmp %eax, %ebx                                              #if array1[i]==array2[j]
je equalValueInArrays_HW1
cmp %eax, %ebx                                              #if array1[i]>array2[j]
jb valueInArray1BiggerThanInArray2_HW1
jmp valueInArray2BiggerThanInArray1_HW1              #if array1[i]<array2[j]

EqualPlaceInArray1_HW1:
add $4, %r8
mov  array1(%r8), %eax
jmp ifThisValueEqualToNextValue1_HW1

EqualPlaceInArray2_HW1:
add $4, %r9
mov  array2(%r9), %ebx
jmp ifThisValueEqualToNextValue2_HW1

##############for value comparisons:############
equalValueInArrays_HW1:
mov %eax, mergedArray(%r10)
add $4, %r8
add $4, %r9
add $4, %r10
jmp ifNotTheEndOfArrays_HW1

valueInArray1BiggerThanInArray2_HW1:
mov %eax, mergedArray(%r10)
add $4, %r8
add $4, %r10
jmp ifNotTheEndOfArrays_HW1

valueInArray2BiggerThanInArray1_HW1:
mov %ebx, mergedArray(%r10)
add $4, %r9
add $4, %r10
jmp ifNotTheEndOfArrays_HW1

##############if any array is not zero yet:############
fromArray2toMerged_HW1:
test %ebx, %ebx                        #if array2[j]==0
je end_HW1
mov %r9, %r12
add $4, %r12
cmp %ebx,  array2(%r12)            #if array2[j]==array1[j+1]
je increaseIndex2_HW1
mov %ebx, mergedArray(%r10)
add $4, %r10
increaseIndex2_HW1:
add $4, %r9
mov array2(%r9), %ebx 
jmp fromArray2toMerged_HW1

fromArray1toMerged_HW1:
test %eax, %eax                        #if array1[i]==0
je end_HW1
mov %r8, %r11
add $4, %r11    
cmp %eax, array1(%r11)          #if array1[i]==array1[i+1]
je increaseIndex1_HW1
mov %eax, mergedArray(%r10)
add $4, %r10
increaseIndex1_HW1:
add $4, %r8
mov array1(%r8), %eax
jmp fromArray1toMerged_HW1

end_HW1:
movl $0, mergedArray(%r10)
