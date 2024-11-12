#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

const uint32_t WINTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};

struct Hello_Triangle_Application
{
    auto run() -> void;

private:
    GLFWwindow* window{};
    VkInstance instance{};
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkDevice device{};
    VkQueue graphics_queue{};
    VkDebugUtilsMessengerEXT debug_messenger{};

    auto init_window() -> void;
    auto init_vulkan() -> void;
    auto main_loop() -> void;
    auto clean_up() -> void;

    auto create_instance() -> void;
    auto pick_physical_device() -> void;
    auto create_logical_device() -> void;

    static auto debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* p_user_data
    ) -> VkBool32;
    auto populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info) -> void;
    auto setup_debug_messenger() -> void;

    auto present_all_available_extensions() -> void;
    auto get_required_extensions() -> std::vector<const char*>;
    auto check_validation_layers_support() -> bool;
};
