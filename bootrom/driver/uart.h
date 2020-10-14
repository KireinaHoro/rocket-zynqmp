// See LICENSE for license details.

#ifndef UART_HEADER_H
#define UART_HEADER_H

#include "mem_map.h"
#include <stdint.h>

#define UART_RX 0x0u
#define UART_TX 0x1u
#define UART_LSR 0x2u
#define UART_LCR 0x3u

// UART APIs
extern void uart_init();
extern void uart_send(uint8_t);
extern uint8_t uart_recv();

#endif
