cmake_minimum_required(VERSION 3.10)
project(MultiPlatformApp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE COMMON_SOURCES "src/*.c")

# OSごとにエントリーポイント違う
if(WIN32)
    message(STATUS "Target OS: Windows")
    set(OS_SPECIFIC_SOURCES src/platform/win32.c)
    set(WIN32_EXE_FLAG WIN32)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Target OS: Linux")
    set(OS_SPECIFIC_SOURCES src/platform/linux.c)

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "Target OS: macOS")
    set(OS_SPECIFIC_SOURCES src/platform/macos.c)

else()
    message(FATAL_ERROR "Unsupported Operating System: ${CMAKE_SYSTEM_NAME}")
endif()

add_executable(mconlyc ${WIN32_EXE_FLAG} ${COMMON_SOURCES} ${OS_SPECIFIC_SOURCES})

target_include_directories(mconlyc PRIVATE src)
