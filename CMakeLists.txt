cmake_minimum_required(VERSION 3.16)
project(BF_ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
endif()

if (WIN32 AND NOT CMAKE_GENERATOR_PLATFORM)
    set(CMAKE_GENERATOR_PLATFORM x64 CACHE STRING "" FORCE)
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Building 64-bit target")
else()
    message(STATUS "Building 32-bit target")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES 
    src/main.cpp
    src/interpreter.cpp
)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(BF_ASM ${SOURCES})

target_compile_definitions(BF_ASM PRIVATE
    $<$<CONFIG:Debug>:DEBUG=1>
    $<$<CONFIG:Release>:RELEASE=1>
)