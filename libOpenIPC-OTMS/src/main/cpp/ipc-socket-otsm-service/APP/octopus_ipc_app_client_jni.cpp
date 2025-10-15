#include <jni.h>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include "octopus_ipc_app_client.hpp"
#include "octopus_logger.hpp"

// ---------------- 客户端管理 ----------------
struct JavaIpcClientInfo {
    jobject java_obj;       // 全局引用
    jclass cls;             // 全局引用类
    jmethodID method;       // Java 方法 ID
    jbyteArray buffer;      // 可复用缓冲区 (全局引用)
    jsize buffer_size;      // 缓冲区大小
};

static std::unordered_map<intptr_t, JavaIpcClientInfo> g_clients;
static std::mutex g_mutex;
static JavaVM *g_jvm = nullptr;

// ---------------- 全局桥函数 ----------------
static void jni_ipc_callback_bridge(const DataMessage &msg, int size) {
    if (!g_jvm) return;

    JNIEnv *env = nullptr;
    bool attached_here = false;

    if (g_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return; // attach 失败
        }
        attached_here = true;
    }

    std::vector<std::pair<intptr_t, JavaIpcClientInfo>> snapshot;
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        for (auto &[key, client]: g_clients) {
            snapshot.emplace_back(key, client);
        }
    }

    for (auto &[key, client]: snapshot)
    {
        // 🔹检查并扩容 buffer
        if (size > client.buffer_size) {
            env->DeleteGlobalRef(client.buffer);
            jbyteArray new_buf = env->NewByteArray(size);
            if (!new_buf) continue;
            client.buffer = (jbyteArray) env->NewGlobalRef(new_buf);
            client.buffer_size = size;
            env->DeleteLocalRef(new_buf);

            // ⚠️ 更新回全局 map
            std::lock_guard<std::mutex> lock(g_mutex);
            if (g_clients.count(key)) {
                g_clients[key].buffer = client.buffer;
                g_clients[key].buffer_size = client.buffer_size;
            }
        }

        // 拷贝数据
        jsize copy_size = std::min(static_cast<size_t>(size), static_cast<size_t>(client.buffer_size));
        env->SetByteArrayRegion(client.buffer, 0, copy_size,
                                reinterpret_cast<const jbyte *>(msg.data.data()));

        // 调用 Java 方法
        env->CallVoidMethod(client.java_obj, client.method,
                            static_cast<jbyte>(msg.msg_group),
                            static_cast<jbyte>(msg.msg_id),
                            client.buffer);

        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }//for (auto &[key, client]: snapshot)

    if (attached_here) {
        g_jvm->DetachCurrentThread();
    }
}

// ---------------- JNI 注册 ----------------
extern "C"
JNIEXPORT void JNICALL
Java_com_zhuchao_octopus_ipc_1otms_IPCClient_nativeIpcRegisterCallback(
        JNIEnv *env, jobject thiz, jobject client) {

    std::lock_guard<std::mutex> lock(g_mutex);
    intptr_t key = reinterpret_cast<intptr_t>(client);

    // 删除旧的引用
    if (g_clients.count(key)) {
        auto &old = g_clients[key];
        env->DeleteGlobalRef(old.java_obj);
        env->DeleteGlobalRef(old.cls);
        env->DeleteGlobalRef(old.buffer);
        g_clients.erase(key);
    }

    // 新全局引用
    jobject global_ref = env->NewGlobalRef(client);
    jclass cls = env->GetObjectClass(client);
    jmethodID method = env->GetMethodID(cls, "onIpcNativeResponse", "(BB[B)V");
    jclass global_cls = (jclass) env->NewGlobalRef(cls);
    env->DeleteLocalRef(cls);

    // 初始化 buffer
    jsize initial_size = 1024;
    jbyteArray buffer = env->NewByteArray(initial_size);
    jbyteArray global_buf = (jbyteArray) env->NewGlobalRef(buffer);
    env->DeleteLocalRef(buffer);

    g_clients[key] = {
            global_ref,
            global_cls,
            method,
            global_buf,
            initial_size
    };

    // 注册全局回调（只需注册一次全局桥函数）
    ipc_register_socket_callback("jni_ipc_callback_bridge", jni_ipc_callback_bridge);

    LOG_CC("Client registered callback, key=" + std::to_string(key));
}

// ---------------- JNI 注销 ----------------
extern "C"
JNIEXPORT void JNICALL
Java_com_zhuchao_octopus_ipc_1otms_IPCClient_nativeIpcUnregisterCallback(
        JNIEnv *env, jobject thiz, jobject client) {

    std::lock_guard<std::mutex> lock(g_mutex);
    intptr_t key = reinterpret_cast<intptr_t>(client);

    auto it = g_clients.find(key);
    if (it != g_clients.end()) {
        auto &c = it->second;
        env->DeleteGlobalRef(c.java_obj);
        env->DeleteGlobalRef(c.cls);
        env->DeleteGlobalRef(c.buffer);
        g_clients.erase(it);
        LOG_CC("Client unregistered callback, key=" + std::to_string(key));
    }

    // ⚠️ 可选：当 g_clients 为空时注销底层回调
    if (g_clients.empty()) {
        ipc_unregister_socket_callback(jni_ipc_callback_bridge);
        LOG_CC("No more clients, unregistered global bridge callback");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zhuchao_octopus_ipc_1otms_IPCClient_nativeIpcSendMessage(
        JNIEnv *env, jobject thiz, jbyte msg_group, jbyte msg_id, jbyteArray data) {

    jsize len = env->GetArrayLength(data);
    std::vector<uint8_t> buffer(len);
    env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte *>(buffer.data()));

    DataMessage msg(static_cast<uint8_t>(msg_group), static_cast<uint8_t>(msg_id), buffer);
    ipc_send_message(msg);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_zhuchao_octopus_ipc_1otms_IPCClient_nativeIpcSendMessageDelay(
        JNIEnv *env, jobject thiz, jbyte msg_group, jbyte msg_id, jbyteArray data, jint delay_ms) {

    jsize len = env->GetArrayLength(data);
    std::vector<uint8_t> buffer(len);
    env->GetByteArrayRegion(data, 0, len, reinterpret_cast<jbyte *>(buffer.data()));

    DataMessage msg(static_cast<uint8_t>(msg_group), static_cast<uint8_t>(msg_id), buffer);

    ipc_send_message_queue_delayed(msg, static_cast<int>(delay_ms));
}

// ---------------- JNI 接口 ----------------
extern "C"
JNIEXPORT void JNICALL
Java_com_zhuchao_octopus_ipc_1otms_IPCClient_nativeIpcInitClient(JNIEnv *env, jobject thiz) {
    env->GetJavaVM(&g_jvm);
}
