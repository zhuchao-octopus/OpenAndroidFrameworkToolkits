/**
 * ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 * Header file for the Octopus Task Manager module.
 * Defines macros, includes required libraries, and declares functions.
 * @version  1.0.0
 * @date 2024-12-12
 * @author   Octopus Team
 *****************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_UART_HAL_H__
#define __OCTOPUS_TASK_MANAGER_UART_HAL_H__

/*********************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

/*******************************************************************************
 * GENERAL MACROS
 * Define common bit manipulation macros and constants.
 */

// Maximum buffer size for UART data
#define UART_BUFF_MAX_SIZE 256

// Event identifier for UART receive data
#define UART_RECEIVE_DATA_EVENT 0x0001

// If the platform is ITE OpenRTOS, configure UART settings
#ifdef PLATFORM_ITE_OPEN_RTOS
#define PROTOCOL_UART_PORT ITP_DEVICE_UART2       // UART port used for communication
#define PROTOCOL_UART_ITH_PORT ITH_UART2          // ITH UART port for the platform
#define PROTOCOL_UART_DEVICE itpDeviceUart2       // UART device for the platform
#define PROTOCOL_UART_BAUDRATE CFG_UART2_BAUDRATE // Baud rate for UART communication
#define PROTOCOL_UART_PARITY CFG_UART2_PARITY     // Parity setting for UART communication
#define PROTOCOL_UART_GPIO_RX CFG_GPIO_UART2_RX   // GPIO pin for UART RX (receive)
#define PROTOCOL_UART_GPIO_TX CFG_GPIO_UART2_TX   // GPIO pin for UART TX (transmit)
#endif

typedef struct
{
    uint16_t wr;                      // Write index for data buffer
    uint16_t rd;                      // Read index for data buffer
    uint8_t data[UART_BUFF_MAX_SIZE]; // Data buffer
} com_uart_data_buff_t;

/*******************************************************************************
 * FUNCTIONS
 * Function prototypes for UART communication and protocol handling.
 */
#ifdef __cplusplus
extern "C"
{
#endif
    void otsm_uart_init(void);
    // Initializes the UART protocol (e.g., UART configuration, GPIO, etc.)
    void hal_uart_init(uint8_t task_id);
    // Sends a string via UART, returning the number of bytes sent
    uint8_t hal_com_uart_send_string(const char *str, uint8_t length);

    // Sends a buffer of data via UART, returning the number of bytes sent
    uint8_t hal_com_uart0_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uartl_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart2_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart3_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart4_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart5_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart6_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart7_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart8_send_buffer(const uint8_t *buffer, uint16_t length);
    uint8_t hal_com_uart9_send_buffer(const uint8_t *buffer, uint16_t length);

    // Reads data from the UART FIFO and stores it in the provided buffer
    uint8_t hal_com_uart_get_fifo_data_1(uint8_t *buffer, uint16_t length);
    void hal_com_uart_receive_callback_ptl_1(const uint8_t *buffer, uint16_t length);

#ifdef PLATFORM_CST_OSAL_RTOS
    uint16_t hal_com_uart_event_handler(uint8_t task_id, uint16 events);
#else
void *hal_com_uart_event_handler(void *arg);
#endif

    /*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif // __OCTOPUS_TASK_MANAGER_UART_HAL_H__
