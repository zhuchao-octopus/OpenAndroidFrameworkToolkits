/**
 * ****************************************************************************
 * @file octopus_4G.c
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
#include "octopus_4g.h"
#include "octopus_task_manager.h"   // Task Manager: handles scheduling and execution of system tasks
#include "octopus_tickcounter.h"    // Tick Counter: provides timing and delay utilities
#include "octopus_message.h"        // Message IDs: defines identifiers for inter-task communication
#include "octopus_msgqueue.h"       // Message Queue: API for sending/receiving messages between tasks
#include "octopus_uart_ptl.h"       // UART Protocol Layer: handles protocol-level UART operations
#include "octopus_uart_upf.h"       // UART Packet Framework: low-level UART packet processing
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#ifdef TASK_MANAGER_STATE_MACHINE_LOT4G
#define IOT_DST_ID 0x21
#define MCU_SRC_ID 0x01

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
upf_module_t upf_module_info_LOT4G = {UPF_MODULE_ID_LOT4G, UPF_CHANNEL_8, UPF_CHANNEL_TYPE_BYTE};
static uint32_t l_t_msg_wait_10_timer = 0;

// static uint32_t l_t_msg_ble_polling_timer_1s = 0;
// static uint32_t l_t_msg_ble_pair_wait_timer = 0;
// static uint32_t l_t_msg_ble_lock_wait_timer = 0;

static bool LOT4G_receive_handler(upf_proc_buff_t *upf_proc_buff);
// static bool module_send_handler(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint16_t param, ptl_proc_buff_t *buff);
// static bool module_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */
void task_4g_init_running(void)
{
    OTMS(TASK_MODULE_4G, OTMS_S_INVALID);

    LOG_LEVEL("task_4g_init_running\r\n");
    // com_uart_ptl_register_module(MSGMODULE_SYSTEM, module_send_handler, module_receive_handler);
}

void task_4g_start_running(void)
{
    LOG_LEVEL("task_4g_start_running\r\n");
    upf_register_module(upf_module_info_LOT4G, LOT4G_receive_handler);
    OTMS(TASK_MODULE_4G, OTMS_S_ASSERT_RUN);
}

void task_4g_assert_running(void)
{
    StartTickCounter(&l_t_msg_wait_10_timer);
    OTMS(TASK_MODULE_4G, OTMS_S_RUNNING);
}

void task_4g_running(void)
{
    if (GetTickCounter(&l_t_msg_wait_10_timer) < 10)
        return;
    StartTickCounter(&l_t_msg_wait_10_timer);

    Msg_t *msg = get_message(TASK_MODULE_4G);
    if (msg->msg_id == NO_MSG)
        return;
}

void task_4g_post_running(void)
{
    OTMS(TASK_MODULE_4G, OTMS_S_ASSERT_RUN);
}

void task_4g_stop_running(void)
{
    LOG_LEVEL("_stop_running\r\n");
    OTMS(TASK_MODULE_4G, OTMS_S_INVALID);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t calc_checksum(uint8_t *data, uint16_t len)
{
    uint32_t sum = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(sum & 0xFF);
}

void iot_send_cmd(uint8_t cmd, uint8_t *data, uint8_t len)
{
    uint8_t frame[260];
    uint8_t index = 0;

    frame[index++] = IOT_DST_ID;
    frame[index++] = MCU_SRC_ID;
    frame[index++] = cmd;
    frame[index++] = len;

    for (uint8_t i = 0; i < len; i++)
    {
        frame[index++] = data[i];
    }

    frame[index++] = calc_checksum(&frame[1], 3 + len);
    frame[index++] = 0xAA;

    upf_send_buffer(upf_module_info_LOT4G,frame, index);
}

void iot_cmd_read_runtime_params(void)
{
    iot_send_cmd(0x82, NULL, 0);
}

void iot_cmd_read_system_params(void)
{
    iot_send_cmd(0x81, NULL, 0);
}

void iot_cmd_clear_total_mileage(uint8_t *password, uint8_t pwd_len)
{
    iot_send_cmd(0x8E, password, pwd_len);
}

void iot_cmd_reset_to_factory(uint8_t *password, uint8_t pwd_len)
{
    iot_send_cmd(0x86, password, pwd_len);
}

void iot_cmd_set_sense_unlock(uint8_t enable)
{
    iot_send_cmd(0xA0, &enable, 1);
}

void iot_parse_frame(uint8_t *frame, uint16_t length)
{
    LOG_BUFF_LEVEL(frame, length);

    if (length < 6 || frame[length - 1] != 0xAA)
        return;
    uint8_t calc = calc_checksum(&frame[1], length - 3);
    if (calc != frame[length - 2])
        return;

    // uint8_t dst  = frame[0];
    // uint8_t src  = frame[1];
    uint8_t cmd = frame[2];
    // uint8_t dlen = frame[3];
    // uint8_t* data = &frame[4];

    LOG_BUFF_LEVEL(frame, length);

    switch (cmd)
    {
    case 0x82:
        break;
    case 0x81:
        break;
    case 0x8E:
        break;
    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool LOT4G_receive_handler(upf_proc_buff_t *upf_proc_buff)
{
    return false;
}

#endif
