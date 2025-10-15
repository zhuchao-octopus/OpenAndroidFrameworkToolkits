cmake_minimum_required(VERSION 3.10)
project(OTSM)

# 收集 OTSM 目录下所有的源文件（.cpp/.c）
file(GLOB OTSM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# 创建 OTSM 动态库
add_library(OTSM SHARED ${OTSM_SOURCES})

# 链接 HAL 库
target_link_libraries(OTSM PRIVATE OHAL)

# 暴露头文件给其他模块（如 APP 使用）
target_include_directories(OTSM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Android Studio 一般不用 install（保留 Linux 用）
install(TARGETS OTSM DESTINATION /usr/local/lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION /usr/local/include/OTSM
        FILES_MATCHING PATTERN "*.hpp")
