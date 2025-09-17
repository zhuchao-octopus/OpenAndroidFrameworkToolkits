/*******************************************************************************
 * @file   octopus_task_manager_uart_hal.c
 * @brief  UART HAL (Hardware Abstraction Layer) for the Octopus Task Manager
 *
 * This file contains functions for managing message queues in the task manager,
 * enabling inter-task communication via message passing. It also handles UART
 * message reception and transmission, as well as initializing message queues
 * for each task module.
 *
 * Functions in this file include:
 * - `send_message()`: Sends a message to a task module's queue.
 * - `get_message()`: Retrieves a message from a task module's queue.
 * - `clear_message()`: Clears a task module's message queue.
 * - `message_queue_init()`: Initializes all message queues for the task manager.
 *
 * The message queues are circular in nature, supporting both full and empty
 * states to prevent data loss and allow efficient message handling. Each task
 * module is associated with its own message queue, allowing tasks to exchange
 * information asynchronously.
 *
 * @version 1.0
 * @date    2024-12-09
 *
 * @note    This implementation assumes a platform with a defined task ID enum
 *          and UART communication interfaces.
 *
 * @author  Your Name
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_msgqueue.h"     // Include message queue header for task communication
#include "octopus_task_manager.h" // Include task manager for scheduling tasks
/*******************************************************************************
 * LOCAL FUNCTIONS DECLARATION
 */
static MsgQueue_t g_msgQueue[TASK_MODULE_MAX_NUM]; // Declare an array of message queues, one for each task module

//***********************************************************************************************************//

/**
 * @brief Send a message to the specified task module's message queue.
 *
 * @param module   The task module to which the message is sent.
 * @param id       The message ID (identifier).
 * @param param1   The first parameter for the message.
 * @param param2   The second parameter for the message.
 */
void send_message(TaskModule_t task_module, uint16_t msg_id, uint16_t param1, uint16_t param2)
{
    Msg_t msg;
    uint8_t enque; // Index of where to enqueue the new message
    uint8_t deque; // Index of where to dequeue the message
    bool full;     // Flag to check if the queue is full

    // Prepare the message to be sent
    msg.msg_id = msg_id;
    msg.param1 = param1;
    msg.param2 = param2;

    // Get the current enqueue and dequeue positions, and check if the queue is full
    enque = g_msgQueue[task_module].nEnque & 0x7f; // Mask the 7 least significant bits for the enqueue index
    full = g_msgQueue[task_module].nEnque & 0x80;  // Check if the most significant bit indicates that the queue is full
    deque = g_msgQueue[task_module].nDeque & 0x7f; // Mask the 7 least significant bits for the dequeue index

    // If the queue is not full, enqueue the new message
    if (full == false)
    {
        g_msgQueue[task_module].queue[enque] = msg; // Store the message at the enqueue position
        enque++;                                    // Move the enqueue index forward

        if (enque >= QUEUE_LENGTH) // Wrap around to the beginning of the queue if we exceed the maximum length
        {
            enque = 0;
        }

        // If the enqueue index meets the dequeue index, the queue is full
        if (enque == deque)
        {
            full = true;
        }
    }
    else
    {
        /// LOG_("message queue full,module:%d\r\n", task_module);  // Log an error if the queue is full
    }

    // Update the full flag in the enqueue index
    if (full == true)
    {
        SetBit(enque, 7); // Set the most significant bit to indicate that the queue is full
    }

    // Update the message queue's enqueue and dequeue indices
    g_msgQueue[task_module].nEnque = enque;
    g_msgQueue[task_module].nDeque = deque;
}

/**
 * @brief Retrieve a message from the specified task module's message queue.
 *
 * @param module   The task module from which the message is retrieved.
 * @return Msg_t*  Pointer to the retrieved message, or a static message if no message is available.
 */
Msg_t *get_message(TaskModule_t task_module)
{
    static Msg_t s_msg; // Static message structure to return
    uint8_t enque;      // Current enqueue index
    uint8_t deque;      // Current dequeue index
    bool empty;         // Flag to check if the queue is empty

    s_msg.msg_id = NO_MSG; // Default to no message if the queue is empty

    // Get the current enqueue and dequeue positions, and check if the queue is empty
    enque = g_msgQueue[task_module].nEnque & 0x7f; // Mask the 7 least significant bits for the enqueue index
    deque = g_msgQueue[task_module].nDeque & 0x7f; // Mask the 7 least significant bits for the dequeue index
    empty = g_msgQueue[task_module].nDeque & 0x80; // Check if the most significant bit indicates that the queue is empty

    // If the queue is not empty, dequeue the next message
    if (empty == false)
    {
        s_msg = g_msgQueue[task_module].queue[deque]; // Retrieve the message from the queue
        deque++;                                      // Move the dequeue index forward

        if (deque >= QUEUE_LENGTH) // Wrap around to the beginning of the queue if we exceed the maximum length
        {
            deque = 0;
        }

        // If the enqueue index meets the dequeue index, the queue is empty
        if (enque == deque)
        {
            empty = true;
        }
    }

    // If the queue is empty, set the empty flag in the dequeue index
    if (empty == true)
    {
        SetBit(deque, 7); // Set the most significant bit to indicate that the queue is empty
    }

    // Update the message queue's enqueue and dequeue indices
    g_msgQueue[task_module].nEnque = enque;
    g_msgQueue[task_module].nDeque = deque;

    return &s_msg; // Return a pointer to the retrieved message
}

void send_message_adapter(uint16_t task_module, uint16_t msg_id, uint16_t param1, uint16_t param2)
{
    send_message((TaskModule_t)task_module, msg_id, param1, param2);
}
/**
 * @brief Clear the message queue for the specified task module.
 *
 * @param module   The task module whose message queue is cleared.
 */
void clear_message(TaskModule_t task_module)
{
    // Reset the enqueue and dequeue indices to initial values
    g_msgQueue[task_module].nEnque = 0x00; // Set enqueue index to 0, indicating the queue is not full
    g_msgQueue[task_module].nDeque = 0x80; // Set dequeue index to 0x80, indicating the queue is empty
}

/**
 * @brief Initialize the message queues for all task modules.
 */
void otsm_message_queue_init(void)
{
    uint8_t i;
    for (i = 0; i < TASK_MODULE_MAX_NUM; i++) // Iterate through all task modules
    {
        clear_message((TaskModule_t)i); // Clear the message queue for each module
    }
}
