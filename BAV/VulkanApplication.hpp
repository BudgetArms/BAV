#pragma once

#include <optional>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class GLFWwindow;

namespace BAV
{
    struct QueueFamilyIndices
    {
    public:
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        [[nodiscard]] bool IsComplete() const
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }

    };

    class VulkanApplication
    {
    public:
        void Run();


        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT             messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void*                                       pUserData
            );

    private:
        void InitWindow();


        void InitVulkan();
        void CreateInstance();
        void MainLoop();
        void CleanUp();

        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLocalDevice();


        [[nodiscard]] bool CheckValidationLayerSupport() const;
        [[nodiscard]] bool IsDeviceSuitable(VkPhysicalDevice device);

        static std::vector<const char*> GetRequiredExtensions();

        [[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;


        GLFWwindow* m_Window{};
        VkInstance m_Instance{};
        VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
        VkDevice m_LogicalDevice{ VK_NULL_HANDLE };
        VkDebugUtilsMessengerEXT m_DebugMessenger{};

        VkSurfaceKHR m_Surface { nullptr };

        // Queue
        VkQueue m_GraphicsQueue{ nullptr };
        VkQueue m_PresentQueue{ nullptr };


        const int m_Width{ 800 };
        const int m_Height{ 600 };
        const std::string m_Title{ "HelloTriangle" };

        const std::vector<std::string> m_ValidationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };


    };
}

