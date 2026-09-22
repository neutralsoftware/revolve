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

set(REVOLVE_SHADER_SOURCE ${CMAKE_SOURCE_DIR}/shaders/minimal.slang)
set(REVOLVE_SHADER_OUTPUT ${CMAKE_BINARY_DIR}/generated/graphics/shader.metal)
set(REVOLVE_SHADER_HEADER ${CMAKE_SOURCE_DIR}/include/graphics/shader.h)
set(REVOLVE_SHADER_EMBED_SCRIPT ${CMAKE_BINARY_DIR}/embed_shader.cmake)

file(WRITE ${REVOLVE_SHADER_EMBED_SCRIPT} [=[file(READ "${INPUT_FILE}" SHADER_SOURCE)
file(WRITE "${OUTPUT_FILE}" "#ifndef REVOLVE_GRAPHICS_SHADER\n#define REVOLVE_GRAPHICS_SHADER\n\ninline constexpr const char *SHADER = R\"REVOLVE_SHADER(\n${SHADER_SOURCE})REVOLVE_SHADER\";\n\n#endif\n")
]=])

find_program(REVOLVE_SLANGC NAMES slangc)
string(TOUPPER "${OPAL_BACKEND}" REVOLVE_OPAL_BACKEND)

if((REVOLVE_OPAL_BACKEND STREQUAL "METAL" OR
    (REVOLVE_OPAL_BACKEND STREQUAL "AUTO" AND APPLE)) AND REVOLVE_SLANGC)
    add_custom_command(
        OUTPUT ${REVOLVE_SHADER_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/generated/graphics
        COMMAND ${REVOLVE_SLANGC} -target metal -entry vertexMain -stage vertex
            -entry fragmentMain -stage fragment -o ${REVOLVE_SHADER_OUTPUT}
            ${REVOLVE_SHADER_SOURCE}
        DEPENDS ${REVOLVE_SHADER_SOURCE}
        VERBATIM
    )
    set(REVOLVE_SHADER_INPUT ${REVOLVE_SHADER_OUTPUT})
else()
    set(REVOLVE_SHADER_INPUT ${REVOLVE_SHADER_SOURCE})
endif()

add_custom_command(
    OUTPUT ${REVOLVE_SHADER_HEADER}
    COMMAND ${CMAKE_COMMAND} -DINPUT_FILE=${REVOLVE_SHADER_INPUT}
        -DOUTPUT_FILE=${REVOLVE_SHADER_HEADER}
        -P ${REVOLVE_SHADER_EMBED_SCRIPT}
    DEPENDS ${REVOLVE_SHADER_INPUT} ${REVOLVE_SHADER_EMBED_SCRIPT}
    VERBATIM
)
add_custom_target(revolve_shader_header ALL DEPENDS ${REVOLVE_SHADER_HEADER})
add_dependencies(revolve revolve_shader_header)

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
add_dependencies(revolve_test revolve_shader_header)
target_link_libraries(revolve_test PRIVATE OpenSSL::Crypto PkgConfig::ZSTD Opal::Opal)
