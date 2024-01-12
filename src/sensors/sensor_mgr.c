/**
 * @brief: 	sensor_mgr.c - worker thread service to request sensor services
 *
 * @notes: 	Creating an interface function for all the applications to access. For each message to be sent we will queue the
 *          details in the message in a request block and then kick this worker thread to perform
 *          the send.
 *
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

#include <zephyr/zephyr.h>
#include <net/aws_iot.h>
#include <zephyr/logging/log.h>

#include "sensor_mgr.h"
#include "bsp/sys_wrapper.h"

LOG_MODULE_REGISTER(sensor_mgr);

// stack size
// #define SENSOR_MGR_THREAD_STACK_SZ      1024

// thread priority
// #define SENSOR_MGR_THREAD_PRIORITY      5

#define SENSOR_MGR_QUEUE_DEPTH 6 // we can hold up to 4 transmit messages in our queue

#define SENSOR_MGR_EVENT_QUEUE_DEPTH 6

/*
 *   Storage for the work thread and stack storage
 */
// K_THREAD_STACK_DEFINE(stack_area, SENSOR_MGR_THREAD_STACK_SZ);
static struct k_work sensor_request_worker;

/*
 * Create storage for sensor Request queue
 */
static char __aligned(4) sensor_request_buffer[sizeof(struct sensor_mgr_request) * SENSOR_MGR_QUEUE_DEPTH];
static struct k_msgq request_queue;

/*
 * Create a queue for event queue
 */
static char __aligned(4) event_queue_buffer[sizeof(struct event_msg) * SENSOR_MGR_QUEUE_DEPTH];
static struct k_msgq event_queue;

// a struct of events for processing a request

struct event_msg
{

    enum event_code event;
};

// control block for processing an event
struct sensor_mgr_control_block
{

    int loop_counter;

    int average_measurement;

    enum states curr_state;

    struct sensor_mgr_request curr_request;

    // character processing struct needs to be added
};

// decalare local storage and point to a pointer

struct sensor_mgr_control_block sensor_mgr_cblk;

struct sensor_mgr_control_block *cblkp;

/**
 * @brief   sensor_queue_request
 *
 * @param    sensor_id
 * @param    callbackp - pointer to function to call when message has been sent.
 * @param    userp - user pointer to call the completion function with
 *
 * @return   no value
 *
 */
void sensor_queue_request(enum sensor_type sensor_id,
                          int sensor_val,
                          void callbackp(char *userp),
                          char *userp)
{
    // error return code
    int err;

    //event message
    struct event_msg event_blk;

    // local storage of the request
    struct sensor_mgr_request sensor_rqst;

    // fill in the transmit request block
    sensor_rqst.sensor_id = sensor_id;
    sensor_rqst.sensor_request_callback = callbackp;
    sensor_rqst.userp = userp;

    event_blk.event = EVT_NEW_REQUEST;

    // now put the sensor request on the queue
    err = k_msgq_put(&request_queue, &sensor_rqst, K_NO_WAIT);
    if (err)
    {

        // given this failed, we need to call the callback function to release the memory
        erabort("Request queue full");
    }
    else

        //insert event into
        k_msgq_put(&event_queue, &event_blk, K_NO_WAIT);

        // kick the worker thread to run and  process sensor request queue
        k_work_submit(&sensor_request_worker);

        
}

/**
 * @brief   process_idle_state - state handler for idle state
 *
 * @param    void
 *
 * @return   no value, will abort if fails to process properly
 *
 * @note     To be called during processing of requests for idle state
 */
void process_idle_state(struct sensor_mgr_control_block *control_blkp, struct event_msg *evt_blk)
{

    switch (evt_blk->event)
    {
    case EVT_NEW_REQUEST:

        power_reg_external_power_on (SENSOR_TYPE_1);

        // turn on power reg

        // start 1ms timer

        //
        break;

    default: 
        break;
    }
}

void sensor_request_work_process(struct k_work *work)
{

    struct event_msg event_block;

    // get an event from the event queue, if nothing return
    int err;

    err = k_msgq_get(&event_queue, &event_block, K_FOREVER);

    if (err)
    {

        // a problem with request queue
        erabort("Error receiving event from event queue");
    }

    // now that we have an event, let's see what state we are in before processing the event
    switch (cblkp->curr_state)
    {
    case IDLE_STATE:
        process_idle_state(cblkp, &event_block);
        break;

    case POWER_ON_DELAY_STATE:
        process_power_on_delay_state(cblkp, &event_block);
        break;

    case VOLTAGE_SETTLE_STATE:
        process_voltage_settle_state(cblkp, &event_block);
        break;

    case WAITING_FOR_CHARACTER_STATE:
        process_waiting_on_character_state(cblkp, &event_block);
        break;
    }

    // switch on state
    // call state handler, pass the control block pointer and the event

    // Always returns, not blocking
}

/**
 * @brief   sensor_request_work_init - Initialize sensor request worker thread
 *
 * @param    void
 *
 * @return   no value, will abort if fails to initialize properly
 *
 * @note     To be called from applications at the startup process
 */
void sensor_request_work_init(void)
{
    k_work_init(&sensor_request_worker, sensor_request_work_process);

    // initialize the transmit request message
    k_msgq_init(&request_queue, sensor_request_buffer, sizeof(struct sensor_mgr_request), SENSOR_MGR_QUEUE_DEPTH);

    // initialize an event queue
    k_msgq_init(&event_queue, event_queue_buffer, sizeof(struct event_msg), SENSOR_MGR_QUEUE_DEPTH);

    //pointing to the control block stack memory
    cblkp = &sensor_mgr_cblk;

    //at start up idle state is set
    cblkp->curr_state = IDLE_STATE;
}