cmake_minimum_required(VERSION 3.16)
project(SDL_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(SHADER_COMPILED_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>/shaders")

include(FetchContent)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG         release-3.4.12
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG         1.0.3
)
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY  https://github.com/syoyo/tinygltf.git
    GIT_TAG         v3.0.0
)

FetchContent_MakeAvailable(SDL3 glm tinygltf)

add_executable(${PROJECT_NAME} WIN32 src/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 glm tinygltf)

find_program(GLSL_VALIDATOR glslc HINTS ${Vulkan_GLSLC_EXECUTABLE} /usr/bin /usr/local/bin ${VULKAN_SDK}/Bin ${VULKAN_SDK_PATH}/Bin32)

if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslc not found! Make sure Vulkan SDK is installed and glslc is in PATH")
endif()

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Debug/shaders)

function(add_shader TARGET SHADER SHADER_TYPE)
    # Get shader name without extension
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_filename_component(SHADER_EXT ${SHADER} EXT)

    string(SUBSTRING ${SHADER_EXT} 1 -1 SHADER_EXT_NO_DOT)

    set(SHADER_OUTPUT ${SHADER_COMPILED_OUTPUT_DIRECTORY}/${SHADER_NAME}.${SHADER_EXT_NO_DOT}.spv)

    add_custom_command(
        OUTPUT ${SHADER_OUTPUT}
        COMMAND ${GLSL_VALIDATOR} -fshader-stage=${SHADER_TYPE} ${CMAKE_SOURCE_DIR}/shaders/${SHADER} -o ${SHADER_OUTPUT}
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders/${SHADER}
        COMMENT "Compiling shader ${SHADER} to SPIR-V"
    )

    target_sources(${TARGET} PRIVATE ${SHADER_OUTPUT})
endfunction()

# Compile shaders
add_shader(${PROJECT_NAME} base.vert vertex)
add_shader(${PROJECT_NAME} base.frag fragment)
