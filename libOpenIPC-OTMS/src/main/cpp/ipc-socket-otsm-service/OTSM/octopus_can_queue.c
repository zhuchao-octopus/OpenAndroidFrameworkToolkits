/*
 * can_msg_queue.c
 *
 * Created on: July 14, 2020
 *     Author: xiaolb
 *
 * Description:
 * This source file implements a fixed-size circular queue to store CAN messages.
 * It is typically used in embedded systems where CAN messages are received via interrupt
 * and need to be processed asynchronously in the main loop or task thread.
 */
#include "octopus_can_queue.h"

#if 1 //def TASK_MANAGER_STATE_MACHINE_CAN

CanQueue_t can_rx_msg_queue;

/**
 * @brief Initializes the CAN queue by resetting the head and tail pointers.
 *
 * @param queue Pointer to the CAN queue structure.
 */
void Can_Queue_Init(CanQueue_t *queue)
{
    queue->head = 0;
    queue->tail = 0;
}

/**
 * @brief Calculates the number of messages currently in the queue.
 *
 * @param queue Pointer to the CAN queue structure.
 * @return Number of messages in the queue.
 */
uint16_t CanQueue_Length(CanQueue_t *queue)
{
    return (queue->tail + CAN_MSG_QUEUE_SIZE - queue->head) & CAN_MSG_QUEUE_SIZE_MASK;
}

/**
 * @brief Pushes a CAN message into the queue if there is available space.
 *
 * This function copies the message data into the internal queue buffer and updates the tail index.
 * It does not handle concurrent access protection — external synchronization is recommended if used in interrupt context.
 *
 * @param queue Pointer to the CAN queue structure.
 * @param channel CAN channel index (e.g., 0 for CAN1).
 * @param id CAN message ID.
 * @param data Pointer to the message payload (array of 8 or fewer bytes).
 * @param data_len Length of the message data (0 ~ 8).
 * @return 0 if push was successful, 1 if the queue is full.
 */
uint8_t CanQueue_Push(CanQueue_t *queue, uint8_t channel, uint32_t std_id, const uint8_t *data, uint8_t data_len)
{
    // Check if the queue is not full
    if (CanQueue_Length(queue) < CAN_MSG_QUEUE_SIZE)
    {
        uint8_t i;
        // Write message fields
        queue->msg[queue->tail].channel = channel;
        queue->msg[queue->tail].std_id = std_id;
        queue->msg[queue->tail].data_len = data_len;

        // Copy data bytes
        for (i = 0; i < data_len; i++)
        {
            queue->msg[queue->tail].data[i] = data[i];
        }

        // Advance tail index (with wrap-around using mask)
        queue->tail = (queue->tail + 1) & CAN_MSG_QUEUE_SIZE_MASK;
        return 0; // Success
    }

    return 1; // Queue is full
}

/**
 * @brief Pops the oldest CAN message from the queue if available.
 *
 * This function copies the message at the head of the queue to the user-provided structure,
 * then advances the head index. It does not handle concurrent access protection — external synchronization is recommended.
 *
 * @param queue Pointer to the CAN queue structure.
 * @param msg Pointer to the structure where the popped message will be stored.
 * @return 1 if a message was popped successfully, 0 if the queue was empty.
 */
uint8_t CanQueue_Pop(CanQueue_t *queue, CanQueueMsg_t *msg)
{
    // Check if the queue is not empty
    if (CanQueue_Length(queue) > 0)
    {
        *msg = queue->msg[queue->head];
        // Advance head index (with wrap-around using mask)
        queue->head = (queue->head + 1) & CAN_MSG_QUEUE_SIZE_MASK;
        return 1; // Success
    }

    return 0; // Queue is empty
}

CanQueueMsg_t *Can_GetMsg(void)
{
    static CanQueueMsg_t s_msg;
    if (CanQueue_Pop(&can_rx_msg_queue, &s_msg))
    {
        return &s_msg;
    }
    return NULL;
}

uint16_t Can_GetMsgQueueSize(void)
{
    return CanQueue_Length(&can_rx_msg_queue);
}

#endif
