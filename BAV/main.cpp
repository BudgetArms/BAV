#include <iostream>

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
        std::cerr << "Catched Error: "  << e.what() << '\n';
        std::cin.get();

        return 1;
    }


    return 0;
}
