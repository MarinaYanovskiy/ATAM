ENTRY(_hw3_unicorn)

PHDRS
{
	segmentA PT_LOAD FLAGS(3);  /*segment for .text*/
	segmentB PT_LOAD FLAGS(6);  /*segment for .data and .bss*/
	segmentC PT_LOAD FLAGS(5);  /*segment for .rodata*/

}

SECTIONS
{
  .text 0x400000:
  {
	*(.text)
  }  :segmentA
  
  
  .data 0x60000: 
  {
	*(.data)
	*(.bss)
  } :segmentB
  
  
  .rodata 0x80000000:
  {
	*(.rodata)
  } :segmentC
  
}

  EXTERN ( purple );
  EXTERN ( white );
