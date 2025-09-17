/*******************************************************************************
 * @file    octopus_task_manager_ble.h
 * @brief   Header file for the Octopus Task Manager BLE module.
 *          This file declares the functions and data structures required for
 *          managing Bluetooth Low Energy (BLE) operations within the Octopus
 *          platform.
 *
 * @details This module is responsible for initializing, starting, running,
 *          and stopping BLE functionality. It manages BLE status, including
 *          locking and unlocking operations and MAC address handling.
 *
 * @author  Octopus Team
 * @version 1.0.0
 * @date    2024-12-09
 *******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_BT_H__
#define __OCTOPUS_TASK_MANAGER_BT_H__

/*******************************************************************************
 * INCLUDES
 * Include necessary headers for BLE management.
 *******************************************************************************/
#include "octopus_base.h"
#include "octopus_uart_upf.h"

#define MAX_BT_DEVICES 10

typedef struct
{
	char name[32];	  // Bluetooth device name
	char address[13]; // 12 hex digits, no colon, e.g. "00755810FD4F"
	int type;
	int rssi; // Signal strength in dBm
	char class_str[8];
} bt_device_t;

typedef enum
{
	BT_STATE_UNINIT = 0,
	BT_STATE_DISCONNECTED,
	BT_STATE_CONNECTING,
	BT_STATE_CONNECTED,
	BT_STATE_PLAYING,
	BT_STATE_SCANNING,
	BT_STATE_SCANNING_END
} bt_state_t;

#define AT_TEST_CMD "AT\r\n"
#define AT_REBOOT_CMD "AT+REBOOT\r\n"
#define AT_BT_ENABLE(enable) "AT+BTEN=" enable "\r\n"

#define AT_SET_SEPARATOR(sep) "AT+SEP=" sep "\r\n"

#define AT_SET_BAUD(baud) "AT+BAUD=" baud "\r\n"
#define AT_GET_BAUD "AT+BAUD\r\n"

#define AT_SET_PAIR_MODE(mode) "AT+PAIR=" mode "\r\n"
#define AT_GET_PAIR_MODE "AT+PAIR\r\n"

#define AT_SET_NAME(name) "AT+NAME=" name "\r\n"
#define AT_GET_NAME "AT+NAME\r\n"

#define AT_SET_I2S_MODE(mode) "AT+I2SCFG=" mode "\r\n"
#define AT_GET_I2S_MODE "AT+I2SCFG\r\n"

#define AT_SET_LED_CFG(cfg) "AT+LEDCFG=" cfg "\r\n"
#define AT_GET_LED_CFG "AT+LEDCFG\r\n"

#define AT_GET_BT_ADDR "AT+ADDR\r\n"

#define AT_GET_PAIR_LIST "AT+PLIST\r\n"
#define AT_DEL_PAIR(addr) "AT+PLIST=" addr "\r\n"
#define AT_DEL_ALL_PAIR "AT+PLIST=0\r\n"

#define AT_SCAN_START(timeout) "AT+SCAN=1," timeout "\r\n"
#define AT_SCAN_STOP "AT+SCAN=0\r\n"

#define AT_DISCONNECT_ALL "AT+DSCA\r\n"

#define AT_SET_AUDIO_ROUTE(route) "AT+AUDROUTE=" route "\r\n" // 1: A2DP, 2: HFP

// A2DP
#define AT_A2DP_CONN(addr) "AT+A2DPCONN=" addr "\r\n"
#define AT_A2DP_DISC "AT+A2DPDISC\r\n"

// HFP
#define AT_HFP_CONN(addr) "AT+HFPCONN=" addr "\r\n"
#define AT_HFP_DISC "AT+HFPDISC\r\n"

#define AT_CALL_INCOMING(num) "AT+INCOMING=" num "\r\n"
#define AT_CALL_OUTGOING(num) "AT+OUTGOING=" num "\r\n"
#define AT_CALL_TALKING(num) "AT+TALKING=" num "\r\n"
#define AT_CALL_HANGUP "AT+HANGUP\r\n"

// (AVRCP)
#define AT_CTL_PLAY "CTPLAY\r\n"
#define AT_CTL_PAUSE "CTPAUSE\r\n"
#define AT_CTL_NEXT "CTFWD\r\n"
#define AT_CTL_PREV "CTBACK\r\n"

#define AT_CTL_DIAL(num) "DIAL=" num "\r\n"
#define AT_CTL_ANSWER "ANSW\r\n"
#define AT_CTL_HANGUP "CHUP\r\n"
#define AT_CTL_DTMF(num) "DTMF=" num "\r\n"

#ifdef TASK_MANAGER_STATE_MACHINE_BT_MUSIC

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

	/*******************************************************************************
	 * MACROS
	 * Define commonly used macros for this module.
	 *******************************************************************************/
	/*******************************************************************************
	 * TYPEDEFS
	 * Define types used in the BLE management process.
	 *******************************************************************************/

	/*******************************************************************************
	 * GLOBAL FUNCTIONS DECLARATION
	 * Declare the public functions provided by this module.
	 *******************************************************************************/

	/**
	 * @brief Initialize BLE functionality.
	 */
	void task_bt_init_running(void);

	/**
	 * @brief Start BLE operations.
	 */
	void task_bt_start_running(void);

	/**
	 * @brief Assert and verify the BLE module is running correctly.
	 */
	void task_bt_assert_running(void);

	/**
	 * @brief Handle the main logic for BLE operations.
	 */
	void task_bt_running(void);

	/**
	 * @brief Perform post-processing for BLE operations.
	 */
	void task_bt_post_running(void);

	/**
	 * @brief Stop BLE operations.
	 */
	void task_bt_stop_running(void);

	void bt_start_scan(void);
	void bt_parse_scan_line(const char *line);
	void bt_clear_device_list(void);
	void bt_connect_by_name(const char *target_name);
	void bt_list_devices(void);
	void bt_connect_by_index(int index);

#ifdef __cplusplus
}
#endif

extern upf_module_t upf_module_info_BT_MUSIC;
#endif //#ifdef TASK_MANAGER_STATE_MACHINE_BT_MUSIC

#endif // __OCTOPUS_TASK_MANAGER_BLE_H__
