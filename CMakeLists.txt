cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(Revolve LANGUAGES C CXX)

find_package(Git REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZSTD REQUIRED IMPORTED_TARGET libzstd)

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

set(OPAL_BACKEND AUTO CACHE STRING "AUTO")
add_subdirectory(external/opal)

target_link_libraries(revolve PRIVATE OpenSSL::Crypto PkgConfig::ZSTD Opal::Opal)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS tests/runner.cpp)
add_compile_definitions(
    REVOLVE_PATH="${CMAKE_BINARY_DIR}/revolve"
    TESTS_PATH="${CMAKE_SOURCE_DIR}/tests"
)
file(GLOB BROADWAY_SOURCES CONFIGURE_DEPENDS src/cpu/*.cpp)
file(GLOB_RECURSE IOS_SOURCES CONFIGURE_DEPENDS src/ios/*.cpp)
file(GLOB_RECURSE GRAPHICS_SOURCES CONFIGURE_DEPENDS src/graphics/*.cpp)
add_executable(revolve_test ${TEST_SOURCES} tests/broadway.cpp tests/disc.cpp
    tests/graphics.cpp tests/ios.cpp ${BROADWAY_SOURCES} src/core/memory.cpp
    src/core/pi.cpp src/core/mi.cpp src/core/utils.cpp ${IOS_SOURCES}
    src/core/executable.cpp src/device.cpp src/disc.cpp ${GRAPHICS_SOURCES})
target_link_libraries(revolve_test PRIVATE OpenSSL::Crypto PkgConfig::ZSTD Opal::Opal)
