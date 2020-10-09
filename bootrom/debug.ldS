OUTPUT_ARCH(riscv)

#include "driver/mem_map.h"

ENTRY(_start)

SECTIONS
{
    /* we relax the page-alignment requirement except for stacks */
    . = 0x90000000;
    .start : { *(.start) }
    .text       : {
        *(.text .text.*)
        *(.init_array*)
        . = ALIGN(8);
    }
    .rodata     : {
        *(.rodata .rodata.*)
        . = ALIGN(8);
        *(.srodata .srodata.*)
        . = ALIGN(8);
    }
    .data       : {
        PROVIDE(_data_start = .);
        *(.data .data.*)
        . = ALIGN(8);
        *(.sdata .sdata.*)
        . = ALIGN(8);
        PROVIDE(_data_end = .);
    }
    .bss        : {
        PROVIDE(_bss_start = .);
        *(.bss .bss.*)
        . = ALIGN(8);
        *(.sbss .sbss.*)
        . = ALIGN(8);
        PROVIDE(_bss_end = .);
    }

    /* heap */
    . = ALIGN(0x1000);
    PROVIDE(_heap_start = .);

    PROVIDE(_real_data = 0);
}
