/*******************************************************************************
 * @file octopus_task_manager_ptl.h
 * @brief Header file for managing Octopus protocol tasks.
 *
 * This file defines the structure, constants, and function prototypes for handling
 * communication tasks between the MCU (Microcontroller Unit) and the APP (Application)
 * within the Octopus platform. It provides the necessary protocol definitions to
 * ensure smooth task management and communication across the platform.
 *
 * The Octopus protocol facilitates seamless interaction between hardware and software,
 * enabling efficient data exchange and task synchronization. This header file serves
 * as the interface for the task manager, providing the necessary tools to integrate
 * protocol handling into the Octopus platform.
 *
 * @version 1.0.0
 * @author   Octopus Team
 * @date     2024-12-12
 *******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_UPF_H__
#define __OCTOPUS_TASK_MANAGER_UPF_H__
/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_cfifo.h"
/*******************************************************************************
 * MACROS
 *******************************************************************************/
#define UPF_FIFO_MAX_SIZE 255
#define UPF_FRAME_MAX_SIZE 255 ///< Maximum frame size

/*******************************************************************************
 * ENUMERATIONS
 *******************************************************************************/
typedef enum
{
	SETTING_WHEEL_16_Inch,
	SETTING_WHEEL_18_Inch,
	SETTING_WHEEL_20_Inch,
	SETTING_WHEEL_22_Inch,
	SETTING_WHEEL_24_Inch,
	SETTING_WHEEL_26_Inch,
	SETTING_WHEEL_27_Inch,
	SETTING_WHEEL_27_5_Inch,
	SETTING_WHEEL_28_Inch,
	SETTING_WHEEL_29_Inch,
} SETTING_WHEEL;

typedef enum
{
	SETTING_MAX_PAS_3_LEVEL = 3,
	SETTING_MAX_PAS_5_LEVEL = 5,
	SETTING_MAX_PAS_9_LEVEL = 9,
} SETTING_MAX_PAS;

typedef enum
{
	_UPF_MODULE_LING_HUI_LIION2_,

#ifdef TASK_MANAGER_STATE_MACHINE_BAFANG
	_UPF_MODULE_BAFANG_, ///< Protocol for Bafang
#endif

#ifdef TASK_MANAGER_STATE_MACHINE_LOT4G
	_UPF_MODULE_LOT4G_,
#endif

#ifdef TASK_MANAGER_STATE_MACHINE_BT_MUSIC
	_UPF_MODULE_BT_MUSIC_,
#endif
	_UPF_MODULE_MAX_,
} UPF_MODULE;

typedef enum
{
	UPF_MODULE_ID_LING_HUI_LIION2 = 0,
	UPF_MODULE_ID_BAFANG, ///< Protocol for Bafang
	UPF_MODULE_ID_LOT4G,
	UPF_MODULE_ID_BT_MUSIC,
} UPF_MODULE_ID;

typedef enum
{
	UPF_CHANNEL_0, ///< Protocol for Bafang
	UPF_CHANNEL_1,
	UPF_CHANNEL_2,
	UPF_CHANNEL_3,
	UPF_CHANNEL_4,
	UPF_CHANNEL_5,
	UPF_CHANNEL_6,
	UPF_CHANNEL_7,
	UPF_CHANNEL_8, // LPUART1
	UPF_CHANNEL_9, // LPUART2
} UPF_CHANNEL_NUMBER;

typedef enum
{
	UPF_CHANNEL_TYPE_BYTE,
	UPF_CHANNEL_TYPE_CHAR
} UPF_CHANNEL_TYPE;

typedef struct
{
	UPF_MODULE_ID id;
	UPF_CHANNEL_NUMBER channel;
	UPF_CHANNEL_TYPE type;
} upf_module_t;

typedef struct
{
	uint16_t size;
	uint8_t buffer[UPF_FRAME_MAX_SIZE];
} upf_proc_buff_t;

typedef bool (*upf_module_receive_handler_t)(upf_proc_buff_t *buffer);

typedef struct
{
	upf_module_t upf_module;
	upf_proc_buff_t upf_proc_buff;
	upf_module_receive_handler_t receive_handler;
	cFifo_t *upf_usart_rx_fifo;
	uint8_t upf_usart_rx_fifo_buff[cFifo_ObjSize(UPF_FIFO_MAX_SIZE)];
} upf_module_info_t;

#ifdef __cplusplus
extern "C"
{
#endif
	void task_upf_init_running(void);
	void task_upf_start_running(void);
	void task_upf_assert_running(void);
	void task_upf_running(void);
	void task_upf_post_running(void);
	void task_upf_stop_running(void);

	//void upf_module_info_init(upf_module_info_t *array, uint16_t length);
	void upf_register_module(upf_module_t upf_module, upf_module_receive_handler_t receive_handler);
	void upf_receive_callback(upf_module_t upf_module, const uint8_t *buffer, uint16_t length);
	void upf_send_buffer(upf_module_t upf_module, const uint8_t *buffer, uint16_t length);
	void upf_print_registered_module(void);

  void otsm_upf_init(upf_module_info_t *array, uint16_t length);
	void otsm_upf_help(void);

#ifdef __cplusplus
}
#endif

extern upf_module_info_t upf_module_array[];
#endif /* __OCTOPUS_TASK_MANAGER_PTL_H__ */
