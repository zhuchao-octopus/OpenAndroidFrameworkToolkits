/**
 * @file octopus_ipc_app_client.hpp
 * @brief Header file for client-side communication with the server.
 *
 * Provides an interface for the UI layer to send queries and receive responses via a callback.
 * Supports asynchronous response handling through a registered callback function.
 *
 * Author: ak47
 * Organization: octopus
 * Date Time: 2025/03/13 21:00
 */
#ifndef __OCTOPUS_IPC_APP_HPP__
#define __OCTOPUS_IPC_APP_HPP__

#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <csignal>
#include <chrono>

#include "../IPC/octopus_ipc_ptl.hpp"
#include "../IPC/octopus_logger.hpp"
#include "../IPC/octopus_ipc_socket.hpp"
#include "../IPC/octopus_ipc_threadpool.hpp"
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @brief Function pointer type for handling server responses.
 * @param response The received response data.
 * @param size The size of the response vector.
 */

typedef void (*OctopusAppResponseCallback)(const DataMessage &query_msg, int size);

/**
 * @brief Registers a callback function to be called when a response is received.
 * @param callback Function pointer to the callback.
 */

void ipc_register_socket_callback(const char *func_name, OctopusAppResponseCallback callback);
void ipc_unregister_socket_callback(OctopusAppResponseCallback callback);

/**
 * @brief Initializes the client connection and starts the response receiver thread.
 * This function is automatically called when the shared library is loaded.
 */
void ipc_client_main();

/**
 * @brief Cleans up the client connection and stops the response receiver thread.
 * This function is automatically called when the shared library is unloaded.
 */
void ipc_exit_cleanup();

/**
 * @brief Send a message immediately through the IPC mechanism.
 *
 * This function sends a message synchronously to the designated recipient via the
 * inter-process communication (IPC) channel. The message is processed as soon as possible.
 *
 * @param message The message to be sent. This should include target task ID, message ID,
 *                command, and optional payload.
 */
void ipc_send_message(DataMessage &message);

/**
 * @brief Send a message asynchronously by pushing it to the IPC message queue.
 *
 * This function places the message into a thread-safe internal queue. It is suitable
 * for high-frequency message dispatching scenarios, ensuring that messages are processed
 * in order without blocking the caller.
 *
 * @param message The message to enqueue and send asynchronously.
 */
//void ipc_send_message_queue(DataMessage &message);

/**
 * @brief Send a message into the IPC queue with a delay.
 *
 * This function enqueues the message but delays its dispatch for a specified time (in milliseconds).
 * It can be used for debouncing, scheduling, or retrying messages.
 *
 * @param message  The message to enqueue.
 * @param delay_ms Delay in milliseconds before the message becomes eligible for dispatch.
 */
void ipc_send_message_queue_delayed(DataMessage &message, int delay_ms);

void ipc_send_message_queue(uint8_t group, uint8_t msg_id, const uint8_t *message_data, int message_size, int delay);


#ifdef __cplusplus
}
#endif

#endif // CLIENT_HPP
