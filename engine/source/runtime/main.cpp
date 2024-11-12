#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

const uint32_t WINTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

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
    VkDebugUtilsMessengerEXT debug_messenger{};

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
        setup_debug_messenger();
    }

    auto main_loop() -> void
    {
        while(!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    auto clean_up() -> void
    {
        if (enable_validation_layers) {
            DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    static auto debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data
    ) -> VkBool32
    {
        if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: " << p_callback_data->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    auto present_all_available_extensions() -> void
    {
        auto extension_count = (uint32_t) 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> extensions{extension_count};
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

        std::cout << "available extensions: \n";
        for (const auto& extension: extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    auto get_required_extensions() -> std::vector<const char*>
    {
        auto glfw_extension_count = (uint32_t) 0;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        auto extensions = std::vector<const char*>{glfw_extensions, glfw_extensions + glfw_extension_count};

        if (enable_validation_layers) {
            extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    auto check_validation_layers_support() -> bool
    {
        auto layer_count = (uint32_t) 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        auto available_layers = std::vector<VkLayerProperties>{layer_count};
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (auto layer_name: validation_layers) {
            for (auto& layer_properties: available_layers) {
                if (strcmp(layer_name, layer_properties.layerName) == 0) {
                    return true;
                }
            }
        }

        return false;
    }

    auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info) {
        *create_info = {};
        create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info->pfnUserCallback = debug_callback;
    }

    auto create_instance() -> void
    {
        if (enable_validation_layers && !check_validation_layers_support()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        auto app_info = VkApplicationInfo{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        auto extensions = get_required_extensions();
        auto create_info = VkInstanceCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();
        auto debug_create_info = VkDebugUtilsMessengerCreateInfoEXT{};
        if (enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populate_debug_messenger_create_info(&debug_create_info);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_create_info;
        } else {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        present_all_available_extensions();

        auto result = vkCreateInstance(&create_info, nullptr, &instance);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create vulkan instance");
        }
    }

    auto setup_debug_messenger() -> void
    {
        if (!enable_validation_layers) return;

        auto create_info = VkDebugUtilsMessengerCreateInfoEXT{};
        populate_debug_messenger_create_info(&create_info);

        if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
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
