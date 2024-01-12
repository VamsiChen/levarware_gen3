/*
 * Copyright (c) 2022 Reliance Foundry Co. Ltd.
 *
 */

#include <zephyr/zephyr.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

/*Logging*/
#include <zephyr/logging/log.h>

#include "power_reg.h"

LOG_MODULE_REGISTER(power_reg);

#define GPIO DT_LABEL(DT_NODELABEL(gpio0))
const struct device *gpio = NULL;

#define EXT_REG_EN_NODE DT_ALIAS(ext_reg_en)

#if DT_NODE_HAS_PROP(EXT_REG_EN_NODE, gpios)
#define EXT_REG_EN_PIN DT_GPIO_PIN(EXT_REG_EN_NODE, gpios)
#define EXT_REG_EN_FLAGS (GPIO_OUTPUT_LOW | DT_GPIO_FLAGS(EXT_REG_EN_NODE, gpios))
#else
/* A build error here means your board isn't set up to drive an LED. */
#error "Unsupported board: ext-reg-en devicetree alias is not defined"
#endif

#define EXT_REG_STATE_NODE DT_ALIAS(ext_reg_status)

#if DT_NODE_HAS_PROP(EXT_REG_STATE_NODE, gpios)
#define EXT_REG_STATE_PIN DT_GPIO_PIN(EXT_REG_STATE_NODE, gpios)
#define EXT_REG_STATE_FLAGS (GPIO_INPUT | DT_GPIO_FLAGS(EXT_REG_STATE_NODE, gpios))
#else
/* A build error here means your board isn't set up to drive an LED. */
#error "Unsupported board: ext-reg-state devicetree alias is not defined"
#endif

#define I2C_NODE 			DT_NODELABEL(i2c1)
#define I2C 				DT_LABEL(I2C_NODE)
#define I2C_SENSOR_SDA_PIN 	DT_PROP(I2C_NODE, sda_pin)
#define I2C_SENSOR_SCL_PIN 	DT_PROP(I2C_NODE, scl_pin)
const struct device *i2c = NULL;

#define I2C_POWER_MGR_SDA_PIN (20)
#define I2C_POWER_MGR_SCL_PIN (19)

bool is_external_reg_init = false;
bool is_charger_init = false;

int power_reg_charger_init(uint16_t input_current_limit_ma);

static i2c1_bus_t current_i2c_bus = I2C1_PINS_SENS;

const struct pinctrl_dev_config *pcfg;


/**
 * @brief Configures the I2C1 peripheral to use on of the two available busses (SENS or UTIL). 
 *        The power regulator is on the UTIL bus, but startup default is the SENS bus
 * 
 * @param i2c_bus_select  I2C bus to use
 * 
 * @return Does not return any
 */
static void power_reg_set_i2c_bus(i2c1_bus_t i2c_bus_select)
{
	if (current_i2c_bus == i2c_bus_select) 
	{
		// nothing to do
		return;
	}

	uint8_t new_sda_pin = 0;
	uint8_t new_scl_pin = 0;

	switch(i2c_bus_select)
	{
		case I2C1_PINS_SENS: 	
			new_sda_pin = I2C_SENSOR_SDA_PIN;
			new_scl_pin = I2C_SENSOR_SCL_PIN;
			current_i2c_bus = I2C1_PINS_SENS;
			break;

		case I2C1_PINS_UTIL: 	
			new_sda_pin = I2C_POWER_MGR_SDA_PIN;
			new_scl_pin = I2C_POWER_MGR_SCL_PIN;
			current_i2c_bus = I2C1_PINS_UTIL;
			break;
			
		default:	
			// NOTHING TO DO - ERROR
			LOG_ERR("Invalid I2C1 Bus: %i", i2c_bus_select);	
			return;
	};

	// LOG_DBG("Remapping the I2C1 pins -> SDA: %d, SCL: %d", new_sda_pin, new_scl_pin);
	//disable the i2c
	NRF_TWIM1->ENABLE = 0x00;

	k_msleep(10);

	//change the pins
	NRF_TWIM1->PSEL.SDA = (1 << 31);
	NRF_TWIM1->PSEL.SCL = (1 << 31);
	NRF_TWIM1->PSEL.SDA = new_sda_pin;
	NRF_TWIM1->PSEL.SCL =  new_scl_pin;

	//re-enable the i2c
	NRF_TWIM1->ENABLE = 0x06;

	k_msleep(10);
}


/**
 * @brief Turn on/off the external regulator by controlling the enable pin
 * 
 * @param state The desired state of the regulator, TRUE -> ENABLED, FALSE -> DISABLED
 * 
 * @return DOes not return any
 */
static void power_reg_external_set_en_pin(bool state)
{
	gpio = device_get_binding(GPIO);

	int pin_val = state ? 1:0;

	gpio_pin_set(gpio, EXT_REG_EN_PIN, pin_val);

	if(state == false)
	{
		is_external_reg_init = false;
	}
}

/**
 * @brief Initializes the on-board regulators
 * 
 * @param voout_t target voltage for the external regulator
 * 
 * @return does not return any 
 */
static int power_reg_init(void)
{
	i2c = device_get_binding(I2C);

	// External Regulator's GPIO
	gpio = device_get_binding(GPIO);
	if (gpio == NULL)
	{
		LOG_ERR("Unable to get the pointer to device struct of GPIO0");
		return -EIO;
	}

	int err = gpio_pin_configure(gpio, EXT_REG_EN_PIN, EXT_REG_EN_FLAGS);
	if (err)
	{
		LOG_ERR("Unable to configure external regulator's enable pin: err = %i", err);
		return err;
	}

	// TODO: add pin interrupt to monitor and action POK state changes
    err = gpio_pin_configure(gpio, EXT_REG_STATE_PIN, EXT_REG_STATE_FLAGS);
	if (err)
	{
		LOG_ERR("Unable to configure external regulator's status pin: err = %i", err);
		return err;
	}

	is_external_reg_init = true;

	return 0;
}

/**
 * @brief Initializes the external regulator
 * 
 * @param vout_v target output voltage (2.60V to 5.14V)
 * 
 * @return does not return any
 */ 
int power_reg_external_power_on(float vout_v)
{
	int err = power_reg_init();
	if (err != 0)
	{
		LOG_ERR("Unable to initialize external power regulator -> err = %i", err);
		return err;
	}
