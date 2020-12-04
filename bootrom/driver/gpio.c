#include "gpio.h"
#include "mem_map.h"

volatile uint32_t *gpio_base_ptr = (uint32_t *)(GPIO_BASE);

void write_gpio_reg(uint32_t reg) {
    *gpio_base_ptr = reg;
}
