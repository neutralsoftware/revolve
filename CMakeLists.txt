cmake_minimum_required(VERSION 3.21)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

project(Revolve LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

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

set(REVOLVE_SHADER_HEADER ${CMAKE_SOURCE_DIR}/include/graphics/shader.h)
set(REVOLVE_SHADER_EMBED_SCRIPT ${CMAKE_BINARY_DIR}/embed_shader.cmake)

file(WRITE ${REVOLVE_SHADER_EMBED_SCRIPT} [=[set(SHADER_HEADER "#ifndef REVOLVE_GRAPHICS_SHADER\n#define REVOLVE_GRAPHICS_SHADER\n\n")
math(EXPR SHADER_LAST_INDEX "${SHADER_COUNT} - 1")
foreach(SHADER_INDEX RANGE ${SHADER_LAST_INDEX})
    file(READ "${INPUT_FILE_${SHADER_INDEX}}" SHADER_SOURCE)
    string(APPEND SHADER_HEADER "inline constexpr const char *${SYMBOL_${SHADER_INDEX}} = R\"SHDR${SHADER_INDEX}(\n${SHADER_SOURCE})SHDR${SHADER_INDEX}\";\n\n")
endforeach()
string(APPEND SHADER_HEADER "#endif\n")
file(WRITE "${OUTPUT_FILE}" "${SHADER_HEADER}")
]=])

find_program(REVOLVE_SLANGC NAMES slangc)
find_program(REVOLVE_SPIRV_CROSS NAMES spirv-cross)
string(TOUPPER "${OPAL_BACKEND}" REVOLVE_OPAL_BACKEND)

file(GLOB_RECURSE REVOLVE_SHADER_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders/*.slang)
list(SORT REVOLVE_SHADER_SOURCES)

set(REVOLVE_SHADER_INPUTS)
set(REVOLVE_SHADER_EMBED_ARGUMENTS)
set(REVOLVE_SHADER_INDEX 0)

foreach(REVOLVE_SHADER_SOURCE IN LISTS REVOLVE_SHADER_SOURCES)
    file(RELATIVE_PATH REVOLVE_SHADER_RELATIVE
        ${CMAKE_SOURCE_DIR}/shaders ${REVOLVE_SHADER_SOURCE})
    string(REGEX REPLACE "\\.slang$" "" REVOLVE_SHADER_NAME
        ${REVOLVE_SHADER_RELATIVE})
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" REVOLVE_SHADER_NAME
        ${REVOLVE_SHADER_NAME})
    string(TOUPPER ${REVOLVE_SHADER_NAME} REVOLVE_SHADER_SYMBOL)
    if((REVOLVE_OPAL_BACKEND STREQUAL "METAL" OR
        (REVOLVE_OPAL_BACKEND STREQUAL "AUTO" AND APPLE)) AND REVOLVE_SLANGC)
        if(NOT REVOLVE_SPIRV_CROSS)
            message(FATAL_ERROR "spirv-cross is required for Metal shader generation")
        endif()

        set(REVOLVE_VERTEX_SPIRV
            ${CMAKE_BINARY_DIR}/generated/graphics/${REVOLVE_SHADER_NAME}.vert.spv)
        set(REVOLVE_FRAGMENT_SPIRV
            ${CMAKE_BINARY_DIR}/generated/graphics/${REVOLVE_SHADER_NAME}.frag.spv)
        set(REVOLVE_VERTEX_INPUT
            ${CMAKE_BINARY_DIR}/generated/graphics/${REVOLVE_SHADER_NAME}.vert.metal)
        set(REVOLVE_FRAGMENT_INPUT
            ${CMAKE_BINARY_DIR}/generated/graphics/${REVOLVE_SHADER_NAME}.frag.metal)
        add_custom_command(
            OUTPUT ${REVOLVE_VERTEX_INPUT} ${REVOLVE_FRAGMENT_INPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_BINARY_DIR}/generated/graphics
            COMMAND ${REVOLVE_SLANGC} ${REVOLVE_SHADER_SOURCE}
                -entry vertexMain -stage vertex -target spirv
                -profile spirv_1_3 -preserve-params -o ${REVOLVE_VERTEX_SPIRV}
            COMMAND ${REVOLVE_SPIRV_CROSS} ${REVOLVE_VERTEX_SPIRV}
                --msl --msl-version 230000 --output ${REVOLVE_VERTEX_INPUT}
            COMMAND ${REVOLVE_SLANGC} ${REVOLVE_SHADER_SOURCE}
                -entry fragmentMain -stage fragment -target spirv
                -profile spirv_1_3 -preserve-params -o ${REVOLVE_FRAGMENT_SPIRV}
            COMMAND ${REVOLVE_SPIRV_CROSS} ${REVOLVE_FRAGMENT_SPIRV}
                --msl --msl-version 230000 --output ${REVOLVE_FRAGMENT_INPUT}
            DEPENDS ${REVOLVE_SHADER_SOURCE}
            VERBATIM
        )
    else()
        set(REVOLVE_VERTEX_INPUT ${REVOLVE_SHADER_SOURCE})
        set(REVOLVE_FRAGMENT_INPUT ${REVOLVE_SHADER_SOURCE})
    endif()

    list(APPEND REVOLVE_SHADER_INPUTS
        ${REVOLVE_VERTEX_INPUT} ${REVOLVE_FRAGMENT_INPUT})
    list(APPEND REVOLVE_SHADER_EMBED_ARGUMENTS
        -DINPUT_FILE_${REVOLVE_SHADER_INDEX}=${REVOLVE_VERTEX_INPUT}
        -DSYMBOL_${REVOLVE_SHADER_INDEX}=${REVOLVE_SHADER_SYMBOL}_VERTEX_SHADER)
    math(EXPR REVOLVE_SHADER_INDEX "${REVOLVE_SHADER_INDEX} + 1")
    list(APPEND REVOLVE_SHADER_EMBED_ARGUMENTS
        -DINPUT_FILE_${REVOLVE_SHADER_INDEX}=${REVOLVE_FRAGMENT_INPUT}
        -DSYMBOL_${REVOLVE_SHADER_INDEX}=${REVOLVE_SHADER_SYMBOL}_FRAGMENT_SHADER)
    math(EXPR REVOLVE_SHADER_INDEX "${REVOLVE_SHADER_INDEX} + 1")
endforeach()

add_custom_command(
    OUTPUT ${REVOLVE_SHADER_HEADER}
    COMMAND ${CMAKE_COMMAND}
        -DSHADER_COUNT=${REVOLVE_SHADER_INDEX}
        -DOUTPUT_FILE=${REVOLVE_SHADER_HEADER}
        ${REVOLVE_SHADER_EMBED_ARGUMENTS}
        -P ${REVOLVE_SHADER_EMBED_SCRIPT}
    DEPENDS ${REVOLVE_SHADER_INPUTS} ${REVOLVE_SHADER_EMBED_SCRIPT}
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
file(GLOB_RECURSE INPUT_SOURCES CONFIGURE_DEPENDS src/input/*.cpp)
file(GLOB_RECURSE AUDIO_SOURCES CONFIGURE_DEPENDS src/audio/*.cpp)
add_executable(revolve_test ${TEST_SOURCES} tests/broadway.cpp tests/disc.cpp
    tests/advanced_tev.cpp tests/controllers.cpp tests/graphics.cpp tests/ios.cpp tests/tev.cpp ${BROADWAY_SOURCES} src/core/memory.cpp
    src/core/pi.cpp src/core/mi.cpp src/core/utils.cpp src/core/serial.cpp src/core/system_hardware.cpp ${IOS_SOURCES}
    src/core/executable.cpp src/device.cpp src/disc.cpp ${GRAPHICS_SOURCES} ${INPUT_SOURCES} ${AUDIO_SOURCES})
add_dependencies(revolve_test revolve_shader_header)
target_link_libraries(revolve_test PRIVATE OpenSSL::Crypto PkgConfig::ZSTD Opal::Opal)
