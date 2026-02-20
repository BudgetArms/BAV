#include "CreationHelper.hpp"

#include <stdexcept>

#include "VulkanApplication.hpp"


VkResult BAV::CreationHelper::CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
    )
{

    // auto createDebugMessengerFunc = (PFN_vkCreateDebugUtilsMessengerEXT)
    auto createDebugMessengerFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (createDebugMessengerFunc == nullptr)
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    return createDebugMessengerFunc(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void BAV::CreationHelper::DestroyDebugUtilsMesengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
    )
{
    auto destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (!destroyDebugMessenger)
    {
        throw std::runtime_error("Couldn't destroy Debug Messenger");
    }

    destroyDebugMessenger(instance, debugMessenger, pAllocator);

}

void BAV::CreationHelper::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
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
    createInfo.pfnUserCallback = VulkanApplication::DebugCallback;

    // (optional), I don't really see a proper use-case for it
    // but that might change in the future
    createInfo.pUserData = nullptr;

}

