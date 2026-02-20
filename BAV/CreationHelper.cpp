#include "CreationHelper.hpp"

#include <stdexcept>


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

