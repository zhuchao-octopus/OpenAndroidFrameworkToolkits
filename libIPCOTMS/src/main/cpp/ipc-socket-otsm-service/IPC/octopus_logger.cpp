// octopus_logger.hpp - Header file for the Logger class
// Author: ak47
// Date: 2025-03-14
// Description:
// This file defines a Logger class that provides a static method to log messages to the console.
// The log messages are prefixed with a specified TAG and a timestamp in the format [YYYY-MM-DD HH:MM:SS].
// This logger is useful for adding timestamped logs in various modules for debugging or tracking purposes.
//
// Usage Example:
// Logger::log("INFO", "System started successfully");
// This will output something like:
// [INFO] [2025-03-14 12:34:56] System started successfully

#include <octopus_logger.hpp>
#include <iostream>
#include <sstream>   // 必须加这一行
#include <string>

std::mutex Logger::log_mutex;
LogLevel Logger::current_level = LOG_DEBUG; // Default log level
bool Logger::is_is_log_to_file = false;     // By default, don't log to file

const char *Logger::levelToString(LogLevel level)
{
    switch (level)
    {
    case LOG_NONE:
        return "NONE";
    case LOG_ERROR:
        return "ERROR";
    case LOG_WARN:
        return "WARN";
    case LOG_INFO:
        return "INFO";
    case LOG_DEBUG:
        return "DEBUG";
    case LOG_TRACE:
        return "TRACE";
    default:
        return "UNKNOWN";
    }
}

// Helper function to get the current timestamp with microsecond precision
std::string Logger::get_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf;
    localtime_r(&in_time_t, &buf); // Get local time

    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    auto milliseconds = microseconds / 1000;
    auto micros = microseconds % 1000;

    std::ostringstream timestamp;
    timestamp << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << "."
              << std::setw(3) << std::setfill('0') << milliseconds % 1000 << "." // milliseconds
              << std::setw(3) << std::setfill('0') << micros;                    // microseconds
    return timestamp.str();
}

bool Logger::shouldLog(LogLevel level)
{
    return static_cast<int>(level) <= static_cast<int>(current_level);
}

// Set the current log level. Only logs at or above this level will be logged.
void Logger::set_level(LogLevel level)
{
    current_level = level;
}

// Enable or disable file logging
void Logger::enable_file_output(bool enable)
{
    is_is_log_to_file = enable;
}

void Logger::write_log(const std::string &full_message)
{
    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << full_message << std::endl;
    if (is_is_log_to_file)
    {
        write_to_file(full_message);
    }
}

void Logger::write_to_file(const std::string &full_message)
{
    std::ofstream log_file("octopus.log", std::ios::app);
    if (log_file.is_open())
    {
        log_file << full_message << std::endl;
    }
}

void Logger::log(LogLevel level, const std::string &tag, const std::string &message, const std::string &func_name)
{
    if (!shouldLog(level))
        return;
    std::string function_name = func_name.empty() ? __func__ : func_name;
    std::string timestamp = get_timestamp();

    std::ostringstream log_message;
    log_message << "[" << tag << "] "
                << "[" << timestamp << "] "
                << "[" << function_name << "] "
                << message;

    write_log(log_message.str());
}

void Logger::log(LogLevel level, const std::string &message, const std::string &func_name)
{
    if (!shouldLog(level))
        return;
    log(level, "OINFOR", message, func_name);
}

void Logger::log_to_file(LogLevel level, const std::string &tag, const std::string &message, const std::string &func_name)
{
    if (!shouldLog(level))
        return;
    std::string function_name = func_name.empty() ? __func__ : func_name;
    std::string timestamp = get_timestamp();

    std::ostringstream log_message;
    log_message << "[" << tag << "] "
                << "[" << timestamp << "] "
                << "[" << function_name << "] "
                << message;

    write_to_file(log_message.str());
}

void Logger::log_to_file(LogLevel level, const std::string &message, const std::string &func_name)
{
    if (!shouldLog(level))
        return;
    log_to_file(level, "OINFOR", message, func_name);
}

void Logger::rotate()
{
}
