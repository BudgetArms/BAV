#pragma once
#include <string>

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

        GLFWwindow* m_Window{};
        VkInstance m_Instance{};

        const int m_Width{ 800 };
        const int m_Height{ 600 };
        const std::string m_Title{ "HelloTriangle" };

    };
}

