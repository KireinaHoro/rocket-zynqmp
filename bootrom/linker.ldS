OUTPUT_ARCH(riscv)

#include "driver/mem_map.h"

SECTIONS
{
    /* we relax the page-alignment requirement except for stacks */
    . = XIP_BASE;
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

    /* the following sections will be relocated to DDR */
    . = DDR_BASE;
    PROVIDE(_real_data = ADDR(.rodata) + SIZEOF(.rodata));
    .data       : AT(_real_data) {
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
}
