/**
 * @brief: 	config.c - Data configuration datastore manager
 *
 * @notes:  Taken from levaware V2 code base and then cleaned up. Ripped out JSON functionality to seperate 
 *          module. Still needs more work in the area of handling new configuration values. 
 *  
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

// includes for nrf system
#include <zephyr.h>
#include <logging/log.h>
#include <string.h>
#include <stdio.h>
#include <modem/modem_info.h>
#include <logging/log.h>

// includes for Application
#include "config.h"
#include "config_internal.h"
#include "bsp/modem.h"
#include "bsp/sys_wrapper.h"

LOG_MODULE_REGISTER(config);        // register with logging package

/*
*   Storage for structure holding the configuration datastore
*/
static struct device_config_t device_config;

/*
*   This device configuration design needs to be rethought. In the current design, each one of the device attibutes is in it's own file. Those files are created 
*   and initialized during the provisioning process. The problem is that when new attributes are defined, those files do not exist. I am not sure what the thinking 
*   was ... to create the attibutre files later when missing? Why not just initialize all of them that are not present at boot-up. Default them in the application
*   when not present. That would be much cleaner than the current solution. Added to the list of things to refactor. This would simplfy the device provisioning process
*
*   see issue: https://github.com/Reliance-Foundry/levaware_gen3/issues/12
*/


/** 
* @brief   config_get_fw_version_str - Get firmware version string
* 
* @param    none
*
* @return   pointer to version number string in flash (NULL terminated)
*
* @note    
*/
char *config_get_fw_version_str(void)
{
    return CONFIG_FIRMWARE_VERSION;
}

/**
 * @brief config_dev_shadow_attrib_init - Initializes the structure containing the device shadow attribute information
 * 
 * @param - none
 * 
 * @return  none
 */
void config_dev_shadow_attrib_init(void)
{

int num_bytes;      // number of bytes read from file
enum dev_config_shadow_id_t idx;        // index for walking thru all configuration values

    // load defaults for each attribute from it's defined initial value

    // first the DAQ interval
    device_config.dev_shadow_attrib[DEV_CONFIG_DAQ_INTERVAL_S].dev_conf_shadow_id = DEV_CONFIG_DAQ_INTERVAL_S;
    device_config.dev_shadow_attrib[DEV_CONFIG_DAQ_INTERVAL_S].val_type = DEV_CONFIG_VAL_TYPE_INT16;
    device_config.dev_shadow_attrib[DEV_CONFIG_DAQ_INTERVAL_S].int16_val = DAQ_INTERVAL_DEFAULT_VAL_S;
    device_config.dev_shadow_attrib[DEV_CONFIG_DAQ_INTERVAL_S].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_DAQ_INTERVAL_S].filename, DAQ_INTERVAL_FILE_NAME);
    
    // publish interval
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_INTERVAL_S].dev_conf_shadow_id = DEV_CONFIG_PUB_INTERVAL_S;
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_INTERVAL_S].val_type = DEV_CONFIG_VAL_TYPE_INT16;
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_INTERVAL_S].int16_val = PUB_INTERVAL_DEFAULT_VAL_S;
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_INTERVAL_S].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_PUB_INTERVAL_S].filename, PUB_INTERVAL_FILE_NAME);
    
    // iccid value
    device_config.dev_shadow_attrib[DEV_CONFIG_ICCID].dev_conf_shadow_id = DEV_CONFIG_ICCID;
    device_config.dev_shadow_attrib[DEV_CONFIG_ICCID].val_type = DEV_CONFIG_VAL_TYPE_STRING;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_ICCID].str_val, ICCID_DEFAULT_VAL);
    device_config.dev_shadow_attrib[DEV_CONFIG_ICCID].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_ICCID].filename, ICCID_INTERNAL_FILE_NAME);

    // config interval
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_UPDATE_INTERVAL_S].dev_conf_shadow_id = DEV_CONFIG_CONF_UPDATE_INTERVAL_S;
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_UPDATE_INTERVAL_S].val_type = DEV_CONFIG_VAL_TYPE_INT16;
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_UPDATE_INTERVAL_S].int16_val = CONFIG_INTERVAL_DEFAULT_VAL_S;
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_UPDATE_INTERVAL_S].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_CONF_UPDATE_INTERVAL_S].filename, CONFIG_INTERVAL_FILE_NAME);
    
    // sensor type
    device_config.dev_shadow_attrib[DEV_CONFIG_SENSOR_TYPE].dev_conf_shadow_id = DEV_CONFIG_SENSOR_TYPE;
    device_config.dev_shadow_attrib[DEV_CONFIG_SENSOR_TYPE].val_type = DEV_CONFIG_VAL_TYPE_INT16;
    device_config.dev_shadow_attrib[DEV_CONFIG_SENSOR_TYPE].int16_val = EXT_SENSOR_TERABEE;
    device_config.dev_shadow_attrib[DEV_CONFIG_SENSOR_TYPE].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_SENSOR_TYPE].filename, SENSOR_TYPE_FILE_NAME);

    // application type
    device_config.dev_shadow_attrib[DEV_CONFIG_APP_TYPE].dev_conf_shadow_id = DEV_CONFIG_APP_TYPE;
    device_config.dev_shadow_attrib[DEV_CONFIG_APP_TYPE].val_type = DEV_CONFIG_VAL_TYPE_INT16;
    device_config.dev_shadow_attrib[DEV_CONFIG_APP_TYPE].int16_val = APP_TYPE_DEFAULT_VAL;
    device_config.dev_shadow_attrib[DEV_CONFIG_APP_TYPE].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_APP_TYPE].filename, APP_TYPE_FILE_NAME);

    // configuration version
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_VERSION].dev_conf_shadow_id = DEV_CONFIG_CONF_VERSION;
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_VERSION].val_type = DEV_CONFIG_VAL_TYPE_INT;
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_VERSION].int_val = CONFIG_VERSION_DEFAULT_VAL;
    device_config.dev_shadow_attrib[DEV_CONFIG_CONF_VERSION].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_CONF_VERSION].filename, CONFIG_VERSION_FILE_NAME);

    // topic to publish data to
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].dev_conf_shadow_id = DEV_CONFIG_PUB_TOPIC;
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].val_type = DEV_CONFIG_VAL_TYPE_STRING;
    memset(device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].str_val, '\0', sizeof(device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].str_val));
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].str_val, PUB_TOPIC_DEFAULT_VAL);
    device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_PUB_TOPIC].filename, PUB_TOPIC_FILE_NAME);

    // sub-topic? to publish to
    device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].dev_conf_shadow_id = DEV_CONFIG_SUB_TOPIC;
    device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].val_type = DEV_CONFIG_VAL_TYPE_STRING;
    memset(device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].str_val, '\0', sizeof(device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].str_val));
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].str_val, SUB_TOPIC_DEFAULT_VAL);
    device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].is_new_val = false;
    strcpy(device_config.dev_shadow_attrib[DEV_CONFIG_SUB_TOPIC].filename, SUB_TOPIC_FILE_NAME);

    // load from flash and overwrite defaults
    for (idx = 0; idx < DEV_CONFIG_NUM; idx++)
    {
        char file_contents[256] = {'\0'};

        /*
        *   We have a design issue with how the system was orginally designed in that these 
        *   files are created during provisioning. The problem is that as we add new files, 
        *   those files are not present. It would have been much cleaner/better to do that during bootup. 
        *   This will take some more work to fix but for now, for new fields in the shadow, skip over them 
        *   to not cause errors. 
        * 
        *   The ICCID field is new, so not it filesystem, skip over it
        *   Actually it should even be here... TODO - fix this
        * 
        *   see issue: https://github.com/Reliance-Foundry/levaware_gen3/issues/11 
        *       
        */

       if (idx != DEV_CONFIG_ICCID)
       {
            num_bytes = read_file(file_contents, sizeof(file_contents), device_config.dev_shadow_attrib[idx].filename);

            /*
            * If the file read was successful, save the file contents into the shadow table
            */
            if (num_bytes >= 0)
            {
             if (device_config.dev_shadow_attrib[idx].val_type == DEV_CONFIG_VAL_TYPE_INT16)
                {
                    device_config.dev_shadow_attrib[idx].int16_val = *(int16_t *)file_contents;
                }
            else if (device_config.dev_shadow_attrib[idx].val_type == DEV_CONFIG_VAL_TYPE_INT)
                {
                    device_config.dev_shadow_attrib[idx].int_val = *(int32_t *)file_contents;
                }
            else if (device_config.dev_shadow_attrib[idx].val_type == DEV_CONFIG_VAL_TYPE_STRING && strlen(file_contents) > 0)
                {
                    memset(device_config.dev_shadow_attrib[idx].str_val, '\0', sizeof(device_config.dev_shadow_attrib[idx].str_val));
                    strcpy(device_config.dev_shadow_attrib[idx].str_val, file_contents);
                }
            else
                {
                    LOG_ERR("Unknown device shadow attribute type");
                }
            }
            else
            {
                device_config.dev_shadow_attrib[idx].is_new_val = true;
                LOG_DBG("%s file does not exist or is empty, value still set to default", log_strdup(device_config.dev_shadow_attrib[idx].filename));
            }
       }

    }
}

/**
 * @brief config_init - Initializes the device configuration data store
 *
 * @param   none
 * 
 * @return  none
 * 
 * @note    will abort if fails to initialize
 */
void config_init(void)
{ 
    int err;
    enum dev_config_shadow_id_t attr;       // attribute index value

    LOG_INF("Initializing device configuration");

    // init the filesystem required for config datastore
    config_fs_init();

    if (false == is_file_exists(SERIAL_NUMBER_FILE_NAME))
    {
        // should not get here except if not provisioned - ABORT?
        LOG_ERR("The file for serial number does not exist");      
    }
    else
    {
        // read serial number first
        err = read_file(device_config.serial_number, SERIAL_NUMBER_LEN, SERIAL_NUMBER_FILE_NAME);
        if (err < 0)
            erabort("config_init - reading serial number");
    }
    
    // Get IMEI 
    modem_get_imei(device_config.imei, IMEI_LEN);

    // Getting ICCID
    modem_get_iccid(device_config.iccid, ICCID_LEN);

    config_dev_shadow_attrib_init();

    LOG_INF("Serial number: %s", log_strdup(device_config.serial_number));
    LOG_INF("IMEI: %s", log_strdup(device_config.imei));
    LOG_INF("fw-version: %s", CONFIG_FIRMWARE_VERSION);
    LOG_INF("ICCID: %s", log_strdup(device_config.iccid));
    LOG_INF("daq_interval_s: %d s", config_get_int16(DEV_CONFIG_DAQ_INTERVAL_S));
    LOG_INF("pub_interval_s: %d s", config_get_int16(DEV_CONFIG_PUB_INTERVAL_S));
    LOG_INF("config_interval_s: %d s", config_get_int16(DEV_CONFIG_CONF_UPDATE_INTERVAL_S));
    LOG_INF("sensor_type: %d", config_get_int16(DEV_CONFIG_SENSOR_TYPE));
    LOG_INF("app_type: %d", config_get_int16(DEV_CONFIG_APP_TYPE));
    LOG_INF("config_version: %d", config_get_int(DEV_CONFIG_CONF_VERSION));  
    LOG_INF("pub_topic: %s", log_strdup(config_get_str(DEV_CONFIG_PUB_TOPIC)));
    LOG_INF("sub_topic: %s", log_strdup(config_get_str(DEV_CONFIG_SUB_TOPIC)));

    // ensure all the config files exist by walking through them and saving attributes??
    for (attr = 0; attr < DEV_CONFIG_NUM; attr++)
    {

        /* check to ensure the attribute is not one of the newly added fields (because the file will not exist)
        *  The ICCID seems to be the first new file but there will be many more over time. 
        *  TODO - should create that file or any files that don't exist
        */

        if (attr != DEV_CONFIG_ICCID)
        {
            if (false == is_file_exists(device_config.dev_shadow_attrib[attr].filename))
            {
                config_save_attribute_to_file(attr);
            }
        }
    }

    // TODO: implmenet string based values in file contents - compat rewrite could happen here (imoon 20220904)
}

/**
 * @brief config_save_attribute_to_file - Saves the configuration value to a file 
 * 
 * @param idx Index of the desired attribute
 * 
 * @return Returns 0 if it is a success
 * 
 * @note    Only writes to file if value change flag is set (avoid wearing flash)
 */
int config_save_attribute_to_file(enum dev_config_shadow_id_t idx)
{
    int err;
    struct dev_shadow_attrib_t *dev_shadow_attrib;
    
    // set pointer to the attribute
    dev_shadow_attrib = &device_config.dev_shadow_attrib[idx];

    // clear and error code before running thru the chain of 'if's
    err = 0;

    if (dev_shadow_attrib->is_new_val)
    {
        // make this a warning level so that we can see how often we are writing files
        LOG_WRN("Saving %s to file", log_strdup(dev_shadow_attrib->filename));

        if (dev_shadow_attrib->val_type == DEV_CONFIG_VAL_TYPE_INT16)
        {
            int16_t data = dev_shadow_attrib->int16_val;

            err = save_to_file((char *)&data, sizeof(data), dev_shadow_attrib->filename);

        }
        else if (dev_shadow_attrib->val_type == DEV_CONFIG_VAL_TYPE_INT)
        {
            int32_t data = dev_shadow_attrib->int_val;
            err = save_to_file((char *)&data, sizeof(data), dev_shadow_attrib->filename);
        }
        else if (dev_shadow_attrib->val_type == DEV_CONFIG_VAL_TYPE_STRING)
        {
            err = save_to_file(dev_shadow_attrib->str_val, strlen(dev_shadow_attrib->str_val), dev_shadow_attrib->filename);
        }
        else
        {
            LOG_ERR("Value type is invalid when saving the file for %s", log_strdup(dev_shadow_attrib->filename));
        }

        if (!err)
            {
            LOG_ERR("Unable to save %s", log_strdup(dev_shadow_attrib->filename));
            return err;
            }

        // LOG_INF("%s's value saved to file", log_strdup(dev_shadow_attrib->filename), );
    }
    else
    {
        LOG_INF("Attribute not saved to file -> is_new_val = false");
    }

    dev_shadow_attrib->is_new_val = false;

    return 0;

}

/**
 * @brief Save all the configurations to file that have been updated
 * 
 * @param none
 * 
 * @return Returns 0 if it is a success
 */
int config_save_all_attributes_to_file(void)
{
    enum dev_config_shadow_id_t idx;
    int err;

    LOG_DBG("Saving all shadow attributes to file");

    err = 0;    // assume success to begin with
    for (idx = 0; idx < DEV_CONFIG_NUM; idx++)
    {
        err = config_save_attribute_to_file(idx);
    }

    return err;
}

/**
 * @brief config_get_int16 - Get config value for 16 bit field
 * 
 * @param dev_config_shadow_id_t  configuration item index
 * 
 * @return  value
 *
 * @note    aborts if index out of range
 */
int16_t config_get_int16(enum dev_config_shadow_id_t index)
{
    if (index < DEV_CONFIG_NUM)
    {
        return device_config.dev_shadow_attrib[index].int16_val;
    }

    erabort("config - bad index value");
    return 0;
}

/**
 * @brief config_set_int16 - Set config value for 16 bit field
 * 
 * @param  dev_config_shadow_id_t  - configuration item index
 * @param  value                - value to set in the datastore
 * @param  is_new_value        - used to force a file update
 * 
 * @return  none
 *
 * @note    aborts if index out of range
 */
void config_set_int16(enum dev_config_shadow_id_t index, int16_t val, bool is_new_val)
{
    if (index < DEV_CONFIG_NUM)
    {
        device_config.dev_shadow_attrib[index].int16_val = (int16_t)val;
        device_config.dev_shadow_attrib[index].is_new_val = is_new_val;
    }
    else
        erabort("config - bad index value");

}

/**
 * @brief config_get_int - Get config value for integer field (32 bit)
 * 
 * @param dev_config_shadow_id_t  configuration item index
 * 
 * @return  value
 *
 * @note    aborts if index out of range
 */
int32_t config_get_int(enum dev_config_shadow_id_t index)
{
    if (index < DEV_CONFIG_NUM)
    {
        return device_config.dev_shadow_attrib[index].int_val;
    }
    else
    {
        erabort("config - index out of range");
        return 0;
    }
}

/**
 * @brief config_set_int - Set config value for 32 bit field
 * 
 * @param dev_config_shadow_id_t - configuration item index
 * @param value                 - value to write into datastore
 * @param is_new_value        - used to force a file update
 * 
 * @return  none
 *
 * @note    aborts if index out of range
 */
void config_set_int(enum dev_config_shadow_id_t index, int32_t val, bool is_new_val)
{
    if (index < DEV_CONFIG_NUM)
    {
        device_config.dev_shadow_attrib[index].int_val = (int32_t)val;
        device_config.dev_shadow_attrib[index].is_new_val = is_new_val;
    }
    else
    {
        erabort("config - index out of range");
    }
}

/**
 * @brief config_get_str - Get pointer to configuration string in datastore 
 * 
 * @param dev_config_shadow_id_t configuration item index
 * 
 * @return  pointer to string in the configuration store
 *
 * @note    aborts if index out of range
 */
char * config_get_str(enum dev_config_shadow_id_t index)
{
    if (index < DEV_CONFIG_NUM && index != DEV_CONFIG_ICCID)
    {
        return device_config.dev_shadow_attrib[index].str_val;
    }
    if (index == DEV_CONFIG_ICCID)
    {
        return device_config.iccid;
    }
    return NULL;
}

/**
 * @brief config_set_str - Copy string into configuration datastore for the given index
 * 
 * @param dev_config_shadow_id_t - configuration item index
 * @param stringp               - pointer to string to save in datastore
 * @param is_new_value        - used to force a file update
 * 
 * @return  none
 *
 * @note    aborts if index out of range
 */
void config_set_str(enum dev_config_shadow_id_t index, char *val, bool is_new_val)
{
    if (index < DEV_CONFIG_NUM && strlen(val) > 0)
    {
        // Nulls out the storage of string
        memset(device_config.dev_shadow_attrib[index].str_val, '\0', sizeof(device_config.dev_shadow_attrib[index].str_val));
        strcpy(device_config.dev_shadow_attrib[index].str_val, val);
        device_config.dev_shadow_attrib[index].is_new_val = is_new_val;
    }
    erabort("config.c - set_str range index");
}


/**
 * @brief config_diff_flash_int16 - Diff/compare file contents to a value 
 * 
 * @param  dev_config_shadow_id_t index of shadow item
 * @param  value to compare the datastore value
 * 
 * @return  true if values are the same
 *
 * @note    aborts if index out of range
 * 
 */
bool config_diff_flash_int16(enum dev_config_shadow_id_t attribute, int16_t valid_val)
{
    char file_contents[16] = {'\0'};    // define a small amout of storage to read file into

    read_file(file_contents, sizeof(file_contents), device_config.dev_shadow_attrib[attribute].filename);
    int16_t read_val = (int16_t)file_contents[0];     
   
    return read_val == valid_val;

}

/**
 * @brief config_get_serial_number - Get serial number string 
 * 
 * @param  void 
 * 
 * @return  stringp - pointer to serial number string
 *
 * @note    
 * 
 */
char *config_get_serial_number(void)
{
    return device_config.serial_number;
}
