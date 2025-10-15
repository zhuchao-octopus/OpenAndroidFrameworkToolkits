cmake_minimum_required(VERSION 3.10)
project(IPC)

# 收集当前目录下所有源文件
file(GLOB IPC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# 从库源码中排除 server/client
list(REMOVE_ITEM IPC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/octopus_ipc_server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/octopus_ipc_client.cpp
)

# ------------------------
# 共享库 OIPC
# ------------------------
add_library(OIPC SHARED ${IPC_SOURCES})

# IPC 库需要 pthread
find_package(Threads REQUIRED)
target_link_libraries(OIPC
        Threads::Threads
        atomic
        m
)

# 头文件暴露给其他模块（比如 APP）
target_include_directories(OIPC PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ------------------------
# 可执行文件 server
# ------------------------
add_executable(octopus_ipc_server
    ${CMAKE_CURRENT_SOURCE_DIR}/octopus_ipc_server.cpp
)

# server 依赖 OIPC，并且需要 dl pthread
target_link_libraries(octopus_ipc_server PRIVATE
    OIPC
    dl
    Threads::Threads
    atomic
    m
)

# ------------------------
# 可执行文件 client
# ------------------------
add_executable(octopus_ipc_client
    ${CMAKE_CURRENT_SOURCE_DIR}/octopus_ipc_client.cpp
)

# client 依赖 OIPC，并且需要 pthread
target_link_libraries(octopus_ipc_client PRIVATE
    OIPC
    Threads::Threads
    atomic
    m
)

# ------------------------
# 安装规则（主要用于 Linux，本地测试）
# ------------------------
install(TARGETS OIPC DESTINATION /usr/local/lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION /usr/local/include/IPC
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS octopus_ipc_server octopus_ipc_client
        DESTINATION /usr/local/bin)
