/*******************************************************************************
 * @file    octopus_utils.h
 * @brief   Header file for the Octopus Task Manager Update module.
 *          This file declares the functions and types required for managing
 *          MCU firmware updates.
 *
 * @details This module provides functionality for initializing, starting,
 *          and monitoring the MCU firmware update process. It defines the
 *          update states and provides interfaces to query the update status
 *          and progress.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team

 ******************************************************************************/

#ifndef ___OCTOPUS_TASK_MANAGER_UTILS_H___
#define ___OCTOPUS_TASK_MANAGER_UTILS_H___

/*******************************************************************************
 * INCLUDE FILES
 * Include standard libraries and platform-specific headers.
 ******************************************************************************/
#include "octopus_base.h" //  Base include file for the Octopus project.

/**
 * @defgroup APP_SETTING APP:SETTING
 * @brief    Application settings and configurations.
 * @{
 */

/*******************************************************************************
 * DEBUG SWITCH MACROS
 * Define macros to enable or disable debug functionality.
 ******************************************************************************/

/*******************************************************************************
 * MACROS
 * Define commonly used macros for this module.
 ******************************************************************************/
#define APP_MATA_INFO_MAGIC 0xDEADBEEF
/*******************************************************************************
 * TYPEDEFS
 * Define types used in the MCU update process.
 ******************************************************************************/
typedef enum
{
	FILE_READ_DATA_OK = 0,
	FILE_READ_CT_INFOR,
	FILE_READ_EOF,

	FILE_READ_INVALID,
	FILE_READ_ERROR
} file_read_status_t;

typedef struct
{
	uint32_t address;
	uint8_t data[64 + 10];
	uint8_t length;
	uint16_t f_count;
} hex_record_t;

typedef enum
{
	FILE_TYPE_UNKNOWN = 0, // Unknown file type
	FILE_TYPE_HEX,		   // Intel HEX file format
	FILE_TYPE_BIN		   // Raw binary file format
} file_type_t;

typedef struct
{
	file_type_t file_type;
	uint32_t file_size; // target bank size
	uint32_t file_version;
	uint32_t file_crc_32;	// target bank crc
	uint32_t reset_handler; // target bank vect reset address
} file_info_t;				// HEX/BIN FILE

typedef struct
{
	uint32_t magic; // Magic constant for validation
	uint32_t model;
	uint32_t start_address; // Start address (e.g. 0x00000000 or 0x00010000)
	uint32_t size;			// Actual app size in bytes
	uint32_t crc32;			// CRC32 of the app binary
} bank_info_t;

typedef struct
{
	bank_info_t bank1;
	bank_info_t bank2;
} meta_info_t;

/*******************************************************************************
 * CONSTANTS
 * Define any module-specific constants.
 ******************************************************************************/

/*******************************************************************************
 * GLOBAL VARIABLES DECLARATION
 * Declare external variables used across the module.
 ******************************************************************************/

/*******************************************************************************
 * GLOBAL FUNCTIONS DECLARATION
 * Declare the public functions provided by this module.
 ******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
	uint32_t calculateTotalDistance(uint32_t speed_kmh, uint32_t time_sec);
	void calculate_battery_soc_ex(uint32_t voltage_mV,
								  uint32_t capacity_mAh,
								  uint32_t trip_odo_m,
								  float consumption_Wh_per_km, // 例如 18.0
								  float safety_reserve_ratio,  // 例如 0.10 = 10%
								  float avg_speed_kph,		   // 用于功率估算；若未知传 0
								  uint16_t *out_power_w,
								  uint16_t *out_soc_pct,
								  uint16_t *out_range_100m,
								  uint16_t *out_range_max_100m);

	uint32_t calculate_crc_32(uint8_t *data, uint32_t length);
	uint32_t calculate_crc_32_step(uint32_t current_crc, uint8_t *data, uint32_t length);

	void decode_datetime_version(uint32_t encoded,
								 uint16_t *year, uint8_t *month, uint8_t *day,
								 uint8_t *hour, uint8_t *minute,
								 uint8_t *version_code);

	void build_version_string(char *out_str, size_t max_len, const char *date_str, const char *time_str);
	uint32_t build_version_code(const char *date_str, const char *time_str);
	int32_t compare_versions(uint32_t v1, uint32_t v2);
	bool is_version_code_valid(uint32_t version_code);

	file_info_t parse_firmware_file(uint32_t model_number, uint32_t target_bank_offset, const char *filename);
	file_read_status_t read_next_hex_record(FILE *hex_file, long *file_offset, hex_record_t *hex_record);
	file_read_status_t read_next_bin_record(FILE *bin_file, long *file_offset, hex_record_t *hex_record);

	int search_and_copy_oupg_files(const char *dir_path, char *out_path, size_t out_path_size);
	int file_exists(const char *file_path_name);

#ifdef __cplusplus
}
#endif

#endif // ___OCTOPUS_TASK_MANAGER_UPDATE_H___
