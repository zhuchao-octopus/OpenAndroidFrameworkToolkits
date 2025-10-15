/**
 * ****************************************************************************
 * @file octopus_ble.c
 * @brief C file for the Octopus Task Manager module.
 *
 * This file defines the macros, includes required libraries, and declares
 * functions used by the Octopus Task Manager. It includes the initialization,
 * start, stop, and running behaviors of Bluetooth Low Energy (BLE) tasks.
 * Additionally, it handles the bonding and connection status management
 * for BLE devices, such as pairing and locking the system based on BLE status.
 *
 * @copyright Copyright (c) XXX. All rights reserved.
 * @author  Octopus Team
 * @version 1.0.0
 * @date    2024-12-09
 */
/*******************************************************************************
 * INCLUDES
 * Include the necessary header files for the Octopus platform and BLE functionality.
 */

#include "octopus_ble_hal.h" // Include Bluetooth Low Energy Hardware Abstraction Layer (HAL) header
#include "octopus_ble.h"	 // Include Bluetooth Low Energy functionality
#include "octopus_flash.h"

#define BLE_BONDED_MAC_ADDRESS_0 (0x1107c004)
#define BLE_BONDED_MAC_ADDRESS_1 (0x1107c0c4)
#define BLE_BONDED_MAC_ADDRESS_2 (0x1107c0d0)
#define BLE_BONDED_MAC_ADDRESS_3 (0x1107c0dc)

#define BLE_RSSI_UNLOCK_MAX_DB_VALUE 55
#define BLE_RSSI_LOCK_MAX_DB_VALUE 70
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#ifdef TASK_MANAGER_STATE_MACHINE_BLE
/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */
void CheckBLeConnectionStatus(uint8_t *connected_mac, uint16_t c_type);

void StartToLock(void);
void StartToUnlock(void);

void StartToLockRssi(void);
void StartToUnlockRssi(void);

void ble_connecttion_polling(void);
void update_bonded_mac(void);
/*******************************************************************************
 * GLOBAL VARIABLES
 */
BLE_STATUS ble_status = {0};

uint8_t ble_bonded_mac[6][6] = {
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01},

	{0x24, 0xE9, 0x66, 0x19, 0x26, 0x40},
	{0xae, 0xd8, 0xb4, 0xf6, 0x40, 0xc4},
};

static uint32_t l_t_msg_wait_10_timer = 0;

static uint32_t l_t_msg_ble_polling_timer_1s = 0;
static uint32_t l_t_msg_ble_pair_wait_timer = 0;

static uint32_t l_t_msg_ble_lock_wait_timer = 0;

// static bool module_send_handler(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint16_t param, ptl_proc_buff_t *buff);
// static bool module_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */
void task_ble_init_running(void)
{
	OTMS(TASK_MODULE_BLE, OTMS_S_INVALID);
#ifdef TASK_MANAGER_STATE_MACHINE_BLE
	LOG_LEVEL("task_ble_init_running\r\n");
	// com_uart_ptl_register_module(MSGMODULE_SYSTEM, module_send_handler, module_receive_handler);
	// hal_disable_bLe_pair_mode();
	ble_status.mode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
	update_bonded_mac();
#endif
}

void task_ble_start_running(void)
{
	LOG_LEVEL("task_ble_start_running\r\n");
	OTMS(TASK_MODULE_BLE, OTMS_S_ASSERT_RUN);
}

void task_ble_assert_running(void)
{
#ifdef TASK_MANAGER_STATE_MACHINE_BLE
	StartTickCounter(&l_t_msg_wait_10_timer);
	StartTickCounter(&l_t_msg_ble_polling_timer_1s);
	OTMS(TASK_MODULE_BLE, OTMS_S_RUNNING);
#endif
}

void task_ble_running(void)
{
	if (GetTickCounter(&l_t_msg_wait_10_timer) < 10)
		return;
	StartTickCounter(&l_t_msg_wait_10_timer);

	Msg_t *msg = get_message(TASK_MODULE_BLE);
	if (msg->msg_id != NO_MSG)
	{
		if (msg->msg_id == MSG_OTSM_DEVICE_BLE_EVENT)
		{
			switch (msg->param1)
			{
			case MSG_OTSM_CMD_BLE_PAIR_ON:
			case MSG_OTSM_CMD_BLE_PAIR_OFF:
				if (GAPBOND_PAIRING_MODE_NO_PAIRING == ble_status.mode)
					ble_status.mode = hal_set_pairing_mode_onoff(true, ble_status.mode);
				else
					ble_status.mode = hal_set_pairing_mode_onoff(false, ble_status.mode);
				break;

			case MSG_OTSM_CMD_BLE_CONNECTED:
				break;

			case MSG_OTSM_CMD_BLE_PAIRING:
			case MSG_OTSM_CMD_BLE_BONDED:
				ble_status.connected = true;
				ble_status.rssi_unlock = true;
				update_bonded_mac();
				break;

			case MSG_OTSM_CMD_BLE_DISCONNECTED:
				if (ble_status.connected)
				{
					ble_status.connected = false;
					CheckBLeConnectionStatus(ble_status.mac, msg->param1);
				}
				break;

			default:
				break;
			}
		}
	}

	if (GAPBOND_PAIRING_MODE_WAIT_FOR_REQ == ble_status.mode)
	{
		if (!IsTickCounterStart(&l_t_msg_ble_pair_wait_timer))
			StartTickCounter(&l_t_msg_ble_pair_wait_timer);
		if (GetTickCounter(&l_t_msg_ble_pair_wait_timer) >= 1000 * 60)
			ble_status.mode = hal_set_pairing_mode_onoff(false, ble_status.mode);
	}
	else
	{
		StopTickCounter(&l_t_msg_ble_pair_wait_timer);
	}

	if (GetTickCounter(&l_t_msg_ble_polling_timer_1s) >= 1000)
	{
		// LOG_LEVEL("ble_status.rssi:%d\r\n",ble_status.rssi);
		ble_status.rssi = hal_get_ble_rssi(0);
		StartToLockRssi();
		StartToUnlockRssi();
		StartTickCounter(&l_t_msg_ble_polling_timer_1s);
	}

	ble_connecttion_polling();
}

void task_ble_post_running(void)
{
	OTMS(TASK_MODULE_BLE, OTMS_S_ASSERT_RUN);
}

void task_ble_stop_running(void)
{
	LOG_LEVEL("_stop_running\r\n");
	OTMS(TASK_MODULE_BLE, OTMS_S_INVALID);
}

bool is_exists_bonded_mac(uint8_t *connected_mac)
{
	uint8_t length = sizeof(ble_bonded_mac) / sizeof(ble_bonded_mac[0]);
	bool matched = false;

	for (uint8_t i = 0; i < length; i++)
	{
		if (connected_mac[0] == ble_bonded_mac[i][0] && connected_mac[1] == ble_bonded_mac[i][1] && connected_mac[2] == ble_bonded_mac[i][2] && connected_mac[3] == ble_bonded_mac[i][3] && connected_mac[4] == ble_bonded_mac[i][4] && connected_mac[5] == ble_bonded_mac[i][5])
		{
			matched = true;
			break;
		}
	}
	return matched;
}

void update_bonded_mac(void)
{
	FlashReadToBuff(BLE_BONDED_MAC_ADDRESS_0, ble_bonded_mac[0], 6);
	LOG_LEVEL("ble bonded mac:");
	LOG_BUFF(ble_bonded_mac[0], 6);
	FlashReadToBuff(BLE_BONDED_MAC_ADDRESS_1, ble_bonded_mac[1], 6);
	LOG_LEVEL("ble bonded mac:");
	LOG_BUFF(ble_bonded_mac[1], 6);
	FlashReadToBuff(BLE_BONDED_MAC_ADDRESS_2, ble_bonded_mac[2], 6);
	LOG_LEVEL("ble bonded mac:");
	LOG_BUFF(ble_bonded_mac[2], 6);
	FlashReadToBuff(BLE_BONDED_MAC_ADDRESS_3, ble_bonded_mac[3], 6);
	LOG_LEVEL("ble bonded mac:");
	LOG_BUFF(ble_bonded_mac[3], 6);
	LOG_LEVEL("ble_status.connected:%d,ble_status.rssi:%d\r\n", ble_status.connected, ble_status.rssi);
}

void CheckBLeConnectionStatus(uint8_t *connected_mac, uint16_t c_type)
{
	if (c_type == MSG_OTSM_CMD_BLE_DISCONNECTED)
	{
		LOG_LEVEL("ble disconnected mac:");
		LOG_BUFF(ble_status.mac, 6);
	}
	else if (c_type == MSG_OTSM_CMD_BLE_CONNECTED || c_type == MSG_OTSM_CMD_BLE_PAIRING)
	{
		LOG_LEVEL("ble connected mac:");
		LOG_BUFF(ble_status.mac, 6);
		bool matched = is_exists_bonded_mac(connected_mac);
		if (!matched)
		{
			/// LOG_LEVEL("no exists bonded mac:");
			/// LOG_BUFF(ble_status.mac, 6);
			/// return;
		}
	}

	switch (c_type)
	{
	case MSG_OTSM_CMD_BLE_PAIRING:
	case MSG_OTSM_CMD_BLE_BONDED:
		break;
	case MSG_OTSM_CMD_BLE_DISCONNECTED:
		send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_DISCONNECTED, MSG_OTSM_CMD_BLE_DISCONNECTED);
		StartToLock();

		break;
	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StartToLockRssi(void)
{
	if (ble_status.rssi > BLE_RSSI_LOCK_MAX_DB_VALUE && ble_status.connected)
	{
		if (!IsTickCounterStart(&l_t_msg_ble_lock_wait_timer))
		{
			ble_status.to_lock = true;
			StartTickCounter(&l_t_msg_ble_lock_wait_timer);
			LOG_LEVEL("Start to lock system rssi=%d ...\r\n", ble_status.rssi);
		}
	}
}

void StartToUnlockRssi(void)
{
	if (ble_status.rssi > BLE_RSSI_UNLOCK_MAX_DB_VALUE || ble_status.rssi == 0)
		return;
	if (!ble_status.connected)
		return;

	if (IsTickCounterStart(&l_t_msg_ble_lock_wait_timer))
		StopTickCounter(&l_t_msg_ble_lock_wait_timer);

	if (!ble_status.rssi_unlock)
		return;

	ble_status.to_lock = false;
	// ble_status.locked = false;
	ble_status.rssi_unlock = false;
	LOG_LEVEL("Start to unlock system rssi=%d ...\r\n", ble_status.rssi);
	send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_CONNECTED, FRAME_CMD_SYSTEM_POWER_ON);
}

void StartToLock(void)
{
	if (!IsTickCounterStart(&l_t_msg_ble_lock_wait_timer))
	{
		ble_status.to_lock = true;
		StartTickCounter(&l_t_msg_ble_lock_wait_timer);
		LOG_LEVEL("Start to lock system...\r\n");
	}
}

void StartToUnlock(void)
{
	if (!ble_status.connected)
		return;
	if (IsTickCounterStart(&l_t_msg_ble_lock_wait_timer))
		StopTickCounter(&l_t_msg_ble_lock_wait_timer);

	ble_status.to_lock = false;
	// ble_status.locked = false;
	LOG_LEVEL("Start to unlock system...\r\n");
	send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_CONNECTED, FRAME_CMD_SYSTEM_POWER_ON);
}

void ble_connecttion_polling(void)
{
	ble_status.rssi = hal_get_ble_rssi(0);

	if (ble_status.to_lock && GetTickCounter(&l_t_msg_ble_lock_wait_timer) > 8000)
	{
		StopTickCounter(&l_t_msg_ble_lock_wait_timer);
		// ble_status.locked = true;
		ble_status.to_lock = false;

		LOG_LEVEL("Start to power off system ...\r\n");
		send_message(TASK_MODULE_SYSTEM, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_DISCONNECTED, FRAME_CMD_SYSTEM_POWER_OFF);
	}
}
#endif
