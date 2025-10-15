cmake_minimum_required(VERSION 3.10)
project(HAL)

# 收集 HAL 目录下所有的源文件（.cpp/.c）
file(GLOB HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# 创建 HAL 动态库
add_library(OHAL SHARED ${HAL_SOURCES})

# 头文件暴露给其他模块
target_include_directories(OHAL PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Android Studio 项目中，一般不做 install（可选，留着 Linux 用）
install(TARGETS OHAL DESTINATION /usr/local/lib)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION /usr/local/include/HAL
        FILES_MATCHING PATTERN "*.hpp")
