/*
 * File: octopus_ipc_socket.cpp
 * Organization: Octopus
 * Description:
 *  This file contains the implementation of the `Socket` class, which handles the creation, binding,
 *  listening, and communication over a Unix Domain Socket (UDS). The class supports both server-side
 *  and client-side socket operations such as opening a socket, sending queries and responses, and
 *  handling client connections.
 *
 *  - Server-side operations include socket creation, binding, listening, accepting client connections,
 *    reading client queries, and sending responses.
 *  - Client-side operations include connecting to the server, sending queries, and receiving responses.
 *
 * Author		: [ak47]
 * Organization	: [octopus]
 * Date Time	: [2025/0313/21:00]
 */

#include "octopus_ipc_socket.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_EVENTS 10 // Maximum number of events
///////////////////////////////////////////////////////////////////////////////////////////////////////
// Signal handler for server shutdown on interruption signals
static void signal_handler(int signum)
{
    std::cout << "Server Octopus IPC Socket Received Signal: " << signum << std::endl;
    std::exit(-1);
}

// Constructor for the Socket class
Socket::Socket()
{
    // Setup signal handling for interrupts
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    signal(SIGTERM, signal_handler);
    // socket_fd = -1;
    // Server configuration
    max_waiting_requests = 10; // Max number of clients waiting to be served
    init_socket_structor();
}

void Socket::init_socket_structor()
{
    // Initialize socket properties
    socket_path = "/tmp/octopus/ipc_socket";
    domain = AF_UNIX;   // Domain type: Unix domain socket
    type = SOCK_STREAM; // Type: Stream socket (TCP-like behavior)
    protocol = 0;       // Protocol: Default
    // Initialize socket address structure for binding
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;                                      // Set address family to Unix domain
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1); // Set socket path
}

void Socket::init_epoll(int socket_fd)
{
    std::lock_guard<std::mutex> lock(epoll_mutex);

    if (!epoll_initialized)
    {
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1)
        {
            std::cerr << "Socket: init_epoll epoll_create1 failed\n";
            return;
        }

        epoll_event ev;
        ev.events = EPOLLIN | EPOLLET; // Edge-triggered, read ready
        ev.data.fd = socket_fd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1)
        {
            std::cerr << "Socket: init_epoll epoll_ctl ADD failed\n";
            close(epoll_fd);
            epoll_fd = -1;
            return;
        }

        epoll_initialized = true;
    }
}
void Socket::init_epoll()
{
    std::lock_guard<std::mutex> lock(epoll_mutex);

    // If epoll has already been initialized, no need to recreate it
    if (epoll_initialized)
        return;

    // Create an epoll instance using epoll_create1
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        std::cout << "Socket: Failed to create epoll instance (epoll_create1)\n";
        return;
    }

    // Mark the epoll system as initialized
    epoll_initialized = true;
    std::cout << "Socket: Epoll instance created successfully (epoll_fd = " << epoll_fd << ")\n";
}

bool Socket::register_socket_fd(int socket_fd)
{
    std::lock_guard<std::mutex> lock(epoll_mutex);

    // Ensure epoll has been initialized before trying to register any sockets
    if (!epoll_initialized)
    {
        std::cout << "Socket: Cannot register socket fd, epoll is not initialized.\n";
        return false;
    }

    // Configure the event: EPOLLIN means "ready to read",
    // EPOLLET means "edge-triggered" mode (requires non-blocking IO)
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = socket_fd;

    // Register the given socket_fd to the epoll instance
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1)
    {
        std::cout << "Socket: Failed to add socket fd to epoll (fd = " << socket_fd << ")\n";
        return false;
    }

    std::cout << "Socket: Socket fd registered to epoll successfully (fd = " << socket_fd << ")\n";
    return true;
}

// Open a socket for communication
int Socket::open_socket()
{
    int socket_fd = socket(domain, type, protocol); // Create the socket

    // Error handling if socket creation fails
    if (socket_fd == -1)
    {
        int err = errno;
        std::cerr << "Socket: Open socket failed to create socket.\n"
                  << "Error: " << strerror(err) << " (errno: " << err << ")\n"
                  << "Domain: " << domain << ", Type: " << type << ", Protocol: " << protocol << std::endl;
        close_socket(socket_fd); // Ensure socket is closed if creation fails
        //  Don't call close_socket because socket_fd is invalid
    }

    return socket_fd;
}

int Socket::open_socket(int domain, int type, int protocol)
{
    int socket_fd = socket(domain, type, protocol); // Create the socket

    // Error handling if socket creation fails
    if (socket_fd == -1)
    {
        int err = errno;
        std::cout << "Socket: Open socket failed to create socket.\n"
                  << "Error: " << strerror(err) << " (errno: " << err << ")\n"
                  << "Domain: " << domain << ", Type: " << type << ", Protocol: " << protocol << std::endl;
        // close_socket(); // Ensure socket is closed if creation fails
    }

    return socket_fd;
}

// Close the socket
int Socket::close_socket(int socket_fd)
{
    close(socket_fd); // Close the socket file descriptor
    std::cout << "Socket: Close socket [" << socket_fd << "]" << std::endl;
    return socket_fd;
}

// Bind the server to the socket address
bool Socket::bind_server_to_socket(int socket_fd)
{
    // Try binding the socket to the address
    if (bind(socket_fd, (sockaddr *)&addr, sizeof(addr)) == -1)
    {
        int err = errno;
        std::cerr << "Server Socket bind failed: " << strerror(err) << " (errno: " << err << ")" << std::endl;
        close_socket(socket_fd);
        return false;
    }

    std::cout << "Server Socket bound successfully." << std::endl;
    std::cout << "Socket FD: " << socket_fd << ", Domain: " << addr.sun_family << std::endl;
    std::cout << "Socket Path: " << addr.sun_path << std::endl;

    // Set file permissions (for Unix domain sockets)
    int permission = chmod(addr.sun_path, S_IRWXU | S_IRWXG | S_IRWXO);
    if (permission == -1)
    {
        int err = errno;
        std::cerr << "Failed to chmod socket file: " << strerror(err) << " (errno: " << err << ")" << std::endl;
        close_socket(socket_fd);
        return false;
    }

    std::cout << "Socket file permissions set successfully." << std::endl;
    return true;
}

// Start the server to listen for incoming connections
bool Socket::start_listening(int socket_fd)
{
    std::cout << "Server started to listening." << std::endl;

    int listen_result = listen(socket_fd, max_waiting_requests); // Start listening with a queue size

    // Handle errors in listening
    if (listen_result == -1)
    {
        std::cerr << "Server Listen failed." << std::endl;
        close_socket(socket_fd);
        return false;
    }
    return true;
}

// Accept an incoming client connection
int Socket::wait_and_accept(int socket_fd)
{
    // Wait for a client to connect and accept the connection
    int client_fd = accept(socket_fd, NULL, NULL);

    // Handle errors in accepting the connection
    if (client_fd == -1)
    {
        std::cerr << "Server Client connection could not be accepted." << std::endl;
    }

    std::cout << "Server Accepted client connection [" << client_fd << "]" << std::endl;
    return client_fd;
}

// Read the query from the client
QueryResult Socket::get_query(int socket_fd)
{
    char buffer[IPC_SOCKET_QUERY_BUFFER_SIZE];
    struct pollfd pfd = {socket_fd, POLLIN, 0};

    int ret = poll(&pfd, 1, 2000); // 2 秒超时

    if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))
    {
        // std::cerr << "Socket connection was closed by peer." << std::endl;
        return {QueryStatus::Disconnected, {}};
    }

    if (ret == 0)
    {
        // std::cerr << "Socket poll timeout (no events)." << std::endl;
        return {QueryStatus::Timeout, {}};
    }
    else if (ret < 0)
    {
        // std::cerr << "Socket poll error happened." << std::endl;
        return {QueryStatus::Error, {}};
    }

    int query_bytesRead = read(socket_fd, buffer, sizeof(buffer));
    if (query_bytesRead <= 0)
    {
        // std::cerr << "Socket read failed or client disconnected." << std::endl;
        return {QueryStatus::Disconnected, {}};
    }

    return {QueryStatus::Success, std::vector<uint8_t>(buffer, buffer + query_bytesRead)};
}

// Updated function to use epoll for handling client queries
QueryResult Socket::get_query_with_epoll(int socket_fd, int timeout_ms)
{
    init_epoll(socket_fd); // Lazy init for epoll_fd

    if (epoll_fd == -1)
    {
        return {QueryStatus::Error, {}};
    }

    epoll_event events[1];
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = socket_fd;

    // Add client socket to epoll
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) == -1)
    {
        return {QueryStatus::Error, {}};
    }

    // Wait for events on the socket with a timeout of 2000ms
    int n = epoll_wait(epoll_fd, events, 1, 2000);
    if (n == -1)
    {
        return {QueryStatus::Error, {}};
    }
    else if (n == 0)
    {
        return {QueryStatus::Timeout, {}};
    }

    // Check if we have a readable event
    if (events[0].events & EPOLLIN)
    {
        char buffer[IPC_SOCKET_QUERY_BUFFER_SIZE];
        int query_bytesRead = read(socket_fd, buffer, sizeof(buffer));
        if (query_bytesRead <= 0)
        {
            return {QueryStatus::Disconnected, {}};
        }

        return {QueryStatus::Success, std::vector<uint8_t>(buffer, buffer + query_bytesRead)};
    }

    return {QueryStatus::Error, {}};
}

// Send a response to the client
int Socket::send_response(int socket_fd, std::vector<int> &resp_vector)
{
    char resp_buffer[resp_vector.size()]; // Buffer for the response
    for (int i = 0; i < resp_vector.size(); i++)
    {
        resp_buffer[i] = resp_vector[i];
    }
    auto write_result = write(socket_fd, resp_buffer, sizeof(resp_buffer)); // Send the response

    // Handle errors in sending the response
    if (write_result == -1)
    {
        std::cerr << "Server Could not write response to client." << std::endl;
        close_socket(socket_fd);
    }

    return 0;
}
// Send a response to the client
int Socket::send_buff(int socket_fd, uint8_t *resp_buffer, int length)
{
    int total_sent = 0;
    while (total_sent < length)
    {
        ssize_t write_result = write(socket_fd, resp_buffer + total_sent, length - total_sent);
        // ssize_t write_result = send(client_fd, resp_buffer, length, MSG_NOSIGNAL);
        if (write_result > 0)
        {
            total_sent += write_result; // 记录已发送数据
        }
        else if (write_result == -1)
        {
            if (errno == EINTR)
            {
                continue; // 被信号中断，重试 write()
            }
            else if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::cerr << "Socket: Write operation would block, retrying..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 适当等待再试
                continue;
            }
            else if (errno == EPIPE || errno == ECONNRESET)
            {
                std::cerr << "Socket: Client disconnected, closing connection..." << std::endl;
                close_socket(socket_fd); // 关闭该客户端的 socket，防止资源泄漏
                return -1;               // 终止发送
            }
            else
            {
                std::cerr << "Socket: Failed to write response, error: " << strerror(errno) << std::endl;
                return -1; // 发生不可恢复错误
            }
        }
    }

    return 0; // 数据全部发送成功
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connect the client to the server
int Socket::connect_to_socket(int socket_fd)
{
    std::cout << "Socket: [" << socket_fd << "] attempting to connect to server socket at: " << addr.sun_path << std::endl;
    int result = connect(socket_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
    if (result == -1)
    {
        int err = errno;
        std::cerr << "Socket Connect to server failed.\n"
                  << "Error: " << strerror(err) << " (errno: " << err << ")\n"
                  << "Socket FD: " << socket_fd << "\n"
                  << "Socket Path: " << addr.sun_path << "\n";

        // Handle specific error codes
        switch (err)
        {
        case ECONNREFUSED:
            std::cerr << "Socket: Possible cause: The server socket is not running or has crashed.\n";
            break;

        case ENOENT:
            std::cerr << "Socket: Possible cause: The socket file '" << addr.sun_path << "' does not exist.\n";
            break;

        case EADDRINUSE:
            std::cerr << "Socket: Possible cause: The socket is already in use.\n";
            break;

        default:
            std::cerr << "Socket: Unknown error occurred.\n";
            break;
        }

        // Close the socket to prevent resource leak
        close_socket(socket_fd);
        return -1;
    }
    std::cout << "Socket: Successfully connected to server socket." << std::endl;
    return result;
}

int Socket::connect_to_socket(int socket_fd, std::string address)
{
    // 设置 socket_path 为新的 address
    this->socket_path = address.c_str();
    init_socket_structor();
    return this->connect_to_socket(socket_fd);
}

// Send a query from the client to the server
bool Socket::send_query(int socket_fd, const std::vector<int> &query_vector)
{
    char query_buffer[query_vector.size()]; // Buffer for the query
    for (int i = 0; i < query_vector.size(); i++)
    {
        query_buffer[i] = query_vector[i];
    }
    auto write_result = write(socket_fd, query_buffer, sizeof(query_buffer)); // Send the query

    // Handle errors in sending the query
    if (write_result == -1)
    {
        std::cerr << "Client: Could not write query to socket." << std::endl;
        close_socket(socket_fd);
        return false;
    }
    return true;
}

bool Socket::send_query(int socket_fd, const std::vector<uint8_t> &query_vector)
{
    char query_buffer[query_vector.size()]; // Buffer for the query
    for (int i = 0; i < query_vector.size(); i++)
    {
        query_buffer[i] = query_vector[i];
    }
    auto write_result = write(socket_fd, query_buffer, sizeof(query_buffer)); // Send the query

    // Handle errors in sending the query
    if (write_result == -1)
    {
        std::cerr << "Client: Could not write query to socket." << std::endl;
        close_socket(socket_fd);
        return false;
    }
    return true;
}
// Receive the response from the server
QueryResult Socket::get_response(int socket_fd)
{
    uint8_t result_buffer[IPC_SOCKET_RESPONSE_BUFFER_SIZE];

    int resp_bytesRead = read(socket_fd, result_buffer, sizeof(result_buffer));
    if (resp_bytesRead <= 0)
    {
        std::cerr << "Client: Could not read response from server." << std::endl;
        close_socket(socket_fd);
        return {QueryStatus::Disconnected, {}};
    }

    // Efficient conversion without loop
    std::vector<uint8_t> data(result_buffer, result_buffer + resp_bytesRead);
    return {QueryStatus::Success, data};
}

QueryResult Socket::get_response_with_epoll(int socket_fd, int timeout_ms)
{
    init_epoll(socket_fd); // Lazy init
    if (epoll_fd == -1)
    {
        return {QueryStatus::Error, {}};
    }

    epoll_event events[1];

    int n = epoll_wait(epoll_fd, events, 1, timeout_ms);
    if (n == -1)
    {
        return {QueryStatus::Error, {}};
    }
    else if (n == 0)
    {
        return {QueryStatus::Timeout, {}};
    }

    if (!(events[0].events & EPOLLIN))
    {
        return {QueryStatus::Error, {}};
    }

    uint8_t result_buffer[IPC_SOCKET_RESPONSE_BUFFER_SIZE];
    int bytesRead = read(socket_fd, result_buffer, sizeof(result_buffer));

    if (bytesRead <= 0)
    {
        return {QueryStatus::Disconnected, {}};
    }

    std::vector<uint8_t> data(result_buffer, result_buffer + bytesRead);
    return {QueryStatus::Success, data};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Socket::printf_vector_bytes(const std::vector<uint8_t> &vec, int length)
{
    // std::cout << "printf_vector_bytes " << length << std::endl; //
    //  确保 length 不超过 vector 的大小
    int print_length = std::min(length, static_cast<int>(vec.size()));

    for (int i = 0; i < print_length; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (vec[i] & 0xFF) << " ";
    }
    std::cout << std::dec << std::endl; // 恢复十进制格式
}
// 打印 vector 中的前 count 个字节
void Socket::printf_buffer_bytes(const std::vector<uint8_t> &vec, int length)
{
    // 确保打印的字节数不超过 vector 的大小
    int print_count = std::min(length, static_cast<int>(vec.size()));

    for (int i = 0; i < print_count; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (vec[i] & 0xFF) << " "; // 取低 8 位当作 1 字节
    }
    std::cout << std::dec << std::endl; // 恢复十进制格式
}

// 打印指定字节数的 buffer
void Socket::printf_buffer_bytes(const void *buffer, int length)
{
    const uint8_t *byte_ptr = static_cast<const uint8_t *>(buffer);

    // 确保打印的字节数不超过 buffer 的大小
    int print_count = length;

    for (int i = 0; i < print_count; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(byte_ptr[i]) << " ";
    }
    std::cout << std::dec << std::endl; // 恢复十进制格式
}
