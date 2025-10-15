/*******************************************************************************
 * @file octopus_task_manager_gpio_hal.h
 * @brief Header file for handling GPIO (General Purpose Input/Output) operations in the Octopus Task Manager.
 *
 * This file provides macros and function declarations for managing GPIO pins for different hardware components,
 * reading and writing GPIO pin states, and controlling GPIO behavior such as low/high states.
 * It supports platform-specific configurations for different RTOS environments (CST OSAL RTOS, ITE Open RTOS).
 *
 * @note Ensure that platform-specific GPIO pins are defined according to the hardware configuration.
 *
 * @ingroup APP:SUB_TYPE
 * @version  1.0
 * @date     2024-12-12
 * @author   Octopus Team
 ******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_GPIO_HAL_H__
#define __OCTOPUS_TASK_MANAGER_GPIO_HAL_H__

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.
#include "octopus_platform.h"

/*******************************************************************************
 * MACRO DEFINITIONS
 * The following macros define GPIO pin mappings and their operations for different platforms.
 */
typedef enum
{
    BIT_RESET = 0,
    BIT_SET
} Bit_Action_T;

#ifdef PLATFORM_CST_OSAL_RTOS

typedef uint8_t GPIO_GROUP;

// Event identifiers for user test timers
#define USR_TEST_TIMER1_EVT 0x0001 /**< Event identifier for test timer 1 */
#define USR_TEST_TIMER2_EVT 0x0002 /**< Event identifier for test timer 2 */

// Define GPIO pins for various hardware components (mapped to platform-specific pins)
#define GPIO_ACC_SOC_PIN P1 /**< ACC_SOC pin */
#define GPIO_ACC_PIN P0     /**< ACC pin */

#define GPIO_DDD_PIN P31 /**< DDD pin */
#define GPIO_ZZD_PIN P32 /**< ZZD pin */
#define GPIO_YZD_PIN P33 /**< YZD pin */
#define GPIO_SKD_PIN P34 /**< SKD pin */

#define GPIO_KEY_PIN P14 /**< Key pin */

#define GPIO_SIF_R_PIN P15 /**< SIF Receive pin */
#define GPIO_SIF_S_PIN P20 /**< SIF Send pin */

#define GPIO_BMS_R_PIN P11 /**< BMS Receive pin */
// #define GPIO_BMS_S_PIN  P11    /**< BMS Send pin */

#define GPIO_POWER_KEY_GROUP 1
#define GPIO_POWER_KEY_PIN GPIO_ACC_PIN

#define GPIO_POWER_SWITCH_GROUP 1
#define GPIO_POWER_SWITCH_PIN GPIO_ACC_PIN

#define GPIO_POWER_ENABLE_GROUP 1
#define GPIO_POWER_ENABLE_PIN GPIO_ACC_PIN

#define GPIO_ACC_KEY_GROUP 1
#define GPIO_ACC_KEY_PIN GPIO_ACC_PIN

#define GPIO_DDD_KEY_GROUP 1
#define GPIO_DDD_KEY_PIN GPIO_DDD_PIN

#define GPIO_ZZD_KEY_GROUP 1
#define GPIO_ZZD_KEY_PIN GPIO_ZZD_PIN

#define GPIO_YZD_KEY_GROUP 1
#define GPIO_YZD_KEY_PIN GPIO_YZD_PIN

#define GPIO_SKD_KEY_GROUP 1
#define GPIO_SKD_KEY_PIN GPIO_DDD_PIN

#define GPIO_HORN_KEY_GROUP 1
#define GPIO_HORN_KEY_PIN GPIO_DDD_PIN

#define GPIO_PLUS_KEY_GROUP 1
#define GPIO_PLUS_KEY_PIN P34

#define GPIO_SUBT_KEY_GROUP 1
#define GPIO_SUBT_KEY_PIN P11

// Macros for controlling GPIO pin states (Low/High)
#define GPIO_ACC_SOC_LOW() (HalGpioSet(GPIO_ACC_SOC_PIN, Bit_DISABLE)) /**< Set GPIO_ACC_SOC_PIN to Low */
#define GPIO_ACC_SOC_HIGH() (HalGpioSet(GPIO_ACC_SOC_PIN, Bit_ENABLE)) /**< Set GPIO_ACC_SOC_PIN to High */

// Specific macros for reading GPIO states for various pins
#define GPIO_PIN_READ_ACC() (HalGpioGet(GPIO_ACC_PIN)) /**< Read the state of ACC_PIN */
#define GPIO_PIN_READ_DDD() (HalGpioGet(GPIO_DDD_PIN)) /**< Read the state of DDD_PIN */
#define GPIO_PIN_READ_ZZD() (HalGpioGet(GPIO_ZZD_PIN)) /**< Read the state of ZZD_PIN */
#define GPIO_PIN_READ_YZD() (HalGpioGet(GPIO_YZD_PIN)) /**< Read the state of YZD_PIN */
#define GPIO_PIN_READ_SKD() (HalGpioGet(GPIO_SKD_PIN)) /**< Read the state of SKD_PIN */

// Macros for controlling SIF pins
#define GPIO_PIN_SIF_SET_LOW() (HalGpioSet(GPIO_SIF_S_PIN, Bit_DISABLE)) /**< Set GPIO_SIF_S_PIN to Low */
#define GPIO_PIN_SIF_SET_HIGH() (HalGpioSet(GPIO_SIF_S_PIN, Bit_ENABLE)) /**< Set GPIO_SIF_S_PIN to High */
#define GPIO_PIN_READ_SIF() (HalGpioGet(GPIO_SIF_R_PIN))                 /**< Read the state of SIF_R_PIN */

// Macros for controlling BMS pins
#define GPIO_PIN_BMS_SET_LOW()                           //(HalGpioSet(GPIO_BMS_R_PIN, Bit_DISABLE))  /**< Set GPIO_BMS_S_PIN to Low */
#define GPIO_PIN_BMS_SET_HIGH()                          //(HalGpioSet(GPIO_BMS_R_PIN, Bit_ENABLE))   /**< Set GPIO_BMS_S_PIN to High */
#define GPIO_PIN_READ_BMS() (HalGpioGet(GPIO_BMS_R_PIN)) /**< Read the state of BMS_R_PIN */

#elif defined(PLATFORM_ITE_OPEN_RTOS)
typedef uint8_t GPIO_GROUP;
// Define macros for ITE Open RTOS platform (example macros; to be implemented as needed)
#define GPIO_MCU_SDIO_PWR_OUTPUT_PIN 21 /**< SDIO Power Output pin for MCU */
// Placeholder GPIO pins for ITE Open RTOS platform
#define GPIO_ACC_SOC_PIN (0x00)         /**< ACC_SOC pin */
#define GPIO_ACC_PIN (0x00)             /**< ACC pin */
#define GPIO_KEY_PIN (0x00)             /**< Key pin */
#define GPIO_DDD_PIN (0x00)             /**< DDD pin */
#define GPIO_ZZD_PIN (0x00)             /**< ZZD pin */
#define GPIO_YZD_PIN (0x00)             /**< YZD pin */
#define GPIO_SKD_PIN (0x00)             /**< SKD pin */

// Macros for controlling GPIO pin states (Low/High) (platform-specific implementation)
#define GPIO_ACC_SOC_LOW()              // Set GPIO_ACC_SOC_PIN to Low
#define GPIO_ACC_SOC_HIGH()             // Set GPIO_ACC_SOC_PIN to High

// Macros for reading GPIO states for various pins (platform-specific)
#define GPIO_PIN_READ_ACC() (0x00)      // Read the state of ACC_PIN
#define GPIO_PIN_READ_DDD() (0x00)      // Read the state of DDD_PIN
#define GPIO_PIN_READ_ZZD() (0x00)      // Read the state of ZZD_PIN
#define GPIO_PIN_READ_YZD() (0x00)      // Read the state of YZD_PIN
#define GPIO_PIN_READ_SKD() (0x00)      // Read the state of SKD_PIN

// Macros for controlling SIF pins
#define GPIO_PIN_SIF_SET_LOW()          // Set GPIO_SIF_S_PIN to Low
#define GPIO_PIN_SIF_SET_HIGH()         // Set GPIO_SIF_S_PIN to High
#define GPIO_PIN_READ_SIF() (0x00)      // Read the state of SIF_R_PIN

#elif defined(PLATFORM_STM32_RTOS)
typedef GPIO_TypeDef GPIO_GROUP;
// Default GPIO pin definitions for unsupported platforms

#define GPIO_ACC_GROUP 0
#define GPIO_ACC_PIN 0
#define GPIO_POWER_KEY_GROUP GPIOA
#define GPIO_POWER_KEY_PIN GPIO_Pin_12

#define GPIO_POWER_SWITCH_GROUP GPIOB
#define GPIO_POWER_SWITCH_PIN GPIO_Pin_4

#define GPIO_POWER_ENABLE_GROUP GPIOA
#define GPIO_POWER_ENABLE_PIN GPIO_Pin_15

#define GPIO_DDD_KEY_GROUP 0
#define GPIO_DDD_KEY_PIN 0

#define GPIO_ZZD_KEY_GROUP 0
#define GPIO_ZZD_KEY_PIN 0

#define GPIO_YZD_KEY_GROUP 0
#define GPIO_YZD_KEY_PIN 0

#define GPIO_SKD_KEY_GROUP 0
#define GPIO_SKD_KEY_PIN 0

#define GPIO_PLUS_KEY_GROUP 0
#define GPIO_PLUS_KEY_PIN 0

#define GPIO_SUBT_KEY_GROUP 0
#define GPIO_SUBT_KEY_PIN 0

#define GPIO_PAGE_KEY_GROUP GPIOB
#define GPIO_PAGE_KEY_PIN GPIO_Pin_1

// Macros for controlling GPIO pin states (Low/High)
#define GPIO_ACC_SOC_LOW()         // Set GPIO_ACC_SOC_PIN to Low
#define GPIO_ACC_SOC_HIGH()        // Set GPIO_ACC_SOC_PIN to High

// Macros for reading GPIO states for various pins
#define GPIO_PIN_READ_ACC() (0x00) // Read the state of ACC_PIN
#define GPIO_PIN_READ_DDD() (0x00) // Read the state of DDD_PIN
#define GPIO_PIN_READ_ZZD() (0x00) // Read the state of ZZD_PIN
#define GPIO_PIN_READ_YZD() (0x00) // Read the state of YZD_PIN
#define GPIO_PIN_READ_SKD() (0x00) // Read the state of SKD_PIN

// Macros for controlling SIF pins
#define GPIO_PIN_SIF_SET_LOW()     // Set GPIO_SIF_S_PIN to Low
#define GPIO_PIN_SIF_SET_HIGH()    // Set GPIO_SIF_S_PIN to High
#define GPIO_PIN_READ_SIF() (0x00) // Read the state of SIF_R_PIN

#elif defined(PLATFORM_NATION_RTOS)

typedef GPIO_Module GPIO_GROUP;

#define GPIO_POWER_KEY_GROUP GPIOA
#define GPIO_POWER_KEY_PIN GPIO_PIN_12

#define GPIO_POWER_SWITCH_GROUP GPIOB
#define GPIO_POWER_SWITCH_PIN GPIO_PIN_4

#define GPIO_POWER_ENABLE_GROUP GPIOA
#define GPIO_POWER_ENABLE_PIN GPIO_PIN_15

#else

typedef uint8_t GPIO_GROUP;

#define GPIO_POWER_KEY_GROUP 0
#define GPIO_POWER_KEY_PIN 0

#define GPIO_POWER_ENABLE_GROUP 0
#define GPIO_POWER_ENABLE_PIN 0

#define GPIO_POWER_SWITCH_GROUP 0
#define GPIO_POWER_SWITCH_PIN 0

#define GPIO_PAGE_KEY_GROUP 0
#define GPIO_PAGE_KEY_PIN 0

#endif

/*********************************************************************
 * FUNCTION DECLARATIONS
 */
#ifdef __cplusplus
extern "C"
{
#endif
    // Function to initialize GPIOs for the task
    void hal_gpio_init(uint8_t task_id);
    bool hal_gpio_read(GPIO_GROUP *gpiox, uint16_t pin);
    bool hal_gpio_write(GPIO_GROUP *gpiox, uint16_t pin, uint8_t value);

    void hal_gpio_power_on(void);
#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_GPIO_HAL_H__ */
