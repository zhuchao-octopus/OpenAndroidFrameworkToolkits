/******************************************************************************
 * @file    gpio_implementation.c
 * @brief   GPIO management implementation for hardware platform.
 *          This module handles GPIO initialization, status polling, and key
 *          event processing, as well as interaction with the task manager.
 * @version 1.0
 * @date    2025-05-10
 *
 * @note    Platform-specific functions are defined in octopus_gpio.h
 *          and GPIO configuration is handled for status and key polling.
 *          Interrupt-based handling can be added for real-time event capture.
 *          This implementation uses polling for simplicity and clarity.
 *
 *          Key Features:
 *          - GPIO Initialization
 *          - Polling for GPIO status
 *          - Polling for Key Press status
 *          - Event processing for key press and release
 *          - Task management state transitions
 *
 * @author  [ak47]
 ******************************************************************************/

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
 

#include "octopus_gpio.h"  // Include GPIO control and configuration
#include "octopus_flash.h" // Include flash memory access functions
#include "octopus_key.h"   // Include key status and event handling
#include "octopus_task_manager.h" // Include task manager for scheduling tasks
#include "octopus_tickcounter.h" // Include tick counter for timing operations
#include "octopus_msgqueue.h"    // Include message queue header for task communication
#include "octopus_message.h"     // Include message id for inter-task communication

/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#ifdef TASK_MANAGER_STATE_MACHINE_GPIO

extern GPIO_STATUS *gpio_array[];
extern GPIO_KEY_STATUS *gpio_key_array[];

/*******************************************************************************
 * LOCAL FUNCTIONS DECLEAR
 */

void task_gpio_polling_status(GPIO_STATUS *gpio_status);
void task_gpio_event_dispatcher(GPIO_STATUS *gpio_status);

void task_gpio_key_polling_event_status(GPIO_KEY_STATUS *key_status);
void task_gpio_key_polling_event_dispatcher(GPIO_KEY_STATUS *key_status);

void task_gpio_event_polling(void);

// static uint32_t l_t_msg_gpio_wait_timer;

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// GPIO_KEY_STATUS *gpio_key_array[] = {&key_status_power,&key_status_zzd,&key_status_yzd,&key_status_skd,&key_status_ddd,&key_status_plus,&key_status_subt};
// static bool module_send_handler(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint16_t param, ptl_proc_buff_t *buff);
// static bool module_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff);

/*******************************************************************************
 *  GLOBAL FUNCTIONS IMPLEMENTATION
 */
void task_gpio_init_running(void)
{
    LOG_LEVEL("task_gpio_init_running\r\n");
    // com_uart_ptl_register_module(MSGMODULE_SYSTEM, module_send_handler, module_receive_handler);
    OTMS(TASK_MODULE_GPIO, OTMS_S_INVALID);

    /// for (size_t i = 0; i < sizeof(gpio_array) / sizeof(gpio_array[0]); i++)
    ///{
    ///     GPIO_STATUS *gpio_status = gpio_array[i];
    ///     gpio_status->offon = hal_gpio_read(gpio_status->gpiox, gpio_status->pin);
    /// }
    for (GPIO_STATUS **gpio_ = gpio_array; *gpio_ != NULL; gpio_++)
    {
        (*gpio_)->offon = hal_gpio_read((*gpio_)->gpiox, (*gpio_)->pin);
    }
}

void task_gpio_start_running(void)
{
    LOG_LEVEL("task_gpio_start_running\r\n");
    OTMS(TASK_MODULE_GPIO, OTMS_S_ASSERT_RUN);
}

void task_gpio_assert_running(void)
{
    // StartTickCounter(&l_t_msg_gpio_wait_timer);
    OTMS(TASK_MODULE_GPIO, OTMS_S_RUNNING);
}

void task_gpio_running(void)
{
    task_gpio_event_polling();
}

void task_gpio_post_running(void)
{
    OTMS(TASK_MODULE_GPIO, OTMS_S_ASSERT_RUN);
}

void task_gpio_stop_running(void)
{
    LOG_LEVEL("_stop_running\r\n");
    OTMS(TASK_MODULE_GPIO, OTMS_S_INVALID);
}
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Initializes the GPIO configuration.
 *
 * This function is used to initialize the configuration of General Purpose
 * Input/Output (GPIO) pins. This includes setting the pin modes, directions,
 * and any pull-up or pull-down configurations. This function is currently empty
 * and should be implemented based on specific hardware requirements.
 */
void otsm_gpio_init(void)
{
    // TODO: Implement GPIO initialization here
    // LOG_LEVEL("gpio init\r\n"); // Optional log for GPIO initialization (disabled here)
    hal_gpio_init(0);
}

/**
 * @brief Polls the status of a specified GPIO pin and updates its status structure.
 *
 * This function continuously checks the state of a specified GPIO pin and
 * applies a redundancy mechanism to filter out noise or glitches. If the pin
 * remains in the same state for a predefined number of polling cycles
 * (`GPIO_STATUS_REDUNDANCY`), the status is considered stable and is reflected
 * in the `gpio_status` structure.
 *
 * @param gpiox Pointer to the GPIO group (port) to which the pin belongs.
 * @param pin The specific GPIO pin number to be polled.
 * @param gpio_status Pointer to the `GPIO_STATUS` structure to be updated with the current status.
 */
void task_gpio_polling_status(GPIO_STATUS *gpio_status)
{
    // static uint32_t g_start_gpio_tickcounter = 0;
    // Check the current state of the GPIO pin
    if (hal_gpio_read(gpio_status->gpiox, gpio_status->pin))
    {
        // Increment the press counter with redundancy protection
        if (!IsTickCounterStart(&gpio_status->count1))
        {
            StartTickCounter(&gpio_status->count1);
        }
        // If the pin is high, increment the high state counter and reset the low counter
        // gpio_status->count1 = GetTickCounter(&gpio_status->count1);
        gpio_status->count2 = 0;

        // If the high state is consistent for more than the redundancy threshold
        if (GetTickCounter(&gpio_status->count1) > GPIO_STATUS_REDUNDANCY)
        {
            // If the state has changed from low to high, mark it as changed
            if (gpio_status->offon)
                gpio_status->changed = false;
            else
                gpio_status->changed = true;

            // Update the status
            gpio_status->offon = true;
            StopTickCounter(&gpio_status->count1);
            gpio_status->count1 = 0;
        }
    }
    else
    {
        if (!IsTickCounterStart(&gpio_status->count2))
        {
            StartTickCounter(&gpio_status->count2);
        }
        // If the pin is low, reset the high counter and increment the low counter
        gpio_status->count1 = 0;
        // gpio_status->count2++;

        // If the low state is consistent for more than the redundancy threshold
        if (GetTickCounter(&gpio_status->count2) > GPIO_STATUS_REDUNDANCY)
        {
            // If the state has changed from high to low, mark it as changed
            if (gpio_status->offon)
                gpio_status->changed = true;
            else
                gpio_status->changed = false;

            // Update the status
            gpio_status->offon = false;

            StopTickCounter(&gpio_status->count2);
            gpio_status->count2 = 0;
        }
    }
}

void task_gpio_event_dispatcher(GPIO_STATUS *gpio_status)
{
    if (gpio_status->changed)
    {
        if (gpio_status->offon)
            LOG_LEVEL("gpio pin %02d transitioned from 0 to 1 \r\n", gpio_status->pin);
        else
            LOG_LEVEL("gpio pin %02d transitioned from 1 to 0 \r\n", gpio_status->pin);

        send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, gpio_status->pin, gpio_status->offon);
        gpio_status->changed = false;
    }
}
/**
 * @brief Polls the status of a specified GPIO pin configured as a key (button).
 *
 * This function checks the current status of a button connected to a GPIO pin.
 * It identifies different button states including:
 * - Pressed
 * - Long Pressed
 * - Long-Long Pressed
 * - Released
 *
 * The status is updated in the provided `key_status` structure. The function
 * uses debounce logic to ensure reliable detection.
 *
 * @param gpiox Pointer to the GPIO group (port) to which the pin belongs.
 * @param pin The specific GPIO pin number to be polled.
 * @param key_status Pointer to the `GPIO_KEY_STATUS` structure to be updated with the current status.
 */
void task_gpio_key_polling_event_status(GPIO_KEY_STATUS *key_status)
{
    if (key_status->gpiox == 0)
        return; //|| key_status->pin == 0
    // static uint32_t g_start_gpio_tickcounter = 0;
    //  Read the current status of the GPIO pin (1 for high, 0 for low)
    bool g_status = hal_gpio_read(key_status->gpiox, key_status->pin);
    // If the key is pressed (GPIO pin is low, assuming active-low logic)
    if (!g_status)
    {
        // Increment the press counter with redundancy protection
        if (!IsTickCounterStart(&key_status->start_tick_count))
        {
            StartTickCounter(&key_status->start_tick_count);
        }

        key_status->press_duration = GetTickCounter(&key_status->start_tick_count);
        // If the press duration exceeds the defined short-press period
        if (key_status->press_duration > GPIO_KEY_STATUS_PRESS_PERIOD && !key_status->pressed)
        {
            key_status->pressed = true;
            key_status->release = false;
            key_status->dispatched = false;
            key_status->ignore = false;
            key_status->state = KEY_STATE_PRESSED;
        }

        // If the press duration exceeds the defined long-press period
        if (key_status->press_duration > GPIO_KEY_STATUS_LONG_PRESS_PERIOD && key_status->pressed)
        {
            key_status->pressed = true;
            key_status->release = false;
            key_status->dispatched = false;
            key_status->state = KEY_STATE_LONG_PRESSED;
        }

        // If the press duration exceeds the defined long-long-press period
        if (key_status->press_duration > GPIO_KEY_STATUS_LONG_LONG_PRESS_PERIOD && key_status->pressed)
        {
            key_status->pressed = true;
            key_status->release = false;
            key_status->dispatched = false;
            key_status->state = KEY_STATE_LONG_LONG_PRESSED;
        }

        if (key_status->press_duration > GPIO_KEY_STATUS_LONG_LONG_LONG_PRESS_PERIOD && key_status->pressed)
        {
            key_status->pressed = true;
            key_status->release = false;
            key_status->dispatched = false;
            key_status->state = KEY_STATE_LONG_LONG_LONG_PRESSED;
        }
    }
    else
    {
        // If the key is released (GPIO pin is high)
        if (key_status->pressed)
        {
            key_status->pressed = false;
            key_status->dispatched = false;
            key_status->release = true;
            key_status->state = KEY_STATE_RELEASED;
        }

        // Reset the pressed status and the counter
        key_status->pressed = false;
        StopTickCounter(&key_status->start_tick_count);
    }
}

/**
 * @brief Processes key press and release events for a specified GPIO key.
 *
 * This function checks the status of the key (`GPIO_KEY_STATUS`). If an event
 * (press or release) has not been dispatched yet, it sends the appropriate
 * message using `send_message()` and marks the event as dispatched to prevent
 * redundant messages for the same event.
 *
 * @param key_status Pointer to the `GPIO_KEY_STATUS` structure representing the key's status.
 */
void task_gpio_key_polling_event_dispatcher(GPIO_KEY_STATUS *key_status)
{
    if (key_status->gpiox == 0)
        return; //|| key_status->pin == 0
    // Check if the event has not been dispatched already or is ignored by user
    if (!key_status->dispatched) //&& !key_status->ignore
    {
        // If the key is in the "pressed" state, send a "key down" event
        if (key_status->pressed && !key_status->ignore)
        {
            /**
             * TASK_MODULE_KEY            - Identifier for the task handling key events.
             * MSG_DEVICE_KEY_DOWN_EVENT - Message type indicating a key press.
             * key_status->key        - The key identifier.
             * KEY_STATE_PRESSED      - The current state of the key.
             */

            send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_KEY_DOWN_EVENT, key_status->key, KEY_STATE_PRESSED);

            // Mark the event as dispatched to prevent duplicate messages
            key_status->dispatched = true;
        }
        // If the key is in the "release" state, send a "key up" event
        else if (key_status->release)
        {
            /**
             * TASK_MODULE_KEY            - Identifier for the task handling key events.
             * MSG_DEVICE_KEY_UP_EVENT - Message type indicating a key release.
             * key_status->key        - The key identifier.
             * KEY_STATE_RELEASED     - The current state of the key.
             */
            send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_KEY_UP_EVENT, key_status->key, KEY_STATE_RELEASED);

            // Mark the event as dispatched to prevent duplicate messages
            key_status->dispatched = true;
        }
    }
}

/**
 * @brief Retrieves the status structure for a specific key.
 *
 * This function searches through an array of GPIO key status structures
 * (`gpio_key_array`) to find the matching key. If the key is found, a pointer
 * to its status structure is returned; otherwise, the function returns `NULL`.
 *
 * @param key The key identifier to search for.
 * @return GPIO_KEY_STATUS* Pointer to the key's status structure if found; otherwise, `NULL`.
 */
GPIO_KEY_STATUS *gpio_get_key_status_by_key(uint8_t key)
{
    // Calculate the size of the array
#if 0
    size_t array_size = sizeof(gpio_key_array) / sizeof(gpio_key_array[0]);

    // Iterate over the array to find the key
    for (size_t i = 0; i < array_size; i++)
    {
        // If the key matches, return its status structure
        if (gpio_key_array[i]->key == key)
        {
            return gpio_key_array[i];
        }
    }
#endif

    for (GPIO_KEY_STATUS **gpio_key_ = gpio_key_array; *gpio_key_ != NULL; gpio_key_++)
    {
        if ((*gpio_key_)->key == key)
        {
            return *gpio_key_;
        }
    }
    // If the key is not found, return NULL
    return NULL;
}

GPIO_STATUS *gpio_get_gpio_status_by_pin(uint16_t gpio_pin)
{
    // Calculate the size of the array
#if 0
    size_t array_size = sizeof(gpio_array) / sizeof(gpio_array[0]);

    // Iterate over the array to find the key
    for (size_t i = 0; i < array_size; i++)
    {
        // If the key matches, return its status structure
        if (gpio_array[i]->pin == gpio_pin)
        {
            return gpio_array[i];
        }
    }
#endif
    for (GPIO_STATUS **gpio_ = gpio_array; *gpio_ != NULL; gpio_++)
    {
        if ((*gpio_)->pin == gpio_pin)
        {
            return *gpio_;
        }
    }
    // If the key is not found, return NULL
    return NULL;
}

bool gpio_is_power_on(void)
{
    return hal_gpio_read((GPIO_GROUP *)GPIO_POWER_SWITCH_GROUP, GPIO_POWER_SWITCH_PIN);
}

void gpio_power_on_off(bool onoff)
{
    if (onoff)
    {
        hal_gpio_write((GPIO_GROUP *)GPIO_POWER_ENABLE_GROUP, GPIO_POWER_ENABLE_PIN, BIT_SET);
        hal_gpio_write((GPIO_GROUP *)GPIO_POWER_SWITCH_GROUP, GPIO_POWER_SWITCH_PIN, BIT_SET);
    }
    else
    {
        hal_gpio_write((GPIO_GROUP *)GPIO_POWER_ENABLE_GROUP, GPIO_POWER_ENABLE_PIN, BIT_RESET);
        hal_gpio_write((GPIO_GROUP *)GPIO_POWER_SWITCH_GROUP, GPIO_POWER_SWITCH_PIN, BIT_RESET);
    }
}

bool gpio_is_high(GPIO_GROUP *gpiox, uint16_t pin)
{
    return hal_gpio_read(gpiox, pin);
}

void task_gpio_event_polling(void)
{
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // for (size_t i = 0; i < sizeof(gpio_array) / sizeof(gpio_array[0]); i++)
    for (GPIO_STATUS **gpio_status = gpio_array; *gpio_status != NULL; gpio_status++)
    {
        task_gpio_polling_status(*gpio_status);
        task_gpio_event_dispatcher(*gpio_status);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // for (size_t i = 0; i < sizeof(gpio_key_array) / sizeof(gpio_key_array[0]); i++)
    for (GPIO_KEY_STATUS **gpio_key_ = gpio_key_array; *gpio_key_ != NULL; gpio_key_++)
    {
        // GPIO_KEY_STATUS *key_status = gpio_key_array[i];
        // if (key_status == NULL)
        //     continue;
        task_gpio_key_polling_event_status(*gpio_key_);
        task_gpio_key_polling_event_dispatcher(*gpio_key_);
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
    if (power_switch_pin_status.changed)
    {
        if (power_switch_pin_status.offon)
            LOG_LEVEL("power switch pin transitioned from LOW to HIGH %d\r\n", power_switch_pin_status.offon);
        else
            LOG_LEVEL("power switch pin transitioned from HIGH to LOW %d\r\n", power_switch_pin_status.offon);

        //send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, GPIO_POWER_KEY_PIN, power_switch_pin_status.offon);
        power_switch_pin_status.changed = false;
    }
				
    if (gpio_zzd_pin_status.changed)
    {
        if (gpio_zzd_pin_status.offon)
            LOG_LEVEL("car zzd pin transitioned from LOW to HIGH %d\r\n", gpio_zzd_pin_status.offon);
        else
            LOG_LEVEL("car zzd pin transitioned from HIGH to LOW %d\r\n", gpio_zzd_pin_status.offon);

        send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, GPIO_POWER_KEY_PIN, power_switch_pin_status.offon);
        gpio_zzd_pin_status.changed = false;
    }

    if (gpio_yzd_pin_status.changed)
    {
        if (gpio_yzd_pin_status.offon)
            LOG_LEVEL("car yzd pin transitioned from LOW to HIGH %d\r\n", gpio_yzd_pin_status.offon);
        else
            LOG_LEVEL("car yzd pin transitioned from HIGH to LOW %d\r\n", gpio_yzd_pin_status.offon);

        //send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, GPIO_POWER_KEY_PIN, power_switch_pin_status.offon);
        gpio_yzd_pin_status.changed = false;
    }

    if (gpio_skd_pin_status.changed)
    {
        if (gpio_skd_pin_status.offon)
            LOG_LEVEL("car skd pin transitioned from LOW to HIGH %d\r\n", gpio_skd_pin_status.offon);
        else
            LOG_LEVEL("car skd pin transitioned from HIGH to LOW %d\r\n", gpio_skd_pin_status.offon);

        //send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, GPIO_POWER_KEY_PIN, power_switch_pin_status.offon);
        gpio_skd_pin_status.changed = false;
    }

		if (gpio_plus_pin_status.changed)
    {
        if (gpio_plus_pin_status.offon)
            LOG_LEVEL("car plus pin transitioned from LOW to HIGH %d\r\n", gpio_plus_pin_status.offon);
        else
            LOG_LEVEL("car plus pin transitioned from HIGH to LOW %d\r\n", gpio_plus_pin_status.offon);

        //send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, GPIO_POWER_KEY_PIN, power_switch_pin_status.offon);
        gpio_plus_pin_status.changed = false;
    }
		
		if (gpio_subt_pin_status.changed)
    {
        if (gpio_subt_pin_status.offon)
            LOG_LEVEL("car subt pin transitioned from LOW to HIGH %d\r\n", gpio_subt_pin_status.offon);
        else
            LOG_LEVEL("car subt pin transitioned from HIGH to LOW %d\r\n", gpio_subt_pin_status.offon);

        //send_message(TASK_MODULE_KEY, MSG_OTSM_DEVICE_GPIO_EVENT, GPIO_POWER_KEY_PIN, power_switch_pin_status.offon);
        gpio_subt_pin_status.changed = false;
    }
#endif
}

/*
bool module_send_handler(ptl_frame_type_t frame_type, ptl_frame_cmd_t cmd, uint16_t param, ptl_proc_buff_t *buff)
{
    return true;
}

bool module_receive_handler(ptl_frame_payload_t *payload, ptl_proc_buff_t *ackbuff)
{
  return true;
}*/

#endif
