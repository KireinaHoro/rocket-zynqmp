#ifndef MEMORY_HEADER_H
#define MEMORY_HEADER_H

#include "mem_map.h"
#include <stdint.h>

extern volatile uint64_t *get_bram_base();
extern volatile uint64_t *get_ddr_base();
extern volatile uint64_t *get_flash_base();

#endif