/**
 * @brief: 	lte_internal.h - Internal definitions for the LTE Connect Mgr
 * 
 * @note:   Only to be included by the lte_connector module
 * 
 *           Copyright (c) 2023 Reliance Foundry Co. Ltd.
 *
 */
#ifndef LTEINTERN_H_
#define LTEINTERN_H_

/*
*       This include file defines the values for the LTE CatM power savings features - eDRX and PSM. 
*
*       See this link for a primer:
*  https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/maximizing-battery-lifetime-in-cellular-iot-an-analysis-of-edrx-psm-and-as-rai
*
*   Note: both the eDRX and PSM values are defined as STRINGS (so it's a little weird)
*/

/*
*		eDRX settings
* 
* Values for eDRX settings: Half a byte in 4-bit format. The eDRX value refers to bit 4 to 1 of octet 3 of
* the Extended DRX parameters information element (see 3GPP TS 24.008, subclause
* 10.5.5.32). Optional. If not present, the value of the requested eDRX period is set to
* the manufacturer-specific default.
* Bit
* 4 3 2 1 – E-UTRAN eDRX cycle length duration
* 0 0 0 0 – 5.12 seconds2
* 0 0 0 1 – 10.24 seconds2
* 0 0 1 0 – 20.48 seconds
* 0 0 1 1 – 40.96 seconds
* 0 1 0 0 – 61.44 seconds3
* 0 1 0 1 – 81.92 seconds
* 0 1 1 0 – 102.4 seconds3
* 0 1 1 1 – 122.88 seconds3
* 1 0 0 0 – 143.36 seconds3
* 1 0 0 1 – 163.84 seconds
* 1 0 1 0 – 327.68 seconds
* 1 0 1 1 – 655.36 seconds
* 1 1 0 0 – 1310.72 seconds
* 1 1 0 1 – 2621.44 seconds
* 1 1 1 0 – 5242.88 seconds4
* 1 1 1 1 – 10485.76 seconds4
*
*/

// Define the eDRX values - for now at compile time. Todo: we might want to make a AWS configured runtime value 
#define LTE_CATM_EDRX_TIME	"1011"		// using 655.36 seconds

/*
*       PSM - Power Savings Mode settings
*
*      The PSM Requested Periodic TAU (tracking area update) follows below
*   time that modem requests to be effectively "powered off" until the tracking area update occurs 
*
*   Bits 5 to 1 represent the binary coded timer value.
*   Bits 8 to 6 define the timer value unit timer as follows:
*   Bits
*   8 7 6
*   0 0 0 – Value is incremented in multiples of 10 minutes
*   0 0 1 – Value is incremented in multiples of 1 hour
*   0 1 0 – Value is incremented in multiples of 10 hours
*   0 1 1 – Value is incremented in multiples of 2 seconds
*   1 0 0 – Value is incremented in multiples of 30 seconds
*   1 0 1 – Value is incremented in multiples of 1 minute
*   1 1 0 – Value is incremented in multiples of 320 hours
*/

// Define the PSM TAU setting - currently 14 minutes. In future we should configure at runtime from AWS. 
#define LTE_PSM_REQ_RPTAU   "10101110"

/*
*   The PSM Requested Active time (time that modem requests to be active for before entering PSM mode)
*
* Bits 5 to 1 represent the binary coded timer value.
* Bits 8 to 6 define the timer value unit for the GPRS timer as follows:
* Bits
* 8 7 6
* 0 0 0 – Value is incremented in multiples of 2 seconds
* 0 0 1 – Value is incremented in multiples of 1 minute
* 0 1 0 – Value is incremented in multiples of 6 minutes
* 1 1 1 – Value indicates that the timer is deactivated
*
* bits 5 4 3 2 1
*       the value (which is multipled by the values in bits 6, 7, 8)
*/

/* Define the PSM Active time setting - currently 1 minute. 
*   I have made this longer that we really need in order to get SMS messages out to the device
*   when running on Telus network (Telus seems to be sloe to push SMS when queued)
*
*   In future we should configure at runtime from AWS.
*/
#define LTE_PSM_REQ_RAT     "00100001"


#endif /* LTEINTERN_H_*/