#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan.hpp>

#include "VulkanApplication.hpp"


int main()
{
    BAV::VulkanApplication application;

    try
    {
        application.Run();
        
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: "  << e.what() << '\n';
        return 1;
    }


    return 0;
}
