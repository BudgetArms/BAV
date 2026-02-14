cmake_minimum_required(VERSION 3.16)

# BudgetArms Vulkan (Renderer)
project(BAV)

# Set C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

# Add GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)




add_executable(${PROJECT_NAME} BAV/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

target_include_directories(${PROJECT_NAME} PRIVATE glfw)


