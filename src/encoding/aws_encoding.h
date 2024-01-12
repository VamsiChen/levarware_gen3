/**
 * @brief: 	aws_encoding.h - External definitions for the AWS encode/decode package
 *
 * @notes: 	This module drives JSON library to encode and decode the AWS
 *         IoT messages 
 *  
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef AWSENCODE_H_
#define AWSENCODE_H_

#include <zephyr.h>

void encoding_init(void);           
void aws_decode_shadow_msg( char *msg_stringp, size_t len);

#endif /* AWSENCODE_H_*/