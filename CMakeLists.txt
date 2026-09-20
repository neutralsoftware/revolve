cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(Revolve LANGUAGES C CXX)

find_package(Git REQUIRED)

execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule sync --recursive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)

execute_process(
    COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "Using Compiler: ${CMAKE_CXX_COMPILER_ID}
    ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler at path:" ${CMAKE_CXX_COMPILER})

include_directories(extern include)

file(GLOB_RECURSE REVOLVE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(revolve ${REVOLVE_SOURCES})

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/runner.cpp)
add_compile_definitions(
    REVOLVE_PATH="${CMAKE_BINARY_DIR}/revolve"
    TESTS_PATH="${CMAKE_SOURCE_DIR}/tests"
)
add_executable(revolve_test ${TEST_SOURCES})
