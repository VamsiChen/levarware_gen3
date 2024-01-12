/**
 * @brief: 	aws_decoding.c - JSON decoding of AWS IoT messages
 *
 * @notes:    
 *  
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

// includes for nrf system
#include <zephyr.h>
#include <logging/log.h>
#include <cJSON.h>
#include <cJSON_os.h>

// includes for application
#include "bsp/sys_wrapper.h"
#include "encoding/aws_encoding.h"
#include "config/config.h"

/*
*   Range checking constants to ensure data from aws is within range. Allows us to 'peg' the value if needed
*/
#define SECONDS_PER_MINUTE                   (60)
#define MINUTES_PER_HOUR                     (60)
#define HOURS_PER_DAY                        (24)

#define MINUTE_SECONDS                      SECONDS_PER_MINUTE
#define HOUR_SECONDS                        (SECONDS_PER_MINUTE * MINUTES_PER_HOUR)
#define DAY_SECONDS                         (HOUR_SECONDS * HOURS_PER_DAY)


LOG_MODULE_REGISTER(aws_decoding);      // register the logging package

// #ifdef IGNORE

/** 
* @brief    aws_decode_shadow_values - decode values from shadow json message
*
* @param    attributes pointer to the attributes of json message
* @param    version    version of the shadow message
* @param    is_delta   indicates of is from delta message because then
*                       we need to save values to flash
*
* @return   no value
*
* @note     Parses the data to JSON then retrieves the value for each properties and saves in config system
*/
static void aws_decode_shadow_values(cJSON *attributes, int32_t version, bool is_delta)
{
if (version > 0)
    {
        // use shadow service version number in state object to track shadow version
        config_set_int(DEV_CONFIG_CONF_VERSION, version, true);
        config_save_attribute_to_file(DEV_CONFIG_CONF_VERSION);
        LOG_INF("%s updated to %d", log_strdup(DEV_SHADOW_ATTR_CONF_VERSION), version);
    }

    cJSON *item = cJSON_GetObjectItemCaseSensitive(attributes, DEV_SHADOW_ATTR_CONF_INTERVAL_S);
    if (cJSON_IsNumber(item))
    {
        int val = item->valueint;

        // protect against too short or too long intervals
        if (val < MINUTE_SECONDS) 
        {
            // don't request shadow more than once per minute
            val = MINUTE_SECONDS;
        }
        else if (val > DAY_SECONDS) 
        {
            // request shadow update at lease once per day
            val = DAY_SECONDS;
        }

        config_set_int16(DEV_CONFIG_CONF_UPDATE_INTERVAL_S, val, is_delta);
        config_save_attribute_to_file(DEV_CONFIG_CONF_UPDATE_INTERVAL_S);
        LOG_INF("%s updated to %d", log_strdup(DEV_SHADOW_ATTR_CONF_INTERVAL_S), val);
    }
    
    item = cJSON_GetObjectItemCaseSensitive(attributes, DEV_SHADOW_ATTR_DAQ_INTERVAL_S);
    if (cJSON_IsNumber(item))
    {
        int val = item->valueint;

        // protect against too short or too long intervals
        if (val < DAQ_INTERVAL_MINIMUM_S) 
        {
            // don't daq more than once per minute
            val = DAQ_INTERVAL_MINIMUM_S;
        }
        else if (val > DAY_SECONDS) 
        {
            // run daq at lease once per day
            val = DAY_SECONDS;
        }

        config_set_int16(DEV_CONFIG_DAQ_INTERVAL_S, val, is_delta);
        config_save_attribute_to_file(DEV_CONFIG_DAQ_INTERVAL_S);
        LOG_INF("%s updated to %d", log_strdup(DEV_SHADOW_ATTR_DAQ_INTERVAL_S), val);
    }
    
    item = cJSON_GetObjectItemCaseSensitive(attributes, DEV_SHADOW_ATTR_PUB_INTERVAL_S);
    if (cJSON_IsNumber(item))
    {
        int val = item->valueint;

        // protect against too short or too long intervals
        if (val < PUB_INTERVAL_MINIMUM_S) 
        {
            // don't try to publish more than once per minute
            val = PUB_INTERVAL_MINIMUM_S;
        }
        else if (val > DAY_SECONDS) 
        {
            // publish at lease once per day
            val = DAY_SECONDS;
        }

        config_set_int16(DEV_CONFIG_PUB_INTERVAL_S, val, is_delta);
        config_save_attribute_to_file(DEV_CONFIG_PUB_INTERVAL_S);
        LOG_INF("%s updated to %d", log_strdup(DEV_SHADOW_ATTR_PUB_INTERVAL_S), item->valueint);
    }

    item = cJSON_GetObjectItemCaseSensitive(attributes, DEV_SHADOW_ATTR_SENSOR_TYPE);
    if (cJSON_IsNumber(item) 
        && (enum external_sensor)item->valueint > EXT_SENSOR_UNKNOWN
        && (enum external_sensor)item->valueint < EXT_SENSOR_INVALID)
    {
        config_set_int16(DEV_CONFIG_SENSOR_TYPE, item->valueint, is_delta);
        config_save_attribute_to_file(DEV_CONFIG_SENSOR_TYPE);
        LOG_INF("%s updated to %d", log_strdup(DEV_SHADOW_ATTR_SENSOR_TYPE), item->valueint);
    }
    item = cJSON_GetObjectItemCaseSensitive(attributes, DEV_SHADOW_ATTR_PUB_TOPIC);
    if (cJSON_IsString(item) 
        && strlen(item->valuestring) > 0)
    {
        config_set_str(DEV_CONFIG_PUB_TOPIC, item->valuestring, is_delta);
        config_save_attribute_to_file(DEV_CONFIG_PUB_TOPIC);
        LOG_INF("%s updated to \"%s\"", log_strdup(DEV_SHADOW_ATTR_PUB_TOPIC), log_strdup(item->valuestring));
    }
    item = cJSON_GetObjectItemCaseSensitive(attributes, DEV_SHADOW_ATTR_APP_TYPE);
    if (cJSON_IsNumber(item) 
        && (enum app_id_t)item->valueint > APP_ID_UNKNOWN 
        && (enum app_id_t)item->valueint < APP_ID_NUM)
    {
        config_set_int16(DEV_CONFIG_APP_TYPE, item->valueint, is_delta);
        config_save_attribute_to_file(DEV_CONFIG_APP_TYPE);
        LOG_INF("%s updated to %d", log_strdup(DEV_SHADOW_ATTR_APP_TYPE), item->valueint);
        
        //check if app type has changed and reboot 
        if(config_diff_flash_int16(DEV_CONFIG_APP_TYPE, item->valueint))
        {
            // see issue: #7 here: 
            // https://github.com/Reliance-Foundry/levaware_gen3/issues/7
            LOG_INF("App_type changed to %d,...Needs a Reboot", item->valueint);

            // lets not force a reboot here for now and have the user power off and on. 
        }
    }
}

/** 
* @brief    aws_decode_shadow_msg - decode shadow messages from AWS IoT
*
* @param    char *msg_stringp - pointer to message string
*           size_t  len - length of message
*
* @return   no value
*
* @note     Parses the shadow message and calls helper to save values in the config system
*/
void aws_decode_shadow_msg(char *msg_stringp, size_t len)
{

    LOG_DBG("Parsing shadow message from aws ");
    
    cJSON *root_obj = NULL;
    int32_t version = 0;

	root_obj = cJSON_ParseWithLength(msg_stringp, len);
	if (root_obj == NULL) 
    {
        LOG_ERR("cJSON Parse failure at root object");
		goto clean_exit;
	}

    cJSON *state_obj = cJSON_GetObjectItemCaseSensitive(root_obj, "state");
    if (!cJSON_IsObject(state_obj))
    {
        LOG_ERR("cant get 'state' object");
		goto clean_exit;
    }
    
    cJSON *item = cJSON_GetObjectItemCaseSensitive(root_obj, "version");
    if (cJSON_IsNumber(item))
    {
        version = item->valueint;
    }
    
    // if we are processing a delta update sub event, the delta state
    //  attributes are in the state object
    cJSON *attributes = cJSON_GetObjectItemCaseSensitive(state_obj, "attributes");
    if (!cJSON_IsObject(attributes))
    {
        // if we are processing a shadow_get response, the delta state attributes are in the state.delta object
        LOG_DBG("state.attributes not found, trying state.delta.attributes...");

        cJSON *delta_obj = cJSON_GetObjectItemCaseSensitive(state_obj, "delta");
        if (!cJSON_IsObject(delta_obj))
        {
            LOG_WRN("state.delta not found -> discarding");
            goto clean_exit;
        }

        /* So if not delta object, we exited above. If we fell thru to here, then it's a delta object.
        * This is very important to note as we then save the values in the delta state because they are out of 
        * sync with our internally stored data.
        */

        attributes = cJSON_GetObjectItemCaseSensitive(delta_obj, "attributes");
        if (!cJSON_IsObject(attributes))
        {
            LOG_ERR("state.delta.attributes not found -> discarding");
            goto clean_exit;
        }
    }

   LOG_WRN("Saving new delta.attributes: %s", cJSON_PrintUnformatted(attributes));
   
     aws_decode_shadow_values(attributes, version, true);

    // DON'T PROCESS OTHER STATES - if an update is required for a device, it must be in the delta object
    // if the desired state is always processed and we write everything to flash & it will wear out

clean_exit:
	cJSON_Delete(root_obj);
    return;
}

/**  
* @brief    encoding_init - Initialize the AWS encoding/decoding package
*
* @note     aborts if fails to initialize
*
* @param    void
*
* @return   nothing
*/
void encoding_init(void)
{
    // This package uses JSON encoding
    cJSON_Init();
}