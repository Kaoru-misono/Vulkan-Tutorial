#pragma once
#include "loader.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <array>
#include <vector>
#include <optional>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct Vertex final
{
    glm::vec3 position{};
    glm::vec4 color{};
    glm::vec2 tex_coord{};

    static auto get_binding_description() -> VkVertexInputBindingDescription
    {
        auto binding_description = VkVertexInputBindingDescription{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return binding_description;
    }

    static auto get_attribute_descriptions() -> std::array<VkVertexInputAttributeDescription, 3>
    {
        auto attribute_descriptions = std::array<VkVertexInputAttributeDescription, 3>{};
        attribute_descriptions[0].binding = 0;
        attribute_descriptions[0].location = 0;
        attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_descriptions[0].offset = offsetof(Vertex, position);
        attribute_descriptions[1].binding = 0;
        attribute_descriptions[1].location = 1;
        attribute_descriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attribute_descriptions[1].offset = offsetof(Vertex, color);
        attribute_descriptions[2].binding = 0;
        attribute_descriptions[2].location = 2;
        attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

        return attribute_descriptions;
    }
};

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
    VkDescriptorPool descriptor_pool{};
    VkDescriptorSetLayout descriptor_set_layout{};
    std::vector<VkDescriptorSet> descriptor_sets{};
    VkDescriptorPool compute_descriptor_pool{};
    VkDescriptorSetLayout compute_descriptor_set_layout{};
    std::vector<VkDescriptorSet> compute_descriptor_sets{};
    VkPipelineLayout pipeline_layout{};
    VkPipeline graphics_pipeline{};
    VkPipelineLayout pipeline_layout2{};
    VkPipeline graphics_pipeline2{};
    VkPipelineLayout compute_pipeline_layout{};
    VkPipeline compute_pipeline{};
    VkCommandPool command_pool{};
    std::vector<VkCommandBuffer> command_buffers{};
    std::vector<VkCommandBuffer> compute_command_buffers{};

    std::vector<VkBuffer> uniform_buffers{};
    std::vector<VkDeviceMemory> uniform_buffers_memory{};
    std::vector<void*> uniform_buffers_mapped{};
    std::vector<VkBuffer> shader_storage_buffers{};
    std::vector<VkDeviceMemory> shader_storage_buffers_memory{};

    VkImage depth_image{};
    VkDeviceMemory depth_image_memory{};
    VkImageView depth_image_view{};
    VkImage color_image{};
    VkDeviceMemory color_image_memory{};
    VkImageView color_image_view{};

    uint32_t texture_mip_levels{};
    VkSampleCountFlagBits msaa_samples = VK_SAMPLE_COUNT_1_BIT;

    Assimp_Model model{};
    std::vector<Vertex> model_vertices{};
    std::vector<uint32_t> model_indices{};
    VkBuffer vertex_buffer{};
    VkDeviceMemory vertex_buffer_memory{};
    VkBuffer index_buffer{};
    VkDeviceMemory index_buffer_memory{};
    VkImage texture_image{};
    VkDeviceMemory texture_image_memory{};
    VkImageView texture_image_view{};
    VkSampler texture_sampler{};

    VkInstance instance{};
    VkPhysicalDevice physical_device{VK_NULL_HANDLE};
    VkDevice logical_device{};
    VkQueue graphics_queue{};
    VkQueue compute_queue{};
    VkQueue present_queue{};
    VkDebugUtilsMessengerEXT debug_messenger{};

    std::vector<VkSemaphore> image_available_semaphores{};
    std::vector<VkSemaphore> render_finished_semaphores{};
    std::vector<VkFence> in_flight_fences{};
    std::vector<VkFence> compute_in_flight_fences{};
    std::vector<VkSemaphore> compute_finished_semaphores{};
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
    auto create_descriptor_set_layout() -> void;
    auto create_compute_descriptor_set_layout() -> void;
    auto create_graphics_pipeline() -> void;
    auto create_graphics_pipeline2() -> void;
    auto create_compute_pipeline() -> void;
    auto create_framebuffers() -> void;
    auto create_command_pool() -> void;
    auto create_color_resources() -> void;
    auto create_depth_resources() -> void;
    auto create_texture_image() -> void;
    auto create_texture_image_view() -> void;
    auto create_texture_sampler() -> void;
    auto load_models() -> void;
    auto create_vertex_buffer() -> void;
    auto create_index_buffer() -> void;
    auto create_uniform_buffers() -> void;
    auto create_shader_storage_buffers() -> void;
    auto create_descriptor_pool() -> void;
    auto create_compute_descriptor_pool() -> void;
    auto create_descriptor_sets() -> void;
    auto create_compute_descriptor_sets() -> void;
    auto create_command_buffers() -> void;
    auto create_compute_command_buffers() -> void;
    auto create_sync_objects() -> void;

    auto draw_frame() -> void;

    auto create_shader_module(std::vector<unsigned char> const& code) -> VkShaderModule;
    auto record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) -> void;
    auto record_compute_command_buffer(VkCommandBuffer command_buffer) -> void;
    auto create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory) -> void;
    auto copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void;
    auto copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) -> void;
    auto update_uniform_buffer(uint32_t current_image) -> void;
    auto create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkSampleCountFlagBits num_samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory) -> void;
    auto create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) -> VkImageView;
    auto begin_single_time_commands() -> VkCommandBuffer;
    auto end_single_time_commands(VkCommandBuffer command_buffer) -> void;
    auto transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) -> void;
    auto find_depth_format() -> VkFormat;
    auto has_stencil_component(VkFormat format) -> bool;
    auto generate_mipmaps(VkImage image, VkFormat image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels) -> void;

    auto cleanup_swap_chain() -> void;
    auto recreate_swap_chain() -> void;

    auto find_queue_families(VkPhysicalDevice device) -> Queue_Family_Indices;
    auto check_device_extension_support(VkPhysicalDevice device) -> bool;
    auto query_swap_chain_support(VkPhysicalDevice device) -> Swap_Chain_Support_Details;
    auto is_device_suitable(VkPhysicalDevice device) -> bool;
    auto find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) -> uint32_t;
    auto find_support_format(std::vector<VkFormat>* candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat;
    auto get_max_useable_sample_count() -> VkSampleCountFlagBits;

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
