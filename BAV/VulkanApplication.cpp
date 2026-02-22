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
    CreateSwapChain();
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

    vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);

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

    const std::vector<const char*> charDeviceExtensions = ConversionHelpers::StringVectorToCharVector(m_DeviceExtensions);

    VkDeviceCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),

        .enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size()),
        .ppEnabledExtensionNames = charDeviceExtensions.data(),

        .pEnabledFeatures = &deviceFeatures,
    };

    // There used to be a difference between Instance- and Device specific
    // validation layers, but now it's basically the same, so device's valdiation
    // layers are ignored in Vulkan, but that's not the case for older versions of Vulkan.

    // So, for Backwards compatibility, we set the same validation layers for the
    // device as we did for the instance.


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

void BAV::VulkanApplication::CreateSwapChain()
{
    const SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupport.Formats);
    const VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupport.PresentModes);
    const VkExtent2D extent = ChooseSwapChainExtent(swapChainSupport.Capabilities);

    // minImageCount, is not recommended, bc driver sometimes has to do internal operations
    // minImageCount + 1, is more recommended, bc then we can write to the other image
    uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

    imageCount = std::clamp(imageCount, static_cast<uint32_t>(0), swapChainSupport.Capabilities.maxImageCount);


    // The imageArrayLayers specifies the amount of layers
    // each image consists of. This is always 1 unless you
    // are developing a stereoscopic 3D application.

    // imageUsage: for postprocessing: VK_IMAGE_USAGE_TRANSFER_DST_BIT

    // Create SwapChain
    VkSwapchainCreateInfoKHR createInfo
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_Surface,

        .minImageCount = imageCount,
        .imageFormat = swapChainFormat,
        .imageColorSpace = swapChainColorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };



    // Image Sharing Mode:

    // VK_SHARING_MODE_EXCLUSIVE
    // An image is owned by one queue family at a time
    // and ownership must be explicitly transferred before
    // using it in another queue family. This option offers
    // the best performance.

    // VK_SHARING_MODE_CONCURRENT
    // Images can be used across multiple queue families
    // without explicit ownership transfers.


    const QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
    const uint32_t queueFamilyIndices[] = {indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    if (indices.GraphicsFamily.value() == indices.PresentFamily.value())
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;   // optional
        createInfo.pQueueFamilyIndices = nullptr;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }


    // Transforms to be added or not (e.g. 90degrees rotation or horizontal flip)
    createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;

    // Blending with other windows, I think this means transparent windows???
    // TODO: research this more indepth
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Set present mode
    createInfo.presentMode = presentMode;

    // If the clipped member is set to VK_TRUE then that means that
    // we don't care about the color of pixels that are obscured,
    // for example because another window is in front of them.
    // Unless you really need to be able to read these pixels
    // back and get predictable results, you'll get the best
    // performance by enabling clipping.

    // TODO: learn more about this so that I truely know what it means
    createInfo.clipped = VK_TRUE;


    // That leaves one last field, oldSwapchain. With Vulkan,
    // it's possible that your swap chain becomes invalid or
    // unoptimized while your application is running, for example
    // because the window was resized. In that case the swap chain
    // actually needs to be recreated from scratch and a reference
    // to the old one must be specified in this field. This is a
    // complex topic that we'll learn more about in a future chapter.
    // For now, we'll assume that we'll only ever create one swap chain.
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    const VkResult result = vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain");
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

    if (m_bPrintWarnings)
    {
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
    }

    return false;
}


bool BAV::VulkanApplication::IsDeviceSuitable(VkPhysicalDevice device) const
{
    const QueueFamilyIndices indices = FindQueueFamilies(device);

    const bool areRequiredDeviceExtensionsSupported = DoesDeviceSupportRequiredExtensions(device);

    bool isSwapChainSuitable = false;
    if (areRequiredDeviceExtensionsSupported)
    {
        const SwapChainSupportDetails swapChainSupportDetails = QuerySwapChainSupport(device);
        isSwapChainSuitable = !swapChainSupportDetails.Formats.empty() && !swapChainSupportDetails.PresentModes.empty();
    }

    return indices.IsComplete() && areRequiredDeviceExtensionsSupported && isSwapChainSuitable;
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

BAV::SwapChainSupportDetails BAV::VulkanApplication::QuerySwapChainSupport(VkPhysicalDevice device) const
{
    SwapChainSupportDetails details{};

    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get surface capabilities");
    }


    // Get formats
    uint32_t formatCount = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get surface formats (count)");
    }

    if (formatCount == 0)
    {
        throw std::runtime_error("Format count is zero");
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, formats.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get surface formats (vector)");
    }

    if (formats.empty())
    {
        throw std::runtime_error("Formats is empty");
    }

    // Set formats
    details.Formats = formats;


    // Get Present Modes
    uint32_t presentModesCount = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModesCount, nullptr);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get present modes (count)");
    }

    if (presentModesCount == 0)
    {
        throw std::runtime_error("Present Mode count is zero");
    }

    std::vector<VkPresentModeKHR> presentModes(presentModesCount);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModesCount, presentModes.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get present modes (vector)");
    }

    if (presentModes.empty())
    {
        throw std::runtime_error("Present Modes is empty");
    }


    // Set PresentModes
    details.PresentModes = presentModes;


    return details;
}

VkSurfaceFormatKHR BAV::VulkanApplication::ChooseSwapChainSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{

    for (const auto& format : availableFormats)
    {
        if (format.format == swapChainFormat
            && format.colorSpace == swapChainColorSpace)
        {
            return format;
        }
    }

    // since none of the formats are good enough, the first one will do.
    // Could start ranking them, but cannot be assed to do so.

    return availableFormats[0];
}

VkPresentModeKHR BAV::VulkanApplication::ChooseSwapChainPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    // VK_PRESENT_MODE_IMMEDIATE_KHR:
    // Images submitted by your application are transferred to
    // the screen right away, which may result in tearing.

    // VK_PRESENT_MODE_FIFO_KHR:
    // The swap chain is a queue where the display takes an image
    // from the front of the queue when the display is refreshed and
    // the program inserts rendered images at the back of the queue.
    // If the queue is full then the program has to wait. This is
    // most similar to vertical sync as found in modern games.
    // The moment that the display is refreshed is known as "vertical blank".

    // VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from
    // the previous one if the application is late and the queue was
    // empty at the last vertical blank. Instead of waiting for the
    // next vertical blank, the image is transferred right away when
    // it finally arrives. This may result in visible tearing.

    // VK_PRESENT_MODE_MAILBOX_KHR:
    // This is another variation of the second mode. Instead of blocking
    // the application when the queue is full, the images that are already
    // queued are simply replaced with the newer ones. This mode can be
    // used to render frames as fast as possible while still avoiding tearing,
    // resulting in fewer latency issues than standard vertical sync.
    // This is commonly known as "triple buffering", although the
    // existence of three buffers alone does not necessarily mean that the
    // framerate is unlocked.

    // PRESENT_MODE_MAILBOX: 'best' option
    // PRESENT_MODE_FIFO (always available): best for mobile bc energy efficient

    for (const auto& presentMode : availablePresentModes)
    {
        if (presentMode == swapChainPresentMode)
        {
            return presentMode;
        }
    }

    return swapChainPresentModeDefault;
}

VkExtent2D BAV::VulkanApplication::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    // Tutorial:
    // The swap extent is the resolution of the swap chain images, and
    // it's almost always exactly equal to the resolution of the window
    // that we're drawing to in pixels (more on that in a moment).
    // The range of the possible resolutions is defined in the
    // VkSurfaceCapabilitiesKHR structure. Vulkan tells us to match the
    // resolution of the window by setting the width and height in the
    // currentExtent member. However, some window managers do allow us
    // to differ here and this is indicated by setting the width and
    // height in currentExtent to a special value: the maximum value
    // of uint32_t. In that case we'll pick the resolution that best
    // matches the window within the minImageExtent and maxImageExtent
    // bounds. But we must specify the resolution in the correct unit.

    // My conclusion:
    // Vulkans says match the resolution of (GLFW) window,
    // I assume this was because screen cords and resolution
    // was (almost) always 1:1.

    // but window managers indicate that the resolution and
    // screen coordinates are NOT a 1:1 match by setting
    // the currentExtend's width and height to the maximum
    // value of uint32_t. This indicates to us that the
    // currentExtend should be 'fixed'


    // Monitor:
    // GLFW's width and height is not in pixels but in screen coordinates.
    // Pixels and screen coordinates map 1:1 on almost every display/monitor
    // But for example, the (Apple) Mac with Retina monitor, it's different.
    // Physical resolution: 5120 x 2880
    // Logical  resolution: 2560 x 1440


    // if window manager sets correct with/height, return it, manually set it
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()
        && capabilities.currentExtent.height != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width{};
        int height{};

        glfwGetFramebufferSize(m_Window, &width, &height);

        VkExtent2D actualExtent
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width
         = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

        actualExtent.height
         = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }

}

