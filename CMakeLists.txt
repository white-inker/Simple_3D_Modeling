cmake_minimum_required(VERSION 3.15)
project(Simple3DModeler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(/utf-8)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(glog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# -------------------------
# 1) collect sources
# -------------------------
file(GLOB_RECURSE SRC_ALL
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.cc
)

set(APP_MAIN ${PROJECT_SOURCE_DIR}/src/main.cpp)

list(REMOVE_ITEM SRC_ALL ${APP_MAIN})

# -------------------------
# 2) core object library (no main)
# -------------------------
add_library(Simple3DCore OBJECT ${SRC_ALL})

target_include_directories(Simple3DCore PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(Simple3DCore
        PUBLIC
        glad::glad
        glfw
        glm::glm
        imgui::imgui
        glog::glog
        nlohmann_json::nlohmann_json
)

# -------------------------
# 3) main application
# -------------------------
add_executable(${PROJECT_NAME}
        ${APP_MAIN}
        $<TARGET_OBJECTS:Simple3DCore>
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        glad::glad
        glfw
        glm::glm
        imgui::imgui
        glog::glog
        nlohmann_json::nlohmann_json
)

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/config
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/config
    )
endif()

# -------------------------
# 4) tests
# -------------------------
enable_testing()

add_executable(test1
        $<TARGET_OBJECTS:Simple3DCore>
        tests/test1/test1.cpp
        tests/test1/TestApplication.h
)

target_link_libraries(test1 PRIVATE
        glad::glad
        glfw
        glm::glm
        imgui::imgui
        glog::glog
        nlohmann_json::nlohmann_json
)

add_executable(test2
        $<TARGET_OBJECTS:Simple3DCore>
        tests/test2/test2.cpp
        tests/test2/TestApplication.h
)

target_link_libraries(test2 PRIVATE
        glad::glad
        glfw
        glm::glm
        imgui::imgui
        glog::glog
        nlohmann_json::nlohmann_json
)


add_test(NAME test1 COMMAND test1)

