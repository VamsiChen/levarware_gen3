/**
 * @brief: 	aws_internal.h - Internal definitions for the aws connector supervisor
 * 
 * @note:   Only to be included by the aws connector supervisor module. If you are needing to include this 
 *          file from outside of this module then you need to rethink your approach. These info is private
 * 
 *           Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef AWSINTERN_H_
#define AWSINTERN_H_

/*
* define various event codes to drive the aws connector state machine
*/

enum event_code {
    AWS_EVENT_CONNECTING,
    AWS_EVENT_CONNECTED,
    AWS_EVENT_READY,
    AWS_EVENT_DISCONNECTED,
    AWS_IOT_SHADOW_RECEIVED,
    LTE_EVENT       // future
};


void    aws_iot_event_handler(const struct aws_iot_evt *const evtp);
void    aws_queue_event(enum event_code event);

#endif // AWSINTERN_H_