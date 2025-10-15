cmake_minimum_required(VERSION 3.10)
project(APP)

# 收集 APP 目录下所有源文件
set(APP_SOURCES
        octopus_ipc_app_client.cpp
        octopus_ipc_app_client_jni.cpp
)

# ------------------------
# APP 动态库
# ------------------------
add_library(OAPPC SHARED ${APP_SOURCES})

# 链接依赖库
target_link_libraries(OAPPC
        PRIVATE
        OIPC     # 依赖 IPC 库
)

# 暴露头文件给其他模块使用
target_include_directories(OAPPC PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------------
# 安装规则（Linux 下测试用）
# ------------------------
install(TARGETS OAPPC DESTINATION /usr/local/lib)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION /usr/local/include/APP
        FILES_MATCHING PATTERN "*.hpp")
