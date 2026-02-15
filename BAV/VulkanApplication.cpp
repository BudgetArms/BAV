#include "VulkanApplication.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


void BAV::VulkanApplication::Run()
{
    InitVulkan();
    CreateInstance();

    MainLoop();
    CleanUp();

}

void BAV::VulkanApplication::InitVulkan()
{
     if (!glfwInit())
     {
         throw std::runtime_error("GLFW couldn't initialize");
     }

    // Tells GLFW not to create an OpenGL Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // We disable resizing, for some reason that I don't know yet
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


    // Create Window
    m_Window = glfwCreateWindow(m_Width, m_Height,  m_Title.c_str(), nullptr, nullptr);

}

void BAV::VulkanApplication::CreateInstance()
{
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = m_Title.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Developer Specified
    applicationInfo.pEngineName = "What is an Engine";              // If you have an engine???
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // Don't have one
    applicationInfo.apiVersion = VK_API_VERSION_1_4;                // The minimum Vulkan version that is required

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    // Get Vulkan Extensions & ExtensionsCount
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    if (!glfwExtensions)
    {
        throw std::runtime_error("No extensions found");
    }

    if (glfwExtensionCount == 0)
    {
        throw std::runtime_error("ExtensionCount is zero");
    }


    // Fill in instance create info
    instanceCreateInfo.enabledExtensionCount = glfwExtensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

    // This is about the global validation layers, I don't know more (yet)
    instanceCreateInfo.enabledLayerCount = 0;


    // Get Vulkan ExtensionCount
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // Get Vulkan Extensions
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // Print extension names
    std::cout << "Available extensions:" << '\n';

    for (const auto& extension : extensions)
        std::cout << '\t' << extension.extensionName << '\n';

    std::cout << '\n';


    // Create Vulkan Instance
    // We don't have a custom allocator (for now/yet??)
    VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);

    if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        throw std::runtime_error("Failed to create Vulkan Instance: extension not present");
    }
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan Instance");
    }


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
    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_Window);

    glfwTerminate();
}

