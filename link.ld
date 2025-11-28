OUTPUT_FORMAT(binary)
SECTIONS
{
	. = 0x7E00;
	.text : {
		int386.o(.text)
		*(.main)
		*(.text*)
	}
	.data : {
		*(.bss*)
		*(.data*)
		*(.rodata*)
	}
}
	
