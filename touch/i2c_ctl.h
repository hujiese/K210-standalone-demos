#ifndef _I2C_CTL_H
#define _I2C_CTL_H

#include <stdint.h>

#define ADDRESS_WIDTH   7
#define I2C_CLK_SPEED   100000

void i2c_hardware_init(uint16_t addr);
uint16_t i2c_hd_write(uint8_t addr, uint8_t reg, uint8_t data);
uint16_t i2c_hd_read(uint8_t addr, uint8_t reg, uint8_t *data_buf, uint16_t length);

void i2c_read_addr(void);
#endif  /* _I2C_CTL_H */
