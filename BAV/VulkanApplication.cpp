#include "VulkanApplication.hpp"

#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <stdexcept>
#include <GLFW/glfw3.h>


void BAV::VulkanApplication::Run()
{
    InitVulkan();
    MainLoop();
    CleanUp();

}

void BAV::VulkanApplication::InitVulkan()
{
     if (!glfwInit())
     {
         std::cerr << "Error: GLFW couldn't initialize" << '\n';

         std::runtime_error;
         return;
     }

    // Tells GLFW not to create an OpenGL Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // We disable resizing, for some reason that I don't know yet
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


    // Create Window
    m_Window = glfwCreateWindow(m_Width, m_Height,  m_Title.c_str(), nullptr, nullptr);

}

void BAV::VulkanApplication::MainLoop()
{
    while (!glfwWindowShouldClose(m_Window))
    {
        glfwPollEvents();
    }
}

void BAV::VulkanApplication::CleanUp()
{
    glfwDestroyWindow(m_Window);

    glfwTerminate();
}

