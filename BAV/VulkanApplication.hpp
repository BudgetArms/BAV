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
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        [[nodiscard]] bool IsComplete() const
        {
            return GraphicsFamily.has_value() && PresentFamily.has_value();
        }

    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;

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
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateGraphicsPipeline();

        [[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& code) const;

        void RecreateSwapChain();
        void CleanUpSwapChain() const;

        // TODO: add the swap chain recreation functions related
        // to drawFrame & Frame buffers, when both are added in the future


        [[nodiscard]] bool CheckValidationLayerSupport() const;
        [[nodiscard]] bool DoesDeviceSupportRequiredExtensions(VkPhysicalDevice device) const;
        [[nodiscard]] bool IsDeviceSuitable(VkPhysicalDevice device) const;

        [[nodiscard]] static std::vector<const char*> GetRequiredExtensions();

        [[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
        [[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

        [[nodiscard]] VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        [[nodiscard]] VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        [[nodiscard]] VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        [[nodiscard]] static std::vector<char> ReadFile(const std::string& filename);

        GLFWwindow* m_Window{};
        VkInstance m_Instance{};
        VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
        VkDevice m_LogicalDevice{ VK_NULL_HANDLE };
        VkSwapchainKHR m_SwapChain{};
        VkRenderPass m_RenderPass{};
        VkPipelineLayout m_PipelineLayout{};
        VkPipeline m_GraphicsPipeline{};
        VkDebugUtilsMessengerEXT m_DebugMessenger{};

        VkSurfaceKHR m_Surface { nullptr };

        // Queue
        VkQueue m_GraphicsQueue{ nullptr };
        VkQueue m_PresentQueue{ nullptr };


        const int m_Width{ 800 };
        const int m_Height{ 600 };
        const std::string m_Title{ "HelloTriangle" };

        static constexpr bool m_bPrintWarnings{ true };

        // Swap Chain
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};
        VkFormat m_SwapChainImageFormat{};
        VkExtent2D m_SwapChainExtent{};

        static constexpr VkFormat swapChainFormat{ VK_FORMAT_B8G8R8A8_SRGB };
        static constexpr VkColorSpaceKHR swapChainColorSpace{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        static constexpr VkPresentModeKHR swapChainPresentMode{ VK_PRESENT_MODE_MAILBOX_KHR };
        static constexpr VkPresentModeKHR swapChainPresentModeDefault { VK_PRESENT_MODE_FIFO_KHR };


        const std::vector<std::string> m_ValidationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };

        const  std::vector<std::string> m_DeviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        };


    };
}

