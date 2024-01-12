/*
 * @brief: 	sys_wrapper.c - Wrapper around select OS system services
 *
 * @notes: 	helps to hide (and provide portability of general OS services)
 * 
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
// Nordic module includes
#include <zephyr/zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>

// Citysage module includes
#include "sys_wrapper.h"

LOG_MODULE_REGISTER(sys_wrapper); // register module with logging package

/* 
* @brief:   erabort - Error Abort on Fatal error
*
* @param:   error string 
*
* @return:  Never returns, takes system down with abort error message (and future crash dump) followed by reboot 
*
*/
void erabort(const char *errorstrp)
{

  /* Log error string and then reboot system
   *  TODO - Future should perform crash dump to get traceback 
   */
  LOG_ERR("\nFatal error: %s", errorstrp);
  sys_reboot(SYS_REBOOT_COLD);   
}