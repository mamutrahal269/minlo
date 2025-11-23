OUTPUT_FORMAT(binary)
SECTIONS
{
	. = 0x7E00;
	.text : {
		int386.o(.text*)
		test.o(.text*)
		*(.text*)
	}
	.data : {
		*(.bss)
		*(.data)
		*(.rodata)
	}
}
	
