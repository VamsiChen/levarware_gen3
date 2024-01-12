/*
 * Copyright (c) 2022 Reliance Foundry Co. Ltd.
 *
 */

#ifndef POWER_REG_H_
#define POWER_REG_H_

// MAX77816 slave address
#define MAX77816_ADDR       (0x18)
#define MAX77816_WRTIE_ADDR ((MAX77816_ADDR << 1) & 0x30)
#define MAX77816_READ_ADDR  (MAX77816_WRTIE_ADDR | 0x01)

// MAX77816 register address
#define MAX77816_REG_CHIP_ID (0x00)
#define MAX77816_CHIP_ID     (0x01)

#define MAX77816_REG_STATUS             (0x01)
#define MAX77816_REG_STATUS_ILIM_MASK   (0xC0)
#define MAX77816_REG_STATUS_OVPTH_MASK  (0x0C)

#define MAX77816_REG_VOUT       (0x04)
#define MAX77816_REG_VOUT_MASK  (0x7F)

#define MAX77816_VOUT_MIN_V         (2.60f)
#define MAX77816_VOUT_MAX_V         (5.14f)
#define MAX77816_VOUT_RANGE_V       (MAX77816_VOUT_MAX_V - MAX77816_VOUT_MIN_V)
#define MAX77816_VOUT_RESOLUTION    (128)

#define MAX77816_REG_CONFIG2            (0x03)
#define MAX77816_REG_CONFIG2_BB_EN_BIT  (6)

#define MAX77816_REG_CONFIG1            (0x02)
#define MAX77816_REG_CONFIG1_ILIM_MASK  (0xC0)
#define MAX77816_ILIM_1_15_A            (0x00)

typedef enum i2c1_bus
{
    I2C1_PINS_SENS = 0,
    I2C1_PINS_UTIL
} i2c1_bus_t;

int power_reg_external_power_on(float vout_v);
void power_reg_external_power_off(void);
int power_reg_get_power_status(struct tele_message *tele_data);

#endif /*POWER_REG_H_*/