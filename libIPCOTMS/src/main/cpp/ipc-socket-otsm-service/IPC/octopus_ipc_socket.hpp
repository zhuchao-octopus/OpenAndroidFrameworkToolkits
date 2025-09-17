/*
 * File: octopus_ipc_socket.hpp
 * Organization: Octopus
 * Description:
 *  This header file defines the `Socket` class, which encapsulates the functionalities required to
 *  manage communication through a Unix Domain Socket (UDS). It includes methods for setting up a
 *  server to handle incoming client requests, sending responses, and performing arithmetic operations.
 *  The class also supports client-side socket communication by providing methods for connecting to the
 *  server, sending queries, and receiving responses.
 *
 * Includes:
 *  - Socket setup and connection management through Unix domain sockets.
 *  - Query and response handling for arithmetic operations.
 *  - Server-side socket functions including binding, listening, and accepting client connections.
 *  - Client-side socket functions including query sending and response receiving.
 *
 * Author		: [ak47]
 * Organization	: [Octopus]
 * Date Time	: [2025/0313/21:00]
 */

/////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <mutex>
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
#include <iomanip>
#include <sys/epoll.h>
#include <unordered_map>
#include "octopus_ipc_ptl.hpp" // Include custom IPC Protocol header (if needed)
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// Constants for buffer sizes
#define IPC_SOCKET_RESPONSE_BUFFER_SIZE 255
#define IPC_SOCKET_QUERY_BUFFER_SIZE 255

// Structure to store active client information
struct ClientInfo
{
    int fd;         // Client file descriptor (unique identifier for the connection)
    bool flag;      // Status flag for additional state tracking (can be used for tracking connection status)
    std::string ip; // Client IP address for identification and logging

    // Constructor to initialize client information
    ClientInfo(int fd, std::string ip, bool flag)
        : fd(fd), ip(std::move(ip)), flag(flag) {}

    // Overload == operator to allow comparison in unordered_set for efficient lookups
    bool operator==(const ClientInfo &other) const
    {
        return fd == other.fd; // Compare based on fd (assuming it's unique for each client)
    }
};

// Enum for representing query result status
enum class QueryStatus
{
    Success,    // Query was successful
    Timeout,    // Query timed out
    Disconnected, // The connection was lost during the query
    Error       // An error occurred while processing the query
};

// Structure to store the result of a query
struct QueryResult
{
    QueryStatus status; // Status of the query (Success, Timeout, etc.)
    std::vector<uint8_t> data; // Data returned from the query, stored as a byte vector
};

// Custom hash function specialization for ClientInfo (to allow storing in unordered_map or unordered_set)
namespace std
{
    template <>
    struct hash<ClientInfo>
    {
        size_t operator()(const ClientInfo &client) const
        {
            return hash<int>()(client.fd); // Use the client file descriptor (fd) as the hash key
        }
    };
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// Class that handles socket communication, both on the server-side and client-side.
class Socket
{
private:
    const char *socket_path; // Path to the Unix domain socket (the address of the socket)
    int domain;              // Socket domain (AF_UNIX for Unix domain sockets)
    int type;                // Socket type (SOCK_STREAM for stream-based communication)
    int protocol;            // Protocol (0 for default)
    int max_waiting_requests; // Maximum number of pending requests for the server
    sockaddr_un addr;         // Address for the Unix domain socket
    int epoll_fd;             // File descriptor for epoll (used for efficient event-driven communication)
    std::mutex epoll_mutex;   // Mutex to ensure thread safety during epoll initialization and operations
    bool epoll_initialized;   // Flag to indicate whether epoll has been initialized

public:
    // Constructor: Initializes socket parameters and sets default values
    Socket();

    // Initializes the socket structure (setting domain, type, protocol, etc.)
    void init_socket_structor();

    // Initializes epoll for event-driven communication. This method should be called after opening a socket.
    void init_epoll(int socket_fd);

    // Initializes epoll (without needing a socket_fd). Creates the epoll instance.
    void init_epoll();

    // Registers a socket file descriptor (fd) with epoll for monitoring events.
    bool register_socket_fd(int socket_fd);

    // Opens a socket using default parameters (AF_UNIX, SOCK_STREAM, 0).
    int open_socket();

    // Opens a socket with specified domain, type, and protocol.
    int open_socket(int domain, int type, int protocol);

    // Closes the specified socket file descriptor (client_fd).
    int close_socket(int client_fd);

    // Binds the socket to the specified address (IPC socket path).
    bool bind_server_to_socket(int socket_fd);

    // Starts listening for incoming client connections on the specified socket.
    bool start_listening(int socket_fd);

    // Waits for an incoming client connection and accepts it.
    int wait_and_accept(int socket_fd);

    // Sends a response to the client over the specified socket.
    int send_response(int socket_fd, std::vector<int> &resp_vector);

    // Sends a response (buffer) to the client over the specified socket.
    int send_buff(int socket_fd, uint8_t *resp_buffer, int length);

    // Retrieves a query from the client over the specified socket.
    QueryResult get_query(int socket_fd);

    // Retrieves a query from the client with epoll for non-blocking event-driven communication.
    QueryResult get_query_with_epoll(int socket_fd, int timeout_ms);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // Client-side socket functions (for connecting to server and sending queries)

    // Connects to the server using the provided socket file descriptor (socket_fd).
    int connect_to_socket(int socket_fd);

    // Connects to the server at the specified address using the provided socket file descriptor (socket_fd).
    int connect_to_socket(int socket_fd, std::string address);

    // Sends a query to the server (client-side) using the specified socket file descriptor.
    bool send_query(int socket_fd, const std::vector<int> &query_vector);

    // Sends a query to the server (client-side) using the specified socket file descriptor.
    bool send_query(int socket_fd, const std::vector<uint8_t> &query_vector);

    // Retrieves the response from the server for the specified socket.
    QueryResult get_response(int socket_fd);

    // Retrieves the response from the server with epoll for non-blocking event-driven communication.
    QueryResult get_response_with_epoll(int socket_fd, int timeout_ms = 100);

    // Prints the contents of a byte vector (used for debugging and logging).
    void printf_vector_bytes(const std::vector<uint8_t> &vec, int length);

    // Prints the contents of a byte buffer (used for debugging and logging).
    void printf_buffer_bytes(const std::vector<uint8_t> &vec, int length);

    // Prints the contents of a byte buffer (used for debugging and logging).
    void printf_buffer_bytes(const void *buffer, int length);
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////
