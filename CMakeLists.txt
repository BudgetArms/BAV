cmake_minimum_required(VERSION 3.16)

# BudgetArms Vulkan (Renderer)
project(BAV)

# Set C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(ThirdParty/GLFW)

add_executable(${PROJECT_NAME} BAV/main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
