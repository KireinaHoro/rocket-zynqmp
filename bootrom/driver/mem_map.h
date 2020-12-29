// memory regions
#define DDR_BASE 0x80000000
#define DDR_SIZE 0x40000000
#define XIP_BASE 0x71000000 // high 128Mb of configuration flash

#define STACK_SIZE 0x10000
#define STACK_TOP (DDR_BASE + DDR_SIZE)
#define STACK_END (STACK_TOP - STACK_SIZE)

// peripherals
#define SPI_XIP_BASE 0x60000000
#define SPI_PERIPH_BASE 0x60010000
#define UART_BASE 0x60020000
#define GPIO_BASE 0x60030000
#define CMD_BASE 0x60040000 // AXI BRAM
