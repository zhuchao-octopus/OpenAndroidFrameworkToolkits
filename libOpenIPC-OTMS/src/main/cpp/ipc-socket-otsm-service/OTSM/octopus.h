/*******************************************************************************
 * @file     octopus.h
 * @brief    Header file for the Octopus Task Manager.
 *           This file provides function declarations, macros, and type definitions
 *           for managing tasks and event loops in the Octopus system.
 *           It supports platform-specific configurations and RTOS choices.
 *
 * @note     This file is part of the Octopus project and should be included
 *           in the task manager implementation files.
 *           It allows users to define task states, events, and handle
 *           the task scheduling mechanisms.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team

 ******************************************************************************/

#ifndef ___OCTOPUS_TASK_MANAGER_OCTOPUS_H___
#define ___OCTOPUS_TASK_MANAGER_OCTOPUS_H___

/*******************************************************************************
 * INCLUDE FILES
 * Include standard libraries and platform-specific headers.
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

/*******************************************************************************
 * DEBUG SWITCH MACROS
 * Add macros for enabling or disabling debug functionality.
 */

/*******************************************************************************
 * GENERAL MACROS
 * Define common bit manipulation macros and constants.
 */

/*******************************************************************************
 * TYPE DEFINITIONS
 * Define any necessary types or structs here.
 */

/*******************************************************************************
 * CONSTANTS
 * Define any necessary constant values here.
 */

/*******************************************************************************
 * FUNCTION DECLARATIONS
 * Declare external functions used in the task manager.
 */
#ifdef __cplusplus
extern "C"
{
#endif

    // Retrieves the main user task ID.
    uint8_t GetTaskManagerStateMachineId(void);

// The main event loop for the task manager.
#ifdef PLATFORM_CST_OSAL_RTOS
    // Initializes the task manager's main task.
    void TaskManagerStateMachineInit(uint8_t task_id);
    uint16_t TaskManagerStateEventLoop(uint8_t task_id, uint16_t events);

#elif defined(PLATFORM_ITE_OPEN_RTOS)
void TaskManagerStateMachineInit(void);
void TaskManagerStateEventLoop(void *arg);
void *TaskManagerStateEventLoop(void *arg);

#elif defined(PLATFORM_LINUX_RISC)
void TaskManagerStateMachineInit(void);
void TaskManagerStateStopRunning(void);
void *TaskManagerStateEventLoop(void *arg);

#else

void TaskManagerStateMachineInit(void);
void TaskManagerStateEventLoop(void *arg);
#endif

#ifdef __cplusplus
}
#endif

#endif // ___OCTOPUS_TASK_MANAGER_H___
