#include "VulkanApplication.hpp"

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
    constexpr bool g_bEnableValidationLayers = false;
#else
    constexpr bool g_bEnableValidationLayers = true;
#endif


void BAV::VulkanApplication::Run()
{
    InitVulkan();
    CreateInstance();

    MainLoop();
    CleanUp();
}

bool BAV::VulkanApplication::CheckValidationLayerSupport() const
{
    VkResult result;

    uint32_t layerCount = 0;
    result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Couldn't get layer count");
    }

    std::vector<VkLayerProperties> availableLayers(layerCount);
    result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Couldn't get layer properties");
    }


    // if all validations layers are available, then return true
    const bool foundAllLayers = std::ranges::all_of(m_ValidationLayers, [&](const std::string& validationLayer)
    {
       return std::ranges::any_of(availableLayers, [&validationLayer](const VkLayerProperties& layerProperty)
       {
            return validationLayer == layerProperty.layerName;
       });
    });

    return foundAllLayers;
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
    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
}

void BAV::VulkanApplication::CreateInstance()
{
    if (g_bEnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers are required, but not available");
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = m_Title.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // Developer Specified
    applicationInfo.pEngineName = "What is an Engine";              // If you have an engine???
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // Don't have one
    applicationInfo.apiVersion = VK_API_VERSION_1_4;                // The minimum Vulkan version that is required

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;

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
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // This is about the global validation layers, I don't know more (yet)
    createInfo.enabledLayerCount = 0;


    // Get Vulkan ExtensionCount
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // Get Vulkan Extensions
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // Print extension names
    std::cout << "Available extensions:" << '\n';

    for (const auto& extension: extensions)
        std::cout << '\t' << extension.extensionName << '\n';

    std::cout << '\n';


    // Create Vulkan Instance
    // We don't have a custom allocator (for now/yet??)
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

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
