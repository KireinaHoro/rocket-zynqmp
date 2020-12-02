#include "gpio.h"
#include "mem_map.h"

volatile uint32_t *gpio_base_ptr = (uint32_t *)(GPIO_BASE);

static uint32_t o_old_val;
static uint32_t t_old_val;

void write_gpio_reg(uint32_t reg) {
    *gpio_base_ptr = reg;
    o_old_val = reg;
    //printf("o_old_val = %#x\n", o_old_val);
}

void write_gpio_reg_t(uint32_t reg) {
    *(gpio_base_ptr + 1) = reg;
    t_old_val = reg;
    //printf("t_old_val = %#x\n", t_old_val);
}

uint32_t read_gpio_reg() {
    return *gpio_base_ptr;
}

void set_bit(uint32_t id, uint8_t val) {
    //printf("set_bit id=%d val=%d\n", id, val);
    if (id & 0x80000000) {
        id = id & ~0x80000000;
        //printf("set t reg\n");
        // set t reg
        write_gpio_reg_t((t_old_val & ~(1 << id)) | (val << id));
    } else {
        //printf("set o reg\n");
        // set o reg
        write_gpio_reg((o_old_val & ~(1 << id)) | (val << id));
    }
}

uint8_t get_bit(uint32_t id) {
    uint8_t ret = !!(read_gpio_reg() & (1 << id));
    //printf("get_bit id=%d val=%d\n", id, ret);
    return ret;
}
