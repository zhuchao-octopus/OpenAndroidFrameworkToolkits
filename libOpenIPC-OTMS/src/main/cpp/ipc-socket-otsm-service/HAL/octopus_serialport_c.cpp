/**
 * @brief Creates and opens a serial port.
 *
 * This function initializes a SerialPort C++ object and returns it as an opaque handle.
 * The object internally manages file descriptors, settings, and I/O threads.
 *
 * @param port      Path to the serial device (e.g., "/dev/ttyUSB0", "COM3").
 * @param baud_rate The baud rate to use (e.g., 9600, 115200).
 * @return A valid SerialPortHandle on success, or NULL on failure.
 */

#include "octopus_serialport_c.h"
#include "octopus_serialport.hpp"

extern "C"
{
    SerialPortHandle serialport_create(const char *port, int baud_rate)
    {
        if (!port || baud_rate <= 0)
        {
            return nullptr;
        }

        try
        {
            return new SerialPort(port, baud_rate);
        }
        catch (...)
        {
            return nullptr; // Ensure exception safety for C ABI
        }
    }

    /**
     * @brief Destroys the serial port object and releases its resources.
     *
     * @param handle The SerialPortHandle returned by serialport_create().
     */
    void serialport_destroy(SerialPortHandle handle)
    {
        if (handle)
        {
            delete static_cast<SerialPort *>(handle);
        }
    }

    /**
     * @brief Sends data over the serial port.
     *
     * This function sends a null-terminated string as a binary block.
     *
     * @param handle A valid SerialPortHandle.
     * @param data   Pointer to the data to send (must be null-terminated).
     * @return 1 if success, 0 if sending failed, -1 if input is invalid.
     */
    int serialport_write(SerialPortHandle handle, const uint8_t *data, size_t length)
    {
        if (!handle || !data || length == 0)
        {
            return -1;
        }

        SerialPort *serial = static_cast<SerialPort *>(handle);
        return serial->writeData(data, length);
    }
    /**
     * @brief Registers a callback to be invoked when data is received.
     *
     * This version avoids copying to std::string and instead passes raw byte data
     * directly to the user-provided callback function.
     *
     * @param handle   A valid SerialPortHandle.
     * @param callback A function that receives raw uint8_t* data and its length.
     * @return true if the callback was registered and port opened successfully.
     */
    bool serialport_set_callback(SerialPortHandle handle, DataCallback callback)
    {
        if (!handle)
        {
            std::cout << "Failed to serialport_set_callback : handle is null" << std::endl;
            return false;
        }
        if (!callback)
        {
            std::cout << "Failed to serialport_set_callback : callback is null" << std::endl;
            return false;
        }

        SerialPort *serial = static_cast<SerialPort *>(handle);

        // Set internal callback to forward raw byte data directly
        serial->setCallback([callback](const uint8_t *data, size_t length)
                            {
                            if (data && length > 0)
                            {
                                callback(data, length);
                            } });

        // Open the serial port after setting the callback
        return serial->openPort();
    }
}
