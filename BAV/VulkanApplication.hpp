#pragma once

#include <optional>
#include <string>
#include <vector>
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>


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
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );

    private:
        void InitWindow();


        void InitVulkan();
        void CreateInstance();
        void MainLoop();
        void DrawFrame();
        void CleanUp() const;

        void SetupDebugMessenger();
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLocalDevice();
        void CreateVulkanMemoryAllocator() const;
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateDescriptionSetLayout();
        void CreateGraphicsPipeline();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateVertexBuffer();
        void CreateTextureImage();
        void CreateIndexBuffer();
        void CreateUniformBuffers();
        void CreateDescriptorPool();
        void CreateDescriptorSets();
        void CreateCommandBuffers();
        void CreateSyncObjects();

        [[nodiscard]] VkShaderModule CreateShaderModule(const std::vector<char>& code) const;
        static void CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags,
                                 const VmaAllocationCreateInfo& allocationCreateInfo, VmaAllocation& allocation,
                                 VkBuffer& buffer);

        static void CreateStagingBuffer(VkDeviceSize bufferSize, VmaAllocation& allocation, VkBuffer& buffer);

        static void CreateImage(const VkImageCreateInfo& imageCreateInfo, VmaAllocation& allocation, VkImage& image);


        void RecreateSwapChain();
        void CleanUpSwapChain() const;
        void RecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32_t imageIndex) const;
        void CopyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size) const;
        void UpdateUniformBuffer(uint32_t currentImage) const;

        [[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;


        [[nodiscard]] bool CheckValidationLayerSupport() const;
        [[nodiscard]] bool DoesDeviceSupportRequiredExtensions(VkPhysicalDevice device) const;
        [[nodiscard]] bool IsDeviceSuitable(VkPhysicalDevice device) const;

        [[nodiscard]] static std::vector<const char*> GetRequiredExtensions();

        [[nodiscard]] QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
        [[nodiscard]] SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;

        [[nodiscard]] static VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& availableFormats);
        [[nodiscard]] static VkPresentModeKHR ChooseSwapChainPresentMode(
            const std::vector<VkPresentModeKHR>& availablePresentModes);
        [[nodiscard]] VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

        [[nodiscard]] static std::vector<char> ReadFile(const std::string& filename);

        GLFWwindow* m_Window{};
        VkInstance m_Instance{};
        VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
        VkDevice m_LogicalDevice{ VK_NULL_HANDLE };
        VkSwapchainKHR m_SwapChain{};
        VkRenderPass m_RenderPass{};
        VkDescriptorSetLayout m_DescriptorSetLayout{};
        VkPipelineLayout m_PipelineLayout{};
        VkPipeline m_GraphicsPipeline{};
        VkCommandPool m_CommandPool{};
        VkDescriptorPool m_DescriptorPool{};

        VkBuffer m_VertexBuffer{};
        VmaAllocation m_VertexBufferAllocation{};
        VkBuffer m_IndexBuffer{};
        VmaAllocation m_IndexBufferAllocation{};

        std::vector<VkBuffer> m_UniformBuffers{};
        std::vector<VmaAllocation> m_UniformBuffersAllocations{};

        std::vector<VkCommandBuffer> m_CommandBuffers{};
        std::vector<VkDescriptorSet> m_DescriptorSets{};

        // Fences & Semaphores
        std::vector<VkSemaphore> m_ImageAvailableSemaphores{};
        std::vector<VkSemaphore> m_RenderFinishedSemaphores{};
        std::vector<VkFence> m_InFlightFences{};

        VkDebugUtilsMessengerEXT m_DebugMessenger{};

        VkSurfaceKHR m_Surface{ nullptr };

        // Queue
        VkQueue m_GraphicsQueue{ nullptr };
        VkQueue m_PresentQueue{ nullptr };


        const int m_Width{ 800 };
        const int m_Height{ 600 };
        const std::string m_Title{ "HelloTriangle" };

        static constexpr bool m_bPrintWarnings{ true };

        int m_CurrentFrame{};


        // Swap Chain
        std::vector<VkImage> m_SwapChainImages{};
        std::vector<VkImageView> m_SwapChainImageViews{};
        VkFormat m_SwapChainImageFormat{};
        VkExtent2D m_SwapChainExtent{};
        std::vector<VkFramebuffer> m_SwapChainFramebuffers{};

        static constexpr VkFormat swapChainFormat{ VK_FORMAT_B8G8R8A8_SRGB };
        static constexpr VkColorSpaceKHR swapChainColorSpace{ VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

        static constexpr VkPresentModeKHR swapChainPresentMode{ VK_PRESENT_MODE_MAILBOX_KHR };
        static constexpr VkPresentModeKHR swapChainPresentModeDefault{ VK_PRESENT_MODE_FIFO_KHR };


        const std::vector<std::string> m_ValidationLayers =
        {
            "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<std::string> m_DeviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
        };
    };
}

