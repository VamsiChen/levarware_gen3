/**
 * @brief: 	main.c - File containing Main entry point (startup after boot) and associated functions
 *
 * @notes: 	Runs in the context of OS startup thread
 * 
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

// System/Nordic include files
#include <zephyr/zephyr.h>
#include <zephyr/logging/log.h>
#include <modem/modem_info.h>

// Application include files
#include "config/config.h"				// configuration datastore
#include "encoding/aws_encoding.h"		// aws_encoding
#include "cell/lte_connect_mgr.h"		// LTE connection manager
#include "connectors/aws_connector.h"	// AWS connector 
#include "bsp/led.h"

LOG_MODULE_REGISTER(main); // set the logging package name

/** 
* @brief	main - main application function called from OS at start-up 
*
* @param	none 
*		
* @return	never returns
*
* @note		Aborts if function fails to initialize or start properly
*    
*/
void main(void)
{

	LOG_WRN("Levaware Gen3 started, version: %s", CONFIG_FIRMWARE_VERSION);

	/* Skip the sensor init 
	onboard_sensor_init();
    */

	// Init the LED package and turn them all off
	led_init();
	led_set_state(BLUE_LED, LED_OFF);
	led_set_state(GREEN_LED, LED_OFF);
	led_set_state(RED_LED, LED_OFF);


	/* init Nordics modem info system as that is required to be functional by config_init
	* If you fail to do this before config_init() then you will fail to get information from the modem (example: the IMEI) 
	*/
	modem_info_init();

	// init the config datastore as the datastore is required by the rest of the system
	config_init();

	// Initialize the Encode/Decode package
	encoding_init();

	/*
	* Initialize and start the LTE modem.
	* note: currently this function blocks until the system starts up. This is procedure that Nordic tends to 
	* 		use and demos but we could change this. The system needs to handle LTE connectivity going up and down anyway 
	*		so just handle in a generalized way
	*/ 
	lte_connect_init();
	LOG_DBG("LTE system started up");

	//turn uarts off
	
	/* 
	* Init and start-up the AWS IoT connector package
	*/
	aws_connector_init();

	/* 
	* One day, start up the application threads here
	*/
	
	/*
	* All done, just add some hack code to wake up and periodically put out a keep alive string
	*/
	while (1)
	{
		k_msleep(60 * 1000);					// sleep for 60 seconds
		
		// get the current time so we can output it
		// err = date_time_now(&current_time);
		LOG_DBG("Main loop pulse");

	}

}
