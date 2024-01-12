/*
 * @brief: 	lte_connect_mgr.h - External definitions for the LTE Connect Mgr
 *
 * @notes: 	See lte_internal.h for definitions local to the LTE Connect Mgr
 * 
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef LTECONN_H_
#define LTECONN_H_

/*
*   LTE connection manager public functions
*/
void lte_connect_init(void);
bool lte_check_pdp_context(void);
void  lte_application_conn_up(bool aws_up);

void lte_stats_print(void);
void lte_stats_clear(void);


#endif /* LTECONN_H_*/