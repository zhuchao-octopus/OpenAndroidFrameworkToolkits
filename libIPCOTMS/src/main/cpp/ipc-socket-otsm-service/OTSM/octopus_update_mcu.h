/*******************************************************************************
 * @file octopus_task_manager_update_mcu.h
 * @brief Header file for MCU update task management in the Octopus platform.
 *
 * This file defines the interface for managing the MCU update task,
 * including state transitions and associated functionality.
 *
 * @version  1.0.0
 * @date 2024-12-12
 * @author   Octopus Team
 *******************************************************************************/
#ifndef ___OCTOPUS_TASK_MANAGER_UPDATE_MCU_H___
#define ___OCTOPUS_TASK_MANAGER_UPDATE_MCU_H___

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_utils.h"
/**
 * \defgroup APP_SETTING Application: MCU Update Task Manager
 * @{
 */

/*******************************************************************************
 * DEBUG SWITCH MACROS
 *
 * Define macros to enable or disable debugging features for the MCU update
 * task manager. These macros are generally defined for debugging purposes
 * during development.
 */
#ifdef TASK_MANAGER_STATE_MACHINE_UPDATE
/*******************************************************************************
 * MACROS
 *
 * Define any general macros required for the MCU update task manager.
 * These can include constants, helper macros, or flags.
 */

/*******************************************************************************
 * TYPEDEFS
 *
 * Define data types specific to the MCU update task manager. These can include
 * enumerations, structures, or aliases for existing types.
 */
typedef enum
{
  MCU_ERROR_CODE_OK = 0,
  MCU_ERROR_CODE_BANK_MODE,
  MCU_ERROR_CODE_ADDRESS,
  MCU_ERROR_CODE_CRC,
  MCU_ERROR_CODE_FRAME,
  MCU_ERROR_CODE_FILE,
  MCU_ERROR_CODE_UNKNOW_FILE,
  MCU_ERROR_CODE_OPEN_FILE,

} mcu_error_code_t;

typedef struct
{
  uint32_t s_length;
  uint32_t s_total_length;
  uint8_t error_code;
} mcu_update_progress_t;

typedef struct
{
  file_info_t file_info;
  long file_offset;
  uint32_t s_length;
  uint16_t f_counter; // frame counter
  uint8_t error_code;
  uint32_t start_time;
} mcu_update_progress_status_t;

/*******************************************************************************
 * CONSTANTS
 *
 * Define any constant values used in the MCU update task manager.
 * These may include default settings or configuration parameters.
 */

/*******************************************************************************
 * GLOBAL VARIABLES DECLARATION
 *
 * Declare any global variables that are required for the MCU update
 * task manager. These variables will be defined in the implementation file.
 */
// extern mcu_update_progress_status_t mcu_update_status;
/*******************************************************************************
 * GLOBAL FUNCTIONS DECLARATION
 *
 * Declare the functions used to manage the lifecycle of the MCU update task.
 */

/**
 * @brief Initialize the MCU update task.
 *
 * This function is called during system initialization to prepare
 * the MCU update task for execution.
 */

#ifdef __cplusplus
extern "C"
{
#endif
  void task_update_init_running(void);

  /**
   * @brief Start the MCU update task.
   *
   * This function transitions the MCU update task to the start state and
   * performs any necessary initialization or resource allocation.
   */
  void task_update_start_running(void);

  /**
   * @brief Assert the running state of the MCU update task.
   *
   * This function checks the conditions for the MCU update task to run and
   * ensures that all prerequisites are met.
   */
  void task_update_assert_running(void);

  /**
   * @brief Execute the running state of the MCU update task.
   *
   * This function performs the main operations of the MCU update task
   * while it is in the running state.
   */
  void task_update_running(void);

  /**
   * @brief Perform post-run operations for the MCU update task.
   *
   * This function handles any required cleanup or preparations for
   * transitioning out of the running state.
   */
  void task_update_post_running(void);

  /**
   * @brief Stop the MCU update task.
   *
   * This function transitions the MCU update task to the stopped state
   * and releases any allocated resources.
   */
  void task_update_stop_running(void);

  bool update_check_oupg_file_exists(void);
  bool update_is_mcu_updating(void);
  uint8_t update_get_target_bank(void);

  mcu_update_progress_t get_mcu_update_progress(void);
  /** @} end of group APP_SETTING */

#ifdef __cplusplus
}
#endif

#endif

#endif // ___OCTOPUS_TASK_MANAGER_UPDATE_MCU_H___
