/*
 * File: octopus_ipc_ptl_handler.hpp
 * Description: This header file defines the IPC (Inter-Process Communication) message structure and message group types
 *              used in the Octopus system. It includes the necessary declarations for the message serialization and
 *              deserialization process. Additionally, it defines the command types and message groups that are used to
 *              differentiate different types of messages sent and received via the communication channel.
 *
 *              The file also includes utility functions for mapping message IDs and groups to human-readable strings
 *              for debugging or logging purposes.
 *
 * Features:
 * - Defines message groups for categorizing different types of messages
 * - Declares the `DataMessage` structure, which includes message ID, command type, and data elements
 * - Provides functions for serializing and deserializing `DataMessage` objects into binary format
 * - Includes functions for retrieving human-readable message names and group names for debugging and logging
 *
 * Author: [Your Name]
 * Date: [Date]
 * Version: 1.0
 */
/// @brief ///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __OCTOPUS_IPC_PTL_HANDLER_HPP__
#define __OCTOPUS_IPC_PTL_HANDLER_HPP__

#include <vector>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "../OTSM/octopus_message.h"

/// @brief ///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class DataMessage
{
public:
    // A constant for the fixed header value
    static constexpr uint16_t _HEADER_ = 0xA5A5; ///< Fixed header value indicating the start of a message

    uint16_t msg_header;       ///< Header for identifying the message (usually fixed)
    uint8_t msg_group;         ///< Group ID for categorizing the message type
    uint8_t msg_id;            ///< Message ID within the group
    uint16_t msg_length;       ///< Length of the data in the message (max 255) msg_length = data.size();
    std::vector<uint8_t> data; ///< Message data (content of the message)

    /**
     * @brief Default constructor for the DataMessage object.
     *
     * Initializes the header with the fixed HEADER value and other fields to 0.
     */
    DataMessage();

    DataMessage(const std::vector<uint8_t> &data_array);

    DataMessage(uint8_t msg_group, uint8_t msg_id, const std::vector<uint8_t> &data_array); // Constructor declaration
    /**
     * @brief Serializes the DataMessage object into a byte vector.
     *
     * Converts the message into a binary format suitable for sending over a communication channel.
     *
     * @return std::vector<uint8_t> The serialized byte vector representation of the message.
     */
    std::vector<uint8_t> serializeMessage() const;

    /**
     * @brief Deserializes a byte vector into a DataMessage object.
     *
     * Converts a serialized byte array back into a DataMessage object, which represents the original message.
     *
     * @param buffer The byte vector containing the serialized message data.
     * @return DataMessage The deserialized DataMessage object.
     */
    static DataMessage deserializeMessage(const std::vector<uint8_t> &buffer);

    /**
     * @brief Validates if the message has a valid structure.
     *
     * Checks that the message has a valid header, group ID, message ID, and data length.
     *
     * @return true if the message is valid, false otherwise.
     */
    bool isValid() const;

    /**
     * @brief Prints the contents of the DataMessage object.
     *
     * Outputs the message's header, group ID, message ID, length, and data in a human-readable format.
     *
     * @param tag A label to distinguish where the message is being printed from.
     */
    void printMessage(const std::string &tag) const;

    size_t get_base_length() const;

    size_t get_total_length() const; ///< Returns the total length of the serialized message (header + group + msg + length + data).

    size_t get_data_length() const; ///< Returns the length of the data portion of the message.
};

#endif // OCTOPUS_IPC_PTL_HANDLER_HPP
