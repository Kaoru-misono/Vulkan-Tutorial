#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

const uint32_t WINTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct Hello_Triangle_Application final
{
    auto run() -> void;

private:
    GLFWwindow* window{};
    VkSurfaceKHR surface{};
    VkSwapchainKHR swap_chain{};
    std::vector<VkImage> swap_chain_images{};
    std::vector<VkImageView> swap_chain_image_views{};
    std::vector<VkFramebuffer> swap_chain_framebuffers{};
    VkFormat swap_chain_image_format{};
    VkExtent2D swap_chain_extent{};
    VkRenderPass render_pass{};
    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};
    VkCommandPool command_pool{};
    std::vector<VkCommandBuffer> command_buffers{};

    VkInstance instance{};
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkDevice logical_device{};
    VkQueue graphics_queue{};
    VkQueue present_queue{};
    VkDebugUtilsMessengerEXT debug_messenger{};

    std::vector<VkSemaphore> image_available_semaphores{};
    std::vector<VkSemaphore> render_finished_semaphores{};
    std::vector<VkFence> in_flight_fences{};
    uint32_t current_frame{0};
    bool frame_buffer_resized{false};

    struct Queue_Family_Indices final
    {
        std::optional<uint32_t> graphics_family{};
        std::optional<uint32_t> present_family{};

        auto complete() -> bool {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct Swap_Chain_Support_Details final
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};
    };

    auto init_window() -> void;
    auto init_vulkan() -> void;
    auto main_loop() -> void;
    auto clean_up() -> void;

    auto create_instance() -> void;
    auto create_surface() -> void;
    auto pick_physical_device() -> void;
    auto create_logical_device() -> void;
    auto create_swap_chain() -> void;
    auto create_image_views() -> void;
    auto create_render_pass() -> void;
    auto create_graphics_pipeline() -> void;
    auto create_framebuffers() -> void;
    auto create_command_pool() -> void;
    auto create_command_buffers() -> void;
    auto create_sync_objects() -> void;

    auto draw_frame() -> void;

    auto create_shader_module(std::vector<unsigned char> const& code) -> VkShaderModule;
    auto record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) -> void;

    auto cleanup_swap_chain() -> void;
    auto recreate_swap_chain() -> void;

    auto find_queue_families(VkPhysicalDevice device) -> Queue_Family_Indices;
    auto check_device_extension_support(VkPhysicalDevice device) -> bool;
    auto query_swap_chain_support(VkPhysicalDevice device) -> Swap_Chain_Support_Details;
    auto is_device_suitable(VkPhysicalDevice device) -> bool;

    auto choose_swap_surface_format(std::vector<VkSurfaceFormatKHR> const& available_formats) -> VkSurfaceFormatKHR;
    auto choose_swap_present_mode(std::vector<VkPresentModeKHR> const& available_present_modes) -> VkPresentModeKHR;
    auto choose_swap_extent(VkSurfaceCapabilitiesKHR const& capabilities) -> VkExtent2D;

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

    static auto frame_buffer_resized_callback(GLFWwindow* window, int width, int height) -> void;
};
