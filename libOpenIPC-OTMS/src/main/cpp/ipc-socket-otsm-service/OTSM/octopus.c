/*******************************************************************************
 * @file     octopus.c
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

/*******************************************************************************
 * INCLUDES
 */
#include "octopus.h"
#include "octopus_flash.h"
#include "octopus_gpio.h"
#include "octopus_uart_ptl.h"
#include "octopus_uart_upf.h"
#include "octopus_uart_hal.h"
#include "octopus_msgqueue.h"     // Include message queue header for task communication
#include "octopus_task_manager.h" // Include task manager for scheduling tasks
#include "octopus_message.h"      // Include message id for inter-task communication
#include "octopus_tickcounter.h"  // Include tick counter for timing operations
#include "octopus_system.h"
#include "octopus_platform.h"
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
/* Debug macros are not defined here, but this section is usually for enabling
   and disabling debug prints */

/*******************************************************************************
 * LOCAL FUNCTIONS DECLARE
 */
void TaskManagerStateStartRunning(void);
void TaskManagerStateStopRunning(void);

/* Local functions are declared here, but no specific ones are listed */

/*******************************************************************************
 * MACROS
 */
#define MAIN_TASK_TIMER_INTERVAL 10 // Main task loop timer interval (in ms)

/*******************************************************************************
 * TYPEDEFS
 */
/* Type definitions could be included here, but they are not defined */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
#if defined(PLATFORM_ITE_OPEN_RTOS) || defined(PLATFORM_LINUX_RISC)
static pthread_t thread_task = 0;  // Thread handle for task manager event loop
static pthread_attr_t thread_attr; // Thread attributes (for setting stack size, etc.)
bool stop_thread = false;
#endif

uint8_t TaskManagerStateMachine_Id_ = 0; // Task manager state machine identifier

/*******************************************************************************
 * STATIC VARIABLES
 */
/* Static variables are typically used internally and not exposed globally */

/*******************************************************************************
 * FUNCTION DEFINITIONS
 */

/**
 * @brief Gets the current Task Manager State Machine ID.
 * @return The Task Manager State Machine ID.
 */
uint8_t GetTaskManagerStateMachineId(void)
{
    return TaskManagerStateMachine_Id_;
}

void otsm_print_logo(void)
{
    LOG_NONE("-----------------------------------------------------------------------------\r\n");
    LOG_NONE("               _____                                 \r\n");
    LOG_NONE(" ______ _________  /_______ ________ ____  __________\r\n");
    LOG_NONE(" _  __ \\_  ___/_  __/_  __ \\___  __ \\_  / / /__  ___/\r\n");
    LOG_NONE(" / /_/ // /__  / /_  / /_/ /__  /_/ // /_/ / _(__  ) \r\n");
    LOG_NONE(" \\____/ \\___/  \\__/  \\____/ _  .___/ \\__,_/  /____/  \r\n");
    LOG_NONE("                            /_/                       \r\n");
    LOG_NONE(" Embedded Real-Time Task Scheduler + FSM Engine\r\n");

    LOG_NONE(" Firmware  : v%s\r\n", OTMS_VERSION_NAME);
    LOG_NONE(" Compiled  : %s %s\r\n", __DATE__, __TIME__);
    LOG_NONE(" Module    : %s\r\n", flash_get_current_bank_name());
    LOG_NONE(" Author    : Octopus Dev Team\r\n");
    LOG_NONE("-----------------------------------------------------------------------------\r\n");
}

/**
 * @brief Initializes the Task Manager State Machine.
 * @param task_id The task ID to initialize.
 * @return 0 on success, non-zero on failure.
 */
#if defined(PLATFORM_LINUX_RISC)
__attribute__((constructor)) void TaskManagerStateMachineInit(void)
#elif defined(PLATFORM_CST_OSAL_RTOS)
void TaskManagerStateMachineInit(uint8_t task_id)
#else
void TaskManagerStateMachineInit(void)
#endif
{
    otsm_print_logo();
#ifdef PLATFORM_CST_OSAL_RTOS
    TaskManagerStateMachine_Id_ = task_id; // Store the task ID in the global variable
#endif
	LOG_NONE("\r\n");
    /// LOG_NONE("\r\n\r\n");//[1B blob data]
#ifdef TASK_MANAGER_STATE_MACHINE_SOC
    /// LOG_NONE("\r\n######################################BOOT  START######################################\r\n");
    TaskManagerStateStopRunning();
#endif
    char version_str[32];
    flash_decode_active_version(FLASH_BANK_CONFIG_MODE_SLOT, version_str, sizeof(version_str),__DATE__, __TIME__);
	  
    LOG_LEVEL("OTMS initializing  :%02x\r\n", TaskManagerStateMachine_Id_);
    LOG_LEVEL("OTMS version       :%s \r\n", version_str);

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize hardware abstraction layers (HAL)
#ifdef TASK_MANAGER_STATE_MACHINE_GPIO
    otsm_gpio_init(); // Initialize GPIO
#endif
#ifdef TASK_MANAGER_STATE_MACHINE_FLASH
    otsm_flash_init();
#endif
#ifdef TASK_MANAGER_STATE_MACHINE_SIF
    otsm_timer_init(); // Initialize timer with interval of 5 (could be milliseconds)
    otsm_sif_init();
#endif
#ifdef TASK_MANAGER_STATE_MACHINE_BMS
    otsm_bms_init();
#endif
#ifdef TASK_MANAGER_STATE_MACHINE_UPF
    otsm_upf_init(upf_module_array,_UPF_MODULE_MAX_);
#endif
    otsm_uart_init(); // Initialize UART communication protocol
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // Initialize the necessary modules
    otsm_message_queue_init(); // Initialize the task message queue.
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // Initialize user task manager state machine
    otms_task_manager_init();  // Initialize the task manager
    otms_task_manager_start(); // Start the task manager
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    otsm_ptl_help();
	#ifdef TASK_MANAGER_STATE_MACHINE_UPF
    otsm_upf_help();
	#endif
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // Nofify Initialize complete
#if defined(TASK_MANAGER_STATE_MACHINE_SOC) && defined(TASK_MANAGER_STATE_MACHINE_SYSTEM)
    system_handshake_with_mcu();
#endif
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // Enable task manager state matching main loop
#ifdef PLATFORM_CST_OSAL_RTOS
    osal_start_reload_timer(TaskManagerStateMachine_Id_, DEVICE_TIMER_EVENT, MAIN_TASK_TIMER_INTERVAL); // timeout_value unit ms
#endif

#if defined(PLATFORM_ITE_OPEN_RTOS) || defined(PLATFORM_LINUX_RISC)
    TaskManagerStateStartRunning();
#endif
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // LOG_NONE("#####################################BOOT COMPLETE#####################################\r\n");
    LOG_NONE("-----------------------------------------------------------------------------\r\n");
#if defined(TASK_MANAGER_STATE_MACHINE_MCU) && defined(TASK_MANAGER_STATE_MACHINE_SYSTEM)
    system_set_mb_state(MB_POWER_ST_ON);
#endif
}

#if defined(PLATFORM_ITE_OPEN_RTOS) || defined(PLATFORM_LINUX_RISC)
__attribute__((destructor)) void exit_cleanup()
{
    LOG_LEVEL("OTSM so unloaded!\n");
    TaskManagerStateStopRunning();
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef PLATFORM_CST_OSAL_RTOS
/**
 * @brief Handles events in the Task Manager State Machine for CST OSAL RTOS.
 * @param task_id The task ID of the calling task.
 * @param events The event mask that defines which events are active.
 * @return The events that were handled (removed from the active event mask).
 */
uint16_t TaskManagerStateEventLoop(uint8 task_id, uint16 events)
{
    if (events & DEVICE_TIMER_EVENT) // If the timer event is triggered
    {
        //////////////////////////////////////////////////////////////////////////////////////////////////
        otms_task_manager_run();              // Run the task manager to handle pending tasks per MAIN_TASK_TIMER_INTERVAL ms
        return (events ^ DEVICE_TIMER_EVENT); // Remove the timer event from the active events
    }
    else if (events & DEVICE_BLE_PAIR) // If BLE pairing event is triggered
    {
        LOG_LEVEL("task_id=%d events=%d ble pair\r\n", task_id, events);
        send_message(TASK_MODULE_BLE, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_PAIRING, events); // Send BLE pair event to BLE task
        return (events ^ DEVICE_BLE_PAIR);                                                          // Remove the BLE pair event from the active events
    }
    else if (events & DEVICE_BLE_BONDED) // If BLE bonded event is triggered
    {
        LOG_LEVEL("task_id=%d events=%d ble bonded\r\n", task_id, events);
        send_message(TASK_MODULE_BLE, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_BONDED, events); // Send BLE bonded event to BLE task
        return (events ^ DEVICE_BLE_BONDED);                                                       // Remove the BLE bonded event from the active events
    }
    else if (events & DEVICE_BLE_CONNECTED) // If BLE connected event is triggered
    {
        LOG_LEVEL("task_id=%d events=%d ble connected\r\n", task_id, events);
        send_message(TASK_MODULE_BLE, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_CONNECTED, events); // Send BLE connected event to BLE task
        return (events ^ DEVICE_BLE_CONNECTED);                                                       // Remove the BLE connected event from the active events
    }
    else if (events & DEVICE_BLE_DISCONNECTED) // If BLE disconnected event is triggered
    {
        LOG_LEVEL("task_id=%d events=%d ble disconnected\r\n", task_id, events);
        send_message(TASK_MODULE_BLE, MSG_OTSM_DEVICE_BLE_EVENT, MSG_OTSM_CMD_BLE_DISCONNECTED, events); // Send BLE disconnected event to BLE task
        return (events ^ DEVICE_BLE_DISCONNECTED);                                                       // Remove the BLE disconnected event from the active events
    }
    else
    {
        LOG_LEVEL("task_id=%d default events=%d\r\n", task_id, events); // Log unhandled events
    }

    return 0; // Return 0 if no events were handled
}
void TaskManagerStateStartRunning(void)
{
}
void TaskManagerStateStopRunning(void)
{
}
#elif defined(PLATFORM_ITE_OPEN_RTOS)
/**
 * @brief Task manager state machine event loop for ITE Open RTOS.
 * @param arg Arguments passed to the thread (not used here).
 * @return NULL when the thread exits.
 */
void *TaskManagerStateEventLoop(void *arg)
{
    uint32_t wait_cnt = 0;
    LOG_LEVEL("task manager state machine event loop running\r\n"); // Log unhandled events
    while (!stop_thread)
    {
        otms_task_manager_run();                 // Run the task manager to handle tasks in the event loop
        usleep(MAIN_TASK_TIMER_INTERVAL * 1000); // Sleep for 10 millisecond to control loop frequency
    }
    return 0; // Exit the thread
}

#elif defined(PLATFORM_LINUX_RISC)

void *TaskManagerStateEventLoop(void *arg)
{
    uint32_t wait_cnt = 0;
    stop_thread = false;

    /// usleep(MAIN_TASK_TIMER_INTERVAL * 1000);
    LOG_LEVEL("task manager state machine event start running\r\n"); // Log unhandled events
    StartTickCounter(&wait_cnt);
    while (!stop_thread)
    {
        otms_task_manager_run();                 // Run the task manager to handle tasks in the event loop
        usleep(MAIN_TASK_TIMER_INTERVAL * 1000); // Sleep for 10 millisecond to control loop frequency

        if (GetTickCounter(&wait_cnt) >= 1000 * 60)
        {
            /// LOG_LEVEL("task manager state machine event running %d\r\n", wait_cnt); // Log unhandled events
            RestartTickCounter(&wait_cnt);
        }
    }
    LOG_LEVEL("task manager state machine event stoped\r\n"); // Log unhandled events
}

void TaskManagerStateStartRunning(void)
{
    // LOG_LEVEL("task manager state machine thread enter\n");
    pthread_attr_init(&thread_attr);                                                       // Initialize thread attributes
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);                    // Set thread to detached state
    pthread_attr_setstacksize(&thread_attr, CFG_OTSM_STACK_SIZE);                          // Set the stack size for the thread
    int ret = pthread_create(&thread_task, &thread_attr, TaskManagerStateEventLoop, NULL); // Create the task manager event loop thread
    pthread_attr_destroy(&thread_attr);
    if (ret != 0)
    {
        LOG_LEVEL("task manager state machine error creating thread: %s\n", strerror(ret));
    }
    else
    {
        LOG_LEVEL("task manager state machine thread started: %s\n", strerror(ret));
        // pthread_detach(thread_task); // 让线程自动释放资源
        // pthread_join(thread_task, NULL);  // 等待线程结束
        // LOG_LEVEL("task manager state machine thread finished\n");
    }
}

void TaskManagerStateStopRunning(void)
{
    stop_thread = true; // 设置标志位为 true，通知线程停止
    LOG_LEVEL("task manager state machine thread stopped!\n");
}
#else

void TaskManagerStateEventLoop(void *arg)
{
    otms_task_manager_run();
}

void TaskManagerStateStartRunning(void)
{
}

#endif
