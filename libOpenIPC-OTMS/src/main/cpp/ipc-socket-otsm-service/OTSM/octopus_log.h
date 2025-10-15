/*******************************************************************************
 * @file     octopus_log.h
 * @brief    Provides the logging functions for UART communication and formatted
 *           logging output with different log levels.
 *
 * This header file defines the necessary API to handle logging in the system
 * with configurable logging levels, including DEBUG, INFO, WARNING, and ERROR.
 * It supports output via UART with customizable callbacks for character output.
 *
 * The logging system can be used to log messages with varying verbosity,
 * useful for debugging and error tracking. The log messages can be formatted
 * using standard format specifiers and include buffer logging.
 *
 * @note     This file assumes UART or other serial communication for log output
 *           and may require platform-specific configurations.
 *
 * @version  1.0.0
 * @date     2024-12-09
 * @author   Octopus Team
 *******************************************************************************/

#ifndef __OCTOPUS_TASK_MANAGER_LOG_H__
#define __OCTOPUS_TASK_MANAGER_LOG_H__

/*******************************************************************************
 * INCLUDES
 */
#include "octopus_base.h" //  Base include file for the Octopus project.

/**
 * @enum LogLevel
 * @brief Enumeration representing different logging levels.
 */
// #ifdef PLATFORM_CST_OSAL_RTOS
typedef enum
{
    LOG_LEVEL_NONE,    /**< No logging output. */
    LOG_LEVEL_DEBUG,   /**< Debug-level logs. */
    LOG_LEVEL_INFO,    /**< Informational logs. */
    LOG_LEVEL_WARNING, /**< Warning-level logs. */
    LOG_LEVEL_ERROR,   /**< Error-level logs. */
    LOG_LEVEL_NO,      /**< Reserved (potentially unused). */
    LOG_LEVEL_MAX      /**< Maximum logging level. */
} DBG_LOG_LEVEL;

/**
 * @typedef std_putc
 * @brief Callback function type for handling output data.
 * @param data Pointer to the data buffer to output.
 * @param size Size of the data buffer.
 */
typedef void (*std_putc)(char *data, uint16_t size);

/**
 * @brief Internal function for formatted string processing.
 * @param putc Callback function for character output.
 * @param fmt Format string.
 * @param args Variable argument list.
 */

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Sets the global logging level.
     * @param level The desired logging level.
     */
    void dbg_log_set_level(DBG_LOG_LEVEL level);
    void dbg_log_set_channel(uint8_t channel);
    /**
     * @brief Outputs a buffer as a log message.
     * @param buff Pointer to the buffer.
     * @param lenth Length of the buffer.
     */
    void dbg_log_printf_buffer(const uint8_t *buff, uint16_t lenth);

    /**
     * @brief Logs a formatted message.
     * @param format Format string.
     * @param ... Additional arguments for formatting.
     */
    void dbg_log_printf(const char *format, ...);

    /**
     * @brief Logs a formatted message with a function name and log level.
     * @param function_name Name of the calling function.
     * @param format Format string.
     * @param ... Additional arguments for formatting.
     */
    void dbg_log_printf_level(const char *function_name, const char *format, ...);

    /**
     * @brief Logs a buffer with a function name and log level.
     * @param function_name Name of the calling function.
     * @param buff Pointer to the buffer.
     * @param lenth Length of the buffer.
     */
    void dbg_log_printf_buffer_level(const char *function_name, const uint8_t *buff, uint16_t lenth);
#ifdef __cplusplus
}
#endif

/*******************************************************************************
 * MACROS
 */
/** Macro to retrieve the current function name. */
// #define F_NAME __FUNCTION__

/** Macro for formatted string processing. */
#ifdef PLATFORM_CST_OSAL_RTOS
#define VSPRINTF(a, b, ...) vsprintf__(a, b, __VA_ARGS__)
#else
// #define VSPRINTF(a, b, ...) printf(a, b, __VA_ARGS__)
#endif

/** Macro to set the logging level. */
#define LOG_SET_LEVEL(l) dbg_log_set_level(l)

/** Macro to log a message with a specific level. */
#define LOG_LEVEL(...) dbg_log_printf_level(__FUNCTION__, __VA_ARGS__)

/** Macro to log a simple message. */
#define LOG_NONE(...) dbg_log_printf(__VA_ARGS__)

#define LOG_DBG(...) dbg_log_printf(__VA_ARGS__)

/** Macro to log a buffer. */
#define LOG_BUFF(a, b) dbg_log_printf_buffer(a, b)
#define LOG_BUFF_LEVEL(a, b) dbg_log_printf_buffer_level(__FUNCTION__, a, b)

#endif // __OCTOPUS_TASK_MANAGER_LOG_H__
