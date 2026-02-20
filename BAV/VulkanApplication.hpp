#pragma once
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class GLFWwindow;

namespace BAV
{
    class VulkanApplication
    {
    public:
        void Run();


    private:
        void InitVulkan();
        void CreateInstance();
        void MainLoop();
        void CleanUp();

        void SetupDebugMessenger();

        bool CheckValidationLayerSupport() const;

        static std::vector<const char*> GetRequiredExtensions();

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT             messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*                                       pUserData
            );

        GLFWwindow* m_Window{};
        VkInstance m_Instance{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};

        const int m_Width{ 800 };
        const int m_Height{ 600 };
        const std::string m_Title{ "HelloTriangle" };

        const std::vector<std::string> m_ValidationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };


    };
}

