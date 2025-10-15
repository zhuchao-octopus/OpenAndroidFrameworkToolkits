/**************************************************************************************
 * @file octopus_task_manager_key.h
 * @brief Header file for managing key/button input handling within the Octopus Task Manager.
 *
 * This file defines macros, types, and function declarations for managing key/button
 * input events such as detecting key presses, key releases, long presses, and double presses.
 * It includes function declarations for the initialization, starting, and stopping of key input processing,
 * as well as handling various key states.
 *
 * Key events can be used to navigate through menus, confirm selections, or power on/off the system.
 *
 * @note The key mapping (e.g., OCTOPUS_KEY_0) and key codes should be customized based on the actual hardware.
 *
 * @ingroup APP:SUB_TYPE
 * @version  1.0
 * @date     2024-12-12
 * @author   Octopus Team
 **************************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_KEY_H__
#define __OCTOPUS_TASK_MANAGER_KEY_H__

/**************************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

/**************************************************************************************
 * MACROS
 * The following macros define key IDs and their respective actions.
 */

// Key IDs (these should correspond to the physical keys on the device)
// =====================================================================================
// Function Keys
// =====================================================================================
#define OCTOPUS_KEY_ESC 0
#define OCTOPUS_KEY_F1 1
#define OCTOPUS_KEY_F2 2
#define OCTOPUS_KEY_F3 3
#define OCTOPUS_KEY_F4 4
#define OCTOPUS_KEY_F5 5
#define OCTOPUS_KEY_F6 6
#define OCTOPUS_KEY_F7 7
#define OCTOPUS_KEY_F8 8
#define OCTOPUS_KEY_F9 9
#define OCTOPUS_KEY_F10 10
#define OCTOPUS_KEY_F11 11
#define OCTOPUS_KEY_F12 12
#define OCTOPUS_KEY_PRINTSCREEN 13
#define OCTOPUS_KEY_SCROLLLOCK 14
#define OCTOPUS_KEY_PAUSE 15

// =====================================================================================
// Number Row
// =====================================================================================
#define OCTOPUS_KEY_GRAVE 16 // `~
#define OCTOPUS_KEY_1 17
#define OCTOPUS_KEY_2 18
#define OCTOPUS_KEY_3 19
#define OCTOPUS_KEY_4 20
#define OCTOPUS_KEY_5 21
#define OCTOPUS_KEY_6 22
#define OCTOPUS_KEY_7 23
#define OCTOPUS_KEY_8 24
#define OCTOPUS_KEY_9 25
#define OCTOPUS_KEY_0 26
#define OCTOPUS_KEY_MINUS 27 // -_
#define OCTOPUS_KEY_EQUAL 28 // =+
#define OCTOPUS_KEY_BACKSPACE 29

// =====================================================================================
// Tab / Caps / Shift / Ctrl
// =====================================================================================
#define OCTOPUS_KEY_TAB 30
#define OCTOPUS_KEY_Q 31
#define OCTOPUS_KEY_W 32
#define OCTOPUS_KEY_E 33
#define OCTOPUS_KEY_R 34
#define OCTOPUS_KEY_T 35
#define OCTOPUS_KEY_Y 36
#define OCTOPUS_KEY_U 37
#define OCTOPUS_KEY_I 38
#define OCTOPUS_KEY_O 39
#define OCTOPUS_KEY_P 40
#define OCTOPUS_KEY_LEFTBRACE 41  // [
#define OCTOPUS_KEY_RIGHTBRACE 42 // ]
#define OCTOPUS_KEY_BACKSLASH 43  // \|

#define OCTOPUS_KEY_CAPSLOCK 44
#define OCTOPUS_KEY_A 45
#define OCTOPUS_KEY_S 46
#define OCTOPUS_KEY_D 47
#define OCTOPUS_KEY_F 48
#define OCTOPUS_KEY_G 49
#define OCTOPUS_KEY_H 50
#define OCTOPUS_KEY_J 51
#define OCTOPUS_KEY_K 52
#define OCTOPUS_KEY_L 53
#define OCTOPUS_KEY_SEMICOLON 54  // ;
#define OCTOPUS_KEY_APOSTROPHE 55 // '
#define OCTOPUS_KEY_ENTER 56

#define OCTOPUS_KEY_LEFTSHIFT 57
#define OCTOPUS_KEY_Z 58
#define OCTOPUS_KEY_X 59
#define OCTOPUS_KEY_C 60
#define OCTOPUS_KEY_V 61
#define OCTOPUS_KEY_B 62
#define OCTOPUS_KEY_N 63
#define OCTOPUS_KEY_M 64
#define OCTOPUS_KEY_COMMA 65 // ,
#define OCTOPUS_KEY_DOT 66   // .
#define OCTOPUS_KEY_SLASH 67 // /
#define OCTOPUS_KEY_RIGHTSHIFT 68

#define OCTOPUS_KEY_LEFTCTRL 69
#define OCTOPUS_KEY_LEFTWIN 70
#define OCTOPUS_KEY_LEFTALT 71
#define OCTOPUS_KEY_SPACE 72
#define OCTOPUS_KEY_RIGHTALT 73
#define OCTOPUS_KEY_RIGHTWIN 74
#define OCTOPUS_KEY_MENU 75
#define OCTOPUS_KEY_RIGHTCTRL 76

// =====================================================================================
// Arrow / Edit Keys
// =====================================================================================
#define OCTOPUS_KEY_INSERT 77
#define OCTOPUS_KEY_DELETE 78
#define OCTOPUS_KEY_HOME 79
#define OCTOPUS_KEY_END 80
#define OCTOPUS_KEY_PAGEUP 81
#define OCTOPUS_KEY_PAGEDOWN 82
#define OCTOPUS_KEY_UP 83
#define OCTOPUS_KEY_DOWN 84
#define OCTOPUS_KEY_LEFT 85
#define OCTOPUS_KEY_RIGHT 86

// =====================================================================================
// Numpad
// =====================================================================================
#define OCTOPUS_KEY_NUMLOCK 87
#define OCTOPUS_KEY_KP_SLASH 88
#define OCTOPUS_KEY_KP_ASTERISK 89
#define OCTOPUS_KEY_KP_MINUS 90
#define OCTOPUS_KEY_KP_7 91
#define OCTOPUS_KEY_KP_8 92
#define OCTOPUS_KEY_KP_9 93
#define OCTOPUS_KEY_KP_PLUS 94
#define OCTOPUS_KEY_KP_4 95
#define OCTOPUS_KEY_KP_5 96
#define OCTOPUS_KEY_KP_6 97
#define OCTOPUS_KEY_KP_1 98
#define OCTOPUS_KEY_KP_2 99
#define OCTOPUS_KEY_KP_3 100
#define OCTOPUS_KEY_KP_0 101
#define OCTOPUS_KEY_KP_DOT 102
#define OCTOPUS_KEY_KP_ENTER 103

///////////////////////////////////////////////////////////////////////////////////////////
// Function keys
#define OCTOPUS_KEY_POWER 150 ///< Power key
#define OCTOPUS_KEY_ZZD 151   ///< Left center key (ZZD, hardware specific)
#define OCTOPUS_KEY_YZD 152   ///< Right center key (YZD, hardware specific)
#define OCTOPUS_KEY_SKD 153   ///< Up key (SKD, hardware specific)
#define OCTOPUS_KEY_DDD 154   ///< Down key (DDD, hardware specific)
#define OCTOPUS_KEY_PLUS 155  ///< Plus key / volume up
#define OCTOPUS_KEY_SUBT 156  ///< Minus key / volume down
#define OCTOPUS_KEY_PAGE 157  ///< Page key

// #define OCTOPUS_NORMAL_KEY_MAX 100

////////////////////////////////////////////////////////////////////////////////////////////////
// #define OCTOPUS_KEY_PLUS_LONG (OCTOPUS_KEY_PLUS + OCTOPUS_NORMAL_KEY_MAX)
// #define OCTOPUS_KEY_SUBT_LONG (OCTOPUS_KEY_SUBT + OCTOPUS_NORMAL_KEY_MAX)
// #define OCTOPUS_KEY_CUSTOMER_BASE 200

#define OCTOPUS_KEY_NONE 255

// Key states
#define KEY_STATE_NONE (0x00)                   /**< Key is released */
#define KEY_STATE_PRESSED (0x01)                /**< Key is pressed */
#define KEY_STATE_LONG_PRESSED (0x02)           /**< Key is long pressed */
#define KEY_STATE_LONG_LONG_PRESSED (0x03)      /**< Key is double pressed */
#define KEY_STATE_LONG_LONG_LONG_PRESSED (0x04) /**< Key is double pressed */
#define KEY_STATE_RELEASED (0xFF)               /**< Key is released */
/*******************************************************************************
 * DEBUG SWITCH MACROS
 */
#ifdef __cplusplus
extern "C"
{
#endif

    /*******************************************************************************
     * GLOBAL FUNCTIONS DECLARATIONS
     * The following functions manage the state of key handling in the system.
     */

    /**
     * @brief Initializes the key input handling system.
     *
     * This function sets up the necessary configurations and peripherals to start handling key events.
     */
    void task_key_init_running(void);

    /**
     * @brief Starts the key input handling process.
     *
     * This function begins the process of detecting and processing key events, allowing the system
     * to react to key presses.
     */
    void task_key_start_running(void);

    /**
     * @brief Asserts that the key input system is running.
     *
     * This function ensures that the key input system is properly initialized and running.
     */
    void task_key_assert_running(void);

    /**
     * @brief Handles key input events in the running state.
     *
     * This function processes the key events in real-time, updating the system based on key actions.
     */
    void task_key_running(void);

    /**
     * @brief Posts key input events for further processing.
     *
     * This function queues key input events for future processing or handling by other tasks.
     */
    void task_key_post_running(void);

    /**
     * @brief Stops the key input handling process.
     *
     * This function stops the key input event detection, effectively disabling the key input system.
     */
    void task_key_stop_running(void);

#ifdef __cplusplus
}
#endif

#endif /* __OCTOPUS_TASK_MANAGER_KEY_H__ */
