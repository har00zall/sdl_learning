cmake_minimum_required(VERSION 3.16)
project(SDL_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

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
