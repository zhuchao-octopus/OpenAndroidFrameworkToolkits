/*******************************************************************************
 * @file octopus_task_manager_upf.c
 * @brief   Unified Protocol Framework (UPF) - Task Manager Module
 *
 * @details
 * This source file is part of the Octopus Unified Protocol Framework (UPF).
 * It provides task scheduling, management, and dispatching for multiple
 * communication protocols running on MCU-based systems.
 *
 * Key Responsibilities:
 *   - Initialize and register protocol tasks (e.g., CAN, UART, BLE, etc.)
 *   - Manage task queues, priorities, and execution timing
 *   - Provide an abstraction layer for protocol-independent task handling
 *   - Enable concurrent execution and resource sharing between multiple protocols
 *
 * Features:
 *   - Task registration with callback functions
 *   - Event-driven task triggering
 *   - Unified scheduling mechanism across protocols
 *   - Lightweight design suitable for embedded MCUs (low memory footprint)
 *
 * Typical Usage:
 *   - Each protocol module registers its task(s) via UPF task manager API
 *   - The UPF scheduler dispatches tasks based on events, timers, or priorities
 *   - Developers can extend or integrate new protocols without modifying
 *     the core scheduler
 *
 * File Relation:
 *   - Part of UPF core modules
 *   - Works together with: octopus_task_manager_upf.h
 *   - Interfaces with protocol adapters and low-level drivers
 *
 * @note
 * This module abstracts task management from the application and protocol layers,
 * providing a consistent execution model across heterogeneous communication stacks.
 *
 * @version 1.0.0
 * @author   Octopus Team
 * @date     2024-12-12
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_uart_upf.h" // Include UART protocol header
#include "octopus_uart_hal.h" // Include UART hardware abstraction layer header
#include "octopus_task_manager.h" // Include task manager for scheduling tasks
#include "octopus_tickcounter.h" // Include tick counter for timing operations
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
 
// #define TEST_LOG_DEBUG_UPF_RX_FRAME // Enable debugging for receiving frames
// #define TEST_LOG_DEBUG_PTL_TX_FRAME // Enable debugging for transmitting frames

#ifdef TASK_MANAGER_STATE_MACHINE_UPF
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
// extern upf_module_info_t upf_module_info[];
static upf_module_info_t *upf_module_info = NULL;
static size_t s_upf_module_max = 0;
/*******************************************************************************
 * LOCAL FUNCTIONS DECLARATION
 */
// Declare function prototypes for various tasks and processing functions

static void upf_proc_valid_frame(void); // Process the valid frame
static void upf_rx_event_message_handler(void);

/*******************************************************************************
 * STATIC VARIABLES
 */
static uint32_t l_t_ptl_rx_main_timer;
static uint8_t upf_next_empty_module = 0;

/*******************************************************************************
 * GLOBAL FUNCTIONS IMPLEMENTATION
 */

void otsm_upf_help(void)
{
    upf_print_registered_module();
}

void otsm_upf_init(upf_module_info_t *array, uint16_t length)
{
    if (array == NULL || length == 0)
        return;
    upf_module_info = array;
    s_upf_module_max = length;
}

void upf_register_module(upf_module_t upf_module, upf_module_receive_handler_t receive_handler)
{
    if (upf_next_empty_module < s_upf_module_max)
    {
        upf_module_info[upf_next_empty_module].upf_module.channel = upf_module.channel;
        upf_module_info[upf_next_empty_module].upf_module.id = upf_module.id;

        upf_module_info[upf_next_empty_module].receive_handler = receive_handler;

        cFifo_Init(&upf_module_info[upf_next_empty_module].upf_usart_rx_fifo,
                   upf_module_info[upf_next_empty_module].upf_usart_rx_fifo_buff,
                   sizeof(upf_module_info[upf_next_empty_module].upf_usart_rx_fifo_buff));

        upf_next_empty_module++;
    }
}

upf_module_info_t *upf_get_module(upf_module_t upf_module)
{
    upf_module_info_t *module_info = NULL;

    for (uint8_t i = 0; i < upf_next_empty_module; i++)
    {
        if (upf_module_info[i].upf_module.channel == upf_module.channel &&
            upf_module_info[i].upf_module.id == upf_module.id)
        {
            module_info = &upf_module_info[i];
            break;
        }
    }

    return module_info;
}

void upf_print_registered_module(void)
{
    // module_info_t *module_info = NULL;
    for (uint8_t i = 0; i < upf_next_empty_module; i++)
    {
        LOG_LEVEL("registered upf_module_info[%d]=%02x \r\n", i, upf_module_info[i].upf_module.channel);
    }
}

// Initialize UART communication for the task
void task_upf_init_running(void)
{
    LOG_LEVEL("task_upf_init_running\r\n");
    OTMS(TASK_MODULE_UPF, OTMS_S_INVALID);
}

// Start the UART communication for the task
void task_upf_start_running(void)
{
    LOG_LEVEL("task_upf_start_running\r\n");
    OTMS(TASK_MODULE_UPF, OTMS_S_ASSERT_RUN);
}

// Assert that UART communication is running
void task_upf_assert_running(void)
{
    StartTickCounter(&l_t_ptl_rx_main_timer);
    // StartTickCounter(&l_t_ptl_tx_main_timer);
    // StartTickCounter(&l_t_ptl_error_detect_timer);
    OTMS(TASK_MODULE_UPF, OTMS_S_RUNNING);
}

// Main running function for UART communication
void task_upf_running(void)
{
    // if(GetTickCounter(&l_t_ptl_rx_main_timer) < 10)
    //	return;
    // StartTickCounter(&l_t_ptl_rx_main_timer);
    upf_rx_event_message_handler();
    upf_proc_valid_frame();
}

// Post-running function for UART communication
void task_upf_post_running(void)
{
    OTMS(TASK_MODULE_UPF, OTMS_S_ASSERT_RUN);
}

// Stop the UART communication task
void task_upf_stop_running(void)
{
    LOG_LEVEL("_stop_running\r\n");
    OTMS(TASK_MODULE_UPF, OTMS_S_INVALID);
}

void upf_receive_callback(upf_module_t upf_module, const uint8_t *buffer, uint16_t length)
{
#ifdef TEST_LOG_DEBUG_UART_RX_DATA
    LOG_BUFF_LEVEL(buffer, length);
#endif

#if 1
    upf_module_info_t *upf_module_infor = upf_get_module(upf_module);
    if (upf_module_infor != NULL)
    {
        for (uint8_t j = 0; j < length; j++)
        {
            cFifo_Push(upf_module_infor->upf_usart_rx_fifo, buffer[j]);
        }
    }
#endif
}

uint8_t upf_get_fifo_data(cFifo_t *a_ptFifo, uint8_t *buffer, uint16_t length)
{
    uint8_t data = 0;  // Variable to hold each byte read from the FIFO
    uint8_t index = 0; // Index to track how many bytes have been stored in the buffer

    // Get the current size of the FIFO (number of available bytes to read)
    // uint8_t datasize = cFifo_DataSize(ptl_1_usart_rx_fifo);
    // if(datasize <= 0) return index;
    // Loop to read data from FIFO until we either fill the buffer or run out of data
    while (1)
    {
        // If we haven't reached the desired length and there's still data in the FIFO
        if (index < length)
        {
            // Try to pop a byte from the FIFO
            if (true == cFifo_Pop(a_ptFifo, &data))
            {
                // Store the byte in the provided buffer
                buffer[index] = data;
                index++; // Increment the index to store the next byte
            }
            else
            {
                // If no more data is available in the FIFO, exit the loop
                break;
            }
        }
        else
        {
            // If we've already read the desired number of bytes, exit the loop
            break;
        }
    }

    // Return the number of bytes actually read from the FIFO
    return index;
}

/**
 * Handler for receiving UART data byte-by-byte.
 * Calls the UART reception handler to store the received data.
 */
void upf_rx_event_message_handler(void)
{
    uint8_t count = 0;
    // while (1)
    for (uint8_t i = 0; i < upf_next_empty_module; i++) // handle all modules
    {
        upf_module_info_t *module_info = &upf_module_info[i];
        if (module_info->upf_proc_buff.size >= UPF_FRAME_MAX_SIZE)
        { // upf_proc_buff is full must to be processed first before get from fifo
            continue;
        }

        if (module_info->upf_module.type == UPF_CHANNEL_TYPE_CHAR)
        {
            // For AT command module: Read until we get a '\n' or fill the buffer
            while (cFifo_HasLine(module_info->upf_usart_rx_fifo))
            {
                if (module_info->upf_proc_buff.size >= UPF_FRAME_MAX_SIZE / 2)
                    break; // wait for processing
                uint8_t byte = 0;
                count = upf_get_fifo_data(module_info->upf_usart_rx_fifo, &byte, 1);
                if (count > 0)
                {
                    module_info->upf_proc_buff.buffer[module_info->upf_proc_buff.size++] = byte;
                    // Stop when reaching end of an AT command line
                    if (byte == '\n')
                    {
                        module_info->upf_proc_buff.buffer[module_info->upf_proc_buff.size++] = '\0';
                        // LOG_LEVEL("%s",proc_buffer->buffer);
                    }
                }
            }
        }
        else
        {
            count = upf_get_fifo_data(
                module_info->upf_usart_rx_fifo,
                &module_info->upf_proc_buff.buffer[module_info->upf_proc_buff.size],
                UPF_FRAME_MAX_SIZE - module_info->upf_proc_buff.size);

            if (count > 0)
                module_info->upf_proc_buff.size = module_info->upf_proc_buff.size + count;
        }

    } // for
}

/**
 * Processes the valid frame after it has been extracted from the buffer.
 * The payload is passed to the appropriate module handler.
 */
void upf_proc_valid_frame(void)
{
    for (uint8_t i = 0; i < upf_next_empty_module; i++) // handle all modules
    {
        upf_module_info_t *module_info = &upf_module_info[i];

        if (module_info->receive_handler == NULL)
        {
            LOG_LEVEL("l_t_module_info[i].receive_handler is null.");
            continue;
        }
        if (module_info->upf_proc_buff.size == 0)
            continue;

        bool res = module_info->receive_handler(&(module_info->upf_proc_buff));
        if (res)
        {
            module_info->upf_proc_buff.size = 0;
        }
        else
        {
            if (module_info->upf_proc_buff.size > 14)
                module_info->upf_proc_buff.size = 0;
        }
    }
}

// uint8_t hal_com_uart0_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uartl_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uart2_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uart3_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uart4_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uart5_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uart6_send_buffer(const uint8_t *buffer, uint16_t length);
// uint8_t hal_com_uart7_send_buffer(const uint8_t *buffer, uint16_t length);

void upf_send_buffer(upf_module_t upf_module, const uint8_t *buffer, uint16_t length)
{
    if (buffer == NULL || length == 0)
    {
        return;
    }

    switch (upf_module.channel)
    {
    case UPF_CHANNEL_0:
        hal_com_uart0_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_1:
        hal_com_uartl_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_2:
        hal_com_uart2_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_3:
        hal_com_uart3_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_4:
        hal_com_uart4_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_5:
        hal_com_uart5_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_6:
        hal_com_uart6_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_7:
        hal_com_uart7_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_8:
        hal_com_uart8_send_buffer(buffer, length);
        break;

    case UPF_CHANNEL_9:
        hal_com_uart9_send_buffer(buffer, length);
        break;
    }
}

#endif
