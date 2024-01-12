/*
 * @brief: 	led.c - LED control functions to turn LEDs on/off
 *
 * @notes: 	Imported from Levaware Gen1 firmware with some clean-up
 * 
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"

#include  "sys_wrapper.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(led);

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

#define LED0_LABEL DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED1_LABEL DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED2_LABEL DT_GPIO_LABEL(LED2_NODE, gpios)

#define LED0_PIN DT_GPIO_PIN(LED0_NODE, gpios)
#define LED1_PIN DT_GPIO_PIN(LED1_NODE, gpios)
#define LED2_PIN DT_GPIO_PIN(LED2_NODE, gpios)

#define LED0_FLAG DT_GPIO_FLAGS(LED0_NODE, gpios)
#define LED1_FLAG DT_GPIO_FLAGS(LED1_NODE, gpios)
#define LED2_FLAG DT_GPIO_FLAGS(LED2_NODE, gpios)

const struct device *led_dev[LED_NUM];

static uint8_t led_pins[LED_NUM] = {LED0_PIN, LED1_PIN, LED2_PIN};

/** 
* @brief   LED_init - LED initialization and turn off all LEDs
*
* @param    void
*
* @return   Nothing
*
* @note     Asserts/Aborts if initialization fails 
*
*           TODO - add Asserts/Aborts on init failure
*/
void led_init(void)
{
    int err;    // temp error code to track if init process failed

    /* 
     * Run down all the LEDs and initialize the LED device control structures, 
     * check for success and configure to LED OFF
     */


    // first initialize the blue led
    err = -1;       // assume error case in case binding fails. 

    led_dev[BLUE_LED] = device_get_binding(LED0_LABEL);
    if (led_dev[BLUE_LED] != NULL)
        // binding was successful, lets turn the led off
        err = gpio_pin_configure(led_dev[BLUE_LED], LED0_PIN, LED0_FLAG | GPIO_OUTPUT_INACTIVE);
    if (err)
        erabort("ledinit: blue led init failed");

    // then the green led. Comments and logic exactly the same
    err = -1;

    led_dev[GREEN_LED] = device_get_binding(LED1_LABEL);
    if (led_dev[GREEN_LED] != NULL)
        err = gpio_pin_configure(led_dev[GREEN_LED], LED1_PIN, LED1_FLAG | GPIO_OUTPUT_INACTIVE);
    if (err)
        erabort("ledinit: green led init failed");

    // then the red led
    err = -1;

    led_dev[RED_LED] = device_get_binding(LED2_LABEL);
    if (led_dev[RED_LED] != NULL)
        err = gpio_pin_configure(led_dev[RED_LED], LED2_PIN, LED2_FLAG | GPIO_OUTPUT_INACTIVE); 
    if (err)
        erabort("ledinit: red led init failed");
}

/** 
* @brief   led_set_state - Sets the state of an LED (on or off)
*
* @param    led_id - Identifier of the LED to be controlled
* @param    state - on or off
*
* @return   Nothing
*
* @note     Aborts if led control not initialized
*
*/

void led_set_state(led_t led, led_state_t state)
{
    if (gpio_pin_set(led_dev[led], led_pins[led], state) != 0)
        erabort("led_state: failed to set");

}