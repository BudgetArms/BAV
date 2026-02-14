#include <iostream>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// Include Visual Leak Detector
#if _DEBUG && __has_include(<vld.h>)
    #include <vld.h>
#endif


int main()
{
    std::cout << "Hello World!" << "\n";

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW!" << "\n";
        return 1;
    }

    // No clue what these arguments do
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create GLFW Window
    GLFWwindow* window = glfwCreateWindow(800.f, 600.f, "GLFW Window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << "\n";
        glfwTerminate();

        return 1;
    }

    // Get extension count
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << "Total Extensions: " << extensionCount << "\n";


    // Event loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // Destroy window
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}