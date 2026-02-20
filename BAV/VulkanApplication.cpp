#include "VulkanApplication.hpp"

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "ConversionHelpers.hpp"
#include "CreationHelper.hpp"

#ifdef NDEBUG
    constexpr bool g_bEnableValidationLayers = false;
#else
    constexpr bool g_bEnableValidationLayers = true;
#endif


void BAV::VulkanApplication::Run()
{
#ifdef NDEBUG
    std::cout << "RELEASE\n" << '\n';
#else
    std::cout << "DEBUG\n" << '\n';
#endif


    InitVulkan();
    CreateInstance();
    SetupDebugMessenger();

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

std::vector<const char*> BAV::VulkanApplication::GetRequiredExtensions()
{
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

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (g_bEnableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VkBool32 BAV::VulkanApplication::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData
    )
{
    std::cerr << "Validation Layer: " << pCallbackData->pMessage << '\n';

    // MessgeSeverity:

    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    // Diagnostic message

    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    // Informational message (eg. creation of a resource), kind of verbose

    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    // Message about behavior that is problably a bug, but might be an error

    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    // Message about behavior that is invalid and could cause crashes


    // MessgeType:

    // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    // Event unrelated to specification or performance

    // VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    // Violation of specification or indication of possible mistake

    // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    // Non-optimal use of Vulkan


    // pCallback:

    // Refers to VkDebugUtilsMessengerCallbackDataEXT
    // This contains details of the message

    // Like:
    // pMessage: Debug message
    // pObjects: Array of Vulkan Objects related to the message
    // objectCount: The number of objects in the pObjects's array


    // pUserData:

    // idk what it means

    // Vulkan Documentation:
    // pUserData is the application-defined user data pointer,
    // equal to the value of VkDebugUtilsMessengerCreateInfoEXT::pUserData
    // specified when the VkDebugUtilsMessengerEXT object was created


    // Vulkan Tutorial Information:
    // pData contains a pointer that was specified during the setup
    // of the callback and allows you to pass your own data to it.


    // Return boolean:

    // Indicates if the Vulkan call (that triggered the validation layer message)
    // should be aborted or not.

    // If true, the call is aborted with error 'VK_ERROR_VALIDATION_FAILED_EXT'
    // However, this is (normallY) only used to test the validation layers.
    // So, always return with 'VK_FALSE'.


    // Severity: Warning
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {

    }

    return VK_FALSE;
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

    const std::vector<const char*> requiredExtensions = GetRequiredExtensions();

    // Fill in instance create info
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // outside the if-statement below, because of local scope
    const std::vector<const char*> charValidationLayers = ConversionHelpers::StringVectorToCharVector(m_ValidationLayers);

    if (g_bEnableValidationLayers)
    {
        // createInfo.enabledExtensionCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = charValidationLayers.data();
    }
    else
    {
        // If not debugging, don't add any validation layers
        createInfo.enabledLayerCount = 0;
    }


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
   if (g_bEnableValidationLayers)
   {
       CreationHelper::DestroyDebugUtilsMesengerEXT(m_Instance, m_DebugMessenger, nullptr);
   }

    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_Window);

    glfwTerminate();
}

void BAV::VulkanApplication::SetupDebugMessenger()
{
    if (!g_bEnableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    // Linked to function
    createInfo.pfnUserCallback = DebugCallback;

    // (optional), I don't really see a proper use-case for it
    // but that might change in the future
    createInfo.pUserData = nullptr;


    const VkResult result = CreationHelper::CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr,
        &m_DebugMessenger);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Creation of debug messenger failed");
    }

    if (!m_DebugMessenger)
    {
        throw std::runtime_error("DebugMessenger is invalid");
    }



}

