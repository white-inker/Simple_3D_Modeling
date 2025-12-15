cmake_minimum_required(VERSION 3.16)
project(Simple3DModeler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

file(GLOB SOURCES src/*.cpp)

add_executable(Simple3DModeler ${SOURCES})
target_include_directories(Simple3DModeler PRIVATE include external external/stb)
target_link_libraries(Simple3DModeler PRIVATE glfw glm::glm)

if (APPLE)
    target_link_libraries(Simple3DModeler PRIVATE "-framework OpenGL")
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(Simple3DModeler PRIVATE OpenGL::GL OpenGL::GLU)
endif()
