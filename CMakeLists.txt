cmake_minimum_required(VERSION 3.29)
project(StrongInputBuffer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

option(STRONG_BUFFER_TEST_ONLY "Build buffer logic tests without Geode" OFF)
if(STRONG_BUFFER_TEST_ONLY)
    enable_testing()
    add_executable(buffer_engine_tests tests/engine_tests.cpp)
    add_test(NAME StrongInputBufferCore COMMAND buffer_engine_tests)
    return()
endif()

add_library(${PROJECT_NAME} SHARED src/main.cpp)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "GEODE_SDK is not set. Install the Geode SDK first.")
endif()
add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})
