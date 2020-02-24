SECTIONS
{
    . = 0x10000;
    .text.start : { *(.text.start) }
    .text       : { *(.text) }
    .rodata     : { *(.rodata) }
    .sdata      : { *(.sdata) }
}
