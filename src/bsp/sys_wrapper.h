/**
 * @brief: 	sys_wrapper.h - Header file for system wrapper functions 
 * 
 * @notes: 	Provides a level of abstraction over Zephyr OS
 * 
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */

#ifndef SYSWRAPPER_H_
#define SYSWRAPPER_H_

#include <zephyr/zephyr.h>

void erabort(const char *errorstrp);

#endif /* SYSWRAPPER_H_*/