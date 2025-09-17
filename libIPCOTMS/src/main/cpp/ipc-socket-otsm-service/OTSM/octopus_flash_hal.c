/**
 * @file	halPeripheral.c
 * @author	chipsea
 * @brief
 * @version	0.1
 * @date	2020-11-30
 * @copyright Copyright (c) 2020, CHIPSEA Co., Ltd.
 * @note
 */

/*********************************************************************
 * INCLUDES
 */
#include "octopus_flash_hal.h"
#include "octopus_platform.h"

#define FLASH_PAGE_SIZE ((uint32_t)0x00000400) /* FLASH Page Size 1KB*/

void hal_flash_init(uint8_t task_id)
{
    LOG_LEVEL("hal flash init\r\n");
}

#ifdef PLATFORM_CST_OSAL_RTOS

uint32_t hal_flash_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length)
{
    HalFlashRead(startaddr, buffer, length);
    return 0;
}
uint32_t hal_flash_erase_page_(uint32_t startaddr, uint8_t page_count)
{
    return 0;
}
uint32_t hal_flash_erase_area_(uint32_t startaddr, uint32_t endaddr)
{
    return 0;
}

uint32_t hal_flash_write_(uint32_t startaddr, uint8_t *buffer, uint32_t length)
{
    return 0;
}
#elif defined(PLATFORM_NATION_RTOS)

uint32_t hal_flash_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length)
{
    return 0;
}
uint32_t hal_flash_erase_page_(uint32_t startaddr, uint8_t page_count)
{
    return 0;
}
uint32_t hal_flash_erase_area_(uint32_t startaddr, uint32_t endaddr)
{
    return 0;
}

uint32_t hal_flash_write_(uint32_t startaddr, uint8_t *buffer, uint32_t length)
{
    return 0;
}

#elif defined(PLATFORM_STM32_RTOS)

uint32_t hal_flash_erase_page_(uint32_t startaddr, uint8_t page_count)
{
    uint32_t i = 0;
    /* Unlock the Flash to enable the flash control register access *************/
    FLASH_Unlock();

    /* Erase the user Flash area
      (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Clear pending flags (if any) */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_SIZE_ERR);

    /* Define the number of page to be erased */
    // NbrOfPage = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;

    /* Erase the FLASH pages */
    for (i = 0; i < page_count; i++)
    {
        if (FLASH_ErasePage(startaddr + (FLASH_PAGE_SIZE * i)) != FLASH_COMPLETE)
        {
            /* Error occurred while sector erase.
                User can add here some code to deal with this error  */
            while (1)
            {
            }
        }
    }
    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock();
    return i;
}
uint32_t hal_flash_erase_area_(uint32_t startaddr, uint32_t endaddr)
{
    uint32_t i = 0;
    /* Unlock the Flash to enable the flash control register access *************/
    FLASH_Unlock();

    /* Erase the user Flash area
      (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

    /* Clear pending flags (if any) */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_SIZE_ERR);

    /* Define the number of page to be erased */
    uint32_t NbrOfPage = (endaddr - startaddr) / FLASH_PAGE_SIZE;

    /* Erase the FLASH pages */
    for (i = 0; i < NbrOfPage; i++)
    {
        if (FLASH_ErasePage(startaddr + (FLASH_PAGE_SIZE * i)) != FLASH_COMPLETE)
        {
            /* Error occurred while sector erase.
                User can add here some code to deal with this error  */
            while (1)
            {
            }
        }
    }
    /* Lock the Flash to disable the flash control register access (recommended
       to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock();
    return i;
}
/**
 * @brief  Read a data buffer from Flash.
 * @param  addr: Start address in Flash memory
 * @param  buf: Destination buffer to read into
 * @param  len: Length in bytes
 */
uint32_t hal_flash_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length)
{
// Directly read from the flash memory
#ifdef PLATFORM_CST_OSAL_RTOS
    HalFlashRead(startaddr, buffer, length);
#else
    memcpy(buffer, (const void *)startaddr, length);
#endif
    return length; // Return the number of bytes read
}
/**
 * @brief  Write a data buffer to Flash with auto-erasure.
 * @param  addr: Start address in Flash memory
 * @param  buf: Pointer to data buffer
 * @param  len: Length in bytes
 */
uint32_t hal_flash_write_(uint32_t startaddr, uint8_t *buffer, uint32_t length)
{
    if (length % 4 != 0)
    {
        return 0; // Length is not word-aligned, fail
    }

    uint32_t Address = startaddr;
    uint32_t *data = (uint32_t *)buffer;
    uint32_t written_bytes = 0;

    /* Unlock the Flash for write access */
    FLASH_Unlock();
    /* Clear pending flags (if any) */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_SIZE_ERR);
    /* Write the data word by word */
    for (uint32_t i = 0; i < length / 4; i++)
    {
        if (FLASH_ProgramWord(Address, data[i]) == FLASH_COMPLETE)
        {
            Address += 4;
            written_bytes += 4;
        }
        else
        {
            FLASH_Lock();
            return 0; // Writing failed, return 0 bytes
        }
    }

    /* Lock the Flash after writing */
    FLASH_Lock();
    return written_bytes; // Return the number of bytes written
}
#else
uint32_t hal_flash_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length)
{
    return 0;
}
uint32_t hal_flash_erase_page_(uint32_t startaddr, uint8_t page_count)
{
    return 0;
}
uint32_t hal_flash_erase_area_(uint32_t startaddr, uint32_t endaddr)
{
    return 0;
}

uint32_t hal_flash_write_(uint32_t startaddr, uint8_t *buffer, uint32_t length)
{
    return 0;
}
#endif

void hal_eeprom_write_(uint32_t startaddr, uint8_t *buffer, uint8_t length)
{
#ifdef FLASH_USE_EEROM_FOR_DATA_SAVING
    uint8_t ret = I2C_EepromBufferWrite(startaddr, buffer, length);
    if (ret == ERROR)
        LOG_LEVEL("save data to eeprom status=%d\r\n", ret);
#endif
}

void hal_eeprom_read_(uint32_t startaddr, uint8_t *buffer, uint8_t length)
{
#ifdef FLASH_USE_EEROM_FOR_DATA_SAVING
    EEPROM_Read(startaddr, buffer, length);
#endif
}
