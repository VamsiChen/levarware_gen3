/**
 * @brief: 	config.h - External definitions for configuration datastore 
 *
 * @notes: 	This module acts a simple datastore. Refactored from Levaware Gen2
 *  
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef CONFIG_H_
#define CONFIG_H_

// Nordic includes
#include <stdbool.h>
#include <zephyr/types.h>

// Application includes
#include "bsp/modem.h"

// device config files
#define SERIAL_NUMBER_FILE_NAME             "serial_number"
#define CONFIG_VERSION_FILE_NAME            "config_version"
#define CONFIG_INTERVAL_FILE_NAME           "config_interval"
#define DAQ_INTERVAL_FILE_NAME              "daq_interval"
#define PUB_INTERVAL_FILE_NAME              "pub_interval"
#define ICCID_INTERNAL_FILE_NAME            "ICCID_value"   // see issue: https://github.com/Reliance-Foundry/levaware_gen3/issues/11
#define SENSOR_TYPE_FILE_NAME               "sensor_type"
#define APP_TYPE_FILE_NAME                  "app_type"
#define PUB_TOPIC_FILE_NAME                 "pub_topic"
#define SUB_TOPIC_FILE_NAME                 "sub_topic"

#define SERIAL_NUMBER_LEN (50)

// device config default values
#define CONFIG_VERSION_DEFAULT_VAL          (0)
#define CONFIG_INTERVAL_DEFAULT_VAL_S       (60)
#define DAQ_INTERVAL_DEFAULT_VAL_S          (60)
#define DAQ_INTERVAL_MINIMUM_S              (1)
#define PUB_INTERVAL_DEFAULT_VAL_S          (60)
#define PUB_INTERVAL_MINIMUM_S              (60)
#define ICCID_DEFAULT_VAL                   "000"
#define SENSOR_TYPE_DEFAULT_VAL             (1)
#define APP_TYPE_DEFAULT_VAL                (1)
#define PUB_TOPIC_DEFAULT_VAL               "dt/00000000-0000-0000-0000-000000000000"
#define SUB_TOPIC_DEFAULT_VAL               "sub_topic"


// device shadow attributes
#define DEV_SHADOW_ATTR_CONF_VERSION        "config_version"
#define DEV_SHADOW_ATTR_CONF_INTERVAL_S     "config_interval_s"
#define DEV_SHADOW_ATTR_DAQ_INTERVAL_S      "daq_interval_s"
#define DEV_SHADOW_ATTR_PUB_INTERVAL_S      "pub_interval_s"
#define DEV_SHADOW_ATTR_SENSOR_TYPE         "sensor_type"
#define DEV_SHADOW_ATTR_APP_TYPE            "app_type"
#define DEV_SHADOW_ATTR_PUB_TOPIC           "topic"

// device shadow, other attributes
#define DEV_SHADOW_ATTR_FW_V                "fw_version"

/*
*   Enumerated list of application types so that 
*   one firmware load can support multiple applications
*   In order to maintain backwards compatibilty, we are 
*   keeping these application IDs the same (as Levaware Gen2) even if not 
*   currently supported in this generation of firmware.
*/
enum app_id_t
{
    APP_ID_UNKNOWN = 0,
    DISTANCE,
    MOTION,     // Not sure if motion was ever supported
    AUDIO,      // was supported but code was buggy, will likely deprecate one day
    HYBRID,     // not supported
    APP_ID_NUM,
    APP_ID_INVALID
};

// Setting to set what type of sensor device is fitted with. Again needed for backwards compatibility
enum external_sensor
{
    EXT_SENSOR_UNKNOWN = 0,
    EXT_SENSOR_TERABEE,
    EXT_SENSOR_MAXBOTIX,
    EXT_SENSOR_RADAR,
    EXT_SENSOR_NUM,
    EXT_SENSOR_INVALID
};

// When adding to the enumerated list, be sure to add the code in device_config.c where files are read and configured. 
enum dev_config_shadow_id_t
{
    // device shadow values for various setting
    DEV_CONFIG_DAQ_INTERVAL_S = 0,
    DEV_CONFIG_PUB_INTERVAL_S,
    DEV_CONFIG_ICCID,
    DEV_CONFIG_CONF_UPDATE_INTERVAL_S,
    DEV_CONFIG_SENSOR_TYPE, // current implementation
    DEV_CONFIG_APP_TYPE,
    DEV_CONFIG_CONF_VERSION,
    DEV_CONFIG_PUB_TOPIC,
    DEV_CONFIG_SUB_TOPIC,
    DEV_CONFIG_NUM,
    DEV_CONFIG_INVALID
};

// What is the datatype for a given field
enum dev_config_val_type_t
{
    DEV_CONFIG_VAL_TYPE_INT16 = 0,
    DEV_CONFIG_VAL_TYPE_INT,
    DEV_CONFIG_VAL_TYPE_STRING,
    DEV_CONFIG_VAL_TYPE_NUM,
    DEV_CONFIG_VAL_TYPE_INVALID
};

// A structure that contains all the information for the item in the datastore
// It's a bit of RAM pig because it creates string storage even if a simple data type
struct dev_shadow_attrib_t
{
    enum dev_config_shadow_id_t  dev_conf_shadow_id;
    enum dev_config_val_type_t   val_type;
    int16_t                 int16_val;
    int32_t                 int_val;
    char                    str_val[128];
    bool                    is_new_val;
    char                    filename[50];
};

/**
 * @brief Cache memory for device configurations, i.e., non-persistent
 */
struct device_config_t
{
    char                serial_number[SERIAL_NUMBER_LEN];
    char                imei[IMEI_LEN];
    char                iccid[ICCID_LEN];
    struct dev_shadow_attrib_t dev_shadow_attrib[DEV_CONFIG_NUM];
};


void config_init(void);
char *config_get_fw_version_str(void);
void config_dev_shadow_attrib_init(void);
int config_save_attribute_to_file(enum dev_config_shadow_id_t attribute);
int config_save_all_attributes_to_file(void);
int16_t config_get_int16(enum dev_config_shadow_id_t attribute);
void config_set_int16(enum dev_config_shadow_id_t attribute, int16_t val, bool is_new_val);
int32_t config_get_int(enum dev_config_shadow_id_t attribute);
void config_set_int(enum dev_config_shadow_id_t attribute, int32_t val, bool is_new_val);
char *config_get_str(enum dev_config_shadow_id_t attribute);
void config_set_str(enum dev_config_shadow_id_t attribute, char *val, bool is_new_val);
char *config_get_serial_number(void);
char *config_get_iccid(enum dev_config_shadow_id_t attribute);
bool config_diff_flash_int16(enum dev_config_shadow_id_t attribute, int16_t valid_val);

#endif // CONFIG_H_