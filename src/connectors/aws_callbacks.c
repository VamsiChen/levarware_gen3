/**
 * @brief: 	aws_callbacks.c - functions to handle callbacks from nRF  AWS IoT client
 *
 * @notes: 	Keeping these functions in a seperate file in order to keep core files uncluttered 
 *          as these functions can be quite long
 * 
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

// includes for nrf system
#include <zephyr.h>
#include <logging/log.h>
#include <net/aws_iot.h>

// includes for application
#include "aws_connector.h"
#include "aws_internal.h"
#include "encoding/aws_encoding.h"

LOG_MODULE_REGISTER(aws_callbacks);

/** 
* @brief   aws_iot_event_handler - Callback event handler from the Nordic aws client 
*
* @param    evtp    pointer to the aws event structure
*
* @return   nothing
*
* @note     currently implementing the minimum needed
*
*/
void aws_iot_event_handler(const struct aws_iot_evt *const evtp)
{

    switch (evtp->type)
    {
    case AWS_IOT_EVT_CONNECTING:
        LOG_INF("AWS_IOT_EVT_CONNECTING");

        // queue the 'connecting event' to the aws connector task
        aws_queue_event(AWS_EVENT_CONNECTING); 
        break;

    case AWS_IOT_EVT_CONNECTED:
        LOG_INF("AWS_IOT_EVT_CONNECTED");

        if (evtp->data.persistent_session) {
            LOG_INF("Persistent session enabled");
        }

        // queue event for AWS is connected 
        aws_queue_event(AWS_EVENT_CONNECTED); 
        break;

    case AWS_IOT_EVT_READY:
        LOG_INF("AWS_IOT_EVT_READY");

        // event for AWS is now ready and subscribed to all topics
        aws_queue_event(AWS_EVENT_READY); 
        break;

    case AWS_IOT_EVT_DISCONNECTED:
        LOG_INF("AWS_IOT_EVT_DISCONNECTED");

        // event is AWS is disconnected, go deal with it in our context
        aws_queue_event(AWS_EVENT_DISCONNECTED);
        break;

    case AWS_IOT_EVT_DATA_RECEIVED:
        LOG_INF("AWS_IOT_EVT_DATA_RECEIVED");

        printk("Data received from AWS IoT console:\r\nTopic: %.*s\r\nMessage[len: %d]: %s\r\n",
                       evtp->data.msg.topic.len,
                    evtp->data.msg.topic.str,
                    evtp->data.msg.len,
                    evtp->data.msg.ptr);

        /* 
        *   For now we are only expecting classic shadow data for topic & processing, but this will 
        *   need to be expanded in future if we wish to support additional topics.
        * 
        *   So go process the shadow message in the callback context and the send event to complete the 
        *   processing in our thread context 
        */
        aws_decode_shadow_msg(evtp->data.msg.ptr, evtp->data.msg.len);

        // we have parsed and saved the shadow update, queue the event
        aws_queue_event(AWS_IOT_SHADOW_RECEIVED);
        break;

    case AWS_IOT_EVT_FOTA_START:
        LOG_WRN("AWS_IOT_EVT_FOTA_START");
        // ToDo - see issue: https://github.com/Reliance-Foundry/levaware_gen3/issues/8

        break;

    case AWS_IOT_EVT_FOTA_ERASE_PENDING:
        LOG_WRN("AWS_IOT_EVT_FOTA_ERASE_PENDING");
        LOG_WRN("Disconnect LTE link or reboot");
        /* see issue: https://github.com/Reliance-Foundry/levaware_gen3/issues/8
        * leave this shitty code here as an example of what's required
        err = lte_lc_offline();
        if (err) {
            LOG_ERR("Error disconnecting from LTE");
        }
        */

        break;

    case AWS_IOT_EVT_FOTA_ERASE_DONE:
        LOG_WRN("AWS_FOTA_EVT_ERASE_DONE");
        LOG_WRN("Reconnecting the LTE link");
        /* 
        err = lte_lc_connect();
        if (err) {
            LOG_ERR("Error connecting to LTE");
        }
        */

        break;

    case AWS_IOT_EVT_FOTA_DONE:
        LOG_WRN("AWS_IOT_EVT_FOTA_DONE");
        LOG_WRN("FOTA done, rebooting device");
        /* 
        aws_iot_disconnect();
        sys_reboot(0);
        */

        break;

    case AWS_IOT_EVT_FOTA_DL_PROGRESS:
        LOG_WRN("AWS_IOT_EVT_FOTA_DL_PROGRESS, (%d%%)",
               evtp->data.fota_progress);
        break;
        
    case AWS_IOT_EVT_PINGRESP:
        LOG_INF("AWS_IOT_EVT_PINGRESP");
    break;

    case AWS_IOT_EVT_PUBACK:
        LOG_INF("AWS_IOT_EVT_PUBACK");
    break;

    case AWS_IOT_EVT_ERROR:
        LOG_ERR("AWS_IOT_EVT_ERROR, %d", evtp->data.err);
        break;

    case AWS_IOT_EVT_FOTA_ERROR:
        LOG_ERR("AWS_IOT_EVT_FOTA_ERROR");
        break;

    default:
        LOG_ERR("Unknown AWS IoT event type: %d", evtp->type);
        break;
    }
}
