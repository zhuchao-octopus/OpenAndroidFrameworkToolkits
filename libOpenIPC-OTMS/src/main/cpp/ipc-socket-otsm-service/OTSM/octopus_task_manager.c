/*******************************************************************************
 * @file        octopus_task_manager.c
 * @brief       Task manager module for managing multiple tasks and their states
 * octopus task  state machine (otsm)
 * octopus task manager system (otms)
 * This file implements a task manager that controls the lifecycle of tasks
 * in a system. Tasks can transition between various predefined states,
 * such as INIT, START, RUN, and STOP. Each task has its own state and
 * corresponding state-specific function handlers. The task manager provides
 * APIs for initializing, starting, stopping, and running tasks.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *
 * @note        This module assumes a fixed number of tasks (`TASK_MODULE_MAX_NUM`)
 *              and relies on a configuration (`otms_t`) for task-specific
 *              state handling.
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 */
#include "octopus_task_manager.h" // Include task manager for scheduling tasks

/*******************************************************************************
 * LOCAL FUNCTION DECLARATIONS
 ******************************************************************************/
extern const otms_t task_module_config_table[];
/**
 * @brief Executes the state-specific function for a given task.
 *
 * @param TASK_MODULE ID of the task to execute.
 * @param state   State to execute for the task.
 */
inline static void otms_exec_state(otms_id_t TASK_MODULE, otms_state_t state);

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Add any global variables here */

/*******************************************************************************
 * STATIC VARIABLES
 ******************************************************************************/

/**
 * @brief Array to store the current state of each task.
 */
static otms_state_t otms_task_state[TASK_MODULE_MAX_NUM];

/*******************************************************************************
 * EXTERNAL VARIABLES
 ******************************************************************************/

/* Add any external variables here */

/*******************************************************************************
 * FUNCTION IMPLEMENTATIONS
 ******************************************************************************/

/**
 * @brief Gets the task manager configuration.
 *
 * @return Pointer to the task manager configuration.
 */
const otms_t *otms_get_config(void)
{
    return task_module_config_table;
}

/**
 * @brief Initializes the task manager and sets all tasks to the INIT state.
 */
void otms_task_manager_init(void)
{
    otms_id_t i;
    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        otms_exec_state(i, OTMS_S_INIT); // Set each task to the INIT state.
    }
}

/**
 * @brief Starts all tasks by transitioning them to the START state.
 */
void otms_task_manager_start(void)
{
    otms_id_t i;
    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        otms_exec_state(i, OTMS_S_START); // Set each task to the START state.
    }
}

void task_manager_start_module(TaskModule_t TaskModule)
{
    otms_id_t i;
    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        if (i == TaskModule)
            otms_exec_state(i, OTMS_S_START); // Set each task to the START state.
    }
}
/**
 * @brief Stops all tasks by transitioning them to the STOP state.
 */
void otms_task_manager_stop(void)
{
    otms_id_t i;

    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        otms_exec_state(i, OTMS_S_STOP); // Set each task to the STOP state.
    }
}

void task_manager_stop_except_1(TaskModule_t task_module1)
{
    otms_id_t i;

    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        if (i != task_module1)
            otms_exec_state(i, OTMS_S_STOP); // Set each task to the STOP state.
    }
}

void task_manager_stop_except_2(TaskModule_t task_module1, TaskModule_t task_module2)
{
    otms_id_t i;

    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        if (i != task_module1 && i != task_module2)
            otms_exec_state(i, OTMS_S_STOP); // Set each task to the STOP state.
    }
}

/**
 * @brief Runs the current state-specific function for all tasks.
 */
void otms_task_manager_run(void)
{
    otms_id_t i;

    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        otms_exec_state(i, otms_task_state[i]); // Execute the current state for each task.
    }
}

/**
 * @brief Sets the state of a specific task.
 *
 * @param task_module ID of the task.
 * @param state   New state to set for the task.
 */
void otms_set_state(otms_id_t task_module, otms_state_t state)
{
    if (task_module < TASK_MODULE_MAX_NUM)
    {
        otms_task_state[task_module] = state; // Update the task state.
    }
    else
    {
        MY_ASSERT(0); // Invalid task ID.
    }
}

/**
 * @brief Gets the current state of a specific task.
 *
 * @param task_module ID of the task.
 * @return Current state of the task.
 */
otms_state_t otms_get_state(otms_id_t task_module)
{
    otms_state_t state;

    if (task_module < TASK_MODULE_MAX_NUM)
    {
        state = otms_task_state[task_module];
    }
    else
    {
        state = OTMS_S_INVALID; // Invalid state.
        MY_ASSERT(0);           // Invalid task ID.
    }
    return state;
}

/**
 * @brief Handles transitions when entering the RUN state.
 */
void otms_on_enter_run(void)
{
    otms_id_t i = 0;

    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        if (otms_get_state(i) > OTMS_S_POST_RUN)
        {
            otms_set_state(i, OTMS_S_START); // Reset tasks to the START state.
        }
    }
}

/**
 * @brief Handles transitions when exiting the POST_RUN state.
 */
void otms_on_exit_post_run(void)
{
    otms_id_t i = 0;

    for (i = 0; i < TASK_MODULE_MAX_NUM; i++)
    {
        otms_set_state(i, OTMS_S_STOP); // Transition tasks to the STOP state.
    }
}

/**
 * @brief Executes the state-specific function for a given task.
 *
 * @param task_module ID of the task to execute.
 * @param state   State to execute for the task.
 */
static void otms_exec_state(otms_id_t task_module, otms_state_t state)
{
    const otms_t *cfg = otms_get_config();

    if ((task_module < TASK_MODULE_MAX_NUM) && (NULL != cfg) && (OTMS_S_COUNT > state) && (OTMS_S_INVALID != state))
    {
        if (NULL != cfg[task_module].func[state])
        {
            cfg[task_module].func[state](); // Call the state-specific function.
        }
    }
}
