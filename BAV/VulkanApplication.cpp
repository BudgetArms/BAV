#include "VulkanApplication.hpp"

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <set>
#include <ios>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vk_enum_string_helper.h>
#include <glm/glm.hpp>

#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 1
#define VMA_CPP20 1
#include <vk_mem_alloc.h>


#include "ConversionHelpers.hpp"
#include "CreationHelper.hpp"


#ifdef NDEBUG
    constexpr bool g_bEnableValidationLayers = false;
#else
    constexpr bool g_bEnableValidationLayers = true;
#endif

#define FUNCTION_NAME __FUNCTION__

constexpr bool g_bEnableBrokenSynchronization = false;

constexpr bool g_UseSlangShaders = false;
constexpr int g_MaxFramesInFlight = 2;


VmaAllocator g_VmaAllocator = nullptr;


struct Vertex
{
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        constexpr VkVertexInputBindingDescription bindingDescription
        {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };


        // InputRate:
        // VK_VERTEX_INPUT_RATE_VERTEX:
        //     Move to the next data entry after each vertex
        // VK_VERTEX_INPUT_RATE_INSTANCE:
        //     Move to the next data entry after each instance

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> inputAttributeDescriptions
        {
            VkVertexInputAttributeDescription
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, position),
            },
            VkVertexInputAttributeDescription
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            }

        };

        return inputAttributeDescriptions;
    }

};

constexpr std::array<Vertex, 3> g_Vertices =
{
    Vertex({ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}),
    Vertex({ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}),
    Vertex({-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}),
};


void BAV::VulkanApplication::Run()
{
#ifdef NDEBUG
    std::cout << "RELEASE\n" << '\n';
#else
    std::cout << "DEBUG\n" << '\n';
#endif

    if constexpr(g_UseSlangShaders)
    {
        std::cout << "Using Slang Shaders" << '\n';
    }
    else
    {
        std::cout << "Using GLSL Shaders" << '\n';
    }

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

    CreateVulkanMemoryAllocator();

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateVertexBuffer();
    CreateCommandBuffers();
    CreateSyncObjects();
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

    VkInstanceCreateInfo instance
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
        instance.enabledLayerCount = static_cast<uint32_t>(charValidationLayers.size());
        instance.ppEnabledLayerNames = charValidationLayers.data();

        CreationHelper::PopulateDebugMessengerCreateInfo(debugCreateInfo);
        instance.pNext = &debugCreateInfo;
    }
    else
    {
        // If not debugging, don't add any validation layers
        instance.enabledLayerCount = 0;
        instance.pNext = nullptr;
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
    const VkResult result = vkCreateInstance(&instance, nullptr, &m_Instance);

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
        DrawFrame();
    }

    vkDeviceWaitIdle(m_LogicalDevice);
}

void BAV::VulkanApplication::DrawFrame()
{
    VkCommandBuffer currentCommandBuffer = m_CommandBuffers[m_CurrentFrame];
    const VkSemaphore imageAvailableSemaphore = m_ImageAvailableSemaphores[m_CurrentFrame];
    const VkFence currentFence = m_InFlightFences[m_CurrentFrame];

    // Wait for fences
    vkWaitForFences(m_LogicalDevice, 1, &currentFence, VK_TRUE, UINT64_MAX);

    // Reset fences
    vkResetFences(m_LogicalDevice, 1, &currentFence);

    uint32_t imageIndex{};
    vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChain, UINT64_MAX,
        imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    const VkSemaphore renderFinishedSemaphore = m_RenderFinishedSemaphores[imageIndex];
    // Timeout:
    //     if set to UINT64_MAX, it is disabled


    // Recording command buffer
    vkResetCommandBuffer(currentCommandBuffer, 0);
    RecordCommandBuffer(currentCommandBuffer, imageIndex);


    // Creation submit semaphores
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    if constexpr(g_bEnableBrokenSynchronization)
    {
        waitStages[0] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };

    // Submit command buffer
    const VkSubmitInfo submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = &currentCommandBuffer,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };


    const VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, currentFence);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to submit draw command buffer"));
    }


    // Presentation

    VkSwapchainKHR swapchains[] = { m_SwapChain };

    VkPresentInfoKHR presentInfo
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
        .pResults = nullptr,    // optional
    };


    vkQueuePresentKHR(m_PresentQueue, &presentInfo);

    // increment frame
    m_CurrentFrame = (m_CurrentFrame + 1) % g_MaxFramesInFlight;

}

void BAV::VulkanApplication::CleanUp() const
{
    if (g_bEnableValidationLayers)
    {
        CreationHelper::DestroyDebugUtilsMesengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }
    vmaDestroyBuffer(g_VmaAllocator, m_VertexBuffer, m_VertexBufferAllocation);
    vmaDestroyAllocator(g_VmaAllocator);

    for (size_t i = 0; i < g_MaxFramesInFlight; ++i)
    {
        vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_LogicalDevice, m_InFlightFences[i], nullptr);
    }

    for (const VkSemaphore semaphore : m_RenderFinishedSemaphores)
    {
        vkDestroySemaphore(m_LogicalDevice, semaphore, nullptr);
    }

    vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);

    vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);
    vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);

    CleanUpSwapChain();

    vkDestroyDevice(m_LogicalDevice, nullptr);

    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);

    glfwDestroyWindow(m_Window);

    glfwTerminate();
}


void BAV::VulkanApplication::SetupDebugMessenger()
{
    if (!g_bEnableValidationLayers)
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessenger{};
    CreationHelper::PopulateDebugMessengerCreateInfo(debugUtilsMessenger);

    const VkResult result = CreationHelper::CreateDebugUtilsMessengerEXT(m_Instance, &debugUtilsMessenger, nullptr,
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
    auto [GraphicsFamily, PresentFamily] = FindQueueFamilies(m_PhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamiliesIndex =
    {
        GraphicsFamily.value(),
        PresentFamily.value()
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


    if (GraphicsFamily.value() == PresentFamily.value())
    {
        vkGetDeviceQueue(m_LogicalDevice, GraphicsFamily.value(), queueIndex, &m_GraphicsQueue);
        m_PresentQueue = m_GraphicsQueue;
    }
    else
    {
        vkGetDeviceQueue(m_LogicalDevice, GraphicsFamily.value(), queueIndex, &m_GraphicsQueue);
        vkGetDeviceQueue(m_LogicalDevice, PresentFamily.value(), queueIndex, &m_PresentQueue);
    }

}

void BAV::VulkanApplication::CreateVulkanMemoryAllocator() const
{
    VmaAllocatorCreateInfo allocatorCreateInfo
    {
        .flags = 0,
        .physicalDevice = m_PhysicalDevice,
        .device = m_LogicalDevice,
        .preferredLargeHeapBlockSize = 0,   // will default to 256MiB
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = m_Instance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };

    const VkResult result = vmaCreateAllocator(&allocatorCreateInfo, &g_VmaAllocator);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create VMA"));
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
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    };

    if constexpr(g_bEnableBrokenSynchronization)
    {
        createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }


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

    VkResult result = vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_SwapChain);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain");
    }


    // Get SwapChain Images
    result = vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, nullptr);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get swap chain images (count)");
    }

    m_SwapChainImages.resize(imageCount);

    result = vkGetSwapchainImagesKHR(m_LogicalDevice, m_SwapChain, &imageCount, m_SwapChainImages.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to get swap chain images (vector)");
    }

    // Set SwapChain variables
    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;

}

void BAV::VulkanApplication::CreateImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (int i = 0; i < m_SwapChainImages.size(); ++i)
    {
        // The viewType parameter allows you to treat images as
        // 1D textures, 2D textures, 3D textures and cube maps.

        VkImageViewCreateInfo createInfo
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_SwapChainImages[i],

            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_SwapChainImageFormat,

            // .components =  = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components = VkComponentMapping
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },

            .subresourceRange = VkImageSubresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        const VkResult result = vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view");
        }

    }

}

void BAV::VulkanApplication::CreateRenderPass()
{
    // Color attachment:
    VkAttachmentDescription colorAttachment
    {
        .format = m_SwapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    if constexpr(g_bEnableBrokenSynchronization)
    {
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    // Samples:
    // Refers to multisampling samples,
    // since not using multisampling, is set to one

    // LoadOp:
    // VK_ATTACHMENT_LOAD_OP_LOAD: Preserve the existing contents of the attachment
    // VK_ATTACHMENT_LOAD_OP_CLEAR: Clear the values to a constant at the start
    // VK_ATTACHMENT_LOAD_OP_DONT_CARE: Existing contents are undefined; we don't care about them

    // StoreOp:
    // VK_ATTACHMENT_STORE_OP_STORE: Rendered contents will be stored in memory and can be read later
    // VK_ATTACHMENT_STORE_OP_DONT_CARE: Contents of the framebuffer will be undefined after the rendering

    // Stencil:
    // For our triangle example, stencil is not used, so we don't loading/storing
    // isn't important/irrelevant.

    // Layouts:

    // Initial layout:
    // specifies which layout the image will have, before the render pass begins
    // VK_IMAGE_LAYOUT_UNDEFINED: for intial layout, it means
    // we don't care about what the previous image layout was

    // Final layout:
    // specifies the layout to automatically transition to when the render pass finishes.
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: we want the image to be ready to presention using swapchain


    // Color attachment reference:
    VkAttachmentReference colorAttachmentReference
    {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    // Attachment:
    // is the index

    // Layout:
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: this gives the best performance


    // Subpass:
    VkSubpassDescription subpass
    {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
    };

    // Bind point:
    // We have to specify that this is a graphics subpass and not a
    // compute subpass, which might come in the future

    // Types of attachments that can be reference to subpass:
    // pInputAttachments:       Attachments that are read from a shader
    // pResolveAttachments:     Attachments used for multisampling color attachments
    // pDepthStencilAttachment: Attachment for depth and stencil data
    // pPreserveAttachments:    Attachments that are not used by this subpass,
    //                          but for which the data must be preserved

    VkSubpassDependency dependency
    {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,

        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
    };


    // Render pass:
    VkRenderPassCreateInfo renderPassCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency,
    };

    VkResult result = vkCreateRenderPass(m_LogicalDevice, &renderPassCreateInfo, nullptr, &m_RenderPass);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create render pass"));
    }

}

void BAV::VulkanApplication::CreateGraphicsPipeline()
{
    // Shader Stage
    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
    };

    VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkShaderModule slangShaderModule{};
    VkShaderModule vertShaderModule{};
    VkShaderModule fragShaderModule{};

    if constexpr(g_UseSlangShaders)
    {
        const std::vector<char> shaderCode = ReadFile("Shaders/RedTriangleSlang.spv");
        // const std::vector<char> shaderCode = ReadFile("Shaders/ShaderSlang.spv");
        slangShaderModule = CreateShaderModule(shaderCode);

        vertexShaderCreateInfo.module = slangShaderModule;
        vertexShaderCreateInfo.pName = "VertexMain";

        fragmentShaderCreateInfo.module = slangShaderModule;
        fragmentShaderCreateInfo.pName = "FragmentMain";

    }
    else
    {
        const std::vector<char> vertShaderCode = ReadFile("Shaders/ColorTriangleVert.spv");
        const std::vector<char> fragShaderCode = ReadFile("Shaders/ColorTriangleFrag.spv");

        // const std::vector<char> vertShaderCode = ReadFile("Shaders/RedTriangleVert.spv");
        // const std::vector<char> fragShaderCode = ReadFile("Shaders/RedTriangleFrag.spv");

        // const std::vector<char> vertShaderCode = ReadFile("Shaders/ShaderVert.spv");
        // const std::vector<char> fragShaderCode = ReadFile("Shaders/ShaderFrag.spv");

        vertShaderModule = CreateShaderModule(vertShaderCode);
        fragShaderModule = CreateShaderModule(fragShaderCode);

        vertexShaderCreateInfo.module = vertShaderModule;
        vertexShaderCreateInfo.pName = "main";

        fragmentShaderCreateInfo.module = fragShaderModule;
        fragmentShaderCreateInfo.pName = "main";
    }

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vertexShaderCreateInfo,
        fragmentShaderCreateInfo
    };

    // Dynamic State
    std::vector<VkDynamicState> dynamicStates
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    // SOA
    const auto bindingDescription = Vertex::GetBindingDescription();
    const auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };


    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // When using dynamic state the viewport & scissor don't need to be set now,
    // they will be set/changed during "drawing time"
    VkPipelineViewportStateCreateInfo viewportCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1

    };

    // if not using dynamic state, you would need to add this additional
    // information, since it will be immutable during "draw time"
    // viewportCreateInfo.pViewports = &viewport;
    // viewportCreateInfo.pScissors = &scissor;


    // If you want to use multiple viewports or scissors at the same time using an array,
    // you will need to set enable the GPU feature (logical device creation).


    // Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE, // if enabled, rasterizer passes nothing to the framebuffer
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,    // optional
        .depthBiasClamp = VK_FALSE,         // optional
        .depthBiasSlopeFactor = 0.0f,       // optional
        .lineWidth = 1.f,
    };

    // Depth Clamp:
    // if true, fragments clamped to the near and far planes; if false, they are discarded

    // Polygon Mode:
    // Anything other than fill will require a GPU feature to be enabled
    // VK_POLYGON_MODE_FILL : fill the area of the polygon with fragments
    // VK_POLYGON_MODE_LINE : polygon edges are drawn as lines
    // VK_POLYGON_MODE_POINT: polygon vertices are drawn as points

    // Line Width:
    // The maximum line width depends on the hardware
    // Any line width higher than 1.0f, requires enabling a GPU feature.

    // Cull mode:
    // Type of face culling, can be disabled

    // Front face:
    // Vertex order for faces to be front-facing.
    // Clockwise (CW) or counterclockwise (CCW)

    // Depth bias:
    // Depth value can be altered by adding with a constant value or value based on fragement's slope
    // This can be useful for shadow mapping,
    // since we aren't doing shawdow mapping, we disable depth bias.


    // Multisampling:
    // Requires enabling GPU feature

    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,           // optional
        .pSampleMask = nullptr,             // optional
        .alphaToCoverageEnable = VK_FALSE,  // optional
        .alphaToOneEnable = VK_FALSE,       // optional
    };

    // Will be discussed more detailed later on


    // Depth Stencil:
    VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_NEVER, // picked random compare operation
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},                // optional
        .back = {},                // optional
        .minDepthBounds = 0.0f,     // optional
        .maxDepthBounds = 1.0f,     // optional
    };

    // Will be discussed more detailed later on


    // Color blending:
    // Two methods:
    // -Mix old and new color value to get final color values
    // -Combine old and new using bitwise operation

    // Configuration per attached framebuffer
    VkPipelineColorBlendAttachmentState colorBlendAttachment
    {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,     // optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,    // optional
        .colorBlendOp = VK_BLEND_OP_ADD,                // optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,     // optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,    // optional
        .alphaBlendOp = VK_BLEND_OP_ADD,                 // optional
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT,
    };

    // Color blending pseudocode example:
    /*
    if (blendEnable)
    {
        finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <ColorBlendOp> (dstColorBlendFactor *
            oldColor.rgb);

        finalColor.a = (srcColorBlendFactor * newColor.a) <ColorBlendOp> (dstColorBlendFactor *
            oldColor.a);

    }
    else
    {
        finalColor = newColor;
    }

    finalColor = finalColor & colorWriteMask

    */

    // or use alpha blending
    /*
    finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
    finalColor.a = newALpha.a;
    */

    // and then we need to set colorblending attachment to
    /*
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    */

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants =
        {
            0.0f,
            0.0f,
            0.0f,
            0.0f,
        }
    };

    // for the other blending mode, the bitwise blending mode,
    // change logicOpEnable to true, this will disregard the
    // blend attachment for every attached framebuffer.


    // We have disabled both modes with
    // ColorBlendAttachment::blendEnabled set to false, and ColorBlendState::logicOpEnabled
    // this results in the fragment colors not being modified.
    // And then written to the framebuffer


    // Pipeline layout
    VkPipelineLayoutCreateInfo layoutCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,            // optional
        .pSetLayouts = nullptr,         // optional
        .pushConstantRangeCount = 0,    // optional
        .pPushConstantRanges = nullptr, // optional
    };

    VkResult result = vkCreatePipelineLayout(m_LogicalDevice, &layoutCreateInfo, nullptr, &m_PipelineLayout);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create pipeline layout"));
    }


    // Graphics Pipeline:
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,

        .pVertexInputState      = &vertexInputCreateInfo,
        .pInputAssemblyState    = &inputAssemblyCreateInfo,
        .pViewportState         = &viewportCreateInfo,
        .pRasterizationState    = &rasterizationCreateInfo,
        .pMultisampleState      = &multisampleCreateInfo,
        .pDepthStencilState     = &depthStencilCreateInfo,
        .pColorBlendState       = &colorBlendCreateInfo,
        .pDynamicState          = &dynamicStateCreateInfo,

        .layout     = m_PipelineLayout,
        .renderPass = m_RenderPass,
        .subpass    = 0,    // index of subpass

        .basePipelineHandle = VK_NULL_HANDLE,   // optional
        .basePipelineIndex = -1,                // optional
    };

    result = vkCreateGraphicsPipelines(m_LogicalDevice,
        VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr,
        &m_GraphicsPipeline);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create graphics pipeline"));
    }


    // Destroy shader module
    if constexpr(g_UseSlangShaders)
    {
        vkDestroyShaderModule(m_LogicalDevice, slangShaderModule, nullptr);
    }
    else
    {
        vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
        vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
    }



}

void BAV::VulkanApplication::CreateFramebuffers()
{
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

    for (size_t i = 0; i < m_SwapChainFramebuffers.size(); ++i)
    {
        VkImageView attachments[] =
        {
            m_SwapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferCreateInfo
        {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = m_SwapChainExtent.width,
            .height = m_SwapChainExtent.height,
            .layers = 1,
        };


        const VkResult result = vkCreateFramebuffer(m_LogicalDevice, &framebufferCreateInfo, nullptr,
            &m_SwapChainFramebuffers[i]);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(FUNCTION_NAME +
                std::string(" Failed to create framebuffer (index: " + std::to_string(i)
                + ")"));
        }

    }

}

void BAV::VulkanApplication::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

    VkCommandPoolCreateInfo commandPoolCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value(),
    };

    // flags:
    // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT:
    //     Hint that command buffers are rerecorded with new commands
    //     very often (may change memory allocation behavior)
    // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT:
    //     Allow command buffers to be rerecorded individually,
    //     without this flag they all have to be reset together

    VkResult result = vkCreateCommandPool(m_LogicalDevice, &commandPoolCreateInfo, nullptr, &m_CommandPool);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create command pool"));
    }


}

void BAV::VulkanApplication::CreateVertexBuffer()
{
    constexpr VkDeviceSize bufferSize = sizeof(Vertex) * g_Vertices.size();

    constexpr VkBufferUsageFlags stagingBufferUsageFlags
    {
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    constexpr VmaMemoryUsage stagingBufferMemoryUsage
    {
        VMA_MEMORY_USAGE_AUTO
    };

    constexpr VmaAllocationCreateFlags stagingBufferAllocationCreateInfo
    {
        VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };

    VkBuffer stagingBuffer{};
    VmaAllocation stagingBufferAllocation{};

    CreateBuffer(bufferSize, stagingBufferUsageFlags, stagingBufferMemoryUsage, stagingBufferAllocationCreateInfo,
        stagingBufferAllocation, stagingBuffer);


    // Add vertices data to staging buffer
    vmaCopyMemoryToAllocation(g_VmaAllocator, g_Vertices.data(), stagingBufferAllocation, 0, bufferSize);

    constexpr VkBufferUsageFlags vertexBufferUsageFlags
    {
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    constexpr VmaMemoryUsage vertexBufferMemoryUsage
    {
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    constexpr VmaAllocationCreateFlags vertexBufferAllocationCreateInfo
    {
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    };

    CreateBuffer(
        bufferSize,
        vertexBufferUsageFlags,
        vertexBufferMemoryUsage,
        vertexBufferAllocationCreateInfo,
        m_VertexBufferAllocation, m_VertexBuffer);

    CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

    vmaDestroyBuffer(g_VmaAllocator, stagingBuffer, stagingBufferAllocation);
}

void BAV::VulkanApplication::CreateCommandBuffers()
{
    m_CommandBuffers.resize(g_MaxFramesInFlight);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size()),
    };

    // level:
    // VK_COMMAND_BUFFER_LEVEL_PRIMARY:
    //     Can be submitted to a queue for execution, but cannot be called from other command buffers.
    // VK_COMMAND_BUFFER_LEVEL_SECONDARY:
    //     Cannot be submitted directly, but can be called from primary command buffers.

    // CommandBufferCount:
    // the amount of command buffers, since only using 1, we set this to one


    VkResult result = vkAllocateCommandBuffers(m_LogicalDevice,
        &commandBufferAllocateInfo, m_CommandBuffers.data());
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to allocate command buffers"));
    }

}

void BAV::VulkanApplication::CreateSyncObjects()
{
    m_ImageAvailableSemaphores.resize(g_MaxFramesInFlight);
    m_RenderFinishedSemaphores.resize(m_SwapChainImages.size());
    m_InFlightFences.resize(g_MaxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };


    // Create semaphores
    for (size_t i = 0; i < g_MaxFramesInFlight; ++i)
    {
        VkResult result = vkCreateSemaphore(m_LogicalDevice, &semaphoreCreateInfo, nullptr,
            &m_ImageAvailableSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create semaphore ImageAvailable, index: " + std::to_string(i)));
        }

        result = vkCreateFence(m_LogicalDevice, &fenceCreateInfo, nullptr, &m_InFlightFences[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create fence InFlight, index: " + std::to_string(i)));
        }
    }


    // signaled bit set on because otherwise the wait for fences
    // in DrawFrame will never be signaled
    for (size_t i = 0; i < m_SwapChainImages.size(); ++i)
    {
        const VkResult result = vkCreateSemaphore(m_LogicalDevice, &semaphoreCreateInfo, nullptr,
            &m_RenderFinishedSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create semaphore RenderFinsihed, index: " + std::to_string(i)));
        }
    }

}

VkShaderModule BAV::VulkanApplication::CreateShaderModule(const std::vector<char>& code) const
{
    const VkShaderModuleCreateInfo shaderModuleCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size() * sizeof(char),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    VkShaderModule shaderModule{};
    const VkResult result = vkCreateShaderModule(m_LogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to create shader module"));
    }

    return shaderModule;
}

void BAV::VulkanApplication::CreateBuffer(const VkDeviceSize size, const VkBufferUsageFlags usageFlags,
    const VmaMemoryUsage memoryUsage, const VmaAllocationCreateFlags allocationFlags,
    VmaAllocation& allocation, VkBuffer& buffer)
{
    const VkBufferCreateInfo bufferCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usageFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    const VmaAllocationCreateInfo allocationCreateInfo
    {
        .flags = allocationFlags,
        .usage = memoryUsage,
    };

    const VkResult result = vmaCreateBuffer(
        g_VmaAllocator,
        &bufferCreateInfo,
        &allocationCreateInfo,
        &buffer,
        &allocation,
        nullptr);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex buffer");
    }

}

void BAV::VulkanApplication::RecreateSwapChain()
{
    // Wait until logical device is idle, e.g. we don't
    // want to change stuff while it's in use
    vkDeviceWaitIdle(m_LogicalDevice);

    CleanUpSwapChain();

    // Render pass is not getting recreated for simplicity.
    // This should be done when window goes from standard range to
    // high dynamic range monitor (or the other way around, I think).
    CreateSwapChain();
    CreateImageViews();
    CreateFramebuffers();
}

void BAV::VulkanApplication::CleanUpSwapChain() const
{
    for (const auto frameBuffer : m_SwapChainFramebuffers)
    {
        vkDestroyFramebuffer(m_LogicalDevice, frameBuffer, nullptr);
    }

    for (const auto imageView : m_SwapChainImageViews)
    {
        vkDestroyImageView(m_LogicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_LogicalDevice, m_SwapChain, nullptr);

}

void BAV::VulkanApplication::RecordCommandBuffer(const VkCommandBuffer& commandBuffer, const uint32_t imageIndex) const
{
    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType =  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,                     // optional
        .pInheritanceInfo = nullptr,    // optional
    };

    // Flags:
    // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:
    //     The command buffer will be rerecorded right after executing it once.
    // VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT:
    //     This is a secondary command buffer that will be
    //     entirely within a single render pass.
    // VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:
    //     The command buffer can be resubmitted while it
    //     is also already pending execution.


    VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Failed to begin command buffer"));
    }

    VkClearValue clearColor = {0.f, 0.f, 0.f, 1.f};

    VkRenderPassBeginInfo renderPassBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_RenderPass,
        .framebuffer = m_SwapChainFramebuffers[imageIndex],
        .renderArea = { .offset = {0, 0}, .extent = m_SwapChainExtent },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };


    // Begin Render Pass:
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Subpasses content:
    // VK_SUBPASS_CONTENTS_INLINE:
    //     The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
    // VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS:
    //     The render pass commands will be executed from secondary command buffers.


    // Cmd begin pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);


    // Viewports and scissors

    // framebuffer uses swapchain images, and since swapchain's
    // image sizes can differ from WIDTH/HEIGHT, we should use swapchain's dimensions
    const VkViewport viewport
    {
        .x = 0.0f,
        .y = 0.f,
        .width =  static_cast<float>(m_SwapChainExtent.width),
        .height =  static_cast<float>(m_SwapChainExtent.height),
        .minDepth = 0.0f,   // TODO: why can this be higher than maxDepth according to docs
        .maxDepth = 1.0f,
    };

    const VkRect2D scissor
    {
        .offset = {0, 0},
        .extent = m_SwapChainExtent
    };


    // Set commands viewport & scissor
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


    // Bind buffer to pipeline
    constexpr VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer, offsets);
    vkCmdDraw(commandBuffer, static_cast<uint32_t>(g_Vertices.size()), 1, 0, 0);


    // Set command draw
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // End render pass
    vkCmdEndRenderPass(commandBuffer);

    // End command buffer
    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" failed to record command buffer"));
    }

}

void BAV::VulkanApplication::CopyBuffer(const VkBuffer sourceBuffer, const VkBuffer destinationBuffer,
    const VkDeviceSize size) const
{
    const VkCommandBufferAllocateInfo cmdBufferAllocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer{};
    VkResult result = vkAllocateCommandBuffers(m_LogicalDevice, &cmdBufferAllocateInfo, &commandBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" failed to allocate command buffer"));
    }

    constexpr VkCommandBufferBeginInfo cmdBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr,
    };

    result = vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" failed to begin command buffer"));
    }

    const VkBufferCopy copyRegion
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };

    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" failed to end copy command buffer"));
    }


    const VkSubmitInfo submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" failed to submit copy command buffer (graphics queue)"));
    }

    result = vkQueueWaitIdle(m_GraphicsQueue);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" failed to wait for graphics queue after buffer copy"));
    }

    vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);

}


bool BAV::VulkanApplication::CheckValidationLayerSupport() const
{
    uint32_t layerCount = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string(" Couldn't get layer count"));
    }

    std::vector<VkLayerProperties> availableLayers(layerCount);
    result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(FUNCTION_NAME + std::string("Couldn't get layer properties"));
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

    for (const auto& [extensionName, specVersion] : availableExtensions)
    {
        requiredExtensions.erase(extensionName);
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
    const QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device);

    const bool areRequiredDeviceExtensionsSupported = DoesDeviceSupportRequiredExtensions(device);

    bool isSwapChainSuitable = false;
    if (areRequiredDeviceExtensionsSupported)
    {
        const SwapChainSupportDetails swapChainSupportDetails = QuerySwapChainSupport(device);
        isSwapChainSuitable = !swapChainSupportDetails.Formats.empty() && !swapChainSupportDetails.PresentModes.empty();
    }

    return queueFamilyIndices.IsComplete() && areRequiredDeviceExtensionsSupported && isSwapChainSuitable;
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

std::vector<char> BAV::VulkanApplication::ReadFile(const std::string& filename)
{
    // ate: start reading from end of file
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::string errorMessage = std::string(" File (") + filename + std::string(") not found or unable to open");
        throw std::runtime_error( FUNCTION_NAME + errorMessage);
    }

    // std::ifstream::tellg() returns the current position of the input pointer.
    // It indicates where the next read operation will occur in the file.
    std::vector<char> buffer(file.tellg());


    // Read all bytes at once

    // std::ifstream::seek(...) sets input position indicator of streambuf object
    // beg: beginning of file
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();

    if (buffer.empty())
    {
        throw std::runtime_error("Buffer is empty");
    }


    // Close the file handle
    file.close();

    return buffer;
}

