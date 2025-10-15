
/**
 * @file client.cpp
 * @brief This file contains the client-side implementation for communicating with a server via a Unix domain socket.
 * It handles socket creation, connection, sending a query (arithmetic operation), and receiving the server's response.
 *
 * The client takes arguments specifying the operation (addition, subtraction, multiplication, or division),
 * followed by two numbers. It then sends this data to the server and prints the result.
 *
 * Author		: [ak47]
 * Organization	: [octopus]
 * Date Time	: [2025/0313/21:00]
 */
#include <iostream>
#include <vector>
#include <signal.h>
#include <iomanip>
#include <map>
#include <algorithm>
#include "octopus_ipc_ptl.hpp"
#include "octopus_ipc_socket.hpp"

std::unordered_map<std::string, int> operations = {
    {"help", MSG_GROUP_0},
    {"set", MSG_GROUP_SET},
    {"subtract", 2},
    {"multiply", 3},
    {"divide", 4},
    {"car", MSG_GROUP_CAR}};

// Signal handler for clean-up on interrupt (e.g., Ctrl+C)
void signal_handler(int signum)
{
    std::cout << "Client: Interrupt signal received. Cleaning up...\n";
    exit(signum);
}

// 负责信号处理的函数
void setup_signal_handlers()
{
    signal(SIGINT, signal_handler);
}

DataMessage parse_arguments(int argc, char *argv[], std::vector<std::string> &original_args)
{
    DataMessage data_msg;
    data_msg.msg_group = 0; // Default group, can be modified or specified from parameters
    data_msg.msg_id = 0;
    data_msg.msg_length = 0;
    // Save the original argument strings
    for (int i = 0; i < argc; ++i)
    {
        original_args.emplace_back(argv[i]);
    }

    // Ensure there's at least one command (excluding the program name)
    if (argc < 3)
    {
        std::cerr << "Client: Error. No operation command or message ID provided!" << std::endl;
        return data_msg; // Return with default values
    }

    // Find the operation code (group)
    auto key_iter = operations.find(argv[1]);
    data_msg.msg_group = (key_iter != operations.end()) ? key_iter->second : 0;

    // Ensure the second argument is a valid message ID (argv[2])
    try
    {
        // Parse the message ID from argv[2]
        data_msg.msg_id = static_cast<uint8_t>(std::stoul(argv[2])); // Expecting message ID in the range of uint8_t (0-255)
    }
    catch (const std::exception &e)
    {
        std::cerr << "Client: Error. Invalid message ID '" << argv[2] << "'. Using 0 instead.\n";
        data_msg.msg_id = 0; // Default to 0 if invalid
    }

    // Parse the remaining arguments as uint8_t and add them to data
    for (int i = 3; i < argc; ++i)
    {
        try
        {
            int val = std::stoi(argv[i]);
            // Check the range of the argument and clamp it to 0-255
            if (val < 0 || val > 255)
            {
                std::cerr << "Client: Warning Argument '" << argv[i] << "' out of range (0-255). Clamped to fit.\n";
            }
            uint8_t uval = static_cast<uint8_t>(std::clamp(val, 0, 255));
            data_msg.data.push_back(uval);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Client: Warning. Cannot convert '" << argv[i] << "' to integer. Using 0 instead.\n";
            data_msg.data.push_back(0); // Add 0 if conversion fails
        }
    }
    data_msg.msg_length = static_cast<uint8_t>(data_msg.data.size());
    return data_msg;
}

// 负责处理和发送消息的功能
bool send_message(Socket &client, int client_fd, const DataMessage &data_message)
{
    std::vector<uint8_t> serialized_data = data_message.serializeMessage();
    if (!client.send_query(client_fd, serialized_data))
    {
        std::cerr << "Client: Failed to send query to server!" << std::endl;
        return false;
    }
    return true;
}

// 负责连接和获取响应的功能
bool connect_and_receive(Socket &client, int socket_fd)
{
    QueryResult queryResult = client.get_response(socket_fd);
    if (queryResult.data.empty())
    {
        std::cerr << "Client: Received empty response from server!" << std::endl;
        return false;
    }
    std::cout << "Client: Received response: ";
    client.printf_vector_bytes(queryResult.data, queryResult.data.size());
    return true;
}

int main(int argc, char *argv[])
{
    // Set up signal handler for SIGINT (Ctrl+C)
    setup_signal_handlers();

    // Parse command line arguments
    std::vector<std::string> original_arguments;
    DataMessage data_message = parse_arguments(argc, argv, original_arguments);
    data_message.printMessage("Client main");

    // Validate parsed message
    if (!data_message.isValid())
    {
        std::cerr << "Client: Invalid message package!" << std::endl;
        return 1; // Return non-zero code to indicate failure
    }

    // Create client socket
    Socket client;
    int socket_fd = client.open_socket();
    if (socket_fd < 0)
    {
        std::cerr << "Client: Failed to open socket!" << std::endl;
        return 1; // Return non-zero code to indicate failure
    }

    // Attempt to connect to the server
    int result = client.connect_to_socket(socket_fd);
    if (result < 0)
    {
        std::cerr << "Client: Failed to connect to server!" << std::endl;
        client.close_socket(socket_fd); // Ensure socket is closed before exiting
        return 1;
    }
    std::cout << "Client: Opened socket [" << socket_fd << "] connected to server" << std::endl;

    // Send the serialized data to the server
    if (!send_message(client, socket_fd, data_message))
    {
        client.close_socket(socket_fd); // Ensure socket is closed on error
        return 1;
    }

    // Receive and print the response
    if (!connect_and_receive(client, socket_fd))
    {
        client.close_socket(socket_fd); // Ensure socket is closed on error
        return 1;
    }

    // Clean up and close the socket
    client.close_socket(socket_fd);
    return 0;
}
