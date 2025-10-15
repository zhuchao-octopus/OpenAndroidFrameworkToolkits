/*******************************************************************************
 * File Name: octopus_platform.h
 * @version  1.0.0
 * @date     2024-12-11
 * @author   Octopus Team
 * Description:
 *   This file provides platform-specific configurations and utility macros
 *   for the Octopus Task Manager project. It supports multiple platforms
 *   and RTOS environments, allowing developers to switch between different
 *   setups based on project requirements.
 *
 * Features:
 *   - Platform selection macros to switch between ITE and CST platforms.
 *   - Inclusion of standard and platform-specific libraries.
 *   - Definition of general-purpose macros for bit manipulation, system timing,
 *     and mathematical operations.
 *   - Abstracted delay and system tick retrieval for supported platforms.
 *
 * Usage:
 *   1. Define the appropriate platform macro (e.g., PLATFORM_ITE_OPEN_RTOS)
 *      before including this file.
 *   2. Use the provided macros and utility functions as needed for platform-
 *      specific development.
 *
 * Notes:
 *   - Ensure all required libraries and headers are available for the target
 *     platform.
 *   - Validate compatibility between the selected platform and RTOS.
 *   - This file is designed for internal use within the Octopus Task Manager
 *     project.
 *
 * License:
 * Copyright (c) 2024 Octopus Team
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 *

 ******************************************************************************/

#ifndef ___OCTOPUS_TASK_MANAGER_PLATFORM_H___
#define ___OCTOPUS_TASK_MANAGER_PLATFORM_H___

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

#ifdef PLATFORM_ITE_OPEN_RTOS
#include <sys/ioctl.h>         // System I/O control definitions
#include <sys/time.h>          // Time-related functions for UNIX systems
#include <pthread.h>           // POSIX thread support
#include "ite/ith.h"           // ITE hardware-specific definitions
#include "ite/itp.h"           // ITE platform-specific definitions
#include "uart/uart.h"         // UART communication module
#include "saradc/saradc.h"     // SAR ADC module for analog input
#include "math.h"              // Standard mathematical functions
#include "SDL/SDL.h"           // SDL library for multimedia applications
#include "openrtos/FreeRTOS.h" // FreeRTOS kernel for real-time tasks
#include "openrtos/queue.h"    // FreeRTOS queue handling
#include <unistd.h>            // POSIX API for file and process handling
#elif defined(PLATFORM_CST_OSAL_RTOS)
#include "OSAL.h"              // OS abstraction layer
#include "OSAL_PwrMgr.h"       // OS power management utilities
#include "OSAL_Memory.h"       // OS memory management functions
#include "gatt.h"              // Bluetooth Generic Attribute Profile
#include "hci.h"               // Host Controller Interface for Bluetooth
#include "gapgattserver.h"     // GAP GATT server management
#include "gattservapp.h"       // GATT service application interface
#include "gatt_profile_uuid.h" // GATT profile UUID definitions
#include "linkdb.h"            // Link database for Bluetooth connections
#include "peripheral.h"        // Peripheral device support utilities
#include "gapbondmgr.h"        // GAP bonding manager for secure connections
#include "hidkbdservice.h"     // HID keyboard service support
#include "hiddev.h"            // HID device implementation
#include "global_config.h"     // Global configuration settings
#include "hidkbd.h"            // HID keyboard implementation
#include "uart.h"              // UART communication module
#include "ll.h"                // Link layer support for Bluetooth
#include "ll_common.h"         // Common link layer definitions
#include "dma.h"               // Direct Memory Access handling
#include "key.h"               // Key input handling utilities
#include "spi.h"               // SPI communication interface
#include "gpio.h"              // General-Purpose Input/Output support
#include "flash.h"             // Flash memory management utilities
#include "clock.h"             // System clock management utilities
#include "error.h"             // Error handling utilities
#include "pwrmgr.h"            // Power management utilities
#include "rom_sym_def.h"       // ROM symbol definitions
#include "sdk_config.h"        // SDK configuration file
#include "types.h"             // Basic type definitions

#elif defined(PLATFORM_STM32_RTOS)
#include "octopus_bsp_hk32l08x.h"
#elif defined(PLATFORM_NATION_RTOS)
#include "octopus_bsp_nation.h"
#elif defined(PLATFORM_LINUX_RISC)
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>
#include "../HAL/octopus_serialport_c.h"
#else

#include "octopus_bsp.h"

#endif

/*******************************************************************************
 * DEBUG SWITCH MACROS
 * Define macros for enabling or disabling debug functionality.
 ******************************************************************************/

/*******************************************************************************
 * GENERAL MACROS
 * Define common bit manipulation macros and constants.
 ******************************************************************************/

#ifdef PLATFORM_CST_OSAL_RTOS

#define GET_SYSTEM_TICK_COUNT (hal_systick() * 625 / 1000) // Convert system ticks to milliseconds
#define DELAY_US(us) (WaitUs(us))                          // Introduce delay in microseconds
#define DISABLE_IRQ (__disable_irq())
#define ENABLE_IRQ (__enable_irq())

#elif defined(PLATFORM_ITE_OPEN_RTOS)

#define CFG_OTSM_STACK_SIZE (200112L)          // Stack size for Octopus Task Manager
#define GET_SYSTEM_TICK_COUNT (SDL_GetTicks()) // Retrieve system tick count in milliseconds
#define DELAY_US(us) (usleep(us))              // Introduce delay in microseconds

#elif defined(PLATFORM_LINUX_RISC)

#define CFG_OTSM_STACK_SIZE (1024 * 1024) //(200112L)// Stack size for Octopus Task Manager
#define DELAY_US(us) (usleep(us))         // Define empty macro for unsupported platforms
#define GET_SYSTEM_TICK_COUNT ({                                                      \
    struct timespec ts;                                                               \
    unsigned long long tick_count = 0;                                                \
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)                                     \
    {                                                                                 \
        tick_count = (unsigned long long)(ts.tv_sec) * 1000 + (ts.tv_nsec / 1000000); \
    }                                                                                 \
    else                                                                              \
    {                                                                                 \
        perror("GET_SYSTEM_TICK_COUNT");                                              \
    }                                                                                 \
    tick_count;                                                                       \
}) // Return zero for unsupported platforms

#define DISABLE_IRQ
#define ENABLE_IRQ

#else

extern volatile uint32_t system_tick_counter_ms;
#define DISABLE_IRQ (__disable_irq())
#define ENABLE_IRQ (__enable_irq())
#define GET_SYSTEM_TICK_COUNT system_tick_counter_ms // Return zero for unsupported platforms

#endif

#endif // ___OCTOPUS_TASK_MANAGER_PLATFORM_H___
