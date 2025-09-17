# Sets the minimum version of CMake required to build your native library.
# This ensures that a certain set of CMake features is available to
# your build.
cmake_minimum_required(VERSION 3.18)

# 声明项目
project(IPC_OTSM_Service LANGUAGES C CXX)

# 添加子目录
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/ipc-socket-otsm-service)

Set(CMAKE VERBOSE MAKEFILE On)

# Specifies a library name, specifies whether the library is STATIC or
# SHARED, and provides relative paths to the source code. You can
# define multiple libraries by adding multiple add.library() commands,
# and CMake builds them for you. When you build your app, Gradle
# automatically packages shared libraries with your APK.
