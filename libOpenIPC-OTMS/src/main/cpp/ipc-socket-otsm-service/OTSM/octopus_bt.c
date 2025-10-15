/**
 * ****************************************************************************
 * @file octopus_bt.c
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
#include "octopus_bt.h"
#include "octopus_task_manager.h"  // Task Manager: handles scheduling and execution of system tasks
#include "octopus_tickcounter.h"    // Tick Counter: provides timing and delay utilities
#include "octopus_message.h"        // Message IDs: defines identifiers for inter-task communication
#include "octopus_msgqueue.h"       // Message Queue: API for sending/receiving messages between tasks
#include "octopus_uart_ptl.h"       // UART Protocol Layer: handles protocol-level UART operations
#include "octopus_uart_upf.h"       // UART Packet Framework: low-level UART packet processing


/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

/*******************************************************************************
 * MACROS
 */
#define BT_PAIRED_DISTANCE -35
#define BT_SELECT_DISTANCE -45
#define BT_SCAN_TIME_INTERVAL 20

#ifdef TASK_MANAGER_STATE_MACHINE_BT_MUSIC

static bt_device_t bt_device_list[MAX_BT_DEVICES];
static uint8_t bt_device_count = 0;
static bt_state_t bt_current_state = BT_STATE_DISCONNECTED;
// uint32_t connect_start_time = 0;

bt_device_t selected_best_dev = {0};
bt_device_t last_best_dev = {0};

upf_module_t upf_module_info_BT_MUSIC = {UPF_MODULE_ID_BT_MUSIC, UPF_CHANNEL_8, UPF_CHANNEL_TYPE_CHAR};
/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */

void bt_module_init(void);
void bt_state_manager(void);

void bt_connect_last_device(void);
void bt_connect_selected_device(void);
void bt_send_at_command(const char *format, ...);
bool bt_is_audio_device(const bt_device_t *dev);
bool bt_receive_handler(upf_proc_buff_t *upf_proc_buff);

/*******************************************************************************
 * GLOBAL VARIABLES
 */

static uint32_t l_t_bt_auto_link_wait_timer = 0;
static uint32_t l_t_bt_stat_polling_timer_8s = 0;
static uint32_t l_t_bt_scan_polling_timer_15s = 0;
static uint32_t l_t_bt_conn_polling_timer_20s = 0;

bool bt_receive_handler(upf_proc_buff_t *upf_proc_buff);
// static bool module_send_handler(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint16_t param, ptl_proc_buff_t *buff);
// static bool module_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */

void task_bt_init_running(void)
{
	OTMS(TASK_MODULE_BT, OTMS_S_INVALID);
	bt_clear_device_list();
	LOG_LEVEL("task_bt_init_running\r\n");
	// com_uart_ptl_register_module(MSGMODULE_SYSTEM, module_send_handler, module_receive_handler);
}

void task_bt_start_running(void)
{
	LOG_LEVEL("task_bt_start_running\r\n");
#ifdef TASK_MANAGER_STATE_MACHINE_UPF
	upf_register_module(upf_module_info_BT_MUSIC, bt_receive_handler);
#endif
	OTMS(TASK_MODULE_BT, OTMS_S_ASSERT_RUN);
	StartTickCounter(&l_t_bt_auto_link_wait_timer);
	StartTickCounter(&l_t_bt_stat_polling_timer_8s);
	StartTickCounter(&l_t_bt_scan_polling_timer_15s);
	bt_module_init();
}

void task_bt_assert_running(void)
{
	OTMS(TASK_MODULE_BT, OTMS_S_RUNNING);
}

void task_bt_running(void)
{
	// if (GetTickCounter(&l_t_msg_wait_10_timer) < 10)
	//	return;
	// StartTickCounter(&l_t_msg_wait_10_timer);

	bt_state_manager();

	// Msg_t *msg = get_message(TASK_MODULE_BT);
	// if (msg->msg_id == NO_MSG) return;
}

void task_bt_post_running(void)
{
	OTMS(TASK_MODULE_BT, OTMS_S_ASSERT_RUN);
}

void task_bt_stop_running(void)
{
	LOG_LEVEL("_stop_running\r\n");
	OTMS(TASK_MODULE_BT, OTMS_S_INVALID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Send initialization AT commands to AC6956 (Chengqian BT module)
void bt_module_init(void)
{
	// Optional: wait for module to power up and stabilize
	// delay_ms(800);
	LOG_LEVEL("bt module init...\r\n");
	// bt_send_at_command(AT_DEL_ALL_PAIR);
	// bt_send_at_command("AT+LINKCFG=1\r\n");
	//  1. Check if the Bluetooth module is alive
	//  bt_send_at_command("AT\r\n");  // Expect "OK"

	// 2. Enable Bluetooth (in case it's disabled after reboot)
	// bt_send_at_command("AT+BTEN=1\r\n");

	// 3. Set Bluetooth mode to discoverable and connectable
	// bt_send_at_command("AT+PAIR=0\r\n");

	// 4. Set a recognizable Bluetooth name (can skip if already set in module)
	bt_send_at_command("AT+NAME=KLD-BT-MUSIC\r\n");

	// 5. Set audio input type
	// Use "0" for analog (e.g., line-in/mic), "3" for I2S depending on your hardware
	// bt_send_at_command("AT+I2SCFG=0\r\n");  // Analog input assumed here

	// 6. Configure LED status behavior (enabled, high active, mode 1)
	// bt_send_at_command("AT+LEDCFG=3\r\n");

	// 7. (Optional) Auto reconnect to previously paired device
	// This assumes AC6956 firmware supports auto reconnection
	// Otherwise you can retrieve list with AT+PLIST and reconnect via AT+A2DPCONN

	// 8. Optional: scan for new devices if no pairing exists
	// send_at_command("AT+SCAN=1,10\r\n");

	// 9. Optional: manually connect to known speaker
	// send_at_command("AT+A2DPCONN=AC:DE:48:00:11:22\r\n");  // Replace with actual BT address
}

/**
 * @brief Start scanning for nearby Bluetooth devices.
 *
 * This function performs the following:
 *  1. Logs a debug message indicating that scanning is about to begin.
 *  2. Clears the internal list of previously found Bluetooth devices.
 *  3. Sends the AT command "AT+SCAN=1,10" to the Bluetooth module:
 *     - "1" starts the scan mode.
 *     - "10" sets the scan duration to 10 units of 1.28s = ~12.8 seconds.
 *
 * The scan results are expected to be received asynchronously through the UART
 * in the form of "+SCAN=type,rssi,addr,name,class" lines, which should be parsed
 * and stored elsewhere (e.g., in bt_parse_scan_line()).
 */
void bt_start_scan(void)
{
	// Log scan initiation
	LOG_LEVEL("bt scan... devices\r\n");

	// Clear the previously stored device list to start fresh
	bt_clear_device_list();

	// Send AT command to initiate scan
	bt_send_at_command("AT+SCAN=1,%d\r\n", BT_SCAN_TIME_INTERVAL); // Scan for ~12.8 seconds
}

/**
 * @brief Manage Bluetooth state and perform periodic actions.
 *
 * This function is designed to be called periodically (e.g., in a main loop or timer event).
 * It handles:
 *  - Periodic A2DP status querying
 *  - Attempting to reconnect to a previously connected device
 *  - Scanning for available Bluetooth devices if not connected
 */
void bt_state_manager(void)
{
	// Check if 10 seconds have passed since the last state check

	if (GetTickCounter(&l_t_bt_stat_polling_timer_8s) >= 8000)
	{
		// Restart the 10-second timer
		if (bt_current_state != BT_STATE_SCANNING)
			bt_send_at_command("AT+A2DPSTAT\r\n");
		StartTickCounter(&l_t_bt_stat_polling_timer_8s);
	}

	// Handle actions based on the current Bluetooth connection state
	switch (bt_current_state)
	{
	case BT_STATE_DISCONNECTED:
		// Always start scanning when not connected
		if (GetTickCounter(&l_t_bt_conn_polling_timer_20s) >= 20 * 1000)
		{
			if (selected_best_dev.address[0] != '\0')
			{
				if (selected_best_dev.rssi > BT_SELECT_DISTANCE)
				{
					StopTickCounter(&l_t_bt_conn_polling_timer_20s);
					// bt_connect_selected_device();
				}
			}
		}

		if (GetTickCounter(&l_t_bt_scan_polling_timer_15s) >= 15 * 1000)
		{
			bt_start_scan();
			StartTickCounter(&l_t_bt_scan_polling_timer_15s);
		}
		else if (GetTickCounter(&l_t_bt_auto_link_wait_timer) >= 5 * 1000)
		{
			bt_start_scan();
			StopTickCounter(&l_t_bt_auto_link_wait_timer);
		}

		break;

	case BT_STATE_CONNECTING:
	case BT_STATE_CONNECTED:
	case BT_STATE_PLAYING:
		// Do nothing; connected or actively playing audio
		break;

	case BT_STATE_SCANNING:
		break;

	case BT_STATE_SCANNING_END:

		break;
	default:
		// Unknown state, ignore
		break;
	}
}

bool bt_parse_state_response(const char *resp)
{
	if (strncmp(resp, "+A2DPSTAT=", 10) == 0)
	{
		int stat = atoi(&resp[10]);
		switch (stat)
		{
		case BT_STATE_UNINIT:
		case BT_STATE_DISCONNECTED:
			bt_current_state = BT_STATE_DISCONNECTED;
			LOG_LEVEL("bt_current_state=BT_STATE_DISCONNECTED\r\n");
			return true;

		case BT_STATE_CONNECTING:
			bt_current_state = BT_STATE_CONNECTING;
			LOG_LEVEL("bt_current_state=BT_STATE_CONNECTING\r\n");
			return true;

		case BT_STATE_CONNECTED:
			bt_current_state = BT_STATE_CONNECTED;
			LOG_LEVEL("bt_current_state=BT_STATE_CONNECTED\r\n");
			// connect_start_time = 0;
			return true;

		case BT_STATE_PLAYING:
			bt_current_state = BT_STATE_PLAYING;
			// connect_start_time = 0;
			LOG_LEVEL("bt_current_state=BT_STATE_PLAYING\r\n");
			return true;
		}
	}

#if 0
	if (strncmp(resp, "+SCAN=S", 7) == 0)
	{
		bt_current_state = BT_STATE_SCANNING;
		//return true;
	}
	
	if (strncmp(resp, "+SCAN=E", 7) == 0)
	{
		bt_current_state = BT_STATE_SCANNING_END;
		bt_send_at_command("AT+A2DPSTAT\r\n");
		StartTickCounter(&l_t_bt_scan_polling_timer_15s);
		//return true;
	}
#endif
	return false;
}

/**
 * @brief Parse a Bluetooth scan result line and update the device list and best candidates.
 *
 * Example input format: "+SCAN=2,-23,413094F84E3F,KLD_A374,240408"
 * Format breakdown:
 *  - type: 2 = classic Bluetooth
 *  - rssi: signal strength (dBm)
 *  - address: 12-character hex string, no colons
 *  - name: Bluetooth device name
 *  - class_str: Bluetooth class hex string (e.g., 240408 for audio devices)
 *
 * @param line A null-terminated string containing a single scan result line.
 * @return true if the line was successfully parsed and added, false otherwise.
 */
bool bt_parse_scanning_line(const char *line)
{
	bt_device_t dev = {0};

	// Parse the scan result line
	if (sscanf(line, "+SCAN=%d,%d,%12[^,],%31[^,],%7s", &dev.type, &dev.rssi, dev.address, dev.name, dev.class_str) == 5)
	{
		// Add to the device list if there's room
		if (bt_device_count < MAX_BT_DEVICES)
		{
			bt_device_list[bt_device_count++] = dev;
		}

		// Debug print parsed device info
		LOG_LEVEL("bt scan %s %s %s %d %d \r\n", dev.name, dev.address, dev.class_str, dev.rssi, dev.type);

		// Check if it's an audio device (e.g., speaker/headset)
		if (bt_is_audio_device(&dev))
		{
			// Select best device with strongest RSSI
			if (dev.rssi > selected_best_dev.rssi)
			{
				selected_best_dev = dev;
				LOG_LEVEL("got a best bt device %s %d\r\n", dev.name, dev.rssi);
				if (dev.rssi > BT_SELECT_DISTANCE)
				{
					StartTickCounter(&l_t_bt_conn_polling_timer_20s);
				}
			}

			// Update last_best_dev only if RSSI is above a threshold and stronger
			if (dev.rssi > BT_PAIRED_DISTANCE && dev.rssi > last_best_dev.rssi)
			{
				last_best_dev = dev;
				LOG_LEVEL("got a new bt device %s %d\r\n", dev.name, dev.rssi);
				bt_connect_last_device();
			}
		}

		return true;
	}

	return false;
}

// Connect to a device by name match
void bt_connect_by_name(const char *target_name)
{
	for (int i = 0; i < bt_device_count; i++)
	{
		if (strcmp(bt_device_list[i].name, target_name) == 0)
		{
			LOG_LEVEL("bt connecte to %s\r\n", target_name);
			bt_send_at_command("AT+A2DPCONN=%s\r\n", bt_device_list[i].address);
			last_best_dev = bt_device_list[i];
			last_best_dev.address[12] = '\0';
			return;
		}
	}
}

void bt_connect_last_device(void)
{
	if (last_best_dev.address[0] != '\0')
	{
		LOG_LEVEL("bt connecte to %s %s\r\n", last_best_dev.name, last_best_dev.address);
		bt_send_at_command("AT+A2DPCONN=%s\r\n", last_best_dev.address);
	}
}

void bt_connect_selected_device(void)
{
	if (selected_best_dev.address[0] != '\0')
	{
		LOG_LEVEL("bt connecte to %s\r\n", selected_best_dev.name);
		bt_send_at_command("AT+A2DPCONN=%s\r\n", selected_best_dev.address);
	}
}

// Connect to a device by index
void bt_connect_by_index(int index)
{
	if (index >= 0 && index < bt_device_count)
	{
		bt_send_at_command("AT+A2DPCONN=%s\r\n", bt_device_list[index].address);
	}
}

// For debugging: print the device list via UART
void bt_list_devices(void)
{
	for (int i = 0; i < bt_device_count; i++)
	{
		LOG_LEVEL("[%d] %s (%s) RSSI=%d\n", i,
				  bt_device_list[i].name,
				  bt_device_list[i].address,
				  bt_device_list[i].rssi);
	}
}

void bt_clear_device_list(void)
{
	// memset(bt_device_list, 0, sizeof(bt_device_t));
	bt_device_count = 0;
	// memset(&selected_best_dev, 0, sizeof(bt_device_t));
	selected_best_dev.address[0] = '\0';
	selected_best_dev.rssi = -128;

	last_best_dev.address[0] = '\0';
	last_best_dev.rssi = -128;
	bt_current_state = BT_STATE_DISCONNECTED;
}

void bt_send_at_command(const char *format, ...)
{
	char cmd_buf[64];
	va_list args;

	va_start(args, format);
	uint8_t length = vsnprintf(cmd_buf, sizeof(cmd_buf), format, args);
	va_end(args);

#ifdef TASK_MANAGER_STATE_MACHINE_UPF
	if (length > 0 && length < sizeof(cmd_buf))
	{
		LOG_LEVEL("bt send at cmd: %s\r\n", cmd_buf);
		// UART1_Send_Buffer((uint8_t *)cmd_buf, length);
		upf_send_buffer(upf_module_info_BT_MUSIC, (uint8_t *)cmd_buf, length);
	}
#endif
}

bool bt_parser_handler(upf_proc_buff_t *upf_proc_buff)
{
	const uint8_t *src = upf_proc_buff->buffer;
	size_t total_len = upf_proc_buff->size;

	char line_buf[256] = {0};
	size_t line_len = 0;

	for (size_t i = 0; i < total_len; ++i)
	{
		char ch = (char)src[i];

		// Skip invalid leading characters (non-printable, except \r and \n)
		if ((unsigned char)ch < 0x20 && ch != '\r' && ch != '\n')
		{
			continue;
		}

		// Append character to current line buffer if space permits
		if (line_len < sizeof(line_buf) - 1)
		{
			line_buf[line_len++] = ch;
		}

		// End of line detected
		if (ch == '\n')
		{
			line_buf[line_len] = '\0'; // Null-terminate the string

			// Remove leading whitespace characters
			char *line = line_buf;
			while (*line == '\r' || *line == '\n' || *line == ' ')
			{
				++line;
			}

			// Only process lines starting with "AT" or "+"
			if (strncmp(line, "AT", 2) == 0 || strncmp(line, "+", 1) == 0)
			{
				bool ret = false;
				ret = bt_parse_state_response(line); // Parse general AT response
				if (!ret)
					ret = bt_parse_scanning_line(line); // Parse Bluetooth scan line if any
				// ret = bt_parse_connected_line(line);
				if (!ret)
					LOG_LEVEL("%s\r\n", line);
			}

			// Reset line buffer for the next line
			line_len = 0;
			memset(line_buf, 0, sizeof(line_buf));
		}
	}

	return true;
}

static bool strcasestr_custom(const char *haystack, const char *needle)
{
	if (!haystack || !needle)
		return false;

	size_t needle_len = strlen(needle);
	for (; *haystack; ++haystack)
	{
		if (strncasecmp(haystack, needle, needle_len) == 0)
			return true;
	}
	return false;
}

bool bt_is_audio_device(const bt_device_t *dev)
{
	if (dev == NULL)
		return false;

	const char *audio_keywords[] = {
		"JBL", "SPK", "SPEAKER", "BOSE", "SONY", "Marshall", "Harman", "Soundcore",
		"Headset", "Headphone", "Earbuds", "AirPods", "Buds", "FreeBuds", "Neckband"};

	// audio/video wearable/headset/speaker ?
	if (strcmp(dev->class_str, "240408") == 0 ||
		strcmp(dev->class_str, "240404") == 0 ||
		strcmp(dev->class_str, "240414") == 0)
	{
		return true;
	}

	if ((dev->type & 0x1F00) == 0x0400)
	{
		return true;
	}

	if (dev->type == 2)
	{
		return true;
	}

	for (int i = 0; i < sizeof(audio_keywords) / sizeof(audio_keywords[0]); ++i)
	{
		if (strcasestr_custom(dev->name, audio_keywords[i]))
		{
			return true;
		}
	}
	return false;
}

bool bt_receive_handler(upf_proc_buff_t *upf_proc_buff)
{
	bt_parser_handler(upf_proc_buff);
	return true;
}

#endif
