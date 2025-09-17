
#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <csignal>
#include <dlfcn.h>
#include "IPC/octopus_ipc_ptl.hpp"

typedef void (*OctopusAppResponseCallback)(const DataMessage &query_msg, int size);
typedef void (*RegisterCallbackFunc)(std::string func_name, OctopusAppResponseCallback callback);
// Function pointer type for: void ipc_send_message_queue(DataMessage& message);
typedef void (*T_ipc_send_message_queue_func)(uint8_t group, uint8_t msg, int delay, const std::vector<uint8_t> &message_data);

static T_ipc_send_message_queue_func ipc_send_message_queue = nullptr;

static void *handle = nullptr; // 保持全局作用域

void printf_vector_bytes(const std::vector<uint8_t> &vec, int length)
{
    // std::cout << "printf_vector_bytes " << length << std::endl; //
    //  确保 length 不超过 vector 的大小
    int print_length = std::min(length, static_cast<int>(vec.size()));

    for (int i = 0; i < print_length; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << "0x" << (vec[i] & 0xFF) << " "; // 打印每个字节
    }
    std::cout << std::dec << std::endl; // 恢复十进制格式
}

void app_ipc_socket_reesponse_callback(const DataMessage &query_msg, int size)
{
    // printf_vector_bytes(response, size);
    query_msg.printMessage("app callback");
}

void initialize_app_client()
{
    // 加载共享库
    std::cout << "App initialize client library..." << std::endl;
    handle = dlopen("libOAPPC.so", RTLD_LAZY);

    if (!handle)
    {
        std::cerr << "App Failed to load client library: " << dlerror() << std::endl;
        return;
    }

    RegisterCallbackFunc register_ipc_socket_callback = (RegisterCallbackFunc)dlsym(handle, "ipc_register_socket_callback");
    if (!register_ipc_socket_callback)
    {
        std::cerr << "App Failed to load register_ipc_socket_callback function: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    ipc_send_message_queue = (T_ipc_send_message_queue_func)dlsym(handle, "ipc_send_message_queue");
    if (!ipc_send_message_queue)
    {
        std::cerr << "App Failed to load ipc_send_message_queue function: " << dlerror() << std::endl;
        dlclose(handle);
        return;
    }

    if (register_ipc_socket_callback)
    {
        register_ipc_socket_callback("app_ipc_socket_reesponse_callback", app_ipc_socket_reesponse_callback);
    }
}
void cleanup()
{
    if (handle)
    {
        dlclose(handle);
        std::cout << "App cleaned up app client library." << std::endl;
    }
}

int main(int argc, char *argv[])
{

    initialize_app_client();
    // 准备要传输的数据
    uint8_t group = MSG_GROUP_SET;            // 组 ID
    uint8_t msg = MSG_IPC_CMD_CONFIG_FLAG;    // 消息 ID
    int delay = 1000;                         // 延迟时间（毫秒）

    // 准备消息数据
    std::vector<uint8_t> message_data = {0x00, 0x01, 0x02, 0x03}; // 示例数据

    // 调用 ipc_send_message_queue
    if (ipc_send_message_queue)
    {
        ipc_send_message_queue(group, msg, delay, message_data);
        std::cout << "App ipc_send_message_queue:";
        printf_vector_bytes(message_data,message_data.size());
    }
    // 注册信号处理，确保程序退出时释放 SO
    std::signal(SIGINT, [](int)
                {
        cleanup();
        exit(0); });

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
