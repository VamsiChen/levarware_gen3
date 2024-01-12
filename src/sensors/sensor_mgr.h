/**
 * @brief: pervisor
 * 
 * @note:   
 * 
 *           Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

enum sensor_type {
    // Enumeration values for sensor types
    SENSOR_TYPE_1,
    SENSOR_TYPE_2,
    // Add more sensor types as needed
};

//

struct sensor_mgr_request {
    enum sensor_type sensor_id;
    int sensor_value;
    void (*sensor_request_callback)(char *userp);
    char *userp;
};

//list of all possible events that happen in a cycle
enum event_code {
    EVT_NEW_REQUEST,
    EVT_TIMER_EXPIRY_1_MS,
    EVT_TIMER_EXPIRY_2_SECS,
    EVT_PACKETS_RECEIVED
};

// List of all possible states the sensor manager will go through
enum states{
    IDLE_STATE,
    POWER_ON_DELAY_STATE,
    VOLTAGE_SETTLE_STATE,
    WAITING_FOR_CHARACTER_STATE
};


#endif /*SENSOR_MGR_H*/