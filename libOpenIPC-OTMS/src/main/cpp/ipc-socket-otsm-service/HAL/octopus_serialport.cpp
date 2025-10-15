/**
 * @file SerialPort.cpp
 * @brief Optimized implementation file for the SerialPort class using epoll() for efficient asynchronous serial communication.
 *
 * This file contains an improved version of the SerialPort class designed to provide
 * higher performance and responsiveness, particularly suitable for high-frequency car systems, including CAN data parsing.
 */

#include "octopus_serialport.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <termios.h>

// Constructor
/**
 * @brief Constructs a SerialPort object with the specified port name and baud rate.
 *
 * @param port The name of the serial port (e.g., "/dev/ttyS0").
 * @param baud_rate The baud rate for communication (e.g., 115200).
 */
SerialPort::SerialPort(const std::string &port, int baud_rate)
    : portName(port), baudRate(baud_rate), serialFd(-1), epollFd(-1), isRunning(false) {}

// Destructor
/**
 * @brief Destructor for the SerialPort class. Ensures the serial port is closed.
 */
SerialPort::~SerialPort()
{
    closePort();
}

// Open and configure the serial port
/**
 * @brief Opens the serial port and configures it with the specified baud rate.
 * This method also sets up the epoll instance for efficient event-driven communication.
 *
 * @return True if the port was successfully opened and configured, false otherwise.
 */
bool SerialPort::openPort()
{
    // Open the serial port in non-blocking mode
    // O_RDWR     : Open for reading and writing
    // O_NOCTTY   : Do not assign the opened port as the controlling terminal for the process
    // O_NONBLOCK : Enable non-blocking I/O
    serialFd = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serialFd == -1)
    {
        std::cout << "Failed to open serial port: " << portName << std::endl;
        return false;
    }

    // Retrieve current terminal I/O settings
    struct termios options = {0};
    tcgetattr(serialFd, &options);
    speed_t b_baudRate = getBaudRateConstant(baudRate);
    // Set baud rate for both input and output
    cfsetispeed(&options, b_baudRate);
    cfsetospeed(&options, b_baudRate);

    // Configure control modes
    options.c_cflag |= (CLOCAL | CREAD); // Enable receiver, ignore modem control lines
    options.c_cflag &= ~PARENB;          // Disable parity
    options.c_cflag &= ~CSTOPB;          // Set 1 stop bit
    options.c_cflag &= ~CSIZE;           // Clear data bit size setting
    options.c_cflag |= CS8;              // Set 8 data bits

    // options.c_cflag = B115200|CS8|CLOCAL|CREAD;
    // Configure local modes (line discipline)
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input (non-canonical), no echo, no signal chars
    // Configure input modes
    options.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable XON/XOFF software flow control
    options.c_iflag &= ~(ICRNL | INLCR);
    // Configure output modes
    options.c_oflag &= ~OPOST; // Raw output (disable output processing)

    // Set VMIN and VTIME for non-canonical mode
    // VMIN  = 1  : Read will block until at least 1 byte is received
    // VTIME = 1  : Read timeout in tenths of a second (i.e., 0.1s)
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 1;
    tcflush(serialFd, TCIOFLUSH);
    // Apply the modified settings to the serial port immediately
    tcsetattr(serialFd, TCSANOW, &options);

    // Print out the serial port configuration
    // std::cout << "Serial Port Configuration:" << std::endl;
    // std::cout << "Port Name: " << portName << std::endl;
    // std::cout << "Baud Rate: " << baudRate << std::endl;
    // std::cout << "Data Bits: 8" << std::endl;
    // std::cout << "Stop Bits: 1" << std::endl;
    // std::cout << "Parity: None" << std::endl;
    // std::cout << "Flow Control: Disabled" << std::endl;

    // Create an epoll instance for efficient I/O event notification
    epollFd = epoll_create1(0);
    if (epollFd == -1)
    {
        std::cout << "Failed to create epoll instance." << std::endl;
        close(serialFd);
        return false;
    }

    // Register the serial file descriptor with epoll for input events
    struct epoll_event ev;
    ev.events = EPOLLIN;   // Notify when input is available to read
    ev.data.fd = serialFd; // Associate the serial file descriptor with the event

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serialFd, &ev) == -1)
    {
        std::cout << "Failed to add serial fd to epoll." << std::endl;
        close(serialFd);
        return false;
    }

    // Launch a separate thread to continuously monitor and read data
    isRunning = true;
    readThread = std::thread(&SerialPort::readLoop, this);

    // Success: Serial port is configured and reading thread is started
    return true;
}

// Close the serial port and stop the thread
/**
 * @brief Closes the serial port and stops the read loop thread.
 */
void SerialPort::closePort()
{
    if (isRunning)
    {
        isRunning = false;
        if (readThread.joinable())
        {
            readThread.join(); // Join the thread to ensure proper shutdown
        }
    }
    if (serialFd != -1)
    {
        close(serialFd); // Close the serial file descriptor
        serialFd = -1;
    }
    if (epollFd != -1)
    {
        close(epollFd); // Close the epoll instance
        epollFd = -1;
    }
}

// Write data to the serial port
/**
 * @brief Writes the given data to the serial port.
 *
 * @param data The data to be written to the serial port.
 * @return True if the data was successfully written, false otherwise.
 */
int SerialPort::writeData(const uint8_t *buffer, size_t length)
{
    if (serialFd == -1)
    {
        std::cerr << "Serial port not open, cannot write data." << std::endl;
        return 0;
    }

    // Attempt to write the data buffer to the serial port
    ssize_t bytesWritten = write(serialFd, buffer, length);

    if (bytesWritten == static_cast<ssize_t>(length))
    {
// Log the data in hex format for debugging
#if 0
         std::cout << "[Serial Write] Success: ";
         for (size_t i = 0; i < length; ++i)
         {
            printf("%02X ", buffer[i]);
         }
         std::cout << std::endl;
#endif
        return bytesWritten;
    }
    else
    {
        std::cerr << "[Serial Write] Failed: wrote " << bytesWritten << " of " << length << " bytes." << std::endl;
        return bytesWritten;
    }
}

// Set the callback for received data
/**
 * @brief Sets the callback function to be called when data is received from the serial port.
 *
 * @param callback The callback function that will process the received data.
 */
void SerialPort::setCallback(DataCallback callback)
{
    dataCallback = callback;
}

// Read loop using epoll() for efficient event-driven reading
/**
 * @brief The read loop listens for data availability on the serial port using epoll.
 * When data is available, it reads and processes it using the provided callback.
 */
void SerialPort::readLoop()
{
    struct epoll_event events[1]; // Array to store the events returned by epoll_wait
    uint8_t buffer[512];          // Buffer for reading data from the serial port

    // Infinite loop to continually monitor for incoming data on the serial port
    while (isRunning)
    {
        // Wait for events with epoll_wait. The function will block until an event occurs.
        int nfds = epoll_wait(epollFd, events, 1, -1); // Wait for events
        if (nfds == -1)
        {
            // If epoll_wait returns -1, it indicates an error
            // If the error is EINTR (interrupted system call), it is typically safe to retry
            if (errno == EINTR)
            {
                continue; // Retry epoll_wait if it was interrupted
            }

            // Log other errors returned by epoll_wait
            std::cout << "Error in epoll_wait: " << strerror(errno) << std::endl;
            break; // If it's a non-recoverable error, break the loop and stop reading
        }

        // If we have events (i.e., serialFd is ready for reading)
        if (events[0].data.fd == serialFd)
        {
            // Read the available data from the serial port into the buffer
            int bytesRead = read(serialFd, buffer, sizeof(buffer));
            if (bytesRead > 0)
            {
// If data was successfully read, convert it into a string
#if 0
                std::string receivedData(reinterpret_cast<const char*>(buffer), bytesRead);
                // 以十六进制形式打印接收到的数据
                std::cout << "[Serial Read] " << bytesRead << " bytes received: ";
                for (int i = 0; i < bytesRead; ++i) {
                    printf("%02X ", buffer[i]);
                }
                std::cout << " | As string: \"" << receivedData << "\"" << std::endl;
#endif
                // If a callback is set, call the callback function to process the received data
                if (dataCallback)
                {
                    // dataCallback(receivedData); // Process the received data
                    dataCallback(reinterpret_cast<const uint8_t *>(buffer), bytesRead);
                }
            }
        }
    }

    // Optionally, you could perform any necessary cleanup or recovery actions here.
}

speed_t SerialPort::getBaudRateConstant(int baudRateValue)
{
    switch (baudRateValue)
    {
    case 0:
        return B0;
    case 50:
        return B50;
    case 75:
        return B75;
    case 110:
        return B110;
    case 134:
        return B134;
    case 150:
        return B150;
    case 200:
        return B200;
    case 300:
        return B300;
    case 600:
        return B600;
    case 1200:
        return B1200;
    case 1800:
        return B1800;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    case 460800:
        return B460800;
    case 500000:
        return B500000;
    case 576000:
        return B576000;
    case 921600:
        return B921600;
    case 1000000:
        return B1000000;
    case 1152000:
        return B1152000;
    case 1500000:
        return B1500000;
    case 2000000:
        return B2000000;
    case 2500000:
        return B2500000;
    case 3000000:
        return B3000000;
    case 3500000:
        return B3500000;
    case 4000000:
        return B4000000;
    default:
        return B9600; // Fallback default
    }
}

std::string SerialPort::baudRateToString(speed_t baud)
{
    switch (baud)
    {
    case B0:
        return "0";
    case B50:
        return "50";
    case B75:
        return "75";
    case B110:
        return "110";
    case B134:
        return "134";
    case B150:
        return "150";
    case B200:
        return "200";
    case B300:
        return "300";
    case B600:
        return "600";
    case B1200:
        return "1200";
    case B1800:
        return "1800";
    case B2400:
        return "2400";
    case B4800:
        return "4800";
    case B9600:
        return "9600";
    case B19200:
        return "19200";
    case B38400:
        return "38400";
    case B57600:
        return "57600";
    case B115200:
        return "115200";
    case B230400:
        return "230400";
#ifdef B460800
    case B460800:
        return "460800";
#endif
#ifdef B500000
    case B500000:
        return "500000";
#endif
#ifdef B576000
    case B576000:
        return "576000";
#endif
#ifdef B921600
    case B921600:
        return "921600";
#endif
#ifdef B1000000
    case B1000000:
        return "1000000";
#endif
    default:
        return "Unknown";
    }
}
