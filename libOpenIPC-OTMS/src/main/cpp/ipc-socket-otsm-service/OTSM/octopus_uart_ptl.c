/*******************************************************************************
 * @file octopus_task_manager_ptl.c
 * @brief C file for managing Octopus protocol tasks.
 *
 * This file defines the structure, constants, and function prototypes for handling
 * communication tasks between the MCU (Microcontroller Unit) and the APP (Application)
 * within the Octopus platform. It provides the necessary protocol definitions to
 * ensure smooth task management and communication across the platform.
 *
 * The Octopus protocol facilitates seamless interaction between hardware and software,
 * enabling efficient data exchange and task synchronization. This header file serves
 * as the interface for the task manager, providing the necessary tools to integrate
 * protocol handling into the Octopus platform.
 *
 * @version 1.0.0
 * @author   Octopus Team
 * @date     2024-12-12
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_uart_ptl.h"     // Include UART protocol header
#include "octopus_uart_hal.h"     // Include UART hardware abstraction layer header
#include "octopus_task_manager.h" // Include task manager for scheduling tasks
#include "octopus_tickcounter.h"  // Includes the header file that defines tick counter functions
#include "octopus_msgqueue.h"     // Include message queue header for task communication

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
// #define TEST_LOG_DEBUG_PTL_RX_FRAME // Enable debugging for receiving frames
// #define TEST_LOG_DEBUG_PTL_TX_FRAME // Enable debugging for transmitting frames

/*******************************************************************************
 * MACROS
 */
#define PTL_RUNNING_NONE 0 // Define no task running state
#define PTL_TX_BUF_NUM 4   // Define number of transmission buffers
#define PTL_RX_BUF_NUM 16  // Define number of reception buffers

#define PTL_TX_TIMEOUT 50 // Define timeout for TX operations (in ms)

#define PTL_FRAME_TYPE_REQ 1 // Define request frame type
#define PTL_FRAME_TYPE_ACK 0 // Define acknowledgment frame type

#define PTL_MODULE_MIN 1     // Define minimum module number
#define PTL_MODULE_MAX 126   // Define maximum module number
#define PTL_DATA_MAX_LEN 248 // Define maximum data length
#define PTL_RETRY_CNT_MAX 2  // Define maximum retry count for transmission

#define PTL_MODULE_SUPPORT_CNT 16 // Define number of supported modules

#define PTL_DEM_EID DEM_EID_INT_COM_ERROR // Define error event ID for communication errors

/*******************************************************************************
 * TYPEDEFS
 */

// Define a structure for module information, including frame type and send/receive handlers
typedef struct
{
    ptl_frame_type_t frame_type;              // Frame type (e.g., request or acknowledgment)
    module_send_handler_t send_handler;       // Send handler function for the module
    module_receive_handler_t receive_handler; // Receive handler function for the module
} module_info_t;

/*******************************************************************************
 * LOCAL FUNCTIONS DECLARATION
 */

// Declare function prototypes for various tasks and processing functions

static void ptl_remove_none_header_data(ptl_proc_buff_t *buffer);   // Remove data that is not part of the header
static void ptl_find_valid_frame(ptl_proc_buff_t *buffer);          // Find a valid frame from the received data
static void ptl_proc_valid_frame(uint8_t *buffer, uint16_t length); // Process the valid frame
static void ptl_frame_analysis_handler(void);                       // Analyzes the received protocol frame for proper handling.

static void ptl_tx_event_handler(void); // Handle the transmission event message
static void ptl_rx_event_handler(void);
static void ptl_hal_tx(uint8_t channel, uint8_t *buffer, uint16_t length);

static module_info_t *ptl_get_module(ptl_frame_type_t frame_type); // Get module information by frame type

static bool ptl_is_exists_task(void); // Check if sleep mode is enabled
/*******************************************************************************
 * STATIC VARIABLES
 */

// Define static variables for internal management
static ptl_proc_buff_t l_t_tx_proc_buf; // Transmission buffer
static ptl_proc_buff_t l_t_rx_proc_buf; // Reception buffer

static uint32_t l_t_ptl_running_req_mask = 0; // Mask indicating running tasks (initialized to none)
static uint32_t l_t_ptl_rx_main_timer;        // Timer for RX main task
static uint32_t l_t_ptl_tx_main_timer;        // Timer for TX main task

static module_info_t l_t_module_info[PTL_MODULE_SUPPORT_CNT]; // Array holding module information
static uint8_t l_u8_next_empty_module = 0;                    // Index for the next empty module slot

void ptl_print_registered_module(void);

/*******************************************************************************
 * GLOBAL FUNCTIONS IMPLEMENTATION
 */
/**
 * Initializes the Octopus protocol (no specific actions for now).
 */
void otsm_ptl_help(void)
{
    ptl_print_registered_module();
}

// Initialize UART communication for the task
void task_ptl_init_running(void)
{
    LOG_LEVEL("task_ptl_init_running\r\n");

    OTMS(TASK_MODULE_PTL_1, OTMS_S_INVALID);
}

// Start the UART communication for the task
void task_ptl_start_running(void)
{
    LOG_LEVEL("task_ptl_start_running\r\n");
    OTMS(TASK_MODULE_PTL_1, OTMS_S_ASSERT_RUN);
}

// Assert that UART communication is running
void task_ptl_assert_running(void)
{
    StartTickCounter(&l_t_ptl_rx_main_timer);
    StartTickCounter(&l_t_ptl_tx_main_timer);
    OTMS(TASK_MODULE_PTL_1, OTMS_S_RUNNING);
}

// Main running function for UART communication
void task_ptl_running(void)
{
    if (ptl_is_exists_task())
    {
        ptl_tx_event_handler();

        ptl_rx_event_handler();
        ptl_frame_analysis_handler();
    }
}

// Post-running function for UART communication
void task_ptl_post_running(void)
{
    OTMS(TASK_MODULE_PTL_1, OTMS_S_ASSERT_RUN);
}

// Stop the UART communication task
void task_ptl_stop_running(void)
{
    LOG_LEVEL("_stop_running\r\n");
    OTMS(TASK_MODULE_PTL_1, OTMS_S_INVALID);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// Request the UART task to start running (source indicates who requested)
bool ptl_reqest_running(ptl_frame_type_t frame_type)
{
    if (frame_type >= 32)
    {
        l_t_ptl_running_req_mask |= (1 << (frame_type & 0x7f));
        return true;
    }
    else
    {
        l_t_ptl_running_req_mask |= (1 << frame_type);
        return true;
    }
}

// Release the UART task from running (source indicates who is releasing)
bool ptl_release_running(ptl_frame_type_t frame_type)
{
    if (frame_type >= 32)
    {
        return false;
    }
    else
    {
        l_t_ptl_running_req_mask &= ~(1 << frame_type);
        return true;
    }
}

/**
 * Check if sleep mode is enabled based on UART pending state.
 * Returns true if no task is currently running.
 */
bool ptl_is_exists_task(void)
{
    // Check if there are no tasks running
    if (l_t_ptl_running_req_mask != PTL_RUNNING_NONE)
    {
        return true; // Sleep mode is enabled
    }
    else
    {
        return false; // Sleep mode is not enabled, tasks are running
    }
}

// Register a new module for communication (with its send and receive handlers)
void ptl_register_module(ptl_frame_type_t frame_type, module_send_handler_t send_handler, module_receive_handler_t receive_handler)
{
    if (l_u8_next_empty_module < PTL_MODULE_SUPPORT_CNT)
    {
        l_t_module_info[l_u8_next_empty_module].frame_type = (ptl_frame_type_t)(frame_type & 0x7f);
        l_t_module_info[l_u8_next_empty_module].send_handler = send_handler;
        l_t_module_info[l_u8_next_empty_module].receive_handler = receive_handler;
        l_u8_next_empty_module++;
        // LOG_LEVEL("frame_type=%d ptl module count=%d\r\n", l_t_module_info[l_u8_next_empty_module - 1].frame_type, l_u8_next_empty_module);
    }
    else
    {
        LOG_LEVEL("ptl module is full count=%d\r\n", l_u8_next_empty_module);
    }
}

/**
 * Retrieves the module corresponding to the specified frame type.
 */
module_info_t *ptl_get_module(ptl_frame_type_t frame_type)
{
    module_info_t *module_info = NULL;

    for (uint8_t i = 0; i < l_u8_next_empty_module; i++)
    {
        if (l_t_module_info[i].frame_type == (ptl_frame_type_t)(frame_type & 0x7F))
        {
            module_info = &l_t_module_info[i];
            break;
        }
    }

    return module_info;
}

void ptl_print_registered_module(void)
{
    for (uint8_t i = 0; i < l_u8_next_empty_module; i++)
    {
        LOG_LEVEL("registered l_t_module_info[%d]=%02x \r\n", i, l_t_module_info[i].frame_type);
    }
}

//[ Header ][ Frame Type ][ Command ][ Data Length ][ Header Checksum ][ Data... ][ Data Checksum ]
//|   0    |       1     |     2    |       3      |         4        |     n    |       n+1      |
// Build a communication frame with the given data and command
void ptl_build_frame(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t *data, uint8_t datelen, ptl_proc_buff_t *framebuff)
{
    MY_ASSERT(data);
    MY_ASSERT(framebuff);
    MY_ASSERT((datelen + PTL_FRAME_HEADER_SIZE + 1) < PTL_FRAME_MAX_SIZE);
    ptl_build_frame_header(frame_type, cmd, datelen, framebuff);
    for (int i = 0; i < datelen; i++)
    {
        framebuff->buff[PTL_FRAME_HEADER_SIZE + i] = data[i];
    }
    framebuff->buff[PTL_FRAME_HEADER_SIZE + datelen] = ptl_get_checksum(&(framebuff->buff[PTL_FRAME_HEADER_SIZE - 1]), (datelen + 1));

    framebuff->size = PTL_FRAME_HEADER_SIZE + datelen + 1;
}

// Build the header of the communication frame
void ptl_build_frame_header(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t datalen, ptl_proc_buff_t *proc_buff)
{
    // Assert that the process buffer pointer is not NULL
    MY_ASSERT(proc_buff);
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    proc_buff->buff[0] = MCU_TO_SOC_PTL_HEADER;
#elif defined(TASK_MANAGER_STATE_MACHINE_SOC)
    proc_buff->buff[0] = SOC_TO_MCU_PTL_HEADER;
#else
    // Optimize the check: if SOC_TO_MCU_MOD_START is 0, skip unnecessary comparison
    if (frame_type >= SOC_TO_MCU_MOD_START)
    {
        // If the frame type is greater than or equal to SOC_TO_MCU_MOD_START, set the header to SOC_TO_MCU_PTL_HEADER
        proc_buff->buff[0] = SOC_TO_MCU_PTL_HEADER;
    }
    else if (frame_type >= MCU_TO_SOC_MOD_START)
    {
        // If the frame type is greater than or equal to MCU_TO_SOC_MOD_START, set the header to MCU_TO_SOC_PTL_HEADER
        proc_buff->buff[0] = MCU_TO_SOC_PTL_HEADER;
    }
    else
    {
        // If the frame type does not meet any above conditions, set the header to DBG_PTL_HEADER
        // proc_buff->buff[0] = DBG_PTL_HEADER;
    }
#endif

    // Store the frame type at position 1 of the buffer
    proc_buff->buff[1] = frame_type;

    // Store the command at position 2 of the buffer
    proc_buff->buff[2] = cmd;

    // Store the data length at position 3 of the buffer
    proc_buff->buff[3] = datalen;

    // Calculate and store the checksum at position 4 of the buffer (checksum for the header portion)
    proc_buff->buff[4] = ptl_get_checksum(proc_buff->buff, PTL_FRAME_HEADER_SIZE - 1);
}

// Calculate the checksum for the data
uint8_t ptl_get_checksum(uint8_t *data, uint8_t length)
{
    MY_ASSERT(data);
    uint8_t sum = 0;
    for (int i = 0; i < length; i++)
    {
        sum += data[i];
    }
    return (uint8_t)(~sum + 1);
}

/**
 * Handles the event message for transmitting data via UART.
 * Checks if enough time has passed since the last transmission and sends the data if necessary.
 */
void ptl_tx_event_handler(void)
{
    module_info_t *p_module = NULL;

    // If the time since the last TX event is less than 10 ms, do nothing
    if (GetTickCounter(&l_t_ptl_tx_main_timer) < 10)
    {
        return;
    }

    // Restart the timer for the next transmission
    StartTickCounter(&l_t_ptl_tx_main_timer);

    // Retrieve the message from the UART task message queue
    Msg_t *msg = get_message(TASK_MODULE_PTL_1);
    if (msg->msg_id != NO_MSG)
    {
        ptl_frame_type_t frame_type = (ptl_frame_type_t)msg->msg_id;
        uint16_t param1 = msg->param1;
        uint16_t param2 = msg->param2;

        // Get the module corresponding to the frame type
        p_module = ptl_get_module(frame_type);

        // If module and send handler are available, call the send handler
        if ((NULL != p_module) && (NULL != p_module->send_handler))
        {
            bool res = p_module->send_handler(frame_type, (ptl_frame_cmd_t)param1, param2, &l_t_tx_proc_buf);
            if (res)
            {
                ptl_hal_tx(l_t_tx_proc_buf.channel, l_t_tx_proc_buf.buff, l_t_tx_proc_buf.size);
            }
        }
        else if (NULL == p_module->send_handler)
        {
            LOG_LEVEL("module is not mached iframe_type=%d param1=%d param2=%d.\r\n", frame_type, param1, param2);
        }
        else if (NULL == p_module)
        {
            LOG_LEVEL("module is null iframe_type=%d param1=%d param2=%d.\r\n", frame_type, param1, param2);
        }
    }
    else
    {
        // If no message, do nothing (idle state)
        ;
    }
}

/**
 * Handler for receiving UART data byte-by-byte.
 * Calls the UART reception handler to store the received data.
 */
void ptl_rx_event_handler(void)
{
    // Call the HAL function to process received byte
    uint8_t count = 0;
    while (1)
    {
        if (l_t_rx_proc_buf.size < PTL_FRAME_MAX_SIZE)
        {
            count = hal_com_uart_get_fifo_data_1(&l_t_rx_proc_buf.buff[l_t_rx_proc_buf.size], PTL_FRAME_MAX_SIZE - l_t_rx_proc_buf.size);
            if (count <= 0)
                break;
            l_t_rx_proc_buf.size = l_t_rx_proc_buf.size + count;
        }
        else
        {
            break;
        }
    }
}

/**
 * Analyzes the received frame by first removing any non-header data,
 * and then processing any valid frames.
 */
void ptl_frame_analysis_handler(void)
{
    // Remove data that is not part of the header
    ptl_remove_none_header_data(&l_t_rx_proc_buf);

    // Process the valid frame found in the buffer
    ptl_find_valid_frame(&l_t_rx_proc_buf);
}

/**
 * Removes any data from the buffer that is not part of the header.
 * Only keeps the data after the SOC_TO_MCU_PTL_HEADER byte.
 */
void ptl_remove_none_header_data(ptl_proc_buff_t *proc_buff)
{
// If the first byte is SOC_TO_MCU_PTL_HEADER, no action needed
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
    if (proc_buff->buff[0] == SOC_TO_MCU_PTL_HEADER)
        return;
    // Search for SOC_TO_MCU_PTL_HEADER and remove data before it
    for (uint16_t i = 0; i < proc_buff->size; i++)
    {
        if (proc_buff->buff[i] == SOC_TO_MCU_PTL_HEADER)
        {
            // Shift data to remove the position before the header
            for (uint16_t j = i; j < proc_buff->size; j++)
            {
                proc_buff->buff[j - i] = proc_buff->buff[j];
            }
            proc_buff->size -= i;
            return;
        }
    }
// #endif
// #ifdef TASK_MANAGER_STATE_MACHINE_SOC
#else
    if (proc_buff->buff[0] == MCU_TO_SOC_PTL_HEADER)
        return;
    for (uint16_t i = 0; i < proc_buff->size; i++)
    {
        if (proc_buff->buff[i] == MCU_TO_SOC_PTL_HEADER)
        {
            // remove data before header
            for (uint16_t j = i; j < proc_buff->size; j++)
            {
                proc_buff->buff[j - i] = proc_buff->buff[j];
            }
            proc_buff->size -= i;
            return;
        }
    }

#endif
    // If _PTL_HEADER is not found, clear the buffer
    proc_buff->size = 0;
}

/**
 * Finds a valid frame in the received buffer.
 * Validates the header and frame checksums.
 */
void ptl_find_valid_frame(ptl_proc_buff_t *proc_buff)
{
    uint8_t datalen = 0;            // Data length
    uint8_t framelen = 0;           // Frame length
    uint8_t crc;                    // Header checksum
    uint8_t crc_read = 0;           // Read checksum value
    bool head_crc_ok = false;       // Flag for header checksum validity
    bool frame_crc_ok = false;      // Flag for frame checksum validity
    bool found = false;             // Flag to indicate if a valid frame is found
    bool header_invalid = false;    // Flag for invalid header detection
    uint16_t next_valid_offset = 0; // Next valid frame offset
    uint16_t offset = 0;            // Header offset

    if (proc_buff->size < PTL_FRAME_MIN_SIZE)
        return;

    // Iterate through the received buffer to find a valid frame
    for (uint16_t i = 0; i < proc_buff->size; i++)
    {
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
        if (proc_buff->buff[i] == SOC_TO_MCU_PTL_HEADER)
        {
#else
        if (proc_buff->buff[i] == MCU_TO_SOC_PTL_HEADER)
        {
#endif
            offset = i;
            // Ensure there is at least one more byte to read frame length
            if ((i + 3) >= proc_buff->size)
                break;

            datalen = proc_buff->buff[i + 3];
            framelen = datalen + PTL_FRAME_HEADER_SIZE + 1;

            // Ensure minimum frame size is met
            if ((framelen < PTL_FRAME_MIN_SIZE) && (proc_buff->size >= PTL_FRAME_MIN_SIZE) && (offset == 0))
            {
                next_valid_offset = PTL_FRAME_MIN_SIZE;
            }
            else if ((framelen >= PTL_FRAME_MIN_SIZE) && (framelen <= (proc_buff->size - offset)))
            {
                // Validate header checksum
                crc = ptl_get_checksum(&proc_buff->buff[offset], PTL_FRAME_HEADER_SIZE - 1);
                crc_read = proc_buff->buff[offset + PTL_FRAME_HEADER_SIZE - 1];
                head_crc_ok = (crc == crc_read);

                if (head_crc_ok)
                {
                    // Validate frame checksum
                    crc = ptl_get_checksum(&(proc_buff->buff[offset + PTL_FRAME_HEADER_SIZE - 1]), (datalen + 1));
                    crc_read = proc_buff->buff[offset + PTL_FRAME_HEADER_SIZE + datalen];
                    frame_crc_ok = (crc == crc_read);
                }

                // If both header and frame checksums are valid, process the frame
                if (head_crc_ok && frame_crc_ok)
                {
                    next_valid_offset = offset + framelen;
                    found = true;
                    break;
                }
                else
                {
                    // If the first header is invalid, mark it and skip to next frame
                    if ((offset == 0) || (header_invalid == true))
                    {
                        header_invalid = true;
                        next_valid_offset = framelen + offset;
                    }
                }
            }
            else
            {
                // No valid frame found yet, continue searching
            }
        }
    }

    // If a valid frame is found, process it
    if (found == true)
    {
        ptl_proc_valid_frame(proc_buff->buff + offset, framelen);
    }

    // Shift buffer data if a valid frame is found
    if (next_valid_offset != 0)
    {
        for (uint16_t i = next_valid_offset; i < proc_buff->size; i++)
        {
            proc_buff->buff[i - next_valid_offset] = proc_buff->buff[i];
        }
        proc_buff->size = proc_buff->size - next_valid_offset;
    }
}

/**
 * Processes the valid frame after it has been extracted from the buffer.
 * The payload is passed to the appropriate module handler.
 */
void ptl_proc_valid_frame(uint8_t *buffer, uint16_t length)
{
    ptl_frame_payload_t payload;
    module_info_t *module_ = NULL;
    ptl_proc_buff_t *tx_frame = &l_t_tx_proc_buf;

    // Set up the frame payload from the received data
    payload.frame_type = (ptl_frame_type_t)buffer[1];
    payload.frame_cmd = (ptl_frame_cmd_t)buffer[2];
    payload.data_len = buffer[3];
    payload.data = &buffer[PTL_FRAME_DATA_START];

    // Get the module associated with the frame type
    module_ = ptl_get_module(payload.frame_type);

#ifdef TEST_LOG_DEBUG_PTL_RX_FRAME
    LOG_LEVEL("payload.frame_type=%02x cmd=%02x,length=%d data[]=", payload.frame_type, payload.frame_cmd, payload.data_len);
    LOG_BUFF(buffer, length);
#endif

    // If module handler exists, call the receive handler
    if (NULL != module_)
    {
        if (NULL == module_->receive_handler)
        {
            LOG_LEVEL("module is not mached payload.frame_type=%d payload.frame_cmd=%d payload.data_len=%d.\r\n", payload.frame_type, payload.frame_cmd, payload.data_len);
            return;
        }
        bool res = module_->receive_handler(&payload, tx_frame);
        if (res)
        {
            // If the handler succeeds, clear error flag and restart timer
            ptl_hal_tx(tx_frame->channel, tx_frame->buff, tx_frame->size); // Send response via UART
        }
    }
    else /// if (NULL == module_)
    {
        LOG_LEVEL("invalid module is null.\r\n");
    }
}

/**
 * Sends data over UART using the hardware abstraction layer.
 */
static void ptl_hal_tx(uint8_t channel, uint8_t *data, uint16_t length)
{
    // Send the processed data over UART
#ifdef TEST_LOG_DEBUG_PTL_TX_FRAME
    LOG_LEVEL("data[%02d] ", length);
    LOG_BUFF(data, length);
#endif
    if (length <= 0)
        return;

    switch (channel)
    {
    case 1:
        hal_com_uartl_send_buffer(data, length);
        break;

    case 2:
        hal_com_uart2_send_buffer(data, length);
        break;

    case 3:
        hal_com_uart3_send_buffer(data, length);
        break;

    case 4:
        hal_com_uart4_send_buffer(data, length);
        break;

    case 5:
        hal_com_uart5_send_buffer(data, length);
        break;

    case 6:
        hal_com_uart6_send_buffer(data, length);
        break;

    case 7:
        hal_com_uart7_send_buffer(data, length);
        break;

    case 0:
    default:
        hal_com_uart0_send_buffer(data, length);
        break;
    }
}

void ptl_send_buffer(uint8_t channel, uint8_t *data, uint16_t length)
{
    ptl_hal_tx(channel, data, length);
}
