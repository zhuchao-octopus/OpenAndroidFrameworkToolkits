/*******************************************************************************
 * @file     octopus_ipc.h
 * @brief    Provides system management functions for task manager, including
 *           version information, state machine management, and MCU update states.
 *
 * This header file defines the necessary API to manage the system state
 * within the Octopus Task Manager framework. It includes management of the
 * main control state machine, MCU update states, and platform-specific state
 * transitions for the system's MPU (Microprocessor Unit) and boot sequence.
 *
 * The file also includes macros for versioning and the system's state
 * definitions to aid in the initialization, startup, and state transitions of
 * the task manager. Additionally, the system status for various stages of
 * application execution is provided.
 *
 * @note     This file assumes the use of platform-specific configurations
 *           for the task manager's operation and MCU update procedures.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *******************************************************************************/

#ifndef __OCTOPUS_IPC_SOCKET_H__
#define __OCTOPUS_IPC_SOCKET_H__

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "octopus_base.h" //  Base include file for the Octopus project.

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/*******************************************************************************
 * Callback function declaration
 * Define a function pointer type for the callback that takes an integer.
 ******************************************************************************/
// Define a type for the callback that accepts an integer
typedef void (*MessageDataInforCallback_t)(uint16_t msg_grp, uint16_t msg_id, const uint8_t *data, uint16_t length);

/*******************************************************************************
 * GLOBAL FUNCTIONS DECLARATION
 ******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Initialize the application system.
     */
    void task_ipc_init_running(void);

    /**
     * @brief Start running the application system.
     */
    void task_ipc_start_running(void);

    /**
     * @brief Assert and maintain the running state of the application system.
     */
    void task_ipc_assert_running(void);

    /**
     * @brief Execute the main running logic of the application system.
     */
    void task_ipc_running(void);

    /**
     * @brief Post-run procedures for the application system.
     */
    void task_ipc_post_running(void);

    void task_ipc_stop_running(void);

    void register_message_data_callback(MessageDataInforCallback_t callback);

    void update_push_interval_ms(uint16_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_IPC_SOCKET_H__ */
