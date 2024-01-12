/**
 * @brief: 	aws_connector.c - Supervisor module for nRF AWS IoT cloud connector
 *
 * @notes: 	This module drives the nRF AWS IoT client. The nRF client handles more of the implementation 
 *          internals of a AWS iot client but it does need to be started, monitored and subscribed to the application
 *          layer specifics.
 * 
 * @note:   The nRF aws iot client also needs to be restarted if it fails to start or drops it's connection to the 
 *          cloud for various reasons (key reason being state of the TCP/UDP NAT being lost/dropped in the network routers).
 * 
 *          Because of these requriements for monitoring, timing and external events, we will create a thread to 
 *          provide the context for these operations. This allow us to defer processing our own thread instead of 
 *          callback functios from nRF subsystem. In addition, an message passing queue will be used to queue events 
 *          to a state machine in this thread. 
 *  
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

// includes for nrf system
#include <zephyr.h>
#include <logging/log.h>
#include <net/aws_iot.h>

// includes for application
#include "bsp/sys_wrapper.h"
#include "config/config.h"
#include "aws_connector.h"
#include "aws_internal.h"

LOG_MODULE_REGISTER(aws_connector); // set the logging package name

/*
* Internal defines for the AWS connector module thread
*/
#define APP_CONNECTOR_STACK_SIZE    1024    // 1K stack size should be enough

#define APP_CONNECTOR_TASK_PRIORITY 5     // Thread priority - should leave room for data sampling threads

#define MSG_QUEUE_SZ                5       // size of message queue feeding the thread. Thread never blocks so can be small


/*
*       A state machine is used to track the aws connector possible states. This allows for 
*       recovery of connection issues and can also queue data for transmission if aws connection 
*       is down. A re-timer will be implemented to restart connections if needed
*/
enum    aws_state_code {
    AWS_STATE_OFFLINE,
    AWS_STATE_CONNECTING,   
    AWS_STATE_CONNECTED,
    AWS_STATE_READY,
    AWS_STATE_DISCONNECTING
};

/*
*   Define an event block structure that is used for message/event passing into the thread
*   The thread will block on these events in its message queue
*/
struct event_msg {
    enum event_code event;          // define what type of event occurred, also indicates the what the pointer below is referencing
    void *msgp;                     // pointer to additional data being passed along with the event, perhaps a pointer to a union needed here
};

// data structure to house the aws_connector thread
static struct k_thread app_aws_thread;

// and  data storage for the stack
K_THREAD_STACK_DEFINE(app_aws_connector_stack_area, APP_CONNECTOR_STACK_SIZE);

// create storage for event queue for the task
static char __aligned(4) connector_event_buffer[sizeof(struct event_msg) * (MSG_QUEUE_SZ)];

/*
*   aws connector control block - all the important data structure storage for this thread
*/
struct  aws_control_blk  {
    // storage for the telemetry event queue control structure
    struct k_msgq connector_event_queue;

    // current state of the state machine
    enum  aws_state_code state;      
};

// allocate storage for the control block
static struct aws_control_blk aws_cblk;

// storage for aws_iot client control block
static struct aws_iot_config aws_iot;

/*
*
*
*/

/** 
* @brief    state_to_string - Utility function to map the State to a String (used for logging state machine) 
*
* @param    state - state to map into a string
*
* @return   pointer to a string that represents the State
*
* @note     
*/
static  char * state_to_string(enum  aws_state_code state) 
{
	char * stringp;

	switch(state)
	{
		case AWS_STATE_OFFLINE:
		stringp = "Offline";
		break;

		case AWS_STATE_CONNECTING:
		stringp = "Connecting";
		break;

        case AWS_STATE_CONNECTED:
        stringp = "Connected";
        break;

        case AWS_STATE_READY:
        stringp = "Ready";
        break;

        case AWS_STATE_DISCONNECTING:
        stringp = "Disconnecting";
        break;

        default:
        stringp = "Invalid";
    }
    return(stringp);
}

/** 
* @brief    Utility function to map the Event code to a String (used for logging state machine) 
*
* @param    event - state to map into a string
*
* @return   pointer to a string that represents the Event
*
* @note     
*/
static  char * event_to_string(enum  event_code event) 
{
	char * stringp;

	switch(event)
	{
		case AWS_EVENT_CONNECTING:
		stringp = "Connecting";
		break;

		case AWS_EVENT_CONNECTED:
		stringp = "Connected";
		break;

        case AWS_EVENT_READY:
        stringp = "Ready";
        break;

        case AWS_EVENT_DISCONNECTED:
        stringp = "Disconnected";
        break;

        case AWS_IOT_SHADOW_RECEIVED:
        stringp = "Shadow received";
        break;
        
        case LTE_EVENT:
        stringp = "lte_event";
        break;

        default:
        stringp = "Invalid";
    }
    return (stringp);
}

/**  
* @brief    aws_queue_event - Queue an event message to the aws_connector state machine
*           This utility function is to only be used internally by the aws_connector module. 
*
* @param    event
*
* @return   nothing
*
* @abort:   aborts if error queuing message
*/
void    aws_queue_event(enum event_code event)
{
    int err;

    // temp copy of the event message
    struct event_msg task_msg;

    task_msg.event = event; // load the event

    // and queue it to the message queue
    err = k_msgq_put(&aws_cblk.connector_event_queue, &task_msg, K_NO_WAIT); 
    if (err)
        erabort("aws_queue_event");
} 

/**  
* @brief    aws_shadow_request - Request shadow topic from AWS
*
*           Utility function to send request to get shadow topic from AWS 
*
* @param    void
*          
* @return   nothing
*/
void    aws_shadow_request(void)
{
    const struct aws_iot_data tx_data = {
        .qos = MQTT_QOS_1_AT_LEAST_ONCE,
        .topic.type = AWS_IOT_SHADOW_TOPIC_GET,
        .ptr = "",
        .len = 0
    };

    int err = aws_iot_send(&tx_data);
    if (err) {
        LOG_ERR("aws_iot_send, error: %d", err);
    }
}

/**  
* @brief    aws_offline_state - Process events when in offline state
*
* @param    cblkp - control block pointer
* @param    eventp - pointer to the event message 
*
* @return   nothing
*/
void aws_offline_state(struct aws_control_blk *cblkp, struct event_msg *evtp)
{
    switch(evtp->event)
    {
        case    AWS_EVENT_CONNECTING:
        cblkp->state = AWS_STATE_CONNECTING;
        break;

        case    AWS_EVENT_CONNECTED:
        cblkp->state = AWS_STATE_CONNECTED;
        break;

        case    AWS_EVENT_READY:
        cblkp->state = AWS_EVENT_READY;
        break;

        case    AWS_IOT_SHADOW_RECEIVED:
        case    AWS_EVENT_DISCONNECTED:
        case    LTE_EVENT:

        break;

    }

}

/**  
* @brief    aws_connecting_state - Process events when in connecting state
*
* @param    cblkp - control block pointer
* @param    eventp - pointer to event message
*
* @return   nothing
*/
void aws_connecting_state(struct aws_control_blk *cblkp, struct event_msg *evtp)
{
    switch(evtp->event)
    {

        case    AWS_EVENT_CONNECTING:
        cblkp->state = AWS_STATE_CONNECTING;
        break;

        case    AWS_EVENT_CONNECTED:
        cblkp->state = AWS_STATE_CONNECTED;
        break;

        case    AWS_EVENT_READY:
        // push out shadow update

        // and start timer for periodic shadow updates WHY????
        cblkp->state = AWS_EVENT_READY;
        break;
        
       case     AWS_IOT_SHADOW_RECEIVED:
       case     AWS_EVENT_DISCONNECTED:
       case    LTE_EVENT:
        break;
    }
}
/**  
* @brief    aws_connected_state - Process events when in aws connected state, 
*
* @note     It's still not ready until all subscribed
*
* @param    cblkp - control block pointer
* @param    eventp - pointer to event message
*
* @return   nothing
*/
void aws_connected_state(struct aws_control_blk *cblkp, struct event_msg *evtp)
{
    switch(evtp->event)
    {

        case    AWS_EVENT_CONNECTING:
        cblkp->state = AWS_STATE_CONNECTING;
        break;

        case    AWS_EVENT_CONNECTED:
        cblkp->state = AWS_STATE_CONNECTING;
        break;

        case    AWS_EVENT_READY:

        // For now skip the FOTA handling. 
        // I am leaving this old shitty code here to show roughly what needs to be done but will recode properly
        // see ticket: https://github.com/Reliance-Foundry/levaware_gen3/issues/8 

        /** Successfully connected to AWS IoT broker, mark image as
         *  working to avoid reverting to the former image upon reboot.
         */
        // boot_write_img_confirmed();

        /** Send version number to AWS IoT broker to verify that the
         *  FOTA update worked.
         */
        // k_work_submit(&shadow_update_version_work);

        /*
        *   Now that aws is now READY, request the shadow topic to kick things off
        */
        aws_shadow_request();

        // Our new state is Ready
        cblkp->state = AWS_STATE_READY;

        // and start timer for periodic shadow updates WHY????
        break;
        
        case    AWS_IOT_SHADOW_RECEIVED:
        case    AWS_EVENT_DISCONNECTED:
        case    LTE_EVENT:
        break;
    }
}

/**  
* @brief    aws_ready_state - Process events when in ready state
*
* @param    cblkp - control block pointer
* @param    eventp - pointer to event message
*
* @return   nothing
*/
void aws_ready_state(struct aws_control_blk *cblkp, struct event_msg *evtp)
{

}

/**  
* @brief    aws_disconnecting_state - Process events when in disconnecting state
*
* @param    cblkp - control block pointer
* @param    eventp - pointer to event message
*
* @return   nothing
*/
void aws_disconnecting_state(struct aws_control_blk *cblkp, struct event_msg *eventp)
{

}

/** 
* @brief   aws_process_event - Process events to the aws_connector thread 
*
* @param    cblkp   pointer to the thread control block
* @param    eventp  pointer to event message
*
* @return   nothing
*
* @note     
*
*/
static void aws_process_event(struct aws_control_blk *cblkp, struct event_msg *eventp)
{

	// for logging, we need to remember the current/new states
	enum  aws_state_code current_state, new_state;

	// for logging, declare pointers the various strings
	char *event_str, *current_state_str, *new_state_str; 

	current_state = cblkp->state; // save current/previous state

       // let's process by first dispatching on the current state, 
       // the state handler will do the reset of the processing based on the event
        switch (cblkp->state)
        {
            case AWS_STATE_OFFLINE:
            aws_offline_state(cblkp, eventp);
            break;
            
            case AWS_STATE_CONNECTING:
            aws_connecting_state(cblkp, eventp);
            break;

            case AWS_STATE_CONNECTED:
            aws_connected_state(cblkp, eventp);
            break;

            case AWS_STATE_READY:
            aws_ready_state(cblkp, eventp);
            break;

            default:
            // erabort("aws_connector, bad event code");
            LOG_WRN("Bad state");

        }
        	new_state = cblkp->state;	// save new state

	// unified logging for all states/events
	current_state_str = state_to_string(current_state);
	new_state_str = state_to_string(new_state);
    event_str = event_to_string(eventp->event);
	
	printk("AWS Current State: %s, Event %s: New State: %s\n", current_state_str, event_str, new_state_str);
	
}

/** 
* @brief   aws_connector_task_entry - Entry point of aws connector thread that monitors the AWS connection 
*
* @param    p1          These parms are passed to all threads at startup. Not used
* @param    unused2
* @param    unused3
*
* @return   never returns
*
* @note     Need to avoid sleeping anywhere in this process so we can service our message event queue. 
*
*/
static void aws_connector_task_entry(void *p1, int unused2, int32_t unused3)
{

    // create a pointer to the task control block
    struct aws_control_blk *cblkp;

    // need local storage of the incoming event
    struct event_msg event_block;
    int  err;

    cblkp = &aws_cblk;  // initialize the control block pointer

    /*
     * initialize the services in the nordic aws client
     */

    // aws client wants the length of our client ID (which is our serial number)
     aws_iot.client_id_len = strlen(config_get_serial_number());   

    // and give it a pointer to the client ID string
     aws_iot.client_id = config_get_serial_number();         

    // perform the initialization of aws client, messages that come in will be handled by the callback function
    err = aws_iot_init(&aws_iot, aws_iot_event_handler);
    if (err)
        erabort("aws_connect - aws_iot_init failed");

    /*
    *   Now we are all initialized, bring up the aws connection and enter 
    *   the forever loop of waiting/blocking on events and processing them. 
    */

   err = aws_iot_connect(&aws_iot);
   if (err)
    LOG_ERR("aws_iot_connect error: %d", err); // need to handle retries: https://github.com/Reliance-Foundry/levaware_gen3/issues/9

    while (1)
    {
        // wait forever on the message/event queue
        k_msgq_get(&cblkp->connector_event_queue, &event_block, K_FOREVER);

        // go and process the event
        aws_process_event(cblkp, &event_block);
    }
}

/** 
* @brief   Global interface function - AWS Connector Init 
*
* @param    void
*
* @return   no value, will abort if fails to initialize properly
*
* @note     To be called as part of the system start-up process
*/
void aws_connector_init(void)
{
    struct aws_control_blk *cblkp;
    cblkp = &aws_cblk;

    // create a message queue for the sensor threads to queue data on
    // k_msgq_init(&tele_msg_que, tele_msg_buffer, sizeof(struct tele_message), TELEMETRY_DAQ_MSG_QUEUE);

    // create and initialize the aws connector thread event queue
    k_msgq_init(&cblkp->connector_event_queue, connector_event_buffer, sizeof(struct event_msg), MSG_QUEUE_SZ);

    // initial state is offline
    cblkp->state = AWS_STATE_OFFLINE;

    // do other task initialization that needs to occur before the tasks start

    /* 
    *   create a thread to supervise the aws connection 
    */
  
    k_thread_create(&app_aws_thread,
                    app_aws_connector_stack_area, 
                    K_THREAD_STACK_SIZEOF(app_aws_connector_stack_area), 
                    (k_thread_entry_t)aws_connector_task_entry, 
                    NULL,
                    NULL,
                    NULL,
                    APP_CONNECTOR_TASK_PRIORITY,
                    0,
                    K_MSEC(0)); // should be no reason to have a start-up delay

    k_thread_name_set(&app_aws_thread, "app_aws_connector");

}