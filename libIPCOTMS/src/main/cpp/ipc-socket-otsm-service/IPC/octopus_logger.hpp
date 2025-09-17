#ifndef OCTOPUS_LOGGER_HPP
#define OCTOPUS_LOGGER_HPP

#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <string>
#include <fstream>
#include <mutex>

// Enumeration for log levels.
// You can set the current level using Logger::set_level().
// Only messages with a level equal to or higher than the current level will be printed.
enum LogLevel
{
    LOG_NONE = 0, // No logging
    LOG_ERROR,    // Critical errors, must be fixed
    LOG_WARN,     // Warnings that may indicate potential problems
    LOG_INFO,     // General informational messages
    LOG_DEBUG,    // Debug messages for developers
    LOG_TRACE     // Detailed trace messages, suitable for high-frequency logs
};

class Logger
{
private:
    // Write a formatted log string to stdout and file if enabled.
    static void write_log(const std::string &full_message);
    static void write_to_file(const std::string &full_message); 
    // A mutex to make logging thread-safe across multiple threads.
    static std::mutex log_mutex;

    // The current log level setting
    static LogLevel current_level;

    // Whether file output is enabled or not
    static bool is_is_log_to_file;

    static const char *levelToString(LogLevel level);
    static bool shouldLog(LogLevel level);

public:
    // Generate a current timestamp string with microsecond precision.
    static std::string get_timestamp();
    // Set the current logging level; logs below this level will be ignored.
    static void set_level(LogLevel level);

    // Enable or disable file logging.
    // @param enable: If true, logs will be written to a file as well as stdout.
    static void enable_file_output(bool enable);
    // Logs a message with a TAG, timestamp (with microseconds precision), and function name
    // @param tag: The tag associated with the log message (e.g., module name or log level)
    // @param message: The actual log message to be displayed
    // @param func_name: The name of the function from which the log is called
    static void log(LogLevel level, const std::string &tag, const std::string &message, const std::string &func_name);

    // Logs a message without a tag (default TAG: "OINFOR")
    // @param message: The actual log message to be displayed
    // @param func_name: The name of the function from which the log is called
    static void log(LogLevel level, const std::string &message, const std::string &func_name);

    // Logs a message with timestamp accurate to microseconds
    // @param tag: The tag associated with the log message
    // @param message: The actual log message to be displayed
    static void log_with_microseconds(LogLevel level, const std::string &tag, const std::string &message);

    // Logs a message with timestamp accurate to microseconds without a tag (default TAG: "OINFOR")
    // @param message: The actual log message to be displayed
    static void log_with_microseconds(LogLevel level, const std::string &message);

    // Logs a message to a file with a TAG
    // @param tag: The tag associated with the log message
    // @param message: The actual log message to be displayed
    // @param func_name: The name of the function from which the log is called
    static void log_to_file(LogLevel level, const std::string &tag, const std::string &message, const std::string &func_name);

    // Logs a message to a file without a TAG (default TAG: "OINFOR")
    // @param message: The actual log message to be displayed
    // @param func_name: The name of the function from which the log is called
    static void log_to_file(LogLevel level, const std::string &message, const std::string &func_name);

    void rotate();
};

const std::string log_file = "/tmp/octopus_ipc_server.log";
const size_t max_log_size = 1024 * 1024; // 设置日志文件最大为1MB
// Macro for logging with automatic function name
#define LOG_CC(...) Logger::log(LOG_TRACE, __VA_ARGS__, __FUNCTION__)
#define LOG_CCC(tag, ...) Logger::log(LOG_TRACE, tag, __VA_ARGS__, __FUNCTION__)
// Logging macros that automatically include the calling function's name
#define LOG_ERROR(msg) Logger::log(LOG_ERROR, msg, __FUNCTION__)
#define LOG_WARN(msg) Logger::log(LOG_WARN, msg, __FUNCTION__)
#define LOG_INFO(msg) Logger::log(LOG_INFO, msg, __FUNCTION__)
#define LOG_DEBUG(msg) Logger::log(LOG_DEBUG, msg, __FUNCTION__)
#define LOG_TRACE(msg) Logger::log(LOG_TRACE, msg, __FUNCTION__)

// Logging macros with custom tags
#define LOG_TAG_ERROR(tag, msg) Logger::log(LOG_ERROR, tag, msg, __FUNCTION__)
#define LOG_TAG_WARN(tag, msg) Logger::log(LOG_WARN, tag, msg, __FUNCTION__)
#define LOG_TAG_INFO(tag, msg) Logger::log(LOG_INFO, tag, msg, __FUNCTION__)
#define LOG_TAG_DEBUG(tag, msg) Logger::log(LOG_DEBUG, tag, msg, __FUNCTION__)
#define LOG_TAG_TRACE(tag, msg) Logger::log(LOG_TRACE, tag, msg, __FUNCTION__)

#endif // OCTOPUS_LOGGER_HPP
