#ifndef __OCTOPUS_SERIALPORT_C_H__
#define __OCTOPUS_SERIALPORT_C_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h> /**< Boolean type definitions (true/false) */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @file octopus_serialport_c.h
     * @brief C-style interface for serial port communication (suitable for C/C++ integration).
     *
     * This header defines functions to create, manage, and communicate over a serial port,
     * and allows users to register a callback for incoming data.
     *
     * The implementation is expected to use a background thread or non-blocking IO to
     * monitor incoming data and trigger the callback.
     */

    // Abstract handle type for managing serial port instances
    typedef void *SerialPortHandle;

    /**
     * @brief Create and open a serial port with the specified port name and baud rate.
     *
     * @param port The path to the serial device (e.g., "/dev/ttyS1" or "COM3").
     * @param baud_rate The desired baud rate (e.g., 9600, 115200).
     * @return A handle to the opened serial port, or NULL on failure.
     */
    SerialPortHandle serialport_create(const char *port, int baud_rate);

    /**
     * @brief Close the serial port and release associated resources.
     *
     * @param handle The handle returned by serialport_create().
     */
    void serialport_destroy(SerialPortHandle handle);

    /**
     * @brief Write data to the serial port.
     *
     * @param handle The serial port handle.
     * @param data The data to be sent (null-terminated string or binary buffer).
     * @return Number of bytes written, or -1 on failure.
     */
    int serialport_write(SerialPortHandle handle, const uint8_t *data, size_t length);

    /**
     * @brief Function pointer type for receiving data asynchronously.
     *
     * This function will be called whenever data is received from the serial port.
     *
     * @param data Pointer to received data (may not be null-terminated).
     * @param length Length of the received data buffer.
     */
    typedef void (*DataCallback)(const uint8_t  *data, int length);

    /**
     * @brief Register a callback function to be called when data is received.
     *
     * @param handle The serial port handle.
     * @param callback The function to call when data is available.
     */
    bool serialport_set_callback(SerialPortHandle handle, DataCallback callback);

#ifdef __cplusplus
}
#endif

#endif // OCTOPUS_SERIALPORT_C_H
