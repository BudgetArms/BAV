#pragma once
#include <string>

class GLFWwindow;

namespace BAV
{
    class VulkanApplication
    {
    public:
        void Run();

    private:
        void InitVulkan();
        void MainLoop();
        void CleanUp();

        GLFWwindow* m_Window;

        const int m_Width{ 800 };
        const int m_Height{ 600 };
        const std::string m_Title{ "HelloTriangle" };

    };
}

