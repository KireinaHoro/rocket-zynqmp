#pragma once

#include "bits.h"

void write_gpio_reg(uint32_t reg);
void write_gpio_reg_t(uint32_t reg);
uint32_t read_gpio_reg();

void write_gpio_field(uint32_t val, int offset, int len, int is_t);

void set_bit(uint32_t id, uint8_t val);
uint8_t get_bit(uint32_t id);
