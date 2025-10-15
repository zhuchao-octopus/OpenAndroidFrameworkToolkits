/**
 * @file    octopus_uart_hal.c
 * @brief   UART HAL (Hardware Abstraction Layer) implementation for Octopus platform.
 *          This file provides UART initialization, data transmission, and event handling
 *          for both CST OSAL RTOS and ITE OPEN RTOS platforms.
 * @date    2020
 * @copyright Copyright (c) 2020, CHIPSEA Co., Ltd.
 */

/*********************************************************************
 * INCLUDES
 * Includes necessary headers for UART functionality and platform-specific configurations.
 */
#include "octopus_uart_hal.h"
#include "octopus_cfifo.h"
#include "octopus_platform.h"
/*******************************************************************************
 * DEBUG SWITCH MACROS
 * Enable debug logging for specific events during UART operations.
 */
// #define TEST_LOG_DEBUG_UART_RX_DATA
// #define TEST_LOG_DEBUG_UART_TX_DATA
/*******************************************************************************
 * MACROS
 * Define commonly used constants and values for UART operations.
 */
// #define PI_FLOAT (3.14159f)  // Example constant for mathematical computations.
#define FIFO_BUFFER_MAX_SIZE 256
/*******************************************************************************
 * GLOBAL VARIABLES
 * Variables that are accessible throughout the file or program.
 */
uint8_t Hal_TaskID; // Task ID for UART event handling.

// #ifdef PLATFORM_CST_OSAL_RTOS
// com_uart_data_buff_t com_uart_data_buff;            // Buffer structure for received UART data.
// #endif

#ifdef PLATFORM_ITE_OPEN_RTOS
static bool UartIsInit = false;          // Tracks whether UART is initialized.
static sem_t UartSemIntr;                // Semaphore for UART interrupt handling.
uint32_t com_uart_parity_error = 0;      // Placeholder for parity error tracking.
com_uart_data_buff_t com_uart_data_buff; // Buffer structure for received UART data.
#endif

static cFifo_t *ptl_1_usart_rx_fifo = NULL;
static uint8_t ptl_1_usart_rx_fifo_buff[cFifo_ObjSize(FIFO_BUFFER_MAX_SIZE)];

/*******************************************************************************
 * LOCAL FUNCTIONS DECLARATION
 * Declare static functions used only within this file.
 */
#ifdef PLATFORM_CST_OSAL_RTOS
static void hal_com_uart_receive_callback(uart_Evt_t *pev);
#elif defined(PLATFORM_ITE_OPEN_RTOS)
static void hal_com_uart_receive_callback(void *arg1, uint32_t arg2);
#elif defined(PLATFORM_LINUX_RISC)
static SerialPortHandle linux_uart_serial_handle = NULL;
static void hal_com_uart_receive_callback(const uint8_t *data, int length);
#else
// static void hal_com_uart_receive_callback(const uint8_t *data, int length);
#endif

#ifndef PLATFORM_LINUX_RISC
#ifdef TEST_LOG_DEBUG_UART_TX_DATA
static void *linux_uart_serial_handle = NULL;
#endif
#endif

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */

#ifdef PLATFORM_CST_OSAL_RTOS

void dbg_log_printf_init(void)
{
    uart_Cfg_t cfg = {
        .tx_pin = P9,
        .rx_pin = P10,
        .baudrate = 115200,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity = FALSE,
        .evt_handler = NULL,
    };
    HalUartInit(UART0, cfg); // uart init
}

static void ptl_uart_init(void)
{
    uart_Cfg_t cfg = {
        .tx_pin = P26,
        .rx_pin = P27,
        .baudrate = 115200,
        .use_fifo = TRUE,
        .hw_fwctrl = FALSE,
        .use_tx_buf = FALSE,
        .parity = FALSE,
        .evt_handler = hal_com_uart_receive_callback,
    };
    // Initialize UART1 with the specified configuration.
    HalUartInit(UART1, cfg);
    cFifo_Init(&ptl_1_usart_rx_fifo, ptl_1_usart_rx_fifo_buff, sizeof(ptl_1_usart_rx_fifo_buff));
}
/**
 * @brief   Initializes the UART hardware for CST OSAL RTOS platform.
 * @param   task_id  Task identifier for UART event handling.
 * @return  None.
 */

void hal_uart_init(uint8 task_id)
{
    Hal_TaskID = task_id;
    if (task_id == 0)
    {
        LOG_LEVEL("hal uart1 init for protocol\r\n");
    }
    ptl_uart_init();
}
/**
 * @brief   Callback function triggered when UART events occur.
 * @param   pev  Pointer to the UART event structure.
 * @return  None.
 */
static void hal_com_uart_receive_callback(uart_Evt_t *pev)
{
    switch (pev->type)
    {
    case UART_EVT_TYPE_RX_DATA:    // Data received event.
    case UART_EVT_TYPE_RX_DATA_TO: // Timeout on data reception.
    {
        uint16_t i;
        for (i = 0; i < pev->len; i++)
        {
            /// com_uart_data_buff.data[(com_uart_data_buff.wr + i) & (UART_BUFF_MAX_SIZE - 1)] = pev->data[i];
            cFifo_Push(ptl_1_usart_rx_fifo, pev->data[i]);
        }
        /// com_uart_data_buff.wr += pev->len;

        /// Start a timer to handle received data.
        /// osal_start_timerEx(Hal_TaskID, UART_RECEIVE_DATA_EVENT, 5);
    }
    break;

    case UART_EVT_TYPE_TX_COMPLETED: // Transmission complete event.
        LOG_LEVEL("tx completed\r\n");
        break;
    }
}

/**
 * @brief   Handles UART events for CST OSAL RTOS platform.
 * @param   task_id  Task identifier for UART event handling.
 * @param   events   Event flags indicating the specific events to handle.
 * @return  Remaining unhandled events.
 */
uint16_t hal_com_uart_event_handler(uint8_t task_id, uint16 events)
{
    if (task_id != Hal_TaskID)
        return 0;

    if (events & UART_RECEIVE_DATA_EVENT)
    {
#ifdef TEST_LOG_DEBUG_UART_RX_DATA
        LOG_LEVEL("Hal receive data:\r\n");
#endif
        return (events ^ UART_RECEIVE_DATA_EVENT);
    }

    return 0;
}

#elif defined(PLATFORM_ITE_OPEN_RTOS)
/**
 * @brief   Configures the USART peripheral for ITE OPEN RTOS platform.
 * @return  None.
 */
void hal_uart_init(uint8_t task_id)
{
    UART_OBJ *pUartInfo = (UART_OBJ *)malloc(sizeof(UART_OBJ));
    pUartInfo->port = PROTOCOL_UART_ITH_PORT;
    pUartInfo->parity = PROTOCOL_UART_PARITY;
    pUartInfo->txPin = PROTOCOL_UART_GPIO_TX;
    pUartInfo->rxPin = PROTOCOL_UART_GPIO_RX;
    pUartInfo->baud = PROTOCOL_UART_BAUDRATE;
    pUartInfo->timeout = 0;
    pUartInfo->mode = UART_INTR_MODE;
    pUartInfo->forDbg = false;

    // Register and initialize the UART device.
    itpRegisterDevice(PROTOCOL_UART_PORT, &PROTOCOL_UART_DEVICE);
    ioctl(PROTOCOL_UART_PORT, ITP_IOCTL_INIT, (void *)pUartInfo);
    ioctl(PROTOCOL_UART_PORT, ITP_IOCTL_REG_UART_CB, (void *)hal_com_uart_receive_callback);
    sem_init(&UartSemIntr, 0, 0);
    cFifo_Init(&ptl_1_usart_rx_fifo, ptl_1_usart_rx_fifo_buff, sizeof(ptl_1_usart_rx_fifo_buff));
    UartIsInit = true;

    pthread_t task_receive;
    pthread_attr_t attr_receive;
    pthread_attr_init(&attr_receive);
    LOG_LEVEL("hal uart2 init for protocol\r\n");
    pthread_create(&task_receive, &attr_receive, hal_com_uart_event_handler, NULL);
}

/**
 * @brief   Callback function for UART interrupt handling on ITE platform.
 * @param   arg1  Argument 1 (unused).
 * @param   arg2  Event-specific data.
 * @return  None.
 */
static void hal_com_uart_receive_callback(void *arg1, uint32_t arg2)
{
    sem_post(&UartSemIntr);
    if (arg2 == 1)
    {
        LOG_LEVEL("arg2=1 error\n");
        com_uart_parity_error = arg2;
    }
}

/**
 * @brief   UART event handler for ITE OPEN RTOS platform.
 * @param   arg  Pointer to handler argument (unused).
 * @return  None.
 */
void *hal_com_uart_event_handler(void *arg)
{
    uint8_t length = 0;
    while (1)
    {
        sem_wait(&UartSemIntr);
        length = 0;

        if (UartIsInit)
            length = read(PROTOCOL_UART_PORT, com_uart_data_buff.data, UART_BUFF_MAX_SIZE);
        if (length > 0)
        {
            for (int i = 0; i < length; i++)
            {
#ifdef TEST_LOG_DEBUG_UART_RX_DATA
                LOG_("%02x ", com_uart_data_buff.data[i]);
#endif
                cFifo_Push(ptl_1_usart_rx_fifo, com_uart_data_buff.data[i]);
            }
        }
    }
}

#elif defined(PLATFORM_LINUX_RISC)

void hal_uart_init(uint8_t task_id)
{
    cFifo_Init(&ptl_1_usart_rx_fifo, ptl_1_usart_rx_fifo_buff, sizeof(ptl_1_usart_rx_fifo_buff));
    linux_uart_serial_handle = serialport_create("/dev/ttyS3", 115200);
    if (!linux_uart_serial_handle)
    {
        LOG_LEVEL("Failed to open serial port.\r\n");
        return;
    }
    LOG_LEVEL("hal uart /dev/ttyS3 init for protocol successfully.\r\n");

    bool bret = serialport_set_callback(linux_uart_serial_handle, hal_com_uart_receive_callback);
    if (bret)
        LOG_LEVEL("open uart device /dev/ttyS3 successfully.\r\n");

    // uint8_t test_help[] = {0xaa, 0xf0, 0x00, 0x02, 0x64, 0x00, 0x00, 0x9c};
    // hal_com_uart_send_buffer(test_help, sizeof(test_help)); // for test
}

void *hal_com_uart_event_handler(void *arg)
{
}

static void hal_com_uart_receive_callback(const uint8_t *data, int length)
{
    uint16_t i;
#ifdef TEST_LOG_DEBUG_UART_RX_DATA
    LOG_BUFF_LEVEL(data, length);
#endif
    for (i = 0; i < length; i++)
    {
        cFifo_Push(ptl_1_usart_rx_fifo, data[i]);
    }
}

#else

void hal_uart_init(uint8_t task_id)
{
    LOG_LEVEL("hal uart init for ptl\r\n");
    cFifo_Init(&ptl_1_usart_rx_fifo, ptl_1_usart_rx_fifo_buff, sizeof(ptl_1_usart_rx_fifo_buff));
}

void hal_com_uart_receive_callback_ptl_1(const uint8_t *buffer, uint16_t length)
{
    uint16_t i;
#ifdef TEST_LOG_DEBUG_UART_RX_DATA
    LOG_LEVEL("Receive bytes:%d ", length);
    LOG_BUFF_LEVEL(buffer, length);
    /// LOG_NONE("\r\n");
#endif
    for (i = 0; i < length; i++)
    {
        cFifo_Push(ptl_1_usart_rx_fifo, buffer[i]);
    }
}

void *hal_com_uart_event_handler(void *arg)
{
    return NULL;
}

#endif // PLATFORM_ITE_OPEN_RTOS

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void otsm_uart_init(void)
{
    hal_uart_init(0);
}

/**
 * Reads data from the UART RX FIFO buffer into the provided buffer.
 * It will attempt to read up to `length` bytes from the FIFO.
 *
 * @param buffer Pointer to the buffer where the received data will be stored.
 * @param length Maximum number of bytes to read from the FIFO.
 *
 * @return The number of bytes actually received and copied into the buffer.
 */
uint8_t hal_com_uart_get_fifo_data_1(uint8_t *buffer, uint16_t length)
{
    uint8_t data = 0;  // Variable to hold each byte read from the FIFO
    uint8_t index = 0; // Index to track how many bytes have been stored in the buffer

    // Get the current size of the FIFO (number of available bytes to read)
    uint8_t datasize = cFifo_DataSize(ptl_1_usart_rx_fifo);
    // if(datasize <= 0) return index;
    // Loop to read data from FIFO until we either fill the buffer or run out of data
    // LOG_LEVEL("ptl_1_usart_rx_fifo  datasize=%d length= %d\r\n",datasize,length);
    while (1)
    {
        // If we haven't reached the desired length and there's still data in the FIFO
        if (index < length)
        {
            // Try to pop a byte from the FIFO
            if (true == cFifo_Pop(ptl_1_usart_rx_fifo, &data))
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
 * @brief   Sends a string over UART.
 * @param   str     Pointer to the string.
 * @param   length  Length of the string.
 * @return  Number of bytes sent.
 */
uint8_t hal_com_uart_send_string(const char *str, uint8_t length)
{
    uint8_t ret_code = length;
#ifdef PLATFORM_CST_OSAL_RTOS
    // osal_memcpy(com_uart_data_buff.data, str, strlen(str));
    ret_code = HalUartSendBuf(UART1, (uint8_t *)str, strlen(str));
#endif

#ifdef PLATFORM_ITE_OPEN_RTOS
    ret_code = write(PROTOCOL_UART_PORT, str, length);
#endif
#ifdef PLATFORM_LINUX_RISC
    if (linux_uart_serial_handle)
        ret_code = serialport_write(linux_uart_serial_handle, str, length);
#endif
    return ret_code;
}

/**
 * @brief   Sends a buffer of data over UART.
 * @param   buff    Pointer to the buffer.
 * @param   length  Length of the buffer.
 * @return  Number of bytes sent.
 */
uint8_t hal_com_uart0_send_buffer(const uint8_t *buffer, uint16_t length)
{
    uint8_t ret_code = 0;
#ifdef TEST_LOG_DEBUG_UART_TX_DATA
    if (linux_uart_serial_handle)
        LOG_LEVEL("buffer.length=%d linux_uart_serial_handle is %d\r\n", length, linux_uart_serial_handle);
    else
        LOG_LEVEL("buffer.length=%d linux_uart_serial_handle is null %d\r\n", length, (linux_uart_serial_handle == NULL) ? true : false);
#endif

#ifdef PLATFORM_CST_OSAL_RTOS
    ret_code = HalUartSendBuf(UART1, (uint8_t *)buffer, length);
#elif defined(PLATFORM_ITE_OPEN_RTOS)
    ret_code = write(PROTOCOL_UART_PORT, buff, length);
#elif defined(PLATFORM_LINUX_RISC)
    if (linux_uart_serial_handle)
    {
        ret_code = serialport_write(linux_uart_serial_handle, buffer, length);
    }
#elif defined(PLATFORM_STM32_RTOS)
    PTL_1_UART_Send_Buffer(buffer, length);
#else

#endif
    return ret_code;
}

uint8_t hal_com_uartl_send_buffer(const uint8_t *buffer, uint16_t length)
{
    uint8_t ret_code = length;
#ifdef TEST_LOG_DEBUG_UART_TX_DATA
    LOG_BUFF_LEVEL(buffer, length);
#endif

#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    UART1_Send_Buffer(buffer, length);
#endif
    return ret_code;
}

uint8_t hal_com_uart2_send_buffer(const uint8_t *buffer, uint16_t length)
{
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    // LOG_BUFF_LEVEL(buffer, length);
    UART2_Send_Buffer(buffer, length);
#endif
    return length;
}

uint8_t hal_com_uart3_send_buffer(const uint8_t *buffer, uint16_t length)
{
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    UART3_Send_Buffer(buffer, length);
#endif
    return length;
}

uint8_t hal_com_uart4_send_buffer(const uint8_t *buffer, uint16_t length)
{
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    UART4_Send_Buffer(buffer, length);
#endif
    return length;
}

uint8_t hal_com_uart5_send_buffer(const uint8_t *buffer, uint16_t length)
{
    return length;
}

uint8_t hal_com_uart6_send_buffer(const uint8_t *buffer, uint16_t length)
{
    return length;
}

uint8_t hal_com_uart7_send_buffer(const uint8_t *buffer, uint16_t length)
{
    return length;
}

uint8_t hal_com_uart8_send_buffer(const uint8_t *buffer, uint16_t length) // LPUART
{
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    LPUART_Send_Buffer(buffer, length);
#endif
    return length;
}

uint8_t hal_com_uart9_send_buffer(const uint8_t *buffer, uint16_t length) // LPUART
{
    return length;
}
