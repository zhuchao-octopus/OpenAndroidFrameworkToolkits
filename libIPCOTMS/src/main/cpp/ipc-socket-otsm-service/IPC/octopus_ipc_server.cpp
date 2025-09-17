/*////////////////////////////////////////////////////////////////////////////////////////////////////
 * File: octopus_ipc_server.cpp
 * Description:
 *  This file implements a simple multi-threaded IPC server that listens for client connections
 *  over a Unix domain socket. It supports basic arithmetic operations such as addition, subtraction,
 *  multiplication, and division. Each client connection is handled in a separate thread to allow
 *  simultaneous connections. The server also ensures the socket directory exists and removes old
 *  socket files before starting up.
 *
 * Includes:
 *  - A socket server class that manages the opening, binding, listening, and communication with clients.
 *  - Mutexes to ensure thread safety when modifying shared resources like active client connections.
 *  - Signal handling for graceful cleanup upon interrupt.
 *
 * Compilation:
 *  This file requires the following libraries:
 *   - <mutex>        : For thread synchronization using mutexes.
 *   - <atomic>       : For atomic operations (unused in this example, can be added later).
 *   - <unordered_set>: For storing active client connections in a set.
 *   - <sys/stat.h>   : For checking and creating directories.
 *   - <unistd.h>     : For removing old socket files.
 *   - <iostream>     : For logging messages to the console.
 *   - <vector>       : For storing query and response data.
 *
 * Author		: [ak47]
 * Organization	: [octopus]
 * Date Time	: [2025/0313/21:00]
 */
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include <mutex>
#include <atomic>
#include <unordered_set>
#include <algorithm>
#include <dlfcn.h>

#include "octopus_logger.hpp"
#include "octopus_ipc_socket.hpp"
#include "octopus_ipc_ptl.hpp"

#include "../OTSM/octopus_vehicle.h"
#include "../OTSM/octopus_task_manager.h"
#include "../OTSM/octopus_flash.h"
#include "../OTSM/octopus_update_mcu.h"
#include "../OTSM/octopus_uart_ptl.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*T_otsm_MessageDataCallbackFunc)(uint16_t param1, uint16_t param2, const uint8_t *data, uint16_t length);
typedef void (*T_otsm_SendMessageFunc)(uint16_t task_module, uint16_t id, uint16_t param1, uint16_t param2);
typedef void (*T_otsm_RegisterMsgCallbackFunc)(T_otsm_MessageDataCallbackFunc callback);
typedef void (*T_otsm_StopRunningFunc)();
typedef void (*T_otsm_update_push_interval_msFunc)(uint16_t delay_ms);

typedef carinfo_meter_t *(*T_otsm_get_meter_info)();
typedef carinfo_indicator_t *(*T_otsm_get_indicator_info)();
typedef carinfo_battery_t *(*T_otsm_get_battery_info)();
typedef carinfo_error_t *(*T_otsm_get_error_info)();
// typedef carinfo_drivinfo_t *(*T_otsm_get_drivinfo_info)();

typedef flash_meta_infor_t *(*T_otsm_flash_get_meta_infor)();
typedef mcu_update_progress_t (*T_otsm_get_mcu_upgrade_progress_info)();

T_otsm_RegisterMsgCallbackFunc otsm_RegisterMessageCallback = NULL;
T_otsm_StopRunningFunc otsm_StopRunning = NULL;
T_otsm_update_push_interval_msFunc otsm_update_push_interval_ms = NULL;

T_otsm_get_meter_info otsm_get_meter_info = NULL;
T_otsm_get_indicator_info otsm_get_indicator_info = NULL;
T_otsm_get_battery_info otsm_get_battery_info = NULL;
T_otsm_get_error_info otsm_get_error_info = NULL;
// T_otsm_get_drivinfo_info otsm_get_drivinfo_info = NULL;

T_otsm_SendMessageFunc otsm_SendMessage = NULL;
T_otsm_flash_get_meta_infor otsm_get_mcu_flash_meta_infor = NULL;
T_otsm_get_mcu_upgrade_progress_info otsm_get_mcu_upgrade_progress_info = NULL;
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to handle client communication
template <typename T>
void ipc_server_send_message_to_client(int client_fd, int msg_grp, int msg_id, T *t_info, size_t size, const std::string &info_type);
void ipc_server_message_data_callback(uint16_t msg_grp, uint16_t msg_id, const uint8_t *data, uint16_t length);
void ipc_server_notify_car_infor_to_client(int client_fd, int msg_grp, int msg_id, const uint8_t *data, uint16_t length);
void ipc_server_notify_mcu_infor_to_client(int client_fd, int msg_grp, int msg_id, const uint8_t *data, uint16_t length);
void ipc_server_handle_client_event(int client_fd);

int ipc_server_handle_calculation_event(int client_fd, const DataMessage &query_msg);
int ipc_server_handle_help_event(int client_fd, const DataMessage &query_msg);
int ipc_server_handle_config_event(int client_fd, const DataMessage &query_msg);
int ipc_server_handle_car_event(int client_fd, const DataMessage &query_msg);
int ipc_server_handle_mcu_event(int client_fd, const DataMessage &query_msg);

// Path for the IPC socket file
const char *socket_path = "/tmp/octopus/ipc_socket";
// Server object to handle socket operations
Socket server;
// Mutex for server operations to ensure thread-safety
std::mutex server_mutex;
// Mutex for client operations to ensure thread-safety
std::mutex clients_mutex;
int socket_fd_server = -1;
// Thread-safe unordered set for active clients
std::unordered_set<ClientInfo> active_clients;

bool ipc_server_socket_debug_print_data = false;
// Initialize the global thread pool object
// OctopusThreadPool g_threadPool(4, 100, TaskOverflowStrategy::DropOldest);
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// **Thread-Safe Functions**
void ipc_server_add_client(int fd, const std::string &ip, bool flag)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    active_clients.insert(ClientInfo(fd, ip, flag));
}

void ipc_server_remove_client(int fd)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto it = active_clients.begin(); it != active_clients.end(); ++it)
    {
        if (it->fd == fd)
        {
            active_clients.erase(it);
            break;
        }
    }
}

void ipc_server_print_active_clients()
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    const int fd_width = 8;
    const int ip_width = 16;
    const int flag_width = 10;

    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << "| " << std::left << std::setw(fd_width) << "fd"
              << "| " << std::setw(ip_width) << "ip"
              << "| " << std::setw(flag_width) << "flag" << "|" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    for (const auto &client : active_clients)
    {
        std::cout << "| " << std::left << std::setw(fd_width) << client.fd
                  << "| " << std::setw(ip_width) << client.ip
                  << "| " << std::setw(flag_width) << client.flag << "|" << std::endl;
    }

    std::cout << "--------------------------------------------------" << std::endl;
}

void ipc_server_update_client(int fd, bool new_flag)
{
    std::lock_guard<std::mutex> lock(clients_mutex); // 线程安全

    // 在集合中查找匹配的 `fd`
    auto it = std::find_if(active_clients.begin(), active_clients.end(),
                           [fd](const ClientInfo &client)
                           { return client.fd == fd; });

    if (it != active_clients.end())
    {
        // 先取出原始值，修改后重新插入
        ClientInfo updated_client = *it;
        updated_client.flag = new_flag;

        active_clients.erase(it);                         // 删除旧的
        active_clients.insert(std::move(updated_client)); // 插入更新后的
    }
    else
    {
        std::cerr << "Client FD not found: " << fd << std::endl;
    }
}
void ipc_server_update_client(int fd, const std::string &ip)
{
    std::lock_guard<std::mutex> lock(clients_mutex); // 线程安全

    // 在集合中查找匹配的 `fd`
    auto it = std::find_if(active_clients.begin(), active_clients.end(),
                           [fd](const ClientInfo &client)
                           { return client.fd == fd; });

    if (it != active_clients.end())
    {
        // 先取出原始值，修改后重新插入
        ClientInfo updated_client = *it;
        updated_client.ip = ip;

        active_clients.erase(it);                         // 删除旧的
        active_clients.insert(std::move(updated_client)); // 插入更新后的
    }
    else
    {
        std::cerr << "Client FD not found: " << fd << std::endl;
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to ensure the directory for the socket file exists
bool ipc_server_ensure_directory_exists(const char *path)
{
    if (!path || strlen(path) == 0)
    {
        std::cerr << "[EnsureDir] Invalid path." << std::endl;
        return false;
    }

    std::string dir = path;
    size_t pos = dir.find_last_of('/');
    if (pos == std::string::npos)
    {
        std::cerr << "[EnsureDir] Path has no directory component." << std::endl;
        return false;
    }

    std::string parent_dir = dir.substr(0, pos);
    struct stat st;

    if (stat(parent_dir.c_str(), &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
            return true; // Directory already exists
        else
        {
            std::cerr << "[EnsureDir] Path exists but is not a directory." << std::endl;
            return false;
        }
    }

    // Try to create the directory (non-recursive)
    if (mkdir(parent_dir.c_str(), 0777) == -1)
    {
        std::cerr << "[EnsureDir] Failed to create directory '" << parent_dir
                  << "': " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

// Function to remove the old socket file if it exists
void ipc_server_remove_old_socket_bind_file()
{
    unlink(socket_path);
}

void ipc_server_message_data_callback(uint16_t msg_grp, uint16_t msg_id, const uint8_t *data, uint16_t length)
{
    // std::cout << "Server handling otsm message cmd_parameter=" << cmd_parameter << std::endl;
    if (active_clients.empty())
    {
        std::cout << "[INFO] No clients to notify for cmd_parameter = " << msg_id << std::endl;
        return;
    }

    for (const auto &client : active_clients)
    {
        /// std::thread notify_thread(notify_carInfor_to_client, client_id, cmd_parameter);
        /// threads.push_back(std::move(notify_thread));
        if (client.flag) // need push callback
        {
            try
            {
                switch (msg_grp)
                {
                case MSG_GROUP_CAR:
                    ipc_server_notify_car_infor_to_client(client.fd, msg_grp, msg_id, data, length);
                    break;
                case MSG_GROUP_MCU:
                    ipc_server_notify_mcu_infor_to_client(client.fd, msg_grp, msg_id, data, length);
                    break;
                default:
                    break;
                }
            }
            catch (const std::exception &ex)
            {
                std::cerr << "[ERROR] Notify failed for client.fd=" << client.fd
                          << ": " << ex.what() << std::endl;
            }
        }
    }
}

// Signal handler for clean-up on interrupt (e.g., Ctrl+C)
void ipc_server_signal_handler(int signum)
{
    std::cout << "Server Interrupt signal received. Cleaning up...\n";
    server.close_socket(socket_fd_server);
    if (otsm_StopRunning)
        otsm_StopRunning();
    exit(signum);
}

// Function to handle communication with a specific client
/**
 * @brief Handles communication with a connected client socket.
 *
 * This function enters an infinite loop to continuously read and process queries
 * from the connected client via the server's query interface. It parses the incoming
 * messages according to the protocol, dispatches them to the appropriate handler functions,
 * and gracefully shuts down the connection upon disconnection or error.
 *
 * @param client_fd The file descriptor of the connected client socket.
 */
void ipc_server_handle_client_event(int client_fd)
{
    std::cout << "Server handling client connection [" << client_fd << "]..." << std::endl;
    int handle_result = 0;
    QueryResult query_result;
    DataMessage data_message;

    // Main processing loop: handle client queries continuously
    while (true)
    {
        // Try to fetch a query from the client using a thread-safe interface
        // query_result = server.get_query_with_epoll(client_fd, 1000);
        query_result = server.get_query(client_fd);
        // Check the status of the query attempt
        switch (query_result.status)
        {
        case QueryStatus::Timeout:
            // No data received within timeout, continue waiting
            // std::cout << "Server No data from client yet, waiting again..." << std::endl;
            continue;

        case QueryStatus::Success:
            // Data received, proceed to process it
            break;

        case QueryStatus::Disconnected:
            // Client disconnected, exit loop and clean up
            std::cout << "Server client [" << client_fd << "] disconnected." << std::endl;
            [[fallthrough]];

        case QueryStatus::Error:
        default:
            // Any other error or invalid socket state, terminate connection
            std::cerr << "Server connection for client [" << client_fd << "] closing." << std::endl;
            goto cleanup;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////
        // Call the static function deserializeMessage of the DataMessage class, passing query_result.data byte data for deserialization,
        // and assign the returned DataMessage object to the data_message variable.
        data_message = DataMessage::deserializeMessage(query_result.data);

        // Validate the packet before processing
        if (!data_message.isValid())
        {
            std::cerr << "Server Invalid packet received from client [" << client_fd << "]." << std::endl;
            data_message.printMessage("Server");
            continue;
        }

        // Dispatch to the appropriate handler based on group ID
        switch (data_message.msg_group)
        {
        case MSG_GROUP_HELP:
            handle_result = ipc_server_handle_help_event(client_fd, data_message); // Help/info request
            break;

        // case MSG_GROUP_SET:
        case MSG_GROUP_IPC_CONFIG:
            handle_result = ipc_server_handle_config_event(client_fd, data_message); // Configuration command
            break;

        case MSG_GROUP_MCU:
            handle_result = ipc_server_handle_mcu_event(client_fd, data_message);
            break;
        case 3:
        case 4:
            handle_result = ipc_server_handle_calculation_event(client_fd, data_message); // Placeholder groups
            break;

        case MSG_GROUP_CAR:
            handle_result = ipc_server_handle_car_event(client_fd, data_message); // Vehicle info commands
            break;

        default:
            // Unknown group, fallback to help
            handle_result = ipc_server_handle_help_event(client_fd, data_message);
            break;
        }

        // Log success after handling the message
        std::cout << "Server handling [Client: " << std::setw(2) << std::setfill('0') << client_fd << "] "
                  << "[Group: " << std::setw(2) << std::setfill('0') << static_cast<int>(data_message.msg_group) << "] "
                  << "[Msg: " << std::setw(2) << std::setfill('0') << static_cast<int>(data_message.msg_id) << "] done." << std::endl;
    }

cleanup:

    // Gracefully close the client socket and remove from active list
    close(client_fd);
    ipc_server_remove_client(client_fd);
    // server.cleanup_on_disconnect(client_fd);not good
    std::cout << "Server connection for client [" << client_fd << "] closed." << std::endl;
}
/// @brief /////////////////////////////////////////////////////////////////////////////////////////////////////
/// @param client_fd
/// @param query_msg
/// @return
int ipc_server_handle_help_event(int client_fd, const DataMessage &query_msg)
{
    // Print the parsed DataMessage for debugging purposes
    query_msg.printMessage("Server help"); // Print the incoming query message for visibility

    // Prepare response vector with a predefined response code
    std::vector<int> resp_vector(1);

    // Optionally print active clients to log the current client activity
    ipc_server_print_active_clients();
    //////////////////////////////////////////////////////////////////////////////////////////////
    if ((query_msg.data.empty() || query_msg.data[0] == 1))
        ipc_server_socket_debug_print_data = true;
    //////////////////////////////////////////////////////////////////////////////////////////////
    // Set the response message based on the protocol
    resp_vector[0] = MSG_GROUP_HELP; // Respond with predefined help information message

    // Lock the server mutex to safely send the response to the client
    {
        // Ensure thread safety while sending the response back to the client
        std::lock_guard<std::mutex> lock(server_mutex);
        server.send_response(client_fd, resp_vector); // Send the help info response to client
    }

    // Return success
    return 0;
}

int ipc_server_handle_config_event(int client_fd, const DataMessage &query_msg)
{
    // Extract the file descriptor from the query data or use the provided one
    int cfd = (query_msg.data.empty() || query_msg.data[0] <= 0) ? client_fd : query_msg.data[0];

    // If the message is related to IPC socket configuration, update the client status
    if (query_msg.msg_id == MSG_IPC_CMD_CONFIG_FLAG)
    {
        // Update client based on the first data value
        bool is_active = ((query_msg.data.size() >= 2) && (query_msg.data[1] > 0));
        ipc_server_update_client(cfd, is_active); // Update the client state (active/inactive)
        std::cout << "Server set client [" << cfd << "] request push:" << is_active << std::endl;
        if ((query_msg.data.size() >= 3))
            otsm_update_push_interval_ms(query_msg.data[2] * 10);
    }
    else if (query_msg.msg_id == MSG_IPC_CMD_CONFIG_PUSH_DELAY)
    {
        if (otsm_update_push_interval_ms)
        {
            int time_interval = query_msg.data[1] * 10; // MERGE_BYTES(query_msg.data[1], query_msg.data[2]);
            std::cout << "Server set client [" << cfd << "] time interval:" << time_interval << std::endl;
            otsm_update_push_interval_ms(time_interval);
        }
    }
    else if (query_msg.msg_id == MSG_IPC_CMD_CONFIG_IP)
    {
        // Update client based on the first data value
        // bool is_active = ((query_msg.data.size() >= 2) && (query_msg.data[1] > 0));
        ipc_server_update_client(cfd, std::string(query_msg.data.begin(), query_msg.data.end())); // Update the client state (active/inactive)
    }

    // Log the current active clients
    ipc_server_print_active_clients();
    std::cout << std::endl;

    // Prepare response vector with the set message group
    std::vector<int> resp_vector(1, MSG_GROUP_SET); // Set response to MSG_GROUP_SET

    // Lock the server mutex to safely send the response to the client
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        server.send_response(client_fd, resp_vector); // Send the response to the client
    }

    // Return success
    return 0;
}

int ipc_server_handle_mcu_event(int client_fd, const DataMessage &query_msg)
{
    if (query_msg.msg_id == MSG_IPC_CMD_MCU_REQUEST_UPGRADING) // This task needs to be handled by OTSM
    {
        if (otsm_SendMessage)
        {
            otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_MCU_EVENT, MSG_OTSM_CMD_MCU_REQUEST_UPGRADING, 0);
        }
    }
    else if (query_msg.msg_id == MSG_IPC_CMD_MCU_VERSION)
    {
        ipc_server_notify_mcu_infor_to_client(client_fd, query_msg.msg_group, query_msg.msg_id, NULL, 0);
    }
    return 0;
}

// Function to handle calculation logic
int ipc_server_handle_calculation_event(int client_fd, const DataMessage &query_msg)
{
    int calc_result = 0;
    std::vector<int> resp_vector(1); // Initialize response vector with one element

    // Ensure the data is large enough to hold the operands
    if (query_msg.data.size() < 3)
    {
        std::cerr << "Server Error: Insufficient operands for calculation!" << std::endl;
        resp_vector[0] = -1; // Indicate error with a negative result
    }
    else
    {
        // Switch operation based on the first data element (operation type)
        switch (query_msg.data[0])
        {
        case 1: // Addition
            calc_result = query_msg.data[1] + query_msg.data[2];
            break;
        case 2: // Subtraction
            calc_result = query_msg.data[1] - query_msg.data[2];
            break;
        case 3: // Multiplication
            calc_result = query_msg.data[1] * query_msg.data[2];
            break;
        case 4: // Division
            if (query_msg.data[2] == 0)
            {
                std::cerr << "Server Error: Division by zero!" << std::endl;
                calc_result = 0; // Set result to 0 in case of division by zero
            }
            else
            {
                calc_result = query_msg.data[1] / query_msg.data[2];
            }
            break;
        default:
            std::cerr << "Server Error: Invalid operation requested!" << std::endl;
            calc_result = -1; // Indicate error with a negative result
            break;
        }

        // Set the calculated result in the response vector
        resp_vector[0] = calc_result;
    }

    // Lock the server mutex to safely send the response to the client
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        server.send_response(client_fd, resp_vector);
    }

    return calc_result;
}

int ipc_server_handle_car_event(int client_fd, const DataMessage &data_message)
{
    switch (data_message.msg_id)
    {
    case MSG_IPC_CMD_CAR_SETTING_SAVE:
        if (otsm_SendMessage)
        {
            if (data_message.data.size() >= 1)
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, MSG_IPC_CMD_CAR_SETTING_SAVE, data_message.data[0]);
            else
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, MSG_IPC_CMD_CAR_SETTING_SAVE, 0);
        }
        break;

    case MSG_IPC_CMD_CAR_SET_LIGHT:
        if (otsm_SendMessage)
        {
            if (data_message.data.size() >= 1)
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, MSG_IPC_CMD_CAR_SET_LIGHT, data_message.data[0]);
            else
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, MSG_IPC_CMD_CAR_SET_LIGHT, 0);
        }
        break;

    case MSG_IPC_CMD_CAR_SET_GEAR_LEVEL:
        if (otsm_SendMessage)
        {
            if (data_message.data.size() >= 1)
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, MSG_IPC_CMD_CAR_SET_GEAR_LEVEL, data_message.data[0]);
            else
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, MSG_IPC_CMD_CAR_SET_GEAR_LEVEL, 0);
        }
        break;

    case MSG_IPC_CMD_CAR_METER_TRIP_DISTANCE_CLEAR:
    case MSG_IPC_CMD_CAR_METER_TIME_CLEAR:
    case MSG_IPC_CMD_CAR_METER_ODO_CLEAR:
        if (otsm_SendMessage)
        {
            if (data_message.data.size() >= 1)
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, data_message.msg_id, data_message.data[0]);
            else
                otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, data_message.msg_id, 0);
        }
        break;

    case MSG_IPC_CMD_CAR_SET_INDICATOR:
        if (data_message.data.size() >= sizeof(carinfo_indicator_t))
        {
            carinfo_indicator_t *carinfo_indicator = otsm_get_indicator_info();
            std::memcpy(carinfo_indicator, data_message.data.data(), sizeof(carinfo_indicator_t));
            // otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, data_message.msg_id, 0);
            otsm_SendMessage(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_IPC, FRAME_CMD_CAR_SET_INDICATOR, 0);
        }
        break;

    case MSG_IPC_CMD_CAR_SET_METER:
        if (data_message.data.size() >= sizeof(carinfo_meter_t))
        {
            carinfo_meter_t *carinfo_meter = otsm_get_meter_info();
            // memcpy(carinfo_meter, &data_message.data, sizeof(carinfo_meter_t));
            std::memcpy(carinfo_meter, data_message.data.data(), sizeof(carinfo_meter_t));
            otsm_SendMessage(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_IPC, FRAME_CMD_CAR_SET_METER, 0);
        }
        break;

    case MSG_IPC_CMD_CAR_SET_BATTERY:
        if (data_message.data.size() >= sizeof(carinfo_battery_t))
        {
            carinfo_battery_t *carinfo_battery = otsm_get_battery_info();
            // memcpy(carinfo_battery, &data_message.data, sizeof(carinfo_battery_t));
            std::memcpy(carinfo_battery, data_message.data.data(), sizeof(carinfo_battery_t));
            //  otsm_SendMessage(TASK_MODULE_IPC, MSG_OTSM_DEVICE_CAR_EVENT, data_message.msg_id, 0);
            otsm_SendMessage(TASK_MODULE_PTL_1, SOC_TO_MCU_MOD_IPC, FRAME_CMD_CAR_SET_BATTERY, 0);
        }
        break;

    default:
        ipc_server_notify_car_infor_to_client(client_fd, MSG_GROUP_CAR, data_message.msg_id, NULL, 0);
    }

    return 0;
}

// Main function to notify car info to the client
void ipc_server_notify_car_infor_to_client(int client_fd, int msg_grp, int msg_id, const uint8_t *data, uint16_t length)
{
    switch (msg_id)
    {
    case MSG_IPC_CMD_CAR_GET_INDICATOR_INFO:
    {
        if (otsm_get_indicator_info)
        {
            carinfo_indicator_t *carinfo_indicator = otsm_get_indicator_info();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, carinfo_indicator, sizeof(carinfo_indicator_t), "handle_car_infor (Indicator)");
        }
        else
        {
            std::cout << "Server Error: otsm_get_indicator_info nullptr!" << std::endl;
        }
        break;
    }
    case MSG_IPC_CMD_CAR_GET_METER_INFO:
    {
        if (otsm_get_meter_info)
        {
            carinfo_meter_t *carinfo_meter = otsm_get_meter_info();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, carinfo_meter, sizeof(carinfo_meter_t), "handle_car_infor (Meter)");
        }
        else
        {
            std::cout << "Server Error: otsm_get_meter_info nullptr!" << std::endl;
        }
        break;
    }
    case MSG_IPC_CMD_CAR_GET_BATTERY_INFO:
    {
        if (otsm_get_battery_info)
        {
            carinfo_battery_t *carinfo_battery = otsm_get_battery_info();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, carinfo_battery, sizeof(carinfo_battery_t), "handle_car_infor (battery)");
        }
        else
        {
            std::cout << "Server Error: otsm_get_battery_info nullptr!" << std::endl;
        }
        break;
    }
    case MSG_IPC_CMD_CAR_GET_ERROR_INFO:
    {
        if (otsm_get_error_info)
        {
            carinfo_error_t *carinfo_error = otsm_get_error_info();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, carinfo_error, sizeof(carinfo_error_t), "handle_car_infor (error)");
        }
        else
        {
            std::cout << "Server Error: otsm_get_error_info nullptr!" << std::endl;
        }
        break;
    }

#if 0
    case MSG_IPC_CMD_CAR_GET_DRIVINFO_INFO:
    {
        if (otsm_get_drivinfo_info)
        {
            carinfo_drivinfo_t *carinfo_drivinfo = otsm_get_drivinfo_info();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, carinfo_drivinfo, sizeof(carinfo_drivinfo_t), "handle_car_infor (Driver)");
        }
        else
        {
            std::cout << "Server Error: otsm_get_drivinfo_info nullptr!" << std::endl;
        }
        break;
    }
#endif

    case MSG_OTSM_CMD_MCU_UPDATING:
    {
        break;
    }
    default:
        // If the command is invalid, call handle_help to notify the client
        // handle_help(client_fd, {0});
        break;
    }
}

void ipc_server_notify_mcu_infor_to_client(int client_fd, int msg_grp, int msg_id, const uint8_t *data, uint16_t length)
{
    switch (msg_id)
    {
    case MSG_IPC_CMD_MCU_UPDATING:
        if (otsm_get_mcu_upgrade_progress_info)
        {
            mcu_update_progress_t mcu_update_progress = otsm_get_mcu_upgrade_progress_info();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, &mcu_update_progress, sizeof(mcu_update_progress_t), "handle_mcu_infor (Updating)");
        }
        else
        {
            LOG_INFO("otsm_get_mcu_upgrade_progress_info function is null!");
        }
        break;

    case MSG_IPC_CMD_MCU_VERSION:
        if (otsm_get_mcu_flash_meta_infor)
        {
            flash_meta_infor_t *flash_meta_infor = otsm_get_mcu_flash_meta_infor();
            ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, flash_meta_infor, sizeof(flash_meta_infor_t), "handle_mcu_flash_mata_infor (Meta)");
        }

        break;
    case MSG_IPC_CMD_KEY_EVENT:
    case MSG_IPC_CMD_KEY_DOWN_EVENT:
    case MSG_IPC_CMD_KEY_UP_EVENT:
        ipc_server_send_message_to_client(client_fd, msg_grp, msg_id, data, length, "handle_mcu_key_infor (Key)");
        break;

    default:
        break;
    }
}

// Helper function to handle the car info response logic
template <typename T>
void ipc_server_send_message_to_client(int client_fd, int msg_grp, int msg_id, T *t_info, size_t size, const std::string &info_type)
{
    if (t_info == nullptr)
    {
        std::cout << "Server Error: " << info_type << " returned nullptr!" << std::endl;
        return;
    }

    // Create a DataMessage to follow the protocol format
    DataMessage data_msg;
    data_msg.msg_group = msg_grp; // Set appropriate group based on the info type
    data_msg.msg_id = msg_id;     // Set message ID based on info type

    // Directly copy the car info data into the msg.data vector
    data_msg.data.clear(); // Clear any existing data
    data_msg.data.insert(data_msg.data.end(), reinterpret_cast<const uint8_t *>(t_info), reinterpret_cast<const uint8_t *>(t_info) + size);
    data_msg.msg_length = data_msg.data.size();

    // Serialize the DataMessage into the protocol format
    std::vector<uint8_t> serialized_data = data_msg.serializeMessage();
    size_t data_size = serialized_data.size();
    uint8_t *buffer = reinterpret_cast<uint8_t *>(serialized_data.data());

    if (ipc_server_socket_debug_print_data)
    {
        // Print the buffer contents for debugging
        std::cout << "Server handling client [" << client_fd << "] " << info_type << " " << data_size << " bytes: ";
        server.printf_buffer_bytes(buffer, data_size);
    }
    // Lock the server mutex to safely send the response to the client
    {
        std::lock_guard<std::mutex> lock(server_mutex);
        server.send_buff(client_fd, buffer, data_size);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ipc_server_initialize_otsm()
{
    // 加载共享库
    std::cout << "Server initialize otsm started." << std::endl;
    void *handle = dlopen("libOTSM.so", RTLD_LAZY);
    if (!handle)
    {
        std::cerr << "Server Failed to load otsm library: " << dlerror() << std::endl;
        return;
    }
    /// std::cout << "Server sucecess to load otsm library: " << dlerror() << std::endl;
    /// 你可以在这里调用 OTSM 库中的函数，假设它有一个初始化函数 `initialize`。
    /// 例如，假设 OTSM 库有一个 C 风格的 `initialize` 函数
    otsm_RegisterMessageCallback = (T_otsm_RegisterMsgCallbackFunc)dlsym(handle, "register_message_data_callback");
    if (!otsm_RegisterMessageCallback)
    {
        std::cerr << "Server Failed to find otsm_RegisterMessageCallback: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_StopRunning = (T_otsm_StopRunningFunc)dlsym(handle, "TaskManagerStateStopRunning");
    if (!otsm_StopRunning)
    {
        std::cerr << "Server Failed to find otsm_StopRunning function: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_update_push_interval_ms = (T_otsm_update_push_interval_msFunc)dlsym(handle, "update_push_interval_ms");
    if (!otsm_update_push_interval_ms)
    {
        std::cerr << "Server Failed to find otsm_update_push_interval_ms: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_get_meter_info = (carinfo_meter_t * (*)()) dlsym(handle, "task_carinfo_get_meter_info");
    if (!otsm_get_meter_info)
    {
        std::cerr << "Server Failed to find otsm_get_meter_info: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_get_indicator_info = (carinfo_indicator_t * (*)()) dlsym(handle, "task_carinfo_get_indicator_info");
    if (!otsm_get_indicator_info)
    {
        std::cerr << "Server Failed to find otsm_get_indicator_info: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_get_battery_info = (carinfo_battery_t * (*)()) dlsym(handle, "task_carinfo_get_battery_info");
    if (!otsm_get_battery_info)
    {
        std::cerr << "Server Failed to find otsm_get_battery_info: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_get_error_info = (carinfo_error_t * (*)()) dlsym(handle, "task_carinfo_get_error_info");
    if (!otsm_get_error_info)
    {
        std::cerr << "Server Failed to find otsm_get_error_info: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

#if 0
    otsm_get_drivinfo_info = (carinfo_drivinfo_t * (*)()) dlsym(handle, "app_carinfo_get_drivinfo_info");
    otsm_get_drivinfo_info = (T_otsm_get_drivinfo_info)dlsym(handle, "task_carinfo_get_drivinfo_info");
    if (!otsm_get_drivinfo_info)
    {
        std::cerr << "Server Failed to find otsm_get_drivinfo_info: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }
#endif

    otsm_SendMessage = (T_otsm_SendMessageFunc)dlsym(handle, "send_message_adapter");
    if (!otsm_SendMessage)
    {
        std::cerr << "Server Failed to find otsm_SendMessage: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_get_mcu_upgrade_progress_info = (T_otsm_get_mcu_upgrade_progress_info)dlsym(handle, "get_mcu_update_progress");
    if (!otsm_get_mcu_upgrade_progress_info)
    {
        std::cerr << "Server Failed to find otsm_get_mcu_upgrade_progress_info: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_get_mcu_flash_meta_infor = (T_otsm_flash_get_meta_infor)dlsym(handle, "flash_get_meta_infor");
    if (!otsm_get_mcu_flash_meta_infor)
    {
        std::cerr << "Server Failed to find otsm_get_mcu_flash_meta_infor: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    otsm_RegisterMessageCallback(ipc_server_message_data_callback);
    /// 调用库中的初始化函数
    /// initialize_func();
}

void ipc_server_initialize_server()
{
    std::cout << "[Server] Initialization started." << std::endl;

    // Ignore SIGPIPE to prevent crash when writing to a closed socket
    signal(SIGPIPE, SIG_IGN);

    // Handle Ctrl+C to allow graceful shutdown
    signal(SIGINT, ipc_server_signal_handler);

    // Ensure socket directory exists
    if (!ipc_server_ensure_directory_exists(socket_path))
    {
        std::cerr << "[Server] Failed to ensure socket directory exists." << std::endl;
        return;
    }

    // Clean up old socket file if it exists
    ipc_server_remove_old_socket_bind_file();

    // Create the server socket
    socket_fd_server = server.open_socket();
    if (socket_fd_server <= 0)
    {
        std::cerr << "[Server] Failed to create server socket." << std::endl;
        return;
    }

    // Bind the socket
    if (!server.bind_server_to_socket(socket_fd_server))
    {
        std::cerr << "[Server] Failed to bind server socket." << std::endl;
        return;
    }

    // Begin listening for incoming connections
    if (!server.start_listening(socket_fd_server))
    {
        std::cerr << "[Server] Failed to start listening." << std::endl;
        return;
    }

    // Optional: set buffer sizes
    // server.query_buffer_size = 20;
    // server.respo_buffer_size = 20;

    std::cout << "Server Waiting for client connections..." << std::endl;
}

int main()
{
    LOG_CC("\r\n#######################################################################################\r\n");
    LOG_CC("Octopus IPC Socket Server Started Successfully.");
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ipc_server_initialize_otsm();
    /// std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait before reconnecting
    ipc_server_initialize_server();
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Main loop to accept and handle client connections
    while (true)
    {
        int client_fd = server.wait_and_accept(socket_fd_server);

        // If accepting a client fails, continue to the next iteration
        if (client_fd < 0)
        {
            std::cerr << "Server Failed to accept client connection" << std::endl;
            continue;
            // break; for test
        }

        // Lock the clients mutex to safely modify the active clients set
        {
            /// std::lock_guard<std::mutex> lock(clients_mutex);
            /// active_clients.insert(client_fd);
            ipc_server_add_client(client_fd, "", true);
        }

        // Create a thread to handle the client communication
        std::thread client_thread(ipc_server_handle_client_event, client_fd);

        // Detach the thread so it runs independently
        client_thread.detach();
    }

    // Close the server socket before exiting
    server.close_socket(socket_fd_server);
    if (otsm_StopRunning)
        otsm_StopRunning();

    return 0;
}
