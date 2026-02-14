#pragma once

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

    };
}

