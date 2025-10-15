/** *****************************************************************************
 * @file octopus_task_manager_timer_hal.h
 * @brief Header file for the Timer Hardware Abstraction Layer (HAL) in the Octopus Task Manager.
 *
 * This file contains function prototypes for managing and handling timer events within the Octopus platform.
 * The timer hardware abstraction layer (HAL) provides functions to initialize timers and handle their interrupt
 * events. The defined event (TIMER_1000_MS_EVT) corresponds to a 1000 ms timer interrupt.
 *
 * The functions declared in this file are platform-specific and will be implemented based on the target platform's timer system.
 *
 * @note This header file is part of the Octopus Task Manager system, specifically for timer-related functionality.
 * @version  1.0.0
 * @date 2024-12-12
 * @author   Octopus Team
 *******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_TIMER_HAL_H__
#define __OCTOPUS_TASK_MANAGER_TIMER_HAL_H__

#include "octopus_base.h" //  Base include file for the Octopus project.

#ifdef __cplusplus
extern "C"
{
#endif

// Define the event ID for a 1000 ms timer interrupt.
#define TIMER_1000_MS_EVT 0x0001

    /*********************************************************************
     * FUNCTIONS
     */

    /**
     * @brief Initializes the specified timer.
     *
     * This function is responsible for setting up the timer hardware on the platform, based on the provided timer ID.
     * It ensures the timer is configured properly and ready to trigger events, such as interrupts.
     *
     * @param timer_id The ID of the timer to initialize. This ID corresponds to a specific hardware timer on the platform.
     *
     * @note The implementation of this function depends on the platform's timer hardware configuration.
     */
    void hal_timer_init(uint8_t timer_id);

    /**
     * @brief Callback function that handles timer interrupt events.
     *
     * This function is called whenever a timer interrupt occurs. The event passed to the function identifies the
     * interrupt type or the specific timer event that triggered the callback.
     *
     * @param evt The event triggered by the timer interrupt. This event value is used to determine the appropriate
     *            action or response to the interrupt.
     *
     * @note The actual handling of events should be done within this function based on the event type.
     */
    // void hal_timer_interrupt_callback(uint8_t evt);
    void otsm_timer_init(void);
#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_TIMER_HAL_H__ */
