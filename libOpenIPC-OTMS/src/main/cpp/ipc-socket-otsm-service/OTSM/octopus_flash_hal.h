
/*******************************************************************************
 * @file     octopus_task_manager_flash_hal.h
 * @brief    Header file for the Flash Hardware Abstraction Layer (HAL).
 *           This file provides the interface for initializing and accessing
 *           the flash memory in the Octopus project.
 * @details  The functions declared in this file are used to interact with
 *           flash memory, including initialization and reading data into a
 *           buffer. The implementation of these functions is dependent on
 *           the platform and hardware being used.
 * @version  1.0
 * @date     2024-12-12
 * @author   Octopus Team
 ******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_FLASH_HAL_H__
#define __OCTOPUS_TASK_MANAGER_FLASH_HAL_H__

#include "octopus_base.h" //  Base include file for the Octopus project.

// Macro for user flash testing.
// This could be used for testing purposes or debugging flash operations.
#define USR_FLASH_TEST 0x0001

#ifdef __cplusplus
extern "C"
{
#endif
    /******************************************************************************/
    /**
     * @brief   Initializes the flash memory module.
     * @param   task_id The ID of the task that will use the flash.
     *                  This can be used to associate the flash initialization
     *                  with a specific task or operation.
     * @retval  None
     * @details This function initializes the flash memory for use. It configures
     *          the necessary hardware settings and prepares the flash module for
     *          read/write operations.
     */
    void hal_flash_init(uint8_t task_id);

    /******************************************************************************/
    /**
     * @brief   Reads data from flash memory into a buffer.
     * @param   addr The address in flash memory from which to start reading.
     * @param   buf  Pointer to the buffer where the read data will be stored.
     * @param   len  The number of bytes to read from the flash memory.
     * @retval  None
     * @details This function reads data from the specified flash memory address
     *          into the provided buffer. The function will read up to the length
     *          specified by the `len` parameter. The data is stored in `buf`.
     */

    uint32_t hal_flash_erase_page_(uint32_t startaddr, uint8_t page_count);
    uint32_t hal_flash_erase_area_(uint32_t startaddr, uint32_t endaddr);
    uint32_t hal_flash_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length);
    uint32_t hal_flash_write_(uint32_t startaddr, uint8_t *buffer, uint32_t length);

    void hal_eeprom_write_(uint32_t startaddr, uint8_t *buffer, uint8_t length);
    void hal_eeprom_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length);

#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_FLASH_HAL_H__ */
