/********************************** (C) COPYRIGHT *******************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * All rights reserved.
 *
 * @file    task_octopus_ble_hal.h
 * @brief   This file provides the definitions and function declarations for
 *          managing Bluetooth Low Energy (BLE) functionalities in the Octopus
 *          task manager.
 *
 * @details This file includes functions for handling BLE pairing, enabling and
 *          disabling BLE pairing modes, and tracking device status during
 *          communication. The macros define the state and event flags for BLE
 *          operations, making it easier to control and monitor BLE-related tasks.
 *
 * @note    This software (modified or not) and binary are used for microcontrollers
 *          manufactured by Nanjing Qinheng Microelectronics.
 * @version  1.0
 * @date     2021-10-01
 * @author   Octopus Team
 *******************************************************************************/

#ifndef __TASK_OCTOPUS_BLE_HAL_H__
#define __TASK_OCTOPUS_BLE_HAL_H__

/*******************************************************************************
 * INCLUDES
 * @details  Include necessary platform and library headers for Bluetooth Low Energy (BLE) functionality.
 */

#include "octopus_base.h" //  Base include file for the Octopus project.

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 * MACROS
 * @details  Macros to represent the various states and events for BLE communication.
 */
#define DEVICE_BLE_PAIR 0x0008         /**< Macro for BLE pairing state */
#define DEVICE_BLE_BONDED 0x0010       /**< Macro for BLE bonded state */
#define DEVICE_BLE_CONNECTED 0x0020    /**< Macro for BLE connected state */
#define DEVICE_BLE_DISCONNECTED 0x0040 /**< Macro for BLE disconnected state */
#define DEVICE_TIMER_EVENT 0x0080      /**< Macro for timer event */
#define DEVICE_PIN_KEY_PRESS 0x0100    /**< Macro for pin key press event */

    /*******************************************************************************
     * TYPEDEFS
     * @details  Typedefs for structures related to BLE operations (currently empty).
     */

    /*******************************************************************************
     * GLOBAL FUNCTIONS DECLARE
     * @details  Function declarations for handling BLE pairing and mode settings.
     */

    // Functions to set BLE pairing mode on/off and to enable/disable pairing mode
    uint8_t hal_set_pairing_mode_onoff(bool on_off, uint8_t current_pairing_mode);
    /**< Function to set BLE pairing mode on or off based on the 'on_off' parameter */
    void hal_enable_bLe_pair_mode(void);
    /**< Function to enable BLE pairing mode */
    void hal_disable_bLe_pair_mode(void);
    /**< Function to disable BLE pairing mode */
    uint8_t hal_get_ble_rssi(uint8_t connection_id);
#ifdef __cplusplus
}
#endif

#endif /* __TASK_OCTOPUS_BLE_HAL_H__ */
