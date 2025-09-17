// File: octopus_can.c
// Description: Implementation for CAN data dispatching and parsing logic
// Author: ak47
// Created: 2025-04-17

#include "octopus_task_manager.h" // Include task manager for scheduling tasks
#include "octopus_vehicle.h"
#include "octopus_gpio.h"
#include "octopus_system.h"

#include "octopus_uart_ptl.h"
#include "octopus_can.h"
#include "octopus_can_queue.h"
#include "octopus_can_2E006.h"

#ifdef TASK_MANAGER_STATE_MACHINE_CAN
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static uint32_t l_t_msg_wait_tx_timer = 0;

static bool can_send_handler(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff);
static bool can_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

static void can_rx_message_event_handler(void);
static void can_tx_message_event_handler(void);

/*****************************************************************************************
 * Global Function Implementations
 ****************************************************************************************/

/**
 * @brief Initializes the system for running.
 *
 * This function registers the system module with the communication layer
 * and transitions the system task to an invalid state.
 */
void task_can_init_running(void)
{
  LOG_LEVEL("task_can_init_running\r\n");
  ptl_register_module(MCU_TO_SOC_MOD_CAN, can_send_handler, can_receive_handler);
  Can_Queue_Init(&can_rx_msg_queue);
  OTMS(TASK_MODULE_CAN, OTMS_S_INVALID);
}

void task_can_start_running(void)
{
  LOG_LEVEL("task_can_start_running\r\n");
  OTMS(TASK_MODULE_CAN, OTMS_S_ASSERT_RUN);
}

void task_can_assert_running(void)
{
  ptl_reqest_running(MCU_TO_SOC_MOD_CAN);
  OTMS(TASK_MODULE_CAN, OTMS_S_RUNNING);
	StartTickCounter(&l_t_msg_wait_tx_timer);
}

void task_can_running(void)
{
	if(system_get_mb_state() == MB_POWER_ST_ON)
	{
    can_rx_message_event_handler();
	
	  can_tx_message_event_handler();
	}
}

void task_can_post_running(void)
{
  OTMS(TASK_MODULE_CAN, OTMS_S_ASSERT_RUN);
}

void task_can_stop_running(void)
{
  LOG_LEVEL("_stop_running\r\n");
  OTMS(TASK_MODULE_CAN, OTMS_S_INVALID);
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
static bool can_send_handler(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff)
{
  return false;
}

static bool can_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff)
{
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// Internal function to parse a received CAN message
void can_message_receiver(const CAN_Message_t *message)
{
  // Example: Check message ID and parse accordingly
  // LOG_BUFF_LEVEL((const uint8_t *)message, sizeof(CAN_Message_t));
  // LOG_BUFF_LEVEL((const uint8_t *)&message->Data,message->DLC);
  CanQueue_Push(&can_rx_msg_queue, 1, message->StdId, message->Data, message->DLC);
}

void can_rx_message_event_handler(void)
{
  uint16_t q_size = Can_GetMsgQueueSize();
  if (q_size > 0)
  {
    CanQueueMsg_t *msg = Can_GetMsg();
    if (NULL != msg)
    {
      can_message_dispatcher(msg);
    }
  }
}

void can_tx_message_event_handler(void)
{
	if (GetTickCounter(&l_t_msg_wait_tx_timer) >= 1000)
	{
     //can_message_sender(CAN_ID_BMS_TASK_H_001);
		 StartTickCounter(&l_t_msg_wait_tx_timer);
	}
}

#endif
