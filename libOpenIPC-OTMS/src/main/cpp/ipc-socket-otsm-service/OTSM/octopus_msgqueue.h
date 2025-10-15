/*******************************************************************************
 * @file   octopus_task_manager_msg_queue.h
 * @brief  Header file for the message queue management in the Octopus Task Manager
 *
 * This header defines the data structures and function prototypes for managing
 * message queues in the Octopus Task Manager. The message queues are used for
 * communication between different task modules, allowing asynchronous message
 * passing to handle events, tasks, and other messages within the system.
 *
 * The following components are included:
 * - Message Queue Data Structure (`MsgQueue_t`): Holds the message queue for each task module.
 * - Message Structure (`Msg_t`): Represents a message containing an ID and two parameters.
 * - Function Prototypes: Functions to send, retrieve, and clear messages in the queues.
 *
 * @note    The maximum length of the queue is defined by `QUEUE_LENGTH`, and a special value
 *          `NO_MSG` is used to indicate an empty message state.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_MSG_QUEUE_H__
#define __OCTOPUS_TASK_MANAGER_MSG_QUEUE_H__

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h"         //  Base include file for the Octopus project.
#include "octopus_task_manager.h" // Include task manager for scheduling tasks
/*******************************************************************************
 * DEFINITIONS AND MACROS
 */

#define QUEUE_LENGTH 50 // Maximum size of the message queue
#define NO_MSG 0xFF     // Constant for indicating no message

/*******************************************************************************
 * MESSAGE STRUCTURE
 * Represents a message with an ID and two parameters.
 */
#pragma pack(1)
typedef struct
{
    uint16_t msg_id; // The message identifier
    uint16_t param1; // First parameter for the message
    uint16_t param2; // Second parameter for the message
} Msg_t;

/*******************************************************************************
 * MESSAGE QUEUE STRUCTURE
 * Represents the message queue for a specific task module.
 */
typedef struct
{
    uint8_t nEnque;            // Index for the next message to be enqueued
    uint8_t nDeque;            // Index for the next message to be dequeued
    Msg_t queue[QUEUE_LENGTH]; // Array to store messages in the queue
} MsgQueue_t;
#pragma pack()

#ifdef __cplusplus
extern "C"
{
#endif
    /*******************************************************************************
     * FUNCTION PROTOTYPES
     * Functions for managing the message queues
     */
    void otsm_message_queue_init(void); // Initialize all message queues for the task manager

    Msg_t *get_message(TaskModule_t task_module);                                                   // Retrieve a message from the task module's queue
    void send_message(TaskModule_t task_module, uint16_t msg_id, uint16_t param1, uint16_t param2); // Send a message to the task module's queue

    void clear_message(TaskModule_t task_module); // Clear the task module's message queue

    void send_message_adapter(uint16_t task_module, uint16_t msg_id, uint16_t param1, uint16_t param2); // Send a message to the task module's queue
#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_MSG_QUEUE_H__ */
