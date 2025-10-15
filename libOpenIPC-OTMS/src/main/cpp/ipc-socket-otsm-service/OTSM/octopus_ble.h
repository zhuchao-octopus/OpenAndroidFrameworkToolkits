/*******************************************************************************
 * @file    octopus_task_manager_ble.h
 * @brief   Header file for the Octopus Task Manager BLE module.
 *          This file declares the functions and data structures required for
 *          managing Bluetooth Low Energy (BLE) operations within the Octopus
 *          platform.
 *
 * @details This module is responsible for initializing, starting, running,
 *          and stopping BLE functionality. It manages BLE status, including
 *          locking and unlocking operations and MAC address handling.
 *
 * @author  Octopus Team
 * @version 1.0.0
 * @date    2024-12-09
 *******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_BLE_H__
#define __OCTOPUS_TASK_MANAGER_BLE_H__

/*******************************************************************************
 * INCLUDES
 * Include necessary headers for BLE management.
 *******************************************************************************/
#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_system.h"
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#ifdef TASK_MANAGER_STATE_MACHINE_BLE
    /*******************************************************************************
     * MACROS
     * Define commonly used macros for this module.
     *******************************************************************************/

    /*******************************************************************************
     * TYPEDEFS
     * Define types used in the BLE management process.
     *******************************************************************************/

    /**
     * @struct BLE_STATUS
     * @brief  Structure representing the status of the BLE module.
     */
    typedef struct
    {
        signed char rssi;
        uint8_t mode; /**< Current mode of the BLE module. */
        // bool locked;    /**< Indicates if the BLE module is locked. */
        bool to_lock; /**< Indicates if the BLE module should be locked. */
        bool connected;
        bool rssi_unlock;
        uint8_t mac[6]; /**< MAC address associated with the BLE module. */
    } BLE_STATUS;

    /*******************************************************************************
     * GLOBAL FUNCTIONS DECLARATION
     * Declare the public functions provided by this module.
     *******************************************************************************/

    /**
     * @brief Initialize BLE functionality.
     */
    void task_ble_init_running(void);

    /**
     * @brief Start BLE operations.
     */
    void task_ble_start_running(void);

    /**
     * @brief Assert and verify the BLE module is running correctly.
     */
    void task_ble_assert_running(void);

    /**
     * @brief Handle the main logic for BLE operations.
     */
    void task_ble_running(void);

    /**
     * @brief Perform post-processing for BLE operations.
     */
    void task_ble_post_running(void);

    /**
     * @brief Stop BLE operations.
     */
    void task_ble_stop_running(void);

#ifdef __cplusplus
}
#endif

#endif

#endif // __OCTOPUS_TASK_MANAGER_BLE_H__
