cmake_minimum_required(VERSION 3.16)

# BudgetArms Vulkan (Renderer)
project(BAV)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# Get Vulkan SDK
find_package(Vulkan REQUIRED)


include(FetchContent)

# Add GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)


# Add GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glm)


add_executable(
        ${PROJECT_NAME}
        BAV/main.cpp
        BAV/VulkanApplication.cpp
        BAV/ConversionHelpers.cpp
)


# Link libraries
target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        glfw
        glm
        Vulkan::Vulkan
)


# Include directories
target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        glfw
        glm
)

