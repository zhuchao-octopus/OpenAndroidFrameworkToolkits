/**
 * @file SerialPort.hpp
 * @brief Header file for the SerialPort class
 *
 * This file defines the SerialPort class, which provides functionality
 * for managing serial communication, including opening, closing,
 * reading, and writing data. The class supports a background thread
 * to listen for incoming serial data and invokes a callback function
 * upon receiving data.
 *
 * @author Leiming Li
 * @organization Octopus
 * @date 2025-03-14
 */

#ifndef SERIALPORT_HPP
#define SERIALPORT_HPP

#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <fcntl.h>   // For open()
#include <unistd.h>  // For read(), write(), close()
#include <termios.h> // Serial port configuration
#include <cstring>   // For memset
#include <atomic>    // Thread-safe variables
#include <cstdint> 

/**
 * @class SerialPort
 * @brief A C++ class for managing serial communication
 *
 * This class provides an interface for opening, configuring, reading,
 * and writing to a serial port. It runs a separate thread to continuously
 * read incoming data and pass it to a callback function.
 */
class SerialPort
{
public:
    //using DataCallback = std::function<void(const std::string &)>;
    using DataCallback = std::function<void(const uint8_t *data, size_t length)>;
    /**
     * @brief Constructor for SerialPort
     * @param port The serial port name (e.g., "/dev/ttyS0")
     * @param baud_rate The baud rate for communication (e.g., B115200)
     */
    SerialPort(const std::string &port, int baud_rate);

    /**
     * @brief Destructor to close the serial port
     */
    ~SerialPort();

    /**
     * @brief Opens the serial port and configures it
     * @return True if successfully opened, false otherwise
     */
    bool openPort();

    /**
     * @brief Closes the serial port
     */
    void closePort();

    /**
     * @brief Writes data to the serial port
     * @param data The string data to be sent
     * @return True if data is successfully written, false otherwise
     */
    int writeData(const uint8_t* buffer, size_t length);

    /**
     * @brief Sets a callback function to process received data
     * @param callback The function to be called when data is received
     */
    void setCallback(DataCallback callback);

private:
    /**
     * @brief Background thread function for continuously reading serial data
     */
    void readLoop();

    std::string portName;        ///< Serial port device name
    int baudRate;                ///< Baud rate for communication
    int serialFd;                ///< File descriptor for the serial port
    int epollFd;                 // Add this member for epoll instance
    std::atomic<bool> isRunning; ///< Flag indicating whether the thread is running
    std::thread readThread;      ///< Thread for handling serial read operations
    DataCallback dataCallback;   ///< Callback function for handling received data
    speed_t getBaudRateConstant(int baudRateValue);
    std::string baudRateToString(speed_t baud);
};

#endif // SERIALPORT_HPP
