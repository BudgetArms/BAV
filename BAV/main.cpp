#include <iostream>
#include <GLFW/glfw3.h>

int main()
{
    std::cout << "Hello World!" << "\n";

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW!" << "\n";
        return 1;
    }

    return 0;
}