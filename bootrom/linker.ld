SECTIONS
{
    . = 0x20000;
    .text.start : { *(.text.start) }
    .text       : { *(.text) }
    .rodata     : { *(.rodata) }
    .sdata      : { *(.sdata) }

	. = 0x21000;
	.sbi		: { *(.sbi) }
    .rodata.dtb : { *(.rodata.dtb) } /* dtb after sbi elf */
}
