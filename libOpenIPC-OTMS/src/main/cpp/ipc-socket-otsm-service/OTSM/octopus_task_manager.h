/*******************************************************************************
 * @file    octopus_task_manager.h
 * @brief   Task manager header file for the Octopus platform.
 *          Provides declarations for task state machine management, including
 *          initialization, state transitions, and task-specific configurations.
 *
 * @details This file defines macros, data structures, and function prototypes
 *          for managing tasks within the Octopus system. Each task operates as
 *          a finite state machine with well-defined states and transitions.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *
 * @note    Ensure that task-specific state functions are implemented and linked
 *          correctly in the corresponding source files.
 ******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_H__
#define __OCTOPUS_TASK_MANAGER_H__

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "octopus_base.h" //  Base include file for the Octopus project.

/*******************************************************************************
 * MACROS
 ******************************************************************************/

/** Task states for Octopus Task Manager State Machine (OTMS). */
#define OTMS_S_INIT (0x00U)       /**< Initial state. */
#define OTMS_S_START (0x01U)      /**< Start state. */
#define OTMS_S_ASSERT_RUN (0x02U) /**< Assert run pre-check state. */

#define OTMS_S_RUNNING (0x03U) /**< Actively running state. */

#define OTMS_S_POST_RUN (0x04U) /**< Cleanup or post-run transition state. */
#define OTMS_S_STOP (0x05U)     /**< Task stopped state. */

#define OTMS_S_COUNT (6U) /**< Total number of defined states. */

#define OTMS_S_INVALID (0xFFU) /**< Invalid/uninitialized state. */

/** Macro to set a task's state. */
#define OTMS(TASK_MODULE, state) otms_set_state(TASK_MODULE, state)

/** Macro to get a task's current state. */
#define GET_OTMS_STATE(TASK_MODULE) otms_get_state(TASK_MODULE)

/*******************************************************************************
 * TYPEDEFS
 ******************************************************************************/

/** Task ID type. */
typedef int32_t otms_id_t;

/** Task state type. */
typedef uint8_t otms_state_t;

/** State handler function type. */
typedef void (*otms_state_func_t)(void);

/**
 * @brief Task state machine definition.
 * Contains the maximum state and function pointer array for each state.
 */
typedef struct
{
  // otms_state_t state_limit;                   /**< Max valid state value for task. */
  const otms_state_func_t func[OTMS_S_COUNT]; /**< State handler functions. */
} otms_t;

/**
 * @brief Unique task module identifiers.
 */
typedef enum
{
  TASK_MODULE_SYSTEM = 0, /**< System task. */
  TASK_MODULE_GPIO,       /**< GPIO task. */

  TASK_MODULE_PTL_1, /**< Protocol handling task. */
  TASK_MODULE_UPF,   /**< Protocol handling task. */
  TASK_MODULE_IPC,   /**< IPC socket service. */

  TASK_MODULE_CAR_INFOR, /**< Car information. */
  TASK_MODULE_CAN,

  TASK_MODULE_KEY, /**< Key input. */
  TASK_MODULE_BLE, /**< BLE communication. */
  TASK_MODULE_4G,
  TASK_MODULE_BT,
  TASK_MODULE_BAFANG,
  TASK_MODULE_LING_HUI_LIION2,
  TASK_MODULE_UPDATE_MCU, /**< MCU firmware update. */

  TASK_MODULE_MAX_NUM /**< Total number of tasks. */
} TaskModule_t;

#ifdef __cplusplus
extern "C"
{
#endif

  /*******************************************************************************
   * FUNCTION DECLARATIONS
   ******************************************************************************/

  /**
   * @brief Initialize the task manager, setting all tasks to INIT.
   */
  void otms_task_manager_init(void);

  /**
   * @brief Start all tasks by transitioning them to the START state.
   */
  void otms_task_manager_start(void);

  /**
   * @brief Stop all tasks by setting them to the STOP state.
   */
  void otms_task_manager_stop(void);
  void otms_task_manager_run(void);

  /**
   * @brief Transition all tasks into the RUNNING state.
   */
  void otms_on_enter_run(void);

  /**
   * @brief Transition all tasks out of the POST_RUN state.
   */
  void otms_on_exit_post_run(void);

  /**
   * @brief Set the current state for a specific task.
   * @param task_module Task ID to modify.
   * @param state   New state to apply.
   */
  void otms_set_state(otms_id_t task_module, otms_state_t state);

  /**
   * @brief Get the current state of a specific task.
   * @param task_module Task ID to query.
   * @return Current state.
   */
  otms_state_t otms_get_state(otms_id_t task_module);

  /**
   * @brief Get the global task state configuration table.
   * @return Pointer to array of task state machines.
   */
  const otms_t *otms_get_config(void);

  void task_manager_stop_except_1(TaskModule_t task_module1);
  void task_manager_stop_except_2(TaskModule_t task_module1, TaskModule_t task_module2);
  void task_manager_start_module(TaskModule_t TaskModule);
#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_H__ */
