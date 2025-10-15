/********************************************************************************
 * @file    octopus_configuration.h
 * @author  Octopus Team
 * @date    2025-09-01
 * @version v1.0
 *
 * @brief
 *   Project configuration header file.
 *   This file contains all compile-time switches for platform selection,
 *   RTOS type, Task Manager modules, and feature toggles. It serves as the
 *   central place to control project build options.
 *
 * @details
 *   - Platform/RTOS selection: Choose one platform macro only.
 *   - Task Manager: Enable or disable state machine modules independently.
 *   - Flash options: Configure how flash memory is managed and mapped.
 *
 * @usage
 *   - Include this file in "octopus_platform.h".
 *   - Define only one PLATFORM_xxx macro.
 *   - Define required TASK_MANAGER_STATE_MACHINE_xxx macros.
 *   - Comment unused modules to reduce binary size.
 *
 * @note
 *   - Do not modify this file per source file; changes here affect the whole
 *     project build.
 *   - Some macros may conflict; enable carefully.
 *   - Always rebuild the entire project after modifications.
 ********************************************************************************/

#ifndef __OCTOPUS_CONFIGURATION_H__
#define __OCTOPUS_CONFIGURATION_H__

///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------//
//                            PLATFORM / RTOS SELECTION                         //
//------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////////////////////////////
// Define which platform and RTOS to use by enabling the corresponding macro.
// ⚠ Only one PLATFORM_xxx macro should be enabled at a time.

// #define PLATFORM_ITE_OPEN_RTOS    // Enable ITE platform with OPEN RTOS
// #define PLATFORM_CST_OSAL_RTOS    // CST platform with OSAL RTOS
// #define PLATFORM_X86_WIND_RTOS    // XB6 platform with WIND RTOS
// #define PLATFORM_STM32_RTOS       // STM32 platform with RTOS
#define PLATFORM_LINUX_RISC // X86 / ARM Linux platform

///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------//
//                          TASK MANAGER CONFIGURATION                          //
//------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////////////////////////////

// State machine operation mode
// #define TASK_MANAGER_STATE_MACHINE_MCU  1   /**< MCU main control mode */
#define TASK_MANAGER_STATE_MACHINE_SOC 1 /**< SOC mode (reserved) */

///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------//
//                          Enable / Disable Modules                            //
//------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////////////////////////////

//#define TASK_MANAGER_STATE_MACHINE_GPIO     1   /**< GPIO handling */
//#define TASK_MANAGER_STATE_MACHINE_FLASH    1   /**< Flash memory handling */
#define TASK_MANAGER_STATE_MACHINE_SYSTEM 1 /**< System-level state machine */
#define TASK_MANAGER_STATE_MACHINE_KEY 1    /**< Key input handling */

#define TASK_MANAGER_STATE_MACHINE_CARINFOR 1 /**< Car information processing */
#define TASK_MANAGER_STATE_MACHINE_UPDATE 1   /**< OTA / Firmware update */

#define TASK_MANAGER_STATE_MACHINE_PTL 1 /**< PTL protocol handler */
//#define TASK_MANAGER_STATE_MACHINE_UPF       1   /**< UART Packet Framework */
#define TASK_MANAGER_STATE_MACHINE_IPC 1 /**< Inter-process communication */

//#define TASK_MANAGER_STATE_MACHINE_CAN       1   /**< CAN bus protocol */
//#define TASK_MANAGER_STATE_MACHINE_BAFANG    1   /**< Bafang system support */
//#define TASK_MANAGER_STATE_MACHINE_LING_HUI_LIION2 1 /**< Ling Hui battery */

//#define TASK_MANAGER_STATE_MACHINE_LOT4G        1   /**< 4G module handler */
//#define TASK_MANAGER_STATE_MACHINE_BT_MUSIC     1   /**< Classic Bluetooth */

// #define TASK_MANAGER_STATE_MACHINE_SIF       1   /**< SIF protocol handler */
// #define TASK_MANAGER_STATE_MACHINE_BLE       1   /**< Bluetooth Low Energy */
// #define TASK_MANAGER_STATE_MACHINE_BMS       1   /**< Battery Management System */



///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------//
//                             FLASH CONFIGURATION                              //
//------------------------------------------------------------------------------//
///////////////////////////////////////////////////////////////////////////////////

//#define FLASH_USE_EEROM_FOR_DATA_SAVING   /**< Use EEPROM instead of Flash */
//#define FLASH_MAPPING_VECT_TABLE_TO_SRAM true /**< Remap vector table to SRAM */

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
#ifdef TARGET_BANK_SLOT_A
#define FLASH_BANK_CONFIG_MODE_SLOT BANK_SLOT_A /**< Select Flash bank slot */
#elif defined(TARGET_BANK_SLOT_B)
#define FLASH_BANK_CONFIG_MODE_SLOT BANK_SLOT_B /**< Select Flash bank slot */
#else
#define FLASH_BANK_CONFIG_MODE_SLOT BANK_SLOT_INVALID /**< Select Flash bank slot */
#endif


#endif /* __OCTOPUS_CONFIGURATION_H__ */
