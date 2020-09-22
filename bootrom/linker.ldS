OUTPUT_ARCH(riscv)

#include "driver/mem_map.h"

SECTIONS
{
    /* we relax the page-alignment requirement except for stacks */
    . = XIP_BASE;
    .start : { *(.start) }
    .text       : {
        *(.text .text.*)
        . = ALIGN(8);
    }
    .rodata     : {
        *(.rodata .rodata.*)
        *(.srodata .srodata.*)
        . = ALIGN(8);
    }

    /* the following sections will be relocated to DDR */
    . = DDR_BASE;
    PROVIDE(_real_data = ADDR(.rodata) + SIZEOF(.rodata));
    .data       : AT(_real_data) {
        PROVIDE(_data_start = .);
        *(.data .data.*)
        *(.readmostly.data)
        *(*.data)
        . = ALIGN(8);
        PROVIDE(_data_end = .);
    }
    .bss        : {
        PROVIDE(_bss_start = .);
        *(.bss .bss.*)
        . = ALIGN(8);
    }
    .sbss       : {
        *(.sbss .sbss.*)
        . = ALIGN(8);
        PROVIDE(_sbss_end = .);
    }

    /* heap */
    . = ALIGN(0x1000);
    PROVIDE(_heap_start = .);
    
    /* stack */
    . = DDR_BASE + DDR_SIZE - 0x10000;
    PROVIDE(_stack_end = .);
    . = DDR_BASE + DDR_SIZE;
    PROVIDE(_stack_top = .);
}
