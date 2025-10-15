/*******************************************************************************
 * @file    octopus_mcu_update.c
 * @brief   MCU update management and reboot state machine implementation.
 *
 * This file contains the implementation for managing MCU firmware updates,
 * handling reboot sequences, and interacting with various hardware modules.
 *
 * @details
 * - Provides initialization and control functions for the MCU update task.
 * - Implements a state machine for managing reboot sequences.
 * - Handles communication with other system modules for update and reboot processes.
 * - Offers support for flash memory operations during firmware updates.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *
 * @note    Ensure all dependent modules are correctly initialized before using
 *          the functions in this file.
 */

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_update_mcu.h" // Include header for MCU update management
#include "octopus_flash.h"		// Include flash memory handling utilities
#include "octopus_uart_ptl.h"	 // Include UART protocol header
#include "octopus_uart_upf.h"	 // Include UART protocol header
#include "octopus_tickcounter.h" // Include tick counter for timing operations
#include "octopus_msgqueue.h"	 // Include message queue header for task communication
#include "octopus_message.h"	 // Include message id for inter-task communication
#include "octopus_platform.h"
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */

/*******************************************************************************
 * MACROS
 */
#define MCU_MODEL_NAME "HK32L088KBT6"

#ifdef TASK_MANAGER_STATE_MACHINE_UPDATE
/*******************************************************************************
 * TYPEDEFS
 */
// Define enumeration for MCU reboot state machine states
typedef enum
{
	MCU_UPDATE_STATE_IDLE,
	MCU_UPDATE_STATE_CHECK,
	MCU_UPDATE_STATE_INIT,
	MCU_UPDATE_STATE_ERASE,
	MCU_UPDATE_STATE_RECEIVING,
	MCU_UPDATE_STATE_UPDATING,
	MCU_UPDATE_STATE_COMPLETE,
	MCU_UPDATE_STATE_EXIT,
	MCU_UPDATE_STATE_ERROR,
} mcu_update_state_t;

// Define a buffer structure used for flash programming during firmware updates
typedef struct
{
	uint32_t bank_slot; // Indicates the firmware bank or partition being updated (e.g., bank A or B)
	uint32_t bank_address;
	uint32_t total_crc_32; // Expected total CRC32 checksum for the complete firmware image (used for integrity verification)
	uint32_t total_length; // Total expected length (in bytes) of the firmware image being programmed

	uint32_t r_address; // Current flash memory address where the next chunk of data will be written
	uint8_t r_buff[64]; // Temporary buffer for holding a chunk of data (up to 64 bytes) before writing to flash
	uint8_t r_length;	// Number of valid data bytes currently stored in 'buff'

	uint8_t state;		 // State indicator for the programming process (e.g., idle, receiving, writing, error)
	uint8_t error_count; // Counts how many errors (e.g., CRC mismatch, timeout) have occurred during programming
	uint16_t f_count;	 // Total number of data frame packets received successfully

} program_buf_t;

static program_buf_t lt_mcu_program_buf;

mcu_update_progress_status_t mcu_upgrade_status;
/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */
#ifdef TASK_MANAGER_STATE_MACHINE_SOC
static FILE *file_handle_oupg = NULL;
char file_path_name_upgrade[64] = {0};
#endif
/*******************************************************************************
 * STATIC VARIABLES
 */
// Declare static variables for managing reboot process
static uint32_t l_t_msg_wait_2000_timer; // Timer for 1000 ms message wait
/*******************************************************************************
 * EXTERNAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */
// Function declarations
static bool update_send_handler(ptl_frame_type_t module, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff); // Handle outgoing messages
static bool update_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);						   // Handle incoming messages
static void update_state_process(void);

bool update_and_verify_dest_bank(uint32_t slot_addr);
uint32_t update_get_model_number(void);

#ifdef TASK_MANAGER_STATE_MACHINE_SOC
static file_read_status_t read_next_record(FILE *fp, long *file_offset, file_type_t type, hex_record_t *record);
#endif
/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */

/**
 * @brief Initialize the MCU update module and register communication handlers.
 */
void task_update_init_running(void)
{
	ptl_register_module(MCU_TO_SOC_MOD_UPDATE, update_send_handler, update_receive_handler); // Register handlers for module communication
	OTMS(TASK_MODULE_UPDATE_MCU, OTMS_S_INVALID);											 // Set the task state to invalid (not running)
	LOG_LEVEL("task_update_mcu_init_running\r\n");
}

/**
 * @brief Start the MCU update task by setting it to the assert run state.
 */
void task_update_start_running(void)
{
	OTMS(TASK_MODULE_UPDATE_MCU, OTMS_S_ASSERT_RUN); // Assert task state to running
	LOG_LEVEL("task_update_mcu_start_running\r\n");
	lt_mcu_program_buf.state = MCU_UPDATE_STATE_IDLE;
	lt_mcu_program_buf.f_count = 0;
}

/**
 * @brief Assert the MCU update task to a running state, ensuring it is active.
 */
void task_update_assert_running(void)
{
	ptl_reqest_running(MCU_TO_SOC_MOD_UPDATE);	  // Request module to be active
	OTMS(TASK_MODULE_UPDATE_MCU, OTMS_S_RUNNING); // Set task state to running
}

/**
 * @brief Main task function for managing the MCU update process.
 */
void task_update_running(void)
{
#ifdef TASK_MANAGER_STATE_MACHINE_MCU
	update_state_process(); // Call the reboot state machine processing function
#endif
}

/**
 * @brief Post-task actions after the MCU update task completes.
 */
void task_update_post_running(void)
{
	ptl_release_running(MCU_TO_SOC_MOD_UPDATE);		 // Release the active module
	OTMS(TASK_MODULE_UPDATE_MCU, OTMS_S_ASSERT_RUN); // Reset task state to assert run
}

/**
 * @brief Stop the MCU update task by invalidating its state.
 */
void task_update_stop_running(void)
{
	LOG_LEVEL("_stop_running\r\n");
	OTMS(TASK_MODULE_UPDATE_MCU, OTMS_S_INVALID); // Set the task state to invalid
}

/*******************************************************************************
 * LOCAL FUNCTIONS IMPLEMENTATION
 */

// Function to handle sending update module requests based on the module type and parameters
bool update_send_handler(ptl_frame_type_t frame_type, uint16_t param1, uint16_t param2, ptl_proc_buff_t *buff)
{
	MY_ASSERT(buff);
	uint8_t buffer[8] = {0};
	// PRINT("update_module_send_handler  MOD %02x  CMD %02x PRARM %04x\n", module, cmd, param);
	if (MCU_TO_SOC_MOD_UPDATE == frame_type)
	{
		switch (param1)
		{
		case FRAME_CMD_UPDATE_UPDATE_FW_STATE:
		{
			buffer[0] = lt_mcu_program_buf.state;
			buffer[1] = lt_mcu_program_buf.state; // ack success
			ptl_build_frame(MCU_TO_SOC_MOD_UPDATE, FRAME_CMD_UPDATE_UPDATE_FW_STATE, buffer, 2, buff);
			return true;
		}
		case FRAME_CMD_UPDATE_REQUEST_FW_DATA: // request data from
		{
			// bootloader_iap_mode = 1;
			buffer[0] = lt_mcu_program_buf.state;			// ack success
			buffer[1] = MK_MSB(lt_mcu_program_buf.f_count); // ack success
			buffer[2] = MK_LSB(lt_mcu_program_buf.f_count);
			ptl_build_frame(MCU_TO_SOC_MOD_UPDATE, FRAME_CMD_UPDATE_REQUEST_FW_DATA, buffer, 3, buff);
			return true;
		}

		default:
			break;
		}
	}
#ifdef TASK_MANAGER_STATE_MACHINE_SOC
	else if (SOC_TO_MCU_MOD_UPDATE == frame_type)
	{
		switch (param1)
		{
		case FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE:
		{
			if (!file_exists(file_path_name_upgrade))
			{
				LOG_LEVEL("File do not exists : %s\n", file_path_name_upgrade);
				return false;
			}

			if (!flash_is_allow_update_bank(update_get_target_bank()))
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE failed bank: %d flash_meta_infor.bank_slot_mode=%d\r\n", update_get_target_bank(), flash_meta_infor.bank_slot_mode);
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_BANK_MODE;
				send_message(TASK_MODULE_IPC, MSG_OTSM_DEVICE_MCU_EVENT, MSG_OTSM_CMD_MCU_UPDATING, 0);
				return false;
			}

			uint32_t target_bank_address = flash_get_bank_address(update_get_target_bank());
			mcu_upgrade_status.file_offset = flash_get_bank_offset_address(update_get_target_bank());

			uint32_t model_number = update_get_model_number();

			mcu_upgrade_status.file_info = parse_firmware_file(model_number, mcu_upgrade_status.file_offset, file_path_name_upgrade);
			uint32_t bank_address = mcu_upgrade_status.file_info.reset_handler & FLASH_BANK_MASK;
			mcu_upgrade_status.s_length = 0;
			mcu_upgrade_status.f_counter = 0;

			LOG_LEVEL("Target Bank   : %s\n", flash_get_bank_name(update_get_target_bank()));
			LOG_LEVEL("Model Number  : %08x\n", model_number);
			LOG_LEVEL("File Type     : %d\n", mcu_upgrade_status.file_info.file_type);
			LOG_LEVEL("File Size     : %zu bytes\n", mcu_upgrade_status.file_info.file_size);
			LOG_LEVEL("File Version  : 0x%08X\n", mcu_upgrade_status.file_info.file_version);
			LOG_LEVEL("File CRC-32   : 0x%08X\n", mcu_upgrade_status.file_info.file_crc_32);
			LOG_LEVEL("Reset Handler : 0x%08X\n", mcu_upgrade_status.file_info.reset_handler);

			if ((!flash_is_valid_bank_address(0, bank_address)) || (target_bank_address != bank_address))
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE bank_address is invalid: %08x / %08x\r\n", bank_address, target_bank_address);
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_ADDRESS;
				send_message(TASK_MODULE_IPC, MSG_OTSM_DEVICE_MCU_EVENT, MSG_OTSM_CMD_MCU_UPDATING, 0);
				return false;
			}

			if (mcu_upgrade_status.file_info.file_type == FILE_TYPE_UNKNOWN || mcu_upgrade_status.file_info.file_size == 0)
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_UPDATE_FW_STATE error file_type=%d total_length=%d bank_address=%08x\r\n", mcu_upgrade_status.file_info.file_type,
						  mcu_upgrade_status.file_info.file_size, mcu_upgrade_status.file_info.reset_handler);
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_UNKNOW_FILE;
				send_message(TASK_MODULE_IPC, MSG_OTSM_DEVICE_MCU_EVENT, MSG_OTSM_CMD_MCU_UPDATING, 0);
				return false;
			}

			task_manager_stop_except_2(TASK_MODULE_UPDATE_MCU, TASK_MODULE_PTL_1);
			task_manager_start_module(TASK_MODULE_IPC);
			UINT32_TO_BYTES_LE(bank_address, &buffer[0]);
			UINT32_TO_BYTES_LE(mcu_upgrade_status.file_info.file_size, &buffer[4]);

			file_handle_oupg = fopen(file_path_name_upgrade, "r");
			if (file_handle_oupg)
			{
				ptl_build_frame(SOC_TO_MCU_MOD_UPDATE, FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE, buffer, 8, buff);
				return true;
			}
			else
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE open file error %s\r\n", file_path_name_upgrade);
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_OPEN_FILE;
				send_message(TASK_MODULE_IPC, MSG_OTSM_DEVICE_MCU_EVENT, MSG_OTSM_CMD_MCU_UPDATING, 0);
				return false;
			}
		}
		case FRAME_CMD_UPDATE_EXITS_FW_UPGRADE_MODE:
		{
			if (file_handle_oupg)
			{
				fclose(file_handle_oupg);
				file_handle_oupg = NULL;
			}
			UINT32_TO_BYTES_LE(mcu_upgrade_status.file_info.file_crc_32, &buffer[0]);
			UINT32_TO_BYTES_LE(mcu_upgrade_status.s_length, &buffer[4]);
			ptl_build_frame(SOC_TO_MCU_MOD_UPDATE, FRAME_CMD_UPDATE_EXITS_FW_UPGRADE_MODE, buffer, 8, buff);
			return true;
		}

		default:
			break;
		}
	}
#endif
	return false;
}

// Function to handle receiving update module requests, process the payload, and send acknowledgment.
bool update_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff)
{
	MY_ASSERT(payload);
	MY_ASSERT(ackbuff);
	uint8_t buffer[64] = {0};

	if (SOC_TO_MCU_MOD_UPDATE == payload->frame_type)
	{
		switch (payload->frame_cmd)
		{
		case FRAME_CMD_UPDATE_CHECK_FW_STATE:
		case FRAME_CMD_UPDATE_UPDATE_FW_STATE: // second
		{
			LOG_LEVEL("FRAME_CMD_UPDATE_UPDATE_FW_STATE \n");
			buffer[0] = lt_mcu_program_buf.state;
			buffer[1] = lt_mcu_program_buf.state;
			ptl_build_frame(MCU_TO_SOC_MOD_UPDATE, FRAME_CMD_UPDATE_CHECK_FW_STATE, buffer, 2, ackbuff);
			return true;
		}
		case FRAME_CMD_UPDATE_ENTER_FW_UPGRADE_MODE: // first
		{
			// LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPDATE \n");
			// uint32_t address = BYTES_TO_UINT32_LE(&payload->data[0]);
			lt_mcu_program_buf.bank_address = BYTES_TO_UINT32_LE(&payload->data[0]);
			lt_mcu_program_buf.total_length = BYTES_TO_UINT32_LE(&payload->data[4]);
			lt_mcu_program_buf.total_crc_32 = 0;
			lt_mcu_program_buf.error_count = 0;
			lt_mcu_program_buf.r_length = 0;
			lt_mcu_program_buf.f_count = 0;
			lt_mcu_program_buf.bank_slot = BANK_SLOT_INVALID;
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_IDLE;

			LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPDATE bank_address=%08x total_length=%d\r\n", lt_mcu_program_buf.bank_address, lt_mcu_program_buf.total_length);
			LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPDATE bank_address=%08x bank_slot=%d\r\n", lt_mcu_program_buf.bank_address, lt_mcu_program_buf.bank_slot);

			if (lt_mcu_program_buf.total_length == 0)
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPDATE failed total_length=%d\r\n", lt_mcu_program_buf.total_length);
				return false;
			}

			if (!update_and_verify_dest_bank(lt_mcu_program_buf.bank_address))
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_ENTER_FW_UPDATE failed bank_address=%08x bank_slot=%d\r\n", lt_mcu_program_buf.bank_address, lt_mcu_program_buf.bank_slot);
				return false;
			}

			lt_mcu_program_buf.state = MCU_UPDATE_STATE_INIT;
			return false;
		}
		case FRAME_CMD_UPDATE_EXITS_FW_UPGRADE_MODE:
		{
			lt_mcu_program_buf.total_crc_32 = BYTES_TO_UINT32_LE(&payload->data[0]);
			uint32_t received_length = BYTES_TO_UINT32_LE(&payload->data[4]);

			if (received_length == lt_mcu_program_buf.total_length)
				lt_mcu_program_buf.state = MCU_UPDATE_STATE_EXIT;
			else
				lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;

			LOG_LEVEL("FRAME_CMD_UPDATE_EXIT_FW_UPDATE lt_mcu_program_buf.total_length=%d received_length=%d received_crc_32=%08x\r\n", lt_mcu_program_buf.total_length, received_length, lt_mcu_program_buf.total_crc_32);
			return false;
		}
		case FRAME_CMD_UPDATE_SEND_FW_DATA:
		{
			lt_mcu_program_buf.r_address = (payload->data[3] << 24) | (payload->data[2] << 16) | (payload->data[1] << 8) | payload->data[0];
			// MK_DWORD(MK_WORD(payload->data[0], payload->data[1]), MK_WORD(payload->data[2], payload->data[3]));
			lt_mcu_program_buf.r_length = payload->data_len - 4; // payload->data[4];

			for (int i = 0; i < lt_mcu_program_buf.r_length; i++)
			{
				lt_mcu_program_buf.r_buff[i] = payload->data[4 + i];
			}

			// MCU_Print_Receive_Data(lt_mcu_program_buf.addr, lt_mcu_program_buf.buff, lt_mcu_program_buf.length);

			if (lt_mcu_program_buf.state == MCU_UPDATE_STATE_RECEIVING)
				lt_mcu_program_buf.state = MCU_UPDATE_STATE_UPDATING;
			else
				lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;

			return false;
		}
		case FRAME_CMD_UPDATE_REBOOT:
		{
			LOG_LEVEL("FRAME_CMD_UPDATE_REBOOT\n");
			return true;
		}
		default:
			break;
		}
	}
#ifdef TASK_MANAGER_STATE_MACHINE_SOC
	else if (MCU_TO_SOC_MOD_UPDATE == payload->frame_type)
	{
		switch (payload->frame_cmd)
		{
		case FRAME_CMD_UPDATE_REQUEST_FW_DATA:
		{
			uint16_t frame_index = payload->data[1] << 8 | payload->data[2];
			if (payload->data[0] != MCU_UPDATE_STATE_RECEIVING && frame_index != mcu_upgrade_status.f_counter)
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_REQUEST_FW_DATA error mcu_upgrade_status.progress=%d\r\n", mcu_upgrade_status.f_counter);
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_FRAME;
				return false;
			}

			if (!file_handle_oupg)
			{
				LOG_LEVEL("FRAME_CMD_UPDATE_REQUEST_FW_DATA error file_handle_oupg=NULL\r\n");
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_FILE;
				return false;
			}

			hex_record_t hex_record;
			hex_record.f_count = 0;
			file_read_status_t file_read_status = FILE_READ_INVALID;

			while (file_read_status != FILE_READ_DATA_OK)
			{
				file_read_status = read_next_record(
					file_handle_oupg,
					&mcu_upgrade_status.file_offset,
					mcu_upgrade_status.file_info.file_type,
					&hex_record);

				switch (file_read_status)
				{
				case FILE_READ_DATA_OK:
					hex_record.f_count++;
					break; // go and send data

				case FILE_READ_CT_INFOR:
				case FILE_READ_INVALID: // skip invalid eg: hex data type 0x05
					LOG_LEVEL("FRAME_CMD_UPDATE_REQUEST_FW_DATA FILE_READ_INVALID Continue\r\n");
					break; // continue

				case FILE_READ_EOF:
					LOG_LEVEL("FRAME_CMD_UPDATE_REQUEST_FW_DATA finished file_read_status=FILE_READ_EOF\r\n");
					send_message(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_UPDATE, FRAME_CMD_UPDATE_EXITS_FW_UPGRADE_MODE, 0);
					return false;

				case FILE_READ_ERROR:
					LOG_LEVEL("FRAME_CMD_UPDATE_REQUEST_FW_DATA error file_read_status=FILE_READ_ERROR\r\n");
					mcu_upgrade_status.error_code = MCU_ERROR_CODE_FILE;
					send_message(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_UPDATE, FRAME_CMD_UPDATE_EXITS_FW_UPGRADE_MODE, 0);
					return false;

				default:
					LOG_LEVEL("FRAME_CMD_UPDATE_REQUEST_FW_DATA unimaginable exception  error file_read_status=%d\r\n", file_read_status);
					mcu_upgrade_status.error_code = MCU_ERROR_CODE_FILE;
					return false;
				}
			}
			if (mcu_upgrade_status.file_info.file_type == FILE_TYPE_HEX)
				hex_record.address = (mcu_upgrade_status.file_info.reset_handler & FLASH_BANK_MASK) | hex_record.address;
			else if (mcu_upgrade_status.file_info.file_type == FILE_TYPE_BIN)
				hex_record.address = FLASH_BASE_START_ADDR | hex_record.address; // address of flash

			if (!flash_is_valid_bank_address(mcu_upgrade_status.file_info.reset_handler, hex_record.address))
			{
				LOG_LEVEL("Invalid flash address hex_record.address=%08x\n", hex_record.address);
				mcu_upgrade_status.error_code = MCU_ERROR_CODE_ADDRESS;
				LOG_BUFF_LEVEL(hex_record.data, hex_record.length);
				return false;
			}

			mcu_upgrade_status.s_length = mcu_upgrade_status.s_length + hex_record.length;
			mcu_upgrade_status.f_counter++;

			UINT32_TO_BYTES_LE(hex_record.address, &buffer[0]);
			for (uint16_t i = 0; i < hex_record.length; i++)
			{
				buffer[i + 4] = hex_record.data[i];
			}
			mcu_upgrade_status.error_code = MCU_ERROR_CODE_OK;
			send_message(TASK_MODULE_IPC, MSG_OTSM_DEVICE_MCU_EVENT, MSG_OTSM_CMD_MCU_UPDATING, 0);
			ptl_build_frame(SOC_TO_MCU_MOD_UPDATE, FRAME_CMD_UPDATE_SEND_FW_DATA, buffer, hex_record.length + 4, ackbuff);
			return true;
		}

		default:
			break;
		}
	}
#endif
	return false;
}

void update_print_program_data(uint32_t address, uint8_t *buff, uint8_t length)
{
	LOG_LEVEL("MCU_UPDATE addr:%08x length:%02d, ", address, length);
	LOG_BUFF(buff, length);
}

void update_print_receive_data(uint32_t address, uint8_t *buff, uint8_t length)
{
	LOG_LEVEL("MCU_RECEIV addr:%08x length:%02d, ", address, length);
	LOG_BUFF(buff, length);
}

bool update_and_verify_dest_bank(uint32_t slot_addr)
{
	uint32_t bank_address = slot_addr & FLASH_BANK_MASK;
	lt_mcu_program_buf.bank_slot = update_get_target_bank();
	switch (lt_mcu_program_buf.bank_slot)
	{
	case BANK_SLOT_A:
		if (bank_address != flash_meta_infor.slot_a_addr)
		{
			LOG_LEVEL("band address mismatched target bank:%d02x %08x %08x", lt_mcu_program_buf.bank_slot, bank_address, flash_meta_infor.slot_a_addr);
			return false;
		}
		break;
	case BANK_SLOT_B:
		if (bank_address != flash_meta_infor.slot_b_addr)
		{
			LOG_LEVEL("band address mismatched target bank:%d02x %08x %08x", lt_mcu_program_buf.bank_slot, bank_address, flash_meta_infor.slot_b_addr);
			return false;
		}
		break;
	default:
		return false;
	}
	return true;
}

uint8_t update_get_target_bank(void)
{
	if (flash_get_current_bank() == BANK_SLOT_LOADER)
		return BANK_SLOT_A;
	else if (flash_get_current_bank() == BANK_SLOT_A)
		return BANK_SLOT_B;
	else if (flash_get_current_bank() == BANK_SLOT_B)
		return BANK_SLOT_A;
	else
		return BANK_SLOT_INVALID;
}

uint32_t update_get_model_number(void)
{
	LOG_LEVEL("MCU_MODEL_NAME: %s\r\n", MCU_MODEL_NAME);
	uint32_t crc = calculate_crc_32((uint8_t *)MCU_MODEL_NAME, strlen(MCU_MODEL_NAME));
	return crc;
}

#ifdef TASK_MANAGER_STATE_MACHINE_SOC
bool update_is_mcu_updating(void)
{
	return (file_handle_oupg != NULL);
}

bool update_check_oupg_file_exists(void)
{
	const char *usb_dirs[] = {"/mnt/extsd", "/mnt/usbotg", "/mnt/usb1", "/mnt/usb2", "/mnt/usb3", "/tmp"};
	size_t usb_dirs_len = sizeof(usb_dirs) / sizeof(usb_dirs[0]);
	for (int i = 0; i <= usb_dirs_len; i++)
	{
		if (search_and_copy_oupg_files(usb_dirs[i], file_path_name_upgrade, sizeof(file_path_name_upgrade)))
		{
			LOG_LEVEL("Found and copied to: %s\r\n", file_path_name_upgrade);
			return true;
		}
	}
	LOG_LEVEL("Not found and copied file: %s\r\n", file_path_name_upgrade);
	return false;
}

mcu_update_progress_t get_mcu_update_progress(void)
{
	mcu_update_progress_t mcu_update_progress;
	mcu_update_progress.s_length = mcu_upgrade_status.s_length;
	mcu_update_progress.s_total_length = mcu_upgrade_status.file_info.file_size;
	mcu_update_progress.error_code = mcu_upgrade_status.error_code;
	return mcu_update_progress;
}

file_read_status_t read_next_record(FILE *fp, long *file_offset, file_type_t type, hex_record_t *record)
{
	if (type == FILE_TYPE_HEX)
	{
		return read_next_hex_record(fp, file_offset, record);
	}
	else if (type == FILE_TYPE_BIN)
	{
		return read_next_bin_record(fp, file_offset, record);
	}
	return FILE_READ_INVALID;
}
#endif

#ifdef TASK_MANAGER_STATE_MACHINE_MCU
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to process the MCU reboot state machine. It handles different stages of the reboot process.
static void update_state_process(void)
{
	// static uint16_t writed_total_count;
	uint16_t erase_count = 0;
	uint16_t writed_count = 16;
	uint32_t crc_32;
	switch (lt_mcu_program_buf.state)
	{
	case MCU_UPDATE_STATE_IDLE:
		break;
	case MCU_UPDATE_STATE_CHECK:
		break;
	case MCU_UPDATE_STATE_INIT:
		LOG_LEVEL("MCU_UPDATE_STATE_INIT FLASH_BANK_CONFIG_MODE_SLOT:%d\r\n", flash_get_current_bank());
		if (lt_mcu_program_buf.bank_slot == BANK_SLOT_INVALID)
		{
			// lt_mcu_program_buf.bank_slot = BANK_SLOT_INVALID;
			LOG_LEVEL("MCU_UPDATE_STATE_INIT error! FLASH_BANK_CONFIG_MODE_SLOT:%d\r\n", flash_get_current_bank());
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_IDLE;
			break;
		}

		lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERASE;
		lt_mcu_program_buf.f_count = 0;
		task_manager_stop_except_2(TASK_MODULE_UPDATE_MCU, TASK_MODULE_PTL_1);
		task_manager_start_module(TASK_MODULE_IPC);

		break;
	case MCU_UPDATE_STATE_ERASE:
		LOG_LEVEL("MCU_UPDATE_STATE_ERASE...\n");
		if (lt_mcu_program_buf.bank_slot == BANK_SLOT_A)
		{
			flash_meta_infor.slot_a_crc = 0;
			flash_meta_infor.slot_a_size = 0;
			flash_meta_infor.slot_a_version = 0;
			flash_meta_infor.slot_stat_flags &= ~APP_FLAG_VALID_A;
		}
		else if (lt_mcu_program_buf.bank_slot == BANK_SLOT_B)
		{
			flash_meta_infor.slot_b_crc = 0;
			flash_meta_infor.slot_b_size = 0;
			flash_meta_infor.slot_b_version = 0;
			flash_meta_infor.slot_stat_flags &= ~APP_FLAG_VALID_B;
		}
		else
		{
			lt_mcu_program_buf.bank_slot = BANK_SLOT_INVALID;
			LOG_LEVEL("MCU_UPDATE_STATE_INIT error! bank_slot:%d\r\n", lt_mcu_program_buf.bank_slot);
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_IDLE;
			break;
		}

		erase_count = flash_erase_user_app_bank();
		LOG_LEVEL("MCU_UPDATE_STATE_ERASE... %d / %d\n", erase_count, MAIN_APP_BLOCK_COUNT);
		if (erase_count == MAIN_APP_BLOCK_COUNT)
		{
			flash_save_app_meter_infor();
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_RECEIVING;
			lt_mcu_program_buf.f_count = 0; // start receiving data
			StartTickCounter(&l_t_msg_wait_2000_timer);
			StartTickCounter(&mcu_upgrade_status.start_time);
			send_message(TASK_MODULE_PTL_1, MCU_TO_SOC_MOD_UPDATE, FRAME_CMD_UPDATE_REQUEST_FW_DATA, lt_mcu_program_buf.state);
		}
		else
		{
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;
		}
		break;
	case MCU_UPDATE_STATE_RECEIVING:
		if (GetTickCounter(&l_t_msg_wait_2000_timer) > 5000)
		{
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;
			LOG_LEVEL("MCU_UPDATE_STATE_RECEIVING timeout \n");
		}
		break;
	case MCU_UPDATE_STATE_UPDATING:
		// LOG_LEVEL("MCU_UPDATE_STATE_UPDATING \n");
		lt_mcu_program_buf.error_count = 0;

	RETRY_PROGRAM:
		if (flash_is_valid_bank_address(lt_mcu_program_buf.bank_address, lt_mcu_program_buf.r_address))
		{
			writed_count = FlashWriteBuffTo(lt_mcu_program_buf.r_address, lt_mcu_program_buf.r_buff, lt_mcu_program_buf.r_length);
			update_print_program_data(lt_mcu_program_buf.r_address, lt_mcu_program_buf.r_buff, lt_mcu_program_buf.r_length);
		}
		else
		{
			LOG_LEVEL("Invalid flash address lt_mcu_program_buf.r_address=%08x\n", lt_mcu_program_buf.r_address);
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;
			break;
		}

		if (writed_count == lt_mcu_program_buf.r_length)
		{
			lt_mcu_program_buf.f_count++;
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_RECEIVING; // continue receive data
			StartTickCounter(&l_t_msg_wait_2000_timer);
			send_message(TASK_MODULE_PTL_1, MCU_TO_SOC_MOD_UPDATE, FRAME_CMD_UPDATE_REQUEST_FW_DATA, lt_mcu_program_buf.state);
			break;
		}

		lt_mcu_program_buf.error_count++;
		LOG_LEVEL("FlashWriteBuffTo failed  writed_count=%d\n", writed_count);
		if (lt_mcu_program_buf.error_count >= 3)
		{
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;
			break;
		}

		goto RETRY_PROGRAM;

	case MCU_UPDATE_STATE_EXIT:
		LOG_LEVEL("MCU_UPDATE_STATE_EXIT \n");

		if (lt_mcu_program_buf.bank_slot == BANK_SLOT_A)
		{
			DISABLE_IRQ;
			crc_32 = calculate_crc_32((uint8_t *)(uintptr_t)flash_meta_infor.slot_a_addr, lt_mcu_program_buf.total_length);
			ENABLE_IRQ;
		}
		else if (lt_mcu_program_buf.bank_slot == BANK_SLOT_B)
		{
			DISABLE_IRQ;
			crc_32 = calculate_crc_32((uint8_t *)(uintptr_t)flash_meta_infor.slot_b_addr, lt_mcu_program_buf.total_length);
			ENABLE_IRQ;
		}
		else
		{
			LOG_LEVEL("lt_mcu_program_buf.bank_slot error! bank_slot=%d\n", lt_mcu_program_buf.bank_slot);
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;
			break;
		}

		if (crc_32 == lt_mcu_program_buf.total_crc_32)
		{
			if (lt_mcu_program_buf.bank_slot == BANK_SLOT_A)
			{
				flash_meta_infor.slot_a_crc = crc_32;
				flash_meta_infor.slot_stat_flags |= APP_FLAG_VALID_A;
				flash_meta_infor.slot_a_size = lt_mcu_program_buf.total_length;
				SET_FLAG(flash_meta_infor.slot_stat_flags, APP_FLAG_SLOT_A_UPGRADED);
			}
			if (lt_mcu_program_buf.bank_slot == BANK_SLOT_B)
			{
				flash_meta_infor.slot_b_crc = crc_32;
				flash_meta_infor.slot_stat_flags |= APP_FLAG_VALID_B;
				flash_meta_infor.slot_b_size = lt_mcu_program_buf.total_length;
				SET_FLAG(flash_meta_infor.slot_stat_flags, APP_FLAG_SLOT_B_UPGRADED);
			}
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_COMPLETE;
		}
		else
		{
			if (lt_mcu_program_buf.bank_slot == BANK_SLOT_A)
			{
				flash_meta_infor.slot_a_crc = crc_32;
				flash_meta_infor.slot_stat_flags &= ~APP_FLAG_VALID_A;
				flash_meta_infor.slot_a_size = lt_mcu_program_buf.total_length;
				CLEAR_FLAG(flash_meta_infor.slot_stat_flags, APP_FLAG_SLOT_A_UPGRADED);
			}

			if (lt_mcu_program_buf.bank_slot == BANK_SLOT_B)
			{
				flash_meta_infor.slot_b_crc = crc_32;
				flash_meta_infor.slot_stat_flags &= ~APP_FLAG_VALID_B;
				flash_meta_infor.slot_b_size = lt_mcu_program_buf.total_length;
				CLEAR_FLAG(flash_meta_infor.slot_stat_flags, APP_FLAG_SLOT_B_UPGRADED);
			}
			lt_mcu_program_buf.state = MCU_UPDATE_STATE_ERROR;
		}

		break;

	case MCU_UPDATE_STATE_COMPLETE:

		if (lt_mcu_program_buf.bank_slot == BANK_SLOT_A)
		{
			LOG_LEVEL("MCU_UPDATE_STATE_COMPLETE flash_meta_infor.slot_a_crc=%08X, Received crc_32=%08X\n", flash_meta_infor.slot_a_crc, lt_mcu_program_buf.total_crc_32);
			LOG_LEVEL("It took %d seconds\r\n", GetTickCounter(&mcu_upgrade_status.start_time) / 1000);

			flash_save_app_meter_infor();
			/////JumpToApplication(flash_meta_infor.slot_a_addr);
		}
		else if (lt_mcu_program_buf.bank_slot == BANK_SLOT_B)
		{
			LOG_LEVEL("MCU_UPDATE_STATE_COMPLETE flash_meta_infor.slot_b_crc=%08X, Received crc_32=%08X\n", flash_meta_infor.slot_b_crc, lt_mcu_program_buf.total_crc_32);
			LOG_LEVEL("It took %d seconds\r\n", GetTickCounter(&mcu_upgrade_status.start_time) / 1000);
			flash_save_app_meter_infor();
			flash_JumpToApplication(flash_meta_infor.slot_b_addr);
		}
		else
		{
			/// LOG_LEVEL("It took %d seconds",GetTickCounter(&mcu_upgrade_status.start_time)/1000);
			LOG_LEVEL("lt_mcu_program_buf.bank_slot error! bank_slot=%d\n", lt_mcu_program_buf.bank_slot);
		}
		StopTickCounter(&mcu_upgrade_status.start_time);
		StopTickCounter(&l_t_msg_wait_2000_timer);
		lt_mcu_program_buf.state = MCU_UPDATE_STATE_IDLE;
		break;

	case MCU_UPDATE_STATE_ERROR:
		if (lt_mcu_program_buf.bank_slot == BANK_SLOT_A)
			LOG_LEVEL("MCU_UPDATE_STATE_ERROR flash_meta_infor.slot_a_crc=%08X, Received crc_32=%08X\n", flash_meta_infor.slot_a_crc, lt_mcu_program_buf.total_crc_32);

		else if (lt_mcu_program_buf.bank_slot == BANK_SLOT_B)
			LOG_LEVEL("MCU_UPDATE_STATE_ERROR flash_meta_infor.slot_b_crc=%08X, Received crc_32=%08X\n", flash_meta_infor.slot_b_crc, lt_mcu_program_buf.total_crc_32);

		else
			LOG_LEVEL("lt_mcu_program_buf.bank_slot error! bank_slot=%d\n", lt_mcu_program_buf.bank_slot);

		flash_save_app_meter_infor();

		LOG_LEVEL("It took %d seconds\r\n", GetTickCounter(&mcu_upgrade_status.start_time) / 1000);
		StopTickCounter(&mcu_upgrade_status.start_time);
		StopTickCounter(&l_t_msg_wait_2000_timer);
		lt_mcu_program_buf.state = MCU_UPDATE_STATE_IDLE;
		// NVIC_SystemReset();//user reboot
		break;
	}
}
#endif

#endif
