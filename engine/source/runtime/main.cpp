#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
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
    VkInstance instance{};

    auto init_window() -> void
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WINTH, HEIGHT, "Vulkan", nullptr, nullptr);

    }

    auto init_vulkan() -> void
    {
        create_instance();
    }

    auto main_loop() -> void
    {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    auto clean_up() -> void
    {
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    auto create_instance() -> void
    {
        auto app_info = VkApplicationInfo{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        auto glfw_extension_count = (uint32_t) 0;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        auto create_info = VkInstanceCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
        create_info.enabledLayerCount = 0;

        auto extension_count = (uint32_t) 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        std::cout << "availiable extensions: \n";
        for (const auto& extension: extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }

        auto result = vkCreateInstance(&create_info, nullptr, &instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create vulkan instance");
        }

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
