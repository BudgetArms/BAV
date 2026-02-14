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

    glm::vec3 testGlm = glm::vec3(0.0f, 1.0f, 2.0f);
    std::cout << testGlm.x << testGlm.y << testGlm.z << "\n";

    return 0;
}