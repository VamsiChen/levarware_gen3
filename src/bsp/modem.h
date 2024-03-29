/*
 * @brief: 	modem.h - External definitions for modem specific definitions
 *
 * 			Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef MODEM_H_
#define MODEM_H_

#define IMEI_LEN (15 + 1) //IMEI + null terminator
#define ICCID_LEN (20 + 1)    // length of ICCID of SIM card
#define RSRP_TO_DBM (-140)

typedef enum modem_error_t
{
    MODEM_ERROR_NONE = 0,
    MODEM_ERROR_UNKNOWN,
    MODEM_ERROR_NUM,
    MODEM_ERROR_INVALID
} modem_error_t;

typedef enum modem_state_t
{
    MODEM_POWER_ON = 0,
    MODEM_POWER_OFF,
    MODEM_OFFLINE,
    MODEM_STATE_NUM,
    MODEM_STATE_INVALID
} modem_state_t;

int modem_get_rsrp_dbm_now(void);
int modem_set_state(modem_state_t state);
void modem_factory_reset(bool force);
void modem_get_iccid(char *device_iccid, int iccid_size);
void modem_get_imei(char *device_imei, int imei_size);
void modem_pdp_control(bool up);
void modem_init_apn_pdp_context(void (*cb)(bool pdp_flag));
int modem_init_modem_info(void);
void modem_enable_rsrp_monitor(void);
int modem_get_rsrp_dbm(void);
void modem_get_imei(char *device_imei, int imei_size);
void modem_get_operator(char *namep, int name_size);

#endif // MODEM_H_