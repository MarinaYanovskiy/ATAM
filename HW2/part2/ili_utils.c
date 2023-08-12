#include <asm/desc.h>

void my_store_idt(struct desc_ptr *idtr) {
// <STUDENT FILL> - HINT: USE INLINE ASSEMBLY
    asm("sidt %0;"
        :"=m"(*idtr)
        :
        :
        );
// </STUDENT FILL>
}

void my_load_idt(struct desc_ptr *idtr) {
// <STUDENT FILL> - HINT: USE INLINE ASSEMBLY
    asm("lidt %0;"
            :
            :"m"(*idtr)
            :
            );
// <STUDENT FILL>
}

void my_set_gate_offset(gate_desc *gate, unsigned long addr) {
// <STUDENT FILL> - HINT: NO NEED FOR INLINE ASSEMBLY
    gate->offset_low = addr;
    gate->offset_middle = addr>>16;
    gate->offset_high = addr>>32;

// </STUDENT FILL>
}

unsigned long my_get_gate_offset(gate_desc *gate) {
// <STUDENT FILL> - HINT: NO NEED FOR INLINE ASSEMBLY
    unsigned long result = gate->offset_high;
    result = result << 16;
    result += gate->offset_middle;
    result = result << 16;
    return result + gate->offset_low;

// </STUDENT FILL>
}
