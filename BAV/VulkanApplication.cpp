#include "VulkanApplication.hpp"

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <set>
#include <GLFW/glfw3.h>

#include <vulkan/vk_enum_string_helper.h>

#include "ConversionHelpers.hpp"
#include "CreationHelper.hpp"

#ifdef NDEBUG
    constexpr bool g_bEnableValidationLayers = false;
#else
    constexpr bool g_bEnableValidationLayers = true;
#endif

#define FUNCTION_NAME __FUNCTION__


void BAV::VulkanApplication::Run()
{
#ifdef NDEBUG
    std::cout << "RELEASE\n" << '\n';
#else
    std::cout << "DEBUG\n" << '\n';
#endif


    InitWindow();
    InitVulkan();

    MainLoop();
    CleanUp();
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
    // Informational message (e.g. creation of a resource), kind of verbose

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
    // This contains the details of the message

    // Like:
    // pMessage: Debug message
    // pObjects: Array of Vulkan Objects related to the message
    // objectCount: The number of objects in the pObjects's array


    // pUserData:
    // IDK what this means

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


    return VK_FALSE;
}


void BAV::VulkanApplication::InitWindow()
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

void BAV::VulkanApplication::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    // TODO: learn more about this
    // Surface loaded before physical device selection,
    // because it can influence device selection



    PickPhysicalDevice();
    CreateLocalDevice();
}

void BAV::VulkanApplication::CreateInstance()
{
    if (g_bEnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers are required, but not available");
    }

    VkApplicationInfo applicationInfo
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = m_Title.c_str(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),  // Developer Specified
        .pEngineName = "What is an Engine",              // If you have an engine???
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),       // Don't have one
        .apiVersion = VK_API_VERSION_1_4,                // The minimum Vulkan version that is required
    };

    const std::vector<const char*> requiredExtensions = GetRequiredExtensions();

    VkInstanceCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,

        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    // outside the if-statement below, because of local scope
    const std::vector<const char*> charValidationLayers = ConversionHelpers::StringVectorToCharVector(m_ValidationLayers);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (g_bEnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(charValidationLayers.size());
        createInfo.ppEnabledLayerNames = charValidationLayers.data();

        CreationHelper::PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else
    {
        // If not debugging, don't add any validation layers
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }


    // Get Vulkan ExtensionCount
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // Get Vulkan Extensions
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // Print extension names
    // std::cout << "Available extensions:" << '\n';
    //
    // for (const auto& extension: extensions)
    //     std::cout << '\t' << extension.extensionName << '\n';
    //
    // std::cout << '\n';


    // Create Vulkan Instance
    // We don't have a custom allocator (for now/yet??)
    const VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

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

    vkDestroyDevice(m_LogicalDevice, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_Window);

    glfwTerminate();
}


void BAV::VulkanApplication::SetupDebugMessenger()
{
    if (!g_bEnableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    CreationHelper::PopulateDebugMessengerCreateInfo(createInfo);


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

void BAV::VulkanApplication::CreateSurface()
{
    const VkResult result = glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create surface");
    }

}

void BAV::VulkanApplication::PickPhysicalDevice()
{
    m_PhysicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("No Physical devices are found");
    }

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

    if (physicalDevices.empty())
    {
        throw std::runtime_error("No physical devices (GPU) in array found");
    }

    // Get suitable device
    for (const auto& device : physicalDevices)
    {
        if (IsDeviceSuitable(device))
        {
            m_PhysicalDevice = device;
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Couldn't find a suitable physical device (GPU)");
    }

}

void BAV::VulkanApplication::CreateLocalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamiliesIndex =
    {
        indices.GraphicsFamily.value(),
        indices.PresentFamily.value()
    };

    constexpr float queuePriority = 1.f;

    for (const auto& uniqueQueueFamilyIndex : uniqueQueueFamiliesIndex)
    {
        VkDeviceQueueCreateInfo queueCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueQueueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }


    // Device features to be used, for now empty (so everything VK_FALSE)
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),

        .pEnabledFeatures = &deviceFeatures
    };

    // There used to be a difference between Instance- and Device specific
    // validation layers, but now it's basically the same, so device's valdiation
    // layers are ignored in Vulkan, but that's not the case for older versions of Vulkan.

    // So, for Backwards compatibility, we set the same validation layers for the
    // device as we did for the instance.

    createInfo.enabledExtensionCount = 0;

    const std::vector<const char*> charValidationLayers = ConversionHelpers::StringVectorToCharVector(m_ValidationLayers);

    if (g_bEnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(charValidationLayers.size());
        createInfo.ppEnabledLayerNames = charValidationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }


    const VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create local device");
    }

    // Get the device queue that is implicitely created and destroyed upon
    // creation and destruction respectifly of
    // the logical device
    constexpr uint32_t queueIndex = 0;


    if (indices.GraphicsFamily.value() == indices.PresentFamily.value())
    {
        vkGetDeviceQueue(m_LogicalDevice, indices.GraphicsFamily.value(), queueIndex, &m_GraphicsQueue);
        m_PresentQueue = m_GraphicsQueue;
    }
    else
    {
        vkGetDeviceQueue(m_LogicalDevice, indices.GraphicsFamily.value(), queueIndex, &m_GraphicsQueue);
        vkGetDeviceQueue(m_LogicalDevice, indices.PresentFamily.value(), queueIndex, &m_PresentQueue);
    }

}


bool BAV::VulkanApplication::CheckValidationLayerSupport() const
{
    uint32_t layerCount = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

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

bool BAV::VulkanApplication::DoesDeviceSupportRequiredExtensions(VkPhysicalDevice device) const
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

    for (const VkExtensionProperties& availableExtension : availableExtensions)
    {
        requiredExtensions.erase(availableExtension.extensionName);
    }

    // if all the required extensions are supported, return true
    if (requiredExtensions.empty())
    {
        return true;
    }

    // Log info about the device & the missing required extension
    VkPhysicalDeviceProperties deviceProperties{};
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    std::cout << '\n';
    std::cout << FUNCTION_NAME << '\n';
    std::cout << "GPU name: " << deviceProperties.deviceName << '\n';
    std::cout << "GPU type: " << string_VkPhysicalDeviceType(deviceProperties.deviceType) << '\n';
    std::cout << '\n';
    std::cout << "Required Device Extensions, that aren't supported by your GPU:" << '\n';
    for (const std::string& requiredExtensionName : requiredExtensions)
    {
        std::cout << '\t' << requiredExtensionName << '\n';
    }
    std::cout << '\n';

    // for testing
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        return true;

    return false;
}


bool BAV::VulkanApplication::IsDeviceSuitable(VkPhysicalDevice device)
{
    const QueueFamilyIndices indices = FindQueueFamilies(device);

    const bool areRequiredDeviceExtensionsSupported = DoesDeviceSupportRequiredExtensions(device);

    return indices.IsComplete() && areRequiredDeviceExtensionsSupported;
}

std::vector<const char*> BAV::VulkanApplication::GetRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;

    // Get Vulkan Extensions & ExtensionsCount
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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

BAV::QueueFamilyIndices BAV::VulkanApplication::FindQueueFamilies(VkPhysicalDevice device) const
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());


    // Get the right QueueFamily that has graphics_bit enabled
    int i = 0;
    for (const auto& queueFamilyProperty : queueFamilyProperties)
    {
        // Check for Graphic support
        if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.GraphicsFamily = i;
        }

        // Check for Present support
        VkBool32 hasPresentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i), m_Surface, &hasPresentSupport);
        if (hasPresentSupport)
        {
            indices.PresentFamily = i;
        }

        if (indices.IsComplete())
        {
            break;
        }

        ++i;
    }

    return indices;
}

