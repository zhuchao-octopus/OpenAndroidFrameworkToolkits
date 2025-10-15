
#ifndef __TASK_COM_UART_PTL_BAFANG_H__
#define __TASK_COM_UART_PTL_BAFANG_H__

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_uart_upf.h" // Include UART protocol header

#ifdef __cplusplus
extern "C"
{
#endif
    void task_bfang_ptl_init_running(void);
    void task_bfang_ptl_start_running(void);
    void task_bfang_ptl_assert_running(void);
    void task_bfang_ptl_running(void);
    void task_bfang_ptl_post_running(void);
    void task_bfang_ptl_stop_running(void);

    uint16_t get_top_error_code(void);
    uint16_t get_wheel_radius_mm(void);
    // uint16_t get_wheel_radius_inch(void);
    void add_error_code(uint16_t code);
    void TRIP_Reset_mileage(void);
    void Factory_Setting_mileage(void);

#ifdef __cplusplus
}
#endif

extern upf_module_t upf_module_info_BAFANG;

#endif
