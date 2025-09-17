/*******************************************************************************
 * @file octopus_task_manager_ptl.h
 * @brief Header file for managing Octopus protocol tasks.
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

#ifndef __OCTOPUS_TASK_MANAGER_PTL_1_H__
#define __OCTOPUS_TASK_MANAGER_PTL_1_H__
/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

/*******************************************************************************
 * DEBUG SWITCH MACROS
 *
 * Define debug macros here for enabling or disabling debugging functionality.
 */

/*******************************************************************************
 * MACROS
 */
#define PTL_UART_CHANNEL 1      ///< UART channel for communication
#define PTL_FRAME_HEADER_SIZE 5 ///< Size of the frame header
#define PTL_FRAME_MIN_SIZE 7    ///< Minimum size of the frame
#define PTL_FRAME_MAX_SIZE 255  ///< Maximum size of the frame
#define PTL_FRAME_DATA_START 5  ///< Start index of the data in the frame
                                // #define PTL_HEADER 0xFA         ///< Frame header identifier

/*******************************************************************************
 * TYPEDEFS
 */

/**
 * @brief Enumeration for UART channels.
 */
typedef enum UartChannel
{
    UartChannel_Uart0 = 0, ///< UART channel 0
    UartChannel_Uart1 = 1, ///< UART channel 1
    UartChannel_Uart2 = 2  ///< UART channel 2
} UartChannel;

/**
 * @brief Enumeration for frame headers.
 */
typedef enum
{
    MCU_TO_SOC_PTL_HEADER = 0x55, ///< MCU -> APP frame header
    SOC_TO_MCU_PTL_HEADER = 0xAA, ///< APP -> MCU frame header
    DBG_PTL_HEADER = 0xFF         ///< for debug
} ptl_frame_header_t;

/**
 * @brief Enumeration for module IDs.
 */
typedef enum
{
    /* MCU -> SOC module IDs */
    MCU_TO_SOC_MOD_SYSTEM = 0x00,   ///< System initialization
    MCU_TO_SOC_MOD_UPDATE = 0x01,   ///< System update
    MCU_TO_SOC_MOD_TRANSFER = 0x02, ///< Data transfer
    MCU_TO_SOC_MOD_CARINFOR = 0x03, ///< IPC carinfor
    MCU_TO_SOC_MOD_SETUP = 0x04,    ///< Settings
    MCU_TO_SOC_MOD_KEY = 0x05,      ///< KEY
    MCU_TO_SOC_MOD_CAN = 0x06,      ///< CAN
    MCU_TO_SOC_MOD_IPC = 0x07,      ///< IPC socket

    /* SOC -> MCU module IDs */
    SOC_TO_MCU_MOD_SYSTEM = 0x80,   ///< System initialization
    SOC_TO_MCU_MOD_UPDATE = 0x81,   ///< System update
    SOC_TO_MCU_MOD_TRANSFER = 0x82, ///< Data transfer
    SOC_TO_MCU_MOD_CARINFOR = 0x83, ///< IPC carinfor
    SOC_TO_MCU_MOD_SETUP = 0x84,    ///< Settings
    SOC_TO_MCU_MOD_KEY = 0x85,      ///< KEY
    SOC_TO_MCU_MOD_CAN = 0x86,      ///< CAN
    SOC_TO_MCU_MOD_IPC = 0x87,      ///< IPC socket

} ptl_frame_type_t;

/**
 * @brief Enumeration for commands within each module.
 */
typedef enum
{
    /* MOD_SYSTEM commands */
    FRAME_CMD_SYSTEM_HANDSHAKE = 0x00, ///< System handshake
    FRAME_CMD_SYSTEM_ACC_STATE = 0x01, ///< ACC state
    FRAME_CMD_SYSTEM_MCU_META = 0x02,  ///< Application state
    FRAME_CMD_SYSTEM_POWER_ON = 0x03,  ///< Power on
    FRAME_CMD_SYSTEM_POWER_OFF = 0x04, ///< Power off
    FRAME_CMD_SYSTEM_SAVE_DATA = 0x05, ///< Power off

    /* MOD_UPDATE commands */
    FRAME_CMD_UPDATE_CHECK_FW_STATE = 0x06,        ///< Check firmware state
    FRAME_CMD_UPDATE_UPDATE_FW_STATE = 0x07,       ///< Update firmware state
    FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE = 0x08, ///< Enter firmware update mode
    FRAME_CMD_UPDATE_EXITS_FW_UPGRADE_MODE = 0x09, ///< Exit firmware update mode
    FRAME_CMD_UPDATE_REQUEST_FW_DATA = 0x0A,       ///< Send firmware data
    FRAME_CMD_UPDATE_SEND_FW_DATA = 0x0B,          ///< Send firmware data
    FRAME_CMD_UPDATE_REBOOT = 0x0C,                ///< Reboot system

    /* MOD_TRANSFER commands */
    FRAME_CMD_TRANSFER_A2M = 0x0D, ///< A2M data transfer
    FRAME_CMD_TRANSFER_M2A = 0x0E, ///< M2A data transfer

    /* MOD_METER commands */
    FRAME_CMD_METER_RPM_SPEED = 0x0F,    ///< RPM and speed
    FRAME_CMD_METER_FUEL_TEMPTER = 0x10, ///< Fuel and temperature
    FRAME_CMD_METER_SOC = 0x11,          ///< State of charge (SOC)

    /* MOD_INDICATOR commands */
    FRAME_CMD_CARINFOR_INDICATOR = 0x12, ///< Indicator status
    FRAME_CMD_CARINFOR_METER = 0x13,     ///< Indicator status
    FRAME_CMD_CARINFOR_BATTERY = 0x14,
    FRAME_CMD_CARINFOR_ERROR = 0x15, ///< Error information

    /* MOD_DRIV_INFO commands */
    FRAME_CMD_DRIVINFO_ODO = 0x16,             ///< Odometer data
    FRAME_CMD_DRIVINFO_DRIV_DATA = 0x17,       ///< Driving data
    FRAME_CMD_DRIVINFO_GEAR = 0x18,            ///< Gear information
    FRAME_CMD_DRIVINFO_NAVI = 0x19,            ///< Navigation data
    FRAME_CMD_DRIVINFO_DRIV_DATA_CLEAR = 0x1A, ///< Clear driving data

    /* MOD_SETUP commands */
    FRAME_CMD_SETUP_UPDATE_TIME = 0x1B, ///< Update time
    FRAME_CMD_SETUP_SET_TIME = 0x1C,    ///< Set time
    FRAME_CMD_SETUP_KEY = 0x1D,         ///< Key input

    FRAME_CMD_CAR_SET_LIGHT = 0x1E,
    FRAME_CMD_CAR_SET_GEAR_LEVEL = 0x1F,

    FRAME_CMD_CAR_METER_TRIP_DISTANCE_CLEAR = 0x20,
    FRAME_CMD_CAR_METER_TIME_CLEAR = 0x21,
    FRAME_CMD_CAR_METER_ODO_CLEAR = 0x22,

    FRAME_CMD_CAR_SET_INDICATOR = 0x23,
    FRAME_CMD_CAR_SET_METER = 0x24,
    FRAME_CMD_CAR_SET_BATTERY = 0x25,
    FRAME_CMD_CAR_RESET_BATTERY = 0x26,
    FRAME_CMD_CAR_RESET_SYSTEM = 0x27,

    FRAME_CMD_CARINFOR_MAX = 0x64
} ptl_frame_cmd_t;

/**
 * @brief Protocol processing buffer.
 */
typedef struct
{
    uint8_t channel;
    uint8_t buff[PTL_FRAME_MAX_SIZE]; ///< Data buffer
    uint16_t size;                    ///< Buffer size
} ptl_proc_buff_t;

/**
 * @brief Protocol frame payload.
 */
typedef struct
{
    ptl_frame_type_t frame_type; ///< Frame type
    uint8_t frame_cmd;           ///< Command
    uint8_t data_len;            ///< Length of the data
    uint8_t *data;               ///< Pointer to the data
} ptl_frame_payload_t;

/**
 * @brief Module handlers for sending and receiving frames.
 */
typedef bool (*module_send_handler_t)(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff);
typedef bool (*module_receive_handler_t)(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

#ifdef __cplusplus
extern "C"
{
#endif
    /*******************************************************************************
     * GLOBAL FUNCTIONS DECLARATION
     */
    /* Protocol state management */
    /**
     * Initializes the protocol in the running state.
     */
    void task_ptl_init_running(void);

    /**
     * Starts the protocol in the running state.
     */
    void task_ptl_start_running(void);

    /**
     * Asserts that the protocol is running.
     */
    void task_ptl_assert_running(void);

    /**
     * Performs the actions for the protocol in the running state.
     */
    void task_ptl_running(void);

    /**
     * Handles post-running actions for the protocol.
     */
    void task_ptl_post_running(void);

    /**
     * Stops the protocol from running.
     */
    void task_ptl_stop_running(void);

    /* Protocol control functions */
    /**
     * Requests to start the protocol running from a specific source.
     * @param source The source requesting to start the protocol.
     * @return True if the request is successful, false otherwise.
     */
    bool ptl_reqest_running(ptl_frame_type_t frame_type);

    /**
     * Releases the protocol from running by a specific source.
     * @param source The source releasing the protocol.
     * @return True if the release is successful, false otherwise.
     */
    bool ptl_release_running(ptl_frame_type_t frame_type);

    /* Protocol frame operations */
    /**
     * Registers a module with the specified frame type and associated send and receive handlers.
     * @param frame_type The type of the frame.
     * @param send_handler The function to handle sending data.
     * @param receive_handler The function to handle receiving data.
     */
    void ptl_register_module(ptl_frame_type_t frame_type, module_send_handler_t send_handler, module_receive_handler_t receive_handler);

    /**
     * Builds a protocol frame with the specified parameters.
     * @param frame_type The type of the frame.
     * @param cmd The command for the frame.
     * @param data The data to include in the frame.
     * @param datelen The length of the data.
     * @param framebuff The buffer to store the built frame.
     */
    void ptl_build_frame(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t *data, uint8_t datelen, ptl_proc_buff_t *framebuff);

    /**
     * Builds the header of a protocol frame with the specified parameters.
     * @param frame_type The type of the frame.
     * @param cmd The command for the frame.
     * @param datalen The length of the data.
     * @param buff The buffer to store the built header.
     */
    void ptl_build_frame_header(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint8_t datalen, ptl_proc_buff_t *buff);

    /**
     * Calculates the checksum for the given data.
     * @param data The data for which the checksum will be calculated.
     * @param len The length of the data.
     * @return The calculated checksum.
     */
    uint8_t ptl_get_checksum(uint8_t *data, uint8_t length);

    void ptl_send_buffer(uint8_t channel, uint8_t *data, uint16_t length);

    void otsm_ptl_help(void);
#ifdef __cplusplus
}
#endif

/* Range definitions for validity checks */
#define MCU_TO_SOC_MOD_START MCU_TO_SOC_MOD_SYSTEM
#define MCU_TO_SOC_MOD_END MCU_TO_SOC_MOD_KEY
#define SOC_TO_MCU_MOD_START SOC_TO_MCU_MOD_SYSTEM
#define SOC_TO_MCU_MOD_END SOC_TO_MCU_MOD_KEY

#endif /* __OCTOPUS_TASK_MANAGER_PTL_H__ */
