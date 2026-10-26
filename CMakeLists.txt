cmake_minimum_required(VERSION 3.10)
project(MultiPlatformApp C)

include(cmake/embed.cmake)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE COMMON_SOURCES "src/*.c")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# OSごとにエントリーポイント違う
if(WIN32)
    message(STATUS "Target OS: Windows")
    set(OS_SPECIFIC_SOURCES src/platform/win32.c)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "Target OS: Linux")
    set(OS_SPECIFIC_SOURCES src/platform/linux.c)

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "Target OS: macOS")
    set(OS_SPECIFIC_SOURCES src/platform/macos.c)

else()
    message(FATAL_ERROR "Unsupported Operating System: ${CMAKE_SYSTEM_NAME}")
endif()

embed_assets(ASSET_SOURCES "${CMAKE_SOURCE_DIR}/assets")

add_executable(mconlyc
    ${COMMON_SOURCES}
    ${OS_SPECIFIC_SOURCES}
    ${ASSET_SOURCES}
)

if(WIN32)
    set_target_properties(mconlyc PROPERTIES
        WIN32_EXECUTABLE "$<NOT:$<CONFIG:Debug>>"
    )
endif()

target_include_directories(mconlyc PRIVATE
    src
    ${CMAKE_BINARY_DIR}/embedded
)

target_compile_definitions(mconlyc PRIVATE EMBED_ASSETS)

target_compile_definitions(mconlyc PRIVATE
    EMBED_ASSETS
    $<$<CONFIG:Debug>:DEBUG>
)

if(UNIX)
    target_link_libraries(mconlyc PRIVATE m)
endif()

install(TARGETS mconlyc
    RUNTIME DESTINATION .
)
