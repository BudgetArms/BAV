#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

int main()
{
    std::cout << "Hello World!" << "\n";

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW!" << "\n";
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(800.f, 600.f, "GLFW Window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << "\n";
        glfwTerminate();

        return 1;
    }

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