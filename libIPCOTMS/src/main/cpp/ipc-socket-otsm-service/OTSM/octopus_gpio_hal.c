/**
 * ****************************************************************************
 * @copyright Copyright (c) XXX
 * All rights reserved.
 *
 * C file for the Octopus Task Manager module.
 * Defines macros, includes required libraries, and implements functions.
 */

/*********************************************************************
 * INCLUDES
 */
#include "octopus_gpio_hal.h" // Include the GPIO hardware abstraction layer header

bool hal_gpio_read(GPIO_GROUP *gpiox, uint16_t pin)
{
// Macros for writing to and reading from GPIO pins
#ifdef PLATFORM_STM32_RTOS
    return (GPIO_ReadInputDataBit(gpiox, pin)); // Read the state of the specified GPIO pin
#elif defined(PLATFORM_CST_OSAL_RTOS)
    return HalGpioGet((GpioPin_t)pin);
#elif defined(PLATFORM_ITE_OPEN_RTOS)
    return 0;
#else
    return 0;
#endif
}

bool hal_gpio_write(GPIO_GROUP *gpiox, uint16_t pin, uint8_t value)
{
    // Macros for writing to and reading from GPIO pins
#ifdef PLATFORM_STM32_RTOS
    GPIO_WriteBit(gpiox, pin, (BitAction)value); // Write to the specified GPIO pin
#elif defined(PLATFORM_CST_OSAL_RTOS)
    HalGpioSet((GpioPin_t)pin, (bit_action_e)value);
#elif defined(PLATFORM_ITE_OPEN_RTOS)
#else
#endif
    return value;
}

void hal_gpio_power_on(void)
{
#ifdef PLATFORM_STM32_RTOS
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        // Alternate function mode
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;      // Open-drain output//GPIO_OType_PP;// Push-pull output
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // GPIO_PuPd_UP;// Pull-up resistor enabled
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3; // High speed

    // PA15 - 3.3V power enable for MCU (MCU_3V3_EN)
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    // PB4 - SWB+ power enable (SWB+_EN)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    hal_gpio_write(GPIO_POWER_ENABLE_GROUP, GPIO_POWER_ENABLE_PIN, BIT_SET);
    hal_gpio_write(GPIO_POWER_SWITCH_GROUP, GPIO_POWER_SWITCH_PIN, BIT_SET);
#endif
}

#ifdef PLATFORM_CST_OSAL_RTOS
// Function to handle GPIO callback for different events on specific pins
void hal_gpio_callback(GpioPin_t pin, gpio_polarity_e type)
{
    switch ((uint8)pin)
    {
    case P18:
        LOG("P18 press\r\n"); // Log when P18 is pressed
        if (HalGpioGet(P11))  // Check the state of P11
        {
            LOG("P11 reset\r\n");         // If P11 is set, reset it
            HalGpioSet(P11, Bit_DISABLE); // Set P11 to low
        }
        else
        {
            LOG("P11 set\r\n");          // If P11 is not set, enable it
            HalGpioSet(P11, Bit_ENABLE); // Set P11 to high
        }
        break;
    }
}

// GPIO initialization function
void hal_gpio_init(uint8_t task_id)
{
    // Initialize GPIO pins and set their states
    HalGpioPinInit(GPIO_ACC_SOC_PIN, GPIO_OUTPUT); // Set ACC-->SOC pin (P0) as output
    HalGpioSet(GPIO_ACC_SOC_PIN, Bit_ENABLE);      // Enable the ACC-->SOC pin (P0)

    HalGpioPinInit(GPIO_SIF_S_PIN, GPIO_OUTPUT);
    HalGpioSet(GPIO_SIF_S_PIN, Bit_DISABLE); // Set SIF_S_PIN to low (disabled)

    HalGpioPinInit(GPIO_SIF_R_PIN, GPIO_INPUT);
    HalGpioPupdConfig(GPIO_SIF_R_PIN, GPIO_FLOATING); // Configure SIF_R_PIN as floating input

    // Initialize input pins for various GPIOs
    HalGpioPinInit(GPIO_DDD_PIN, GPIO_INPUT);        // Set DD pin (P31) as input
    HalGpioPupdConfig(GPIO_DDD_PIN, GPIO_PULL_DOWN); // Enable pull-down for DD pin (P31)

    HalGpioPinInit(GPIO_ZZD_PIN, GPIO_INPUT);        // Set ZZ pin (P32) as input
    HalGpioPupdConfig(GPIO_ZZD_PIN, GPIO_PULL_DOWN); // Enable pull-down for ZZ pin (P32)

    HalGpioPinInit(GPIO_YZD_PIN, GPIO_INPUT);        // Set YZ pin (P33) as input
    HalGpioPupdConfig(GPIO_YZD_PIN, GPIO_PULL_DOWN); // Enable pull-down for YZ pin (P33)

    HalGpioPinInit(GPIO_SKD_PIN, GPIO_INPUT);        // Set SKD pin (P34) as input
    HalGpioPupdConfig(GPIO_SKD_PIN, GPIO_PULL_DOWN); // Enable pull-down for SKD pin (P34)

    HalGpioPinInit(GPIO_ACC_PIN, GPIO_INPUT);        // Set ACC pin (P1) as input
    HalGpioPupdConfig(GPIO_ACC_PIN, GPIO_PULL_DOWN); // Enable pull-down for ACC pin (P1)

    HalGpioPinInit(GPIO_KEY_PIN, GPIO_INPUT);      // Set KEY pin (P14) as input
    HalGpioPupdConfig(GPIO_KEY_PIN, GPIO_PULL_UP); // Enable pull-up for KEY pin (P14)
}

#elif defined(PLATFORM_ITE_OPEN_RTOS)
// GPIO initialization function for ITE Open RTOS (currently no implementation)
void hal_gpio_init(uint8_t task_id)
{
    ithGpioSetOut(GPIO_MCU_SDIO_PWR_OUTPUT_PIN);
    ithGpioSetMode(GPIO_MCU_SDIO_PWR_OUTPUT_PIN, ITH_GPIO_MODE0);
    hal_gpio_set_wifi_onoff(true);
    LOG_LEVEL("hal gpio init\r\n"); // Optional log for GPIO initialization (disabled here)
}

void hal_gpio_set_wifi_onoff(bool onoff)
{
    if (onoff)
        ithGpioSet(GPIO_MCU_SDIO_PWR_OUTPUT_PIN);
    else
        ithGpioClear(GPIO_MCU_SDIO_PWR_OUTPUT_PIN);
}

#else
void hal_gpio_init(uint8_t task_id)
{
    LOG_LEVEL("hal gpio init\r\n"); // Optional log for GPIO initialization (disabled here)
}

#endif
