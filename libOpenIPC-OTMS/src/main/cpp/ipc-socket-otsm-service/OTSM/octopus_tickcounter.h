/**
 * @file octopus_task_manager_timer.h
 * @brief Header file for managing system timers within the Octopus Task Manager.
 *
 * This file provides function declarations for managing tick counters, system timekeeping,
 * and the system's tick clock. The timer functionality includes starting, stopping,
 * restarting, and querying the tick counter for elapsed time.
 *
 * The kernel timer is based on a 1ms tick, and the maximum time the timer can track
 * is approximately 49.71 days, as it is represented by a 32-bit unsigned integer.
 *
 * These functions are essential for managing time-dependent operations and scheduling tasks
 * within the Octopus Task Manager system. The timer functions support operations such as
 * time measurement, system clock adjustment, and interrupt handling for time-based events.
 *
 * @note The actual implementation of these functions will depend on the platform-specific
 * timer hardware and system clock configuration.
 *
 * @ingroup APP:SUB_TYPE
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 */

#ifndef __OCTOPUS_TASK_MANAGER_TICK_COUNTER_H__
#define __OCTOPUS_TASK_MANAGER_TICK_COUNTER_H__

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * \defgroup APP:SUB_TYPE
     * @{
     * This group is reserved for application-specific timer functions.
     */

    /*******************************************************************************
     * DEBUG SWITCH MACROS
     * Place for any debug-related macros if needed.
     */

    /*******************************************************************************
     * MACROS
     * Place for general macros used for timers and timekeeping.
     */

    /*******************************************************************************
     * TYPEDEFS
     */
    /* Kernel Timer.
     * uint32_t bit if Tick is 1ms per Tick,
     * then the maximum time is (0xFFFFFFFF) ms = 49.71 days.
     * This describes the kernel timer with a 1ms tick rate.
     */

    /*******************************************************************************
     * CONSTANTS
     * Place for defining constant values used by the timer functions.
     */

    /*******************************************************************************
     * GLOBAL VARIABLES DECLARATION
     * Place for declaring global variables used by the timer functions, if necessary.
     */

    /*******************************************************************************
     * GLOBAL FUNCTIONS DECLARATION
     * The following functions are used to manage system timers and counters.
     */

    /**
     * @brief Starts the tick counter.
     *
     * This function initializes and starts the timer, allowing the system to keep track of elapsed time.
     *
     * @param timer A pointer to the timer variable that will be started.
     */
    void StartTickCounter(uint32_t *timer);

    /**
     * @brief Stops the tick counter.
     *
     * This function stops the timer, halting the time tracking process.
     *
     * @param timer A pointer to the timer variable that will be stopped.
     */
    void StopTickCounter(uint32_t *timer);

    /**
     * @brief Restarts the tick counter.
     *
     * This function restarts the timer, resetting it to zero and beginning the time tracking from scratch.
     *
     * @param timer A pointer to the timer variable that will be restarted.
     */
    void RestartTickCounter(uint32_t *timer);

    /**
     * @brief Checks if the tick counter has started.
     *
     * This function checks if the timer is currently running.
     *
     * @param timer A pointer to the timer variable to check.
     * @return true if the timer is running, false otherwise.
     */
    bool IsTickCounterStart(const uint32_t *timer);

    /**
     * @brief Retrieves the current value of the tick counter.
     *
     * This function returns the current value of the timer, representing the elapsed time since it was started.
     *
     * @param timer A pointer to the timer variable whose value will be returned.
     * @return The current tick count of the timer.
     */
    uint32_t GetTickCounter(const uint32_t *timer);

    /**
     * @brief Retrieves the system tick clock value.
     *
     * This function returns the system tick clock value, which represents the frequency of the system's timer.
     *
     * @return The system tick clock value.
     */
    uint32_t GetSystemTickClock(void);

    /**
     * @brief Sets the system tick clock.
     *
     * This function sets the system's tick clock frequency.
     *
     * @param timer_tick The new tick value to set for the system's clock.
     */
    void SetSystemTickClock(uint32_t timer_tick);

    /**
     * @brief Resets the system tick clock.
     *
     * This function resets the system's tick clock to its default value.
     */
    void ResetSystemTickClock(void);

    /**
     * end of group
     * @}
     */

#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_TIMER_H__ */
