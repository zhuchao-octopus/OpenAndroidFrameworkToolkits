
/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_uart_upf.h" // Include UART protocol header
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef enum
{
    DRIVE_MOD_REAR = 0, // 后驱
    DRIVE_MOD_DOUBLE,   // 双驱
    DRIVE_MOD_FRONT,    // 前驱
} drive_mode_;
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * STATIC VARIABLES
 */

/*******************************************************************************
 * EXTERNAL VARIABLES
 */

void task_lhl2_ptl_init_running(void);
void task_lhl2_ptl_start_running(void);
void task_lhl2_ptl_assert_running(void);
void task_lhl2_ptl_running(void);
void task_lhl2_ptl_post_running(void);
void task_lhl2_ptl_stop_running(void);

extern upf_module_t upf_module_info_LING_HUI_LIION2;
