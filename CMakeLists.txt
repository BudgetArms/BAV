cmake_minimum_required(VERSION 3.13)

# BudgetArms Vulkan (Renderer)
project(BAV)

# Set C++ standard
set(CXX_STANDARD_REQUIRED 20)
set(CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} BAV/main.cpp)