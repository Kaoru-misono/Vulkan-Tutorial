#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WINTH = 800;
const uint32_t HEIGHT = 600;

struct Hello_Triangle_Application
{
    auto run() -> void
    {
        init_window();
        init_vulkan();
        main_loop();
        clean_up();
    }

private:
    GLFWwindow* window{};

    auto init_window() -> void
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WINTH, HEIGHT, "Vulkan", nullptr, nullptr);

    }

    auto init_vulkan() -> void
    {

    }

    auto main_loop() -> void
    {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    auto clean_up() -> void
    {
        glfwDestroyWindow(window);

        glfwTerminate();
    }
};

int main()
{
    Hello_Triangle_Application app{};

    try {
        app.run();
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
