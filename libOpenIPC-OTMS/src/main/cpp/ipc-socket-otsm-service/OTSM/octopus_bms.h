/**
******************************************************************************
* @file     octopus_task_manager_bms.h
* @brief    Header file for managing the Serial Interface (SIF) communication
*           between the system and BMS (Battery Management System).
* @details  This file contains macros, function declarations, and constants
*           related to the SIF communication, which is used for transmitting
*           and receiving data from the BMS.
* @version  1.0
* @date     2021-10-01
* @author   Octopus Team
* @company  Nanjing Qinheng Microelectronics Co., Ltd.
******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_BMS_H__
#define __OCTOPUS_TASK_MANAGER_BMS_H__

#include "octopus_gpio_hal.h" ///< Include GPIO Hardware Abstraction Layer for low-level pin control
#include "octopus_gpio.h"     ///< Include GPIO header for GPIO-related operations

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * CONSTANTS
 * @details  Definitions for constant values used in the SIF communication
 */

/********************************************************************
 * MACROS
 * @details  Macros for controlling the SIF communication pins
 */
#define BMS_RECEIVE_DATA_BIT() GPIO_PIN_READ_BMS()
/**< Macro to read the state of the receive data bit (SIF receive data) */
#define BMS_SEND_DATA_BIT_LOW() GPIO_PIN_BMS_SET_LOW()
/**< Macro to set the send data bit to low (SIF send data) */
#define BMS_SEND_DATA_BIT_HIGH() GPIO_PIN_BMS_SET_HIGH()
    /**< Macro to set the send data bit to high (SIF send data) */

    /*******************************************************************************
     * TYPEDEFS
     * @details  Typedefs for structures used in SIF communication (currently empty)
     */

    /*******************************************************************************
     * GLOBAL VARIABLES
     * @details  Global variables used for managing SIF communication (currently none)
     */

    /*******************************************************************************
     * EXTERNAL FUNCTION DECLARATIONS
     * @details  Function declarations for SIF communication-related operations
     */
    void BMS_IO_IRQHandler(void);
    /**< Interrupt handler for BMS I/O operations */
    uint8_t IsBMSIdle(void);
    /**< Function to check if the BMS is in an idle state */

    /*******************************************************************************
     * LOCAL FUNCTION DECLARATIONS
     * @details  Local function declarations for SIF communication operations
     */
    void otsm_bms_init(void);
    /**< Function to initialize the BMS communication */
    void BMS_DeInit(void);
    /**< Function to deinitialize the BMS communication */
    bool BMS_IsInit(void);
    /**< Function to check if the BMS communication is initialized */
    uint8_t BMS_ReadData(uint8_t *data, uint8_t maxlen);
    /**< Function to read data from the BMS */

#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_SIF_H__ */
