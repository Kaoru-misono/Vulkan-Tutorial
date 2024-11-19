#include "engine.hpp"
#include "triangle_vert.h"
#include "triangle_frag.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <set>
#include <limits>
#include <algorithm>
#include <chrono>
#include <filesystem>

inline namespace
{
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

    struct Uniform_Buffer_Object
    {
        glm::mat4 model_matrix{};
        glm::mat4 view_matrix{};
        glm::mat4 projection_matrix{};
    };

    auto start_time = std::chrono::high_resolution_clock::now();

    std::string bin_dir = std::filesystem::current_path().string();

    auto get_source_dir = [](std::string const& bin_dir) -> std::string
    {
        auto ps = bin_dir.rfind("Vulkan-Tutorial");

        auto source_path = bin_dir.substr(0, ps);

        replace(source_path.begin(), source_path.end(), '\\', '/');
        return source_path;
    };

    auto source_dir = get_source_dir(bin_dir);

    auto asset_dir = source_dir + "Vulkan-Tutorial/engine/asset/";
}

auto Hello_Triangle_Application::run() -> void
{
    init_window();

    init_vulkan();

    main_loop();

    clean_up();
}

auto Hello_Triangle_Application::init_window() -> void
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    //glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WINTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frame_buffer_resized_callback);

}

auto Hello_Triangle_Application::init_vulkan() -> void
{
    create_instance();

    create_surface();

    setup_debug_messenger();

    pick_physical_device();

    create_logical_device();

    create_swap_chain();

    create_image_views();

    create_render_pass();

    create_descriptor_set_layout();

    create_graphics_pipeline();

    create_command_pool();

    create_depth_resources();

    create_framebuffers();

    load_models();

    create_texture_image();

    create_texture_image_view();

    create_texture_sampler();

    create_vertex_buffer();

    create_index_buffer();

    create_uniform_buffers();

    create_descriptor_pool();

    create_descriptor_sets();

    create_command_buffers();

    create_sync_objects();
}

auto Hello_Triangle_Application::load_models() -> void
{
    auto model_path = asset_dir + "viking_room.obj";
    model = load_model(model_path);
    auto& first_mesh = *model.meshes.begin();
    auto& position = first_mesh.vertex_info.position;
    auto& tex_coord = first_mesh.vertex_info.texcoord;
    auto& color = first_mesh.vertex_info.color;
    auto& indices = first_mesh.topology;
    // for (auto& mesh: changli.meshes) {
    //     for (auto& indice: mesh.topology) {
    //         std::cout << "tri idx: " << indice.a << ", " << indice.b << ", " << indice.c << std::endl;
    //     }
    // }
    for (auto triangle: indices) {
        model_indices.emplace_back(triangle.a);
        model_indices.emplace_back(triangle.b);
        model_indices.emplace_back(triangle.c);
    }

    for (auto i = (size_t) 0; i < position.size(); i++) {
        auto vertex = Vertex{};
        vertex.position = position[i];
        vertex.tex_coord = tex_coord[i];
        model_vertices.emplace_back(vertex);
    }
}

auto Hello_Triangle_Application::main_loop() -> void
{
    while(!glfwWindowShouldClose(window)) {
        draw_frame();
        glfwPollEvents();
    }

    vkDeviceWaitIdle(logical_device);
}

auto Hello_Triangle_Application::clean_up() -> void
{
    for (auto i = (size_t) 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(logical_device, image_available_semaphores[i], nullptr);
        vkDestroySemaphore(logical_device, render_finished_semaphores[i], nullptr);
        vkDestroyFence(logical_device, in_flight_fences[i], nullptr);
    }

    vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);

    for (auto i = (size_t) 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(logical_device, uniform_buffers[i], nullptr);
        vkFreeMemory(logical_device, uniform_buffers_memory[i], nullptr);
    }

    vkFreeMemory(logical_device, index_buffer_memory, nullptr);
    vkDestroyBuffer(logical_device, index_buffer, nullptr);

    vkFreeMemory(logical_device, vertex_buffer_memory, nullptr);
    vkDestroyBuffer(logical_device, vertex_buffer, nullptr);

    vkDestroySampler(logical_device, texture_sampler, nullptr);

    vkDestroyImageView(logical_device, texture_image_view, nullptr);
    vkDestroyImage(logical_device, texture_image, nullptr);
    vkFreeMemory(logical_device, texture_image_memory, nullptr);

    vkDestroyImageView(logical_device, depth_image_view, nullptr);
    vkDestroyImage(logical_device, depth_image, nullptr);
    vkFreeMemory(logical_device, depth_image_memory, nullptr);

    for (auto framebuffer: swap_chain_framebuffers) {
        vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
    }

    vkDestroyCommandPool(logical_device, command_pool, nullptr);

    vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);

    vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);

    vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr);

    vkDestroyRenderPass(logical_device, render_pass, nullptr);

    for (auto image_view: swap_chain_image_views) {
        vkDestroyImageView(logical_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);

    vkDestroyDevice(logical_device, nullptr);

    if (enable_validation_layers) {
        DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

auto Hello_Triangle_Application::create_instance() -> void
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

    auto result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create vulkan instance");
    }
}

auto Hello_Triangle_Application::create_surface() -> void
{
    auto result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

auto Hello_Triangle_Application::pick_physical_device() -> void
{
    auto device_count = (uint32_t) 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    auto devices = std::vector<VkPhysicalDevice>{device_count};
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    for (auto const& device: devices) {
        if (is_device_suitable(device)) {
            physical_device = device;
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

auto Hello_Triangle_Application::create_logical_device() -> void
{
    auto indices = find_queue_families(physical_device);

    auto queue_create_infos = std::vector<VkDeviceQueueCreateInfo>{};
    auto unique_queue_families = std::set<uint32_t>{indices.graphics_family.value(), indices.present_family.value()};

    auto queue_priority = 1.0f;
    for (auto queue_family: unique_queue_families) {
        auto queue_create_info = VkDeviceQueueCreateInfo{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = indices.graphics_family.value();
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.emplace_back(std::move(queue_create_info));
    }

    auto device_features = VkPhysicalDeviceFeatures{};
    device_features.samplerAnisotropy = VK_TRUE;
    auto create_info = VkDeviceCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();
    if (enable_validation_layers) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    auto result = vkCreateDevice(physical_device, &create_info, nullptr, &logical_device);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(logical_device, indices.graphics_family.value(), 0, &graphics_queue);
    vkGetDeviceQueue(logical_device, indices.present_family.value(), 0, &present_queue);
}

auto Hello_Triangle_Application::create_swap_chain() -> void
{
    auto swap_chain_support = query_swap_chain_support(physical_device);

    auto surface_format = choose_swap_surface_format(swap_chain_support.formats);
    auto present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
    auto extent = choose_swap_extent(swap_chain_support.capabilities);

    auto image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    auto create_info = VkSwapchainCreateInfoKHR{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = find_queue_families(physical_device);
    uint32_t queue_family_indices[] = {indices.graphics_family.value(), indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    // If you want to create a new swap chain while drawing commands on an image
    // from the old swap chain are still in-flight, please set it
    create_info.oldSwapchain = VK_NULL_HANDLE;

    auto result = vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swap_chain);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, nullptr);
    swap_chain_images.resize(image_count);
    vkGetSwapchainImagesKHR(logical_device, swap_chain, &image_count, swap_chain_images.data());

    swap_chain_image_format = surface_format.format;
    swap_chain_extent = extent;
}

auto Hello_Triangle_Application::create_image_views() -> void
{
    swap_chain_image_views.resize(swap_chain_images.size());

    for (auto i = (size_t) 0; i < swap_chain_images.size(); i++) {
        swap_chain_image_views[i] = create_image_view(swap_chain_images[i], swap_chain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

auto Hello_Triangle_Application::create_render_pass() -> void
{
    auto color_attachment = VkAttachmentDescription{};
    color_attachment.format = swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    auto color_attachment_ref = VkAttachmentReference{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    auto depth_attachment = VkAttachmentDescription{};
    depth_attachment.format = find_depth_format();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto depth_attachment_ref = VkAttachmentReference{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    auto subpass = VkSubpassDescription{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount =1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    auto subpass_dependency = VkSubpassDependency {};
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dependency.dstSubpass = 0;
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.srcAccessMask = 0;
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    auto attachments = std::array<VkAttachmentDescription, 2>{color_attachment, depth_attachment};
    auto render_pass_create_info = VkRenderPassCreateInfo{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    auto result = vkCreateRenderPass(logical_device, &render_pass_create_info, nullptr, &render_pass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create rnder pass!");
    }
}

auto Hello_Triangle_Application::create_descriptor_set_layout() -> void
{
    auto ubo_layout_binding = VkDescriptorSetLayoutBinding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = nullptr;

    auto sampler_layout_binding = VkDescriptorSetLayoutBinding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    sampler_layout_binding.pImmutableSamplers = nullptr;

    auto bindings = std::array<VkDescriptorSetLayoutBinding, 2>{ubo_layout_binding, sampler_layout_binding};

    auto descriptor_set_layout_create_info = VkDescriptorSetLayoutCreateInfo{};
    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptor_set_layout_create_info.pBindings = bindings.data();

    auto result = vkCreateDescriptorSetLayout(logical_device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

auto Hello_Triangle_Application::create_graphics_pipeline() -> void
{
    auto vert_shader_module = create_shader_module(TRIANGLE_VERT);
    auto frag_shader_module = create_shader_module(TRIANGLE_FRAG);

    auto vert_shader_stage_info = VkPipelineShaderStageCreateInfo{};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    auto frag_shader_stage_info = VkPipelineShaderStageCreateInfo{};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    auto shader_stages = std::vector<VkPipelineShaderStageCreateInfo>{vert_shader_stage_info, frag_shader_stage_info};

    auto dynamic_states = std::vector<VkDynamicState>{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    auto dynamic_state_create_info = VkPipelineDynamicStateCreateInfo{};
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state_create_info.pDynamicStates = dynamic_states.data();

    auto viewport = VkViewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swap_chain_extent.width;
    viewport.height = (float) swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    auto scissor = VkRect2D{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;

    auto viewport_state_create_info = VkPipelineViewportStateCreateInfo{};
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;
    viewport_state_create_info.pScissors = &scissor;

    auto binding_description = Vertex::get_binding_description();
    auto attribute_descriptions = Vertex::get_attribute_descriptions();

    auto vertex_input_info = VkPipelineVertexInputStateCreateInfo{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

    auto input_assembly = VkPipelineInputAssemblyStateCreateInfo{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    auto rasterization_state_create_info = VkPipelineRasterizationStateCreateInfo{};
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_create_info.depthClampEnable = VK_FALSE;
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_create_info.lineWidth = 1.0f;
    rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    rasterization_state_create_info.depthBiasClamp = 0.0f;
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

    auto multisampling_state_create_info = VkPipelineMultisampleStateCreateInfo{};
    multisampling_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling_state_create_info.sampleShadingEnable = VK_FALSE;
    multisampling_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_state_create_info.minSampleShading = 1.0f;
    multisampling_state_create_info.pSampleMask = nullptr;
    multisampling_state_create_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_state_create_info.alphaToOneEnable = VK_FALSE;

    auto color_blend_attachment = VkPipelineColorBlendAttachmentState{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    auto color_blend_state_create_info = VkPipelineColorBlendStateCreateInfo{};
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    auto depth_stencil_state_create_info = VkPipelineDepthStencilStateCreateInfo{};
    depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_state_create_info.depthTestEnable = VK_TRUE;
    depth_stencil_state_create_info.depthWriteEnable = VK_TRUE;
    depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
    depth_stencil_state_create_info.minDepthBounds = 0.0f;
    depth_stencil_state_create_info.maxDepthBounds = 1.0f;
    depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
    depth_stencil_state_create_info.front = {};
    depth_stencil_state_create_info.back = {};

    auto pipeline_layout_create_info = VkPipelineLayoutCreateInfo{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.setLayoutCount = 1;
    pipeline_layout_create_info.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    auto layout_result = vkCreatePipelineLayout(logical_device, &pipeline_layout_create_info, nullptr, &pipeline_layout);
    if (layout_result != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    auto pipeline_create_info = VkGraphicsPipelineCreateInfo{};
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = shader_stages.data();
    pipeline_create_info.pVertexInputState = &vertex_input_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisampling_state_create_info;
    pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    pipeline_create_info.layout = pipeline_layout;
    pipeline_create_info.renderPass = render_pass;
    pipeline_create_info.subpass = 0;
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_create_info.basePipelineIndex = -1;

    auto pipeline_result = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics_pipeline);
    if (pipeline_result != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(logical_device, vert_shader_module, nullptr);
    vkDestroyShaderModule(logical_device, frag_shader_module, nullptr);
}

auto Hello_Triangle_Application::create_framebuffers() -> void
{
    swap_chain_framebuffers.resize(swap_chain_image_views.size());

    for (auto i = (size_t) 0; i < swap_chain_image_views.size(); i++) {
        auto attachments = std::vector<VkImageView>{swap_chain_image_views[i], depth_image_view};

        auto framebuffer_create_info = VkFramebufferCreateInfo{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render_pass;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_create_info.pAttachments = attachments.data();
        framebuffer_create_info.width = swap_chain_extent.width;
        framebuffer_create_info.height = swap_chain_extent.height;
        framebuffer_create_info.layers = 1;

        auto result = vkCreateFramebuffer(logical_device, &framebuffer_create_info, nullptr, &swap_chain_framebuffers[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

auto Hello_Triangle_Application::create_command_pool() -> void
{
    auto queue_family_indices = find_queue_families(physical_device);

    auto command_pool_create_info = VkCommandPoolCreateInfo{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    auto result = vkCreateCommandPool(logical_device, &command_pool_create_info, nullptr, &command_pool);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

auto Hello_Triangle_Application::create_depth_resources() -> void
{
    auto depth_format = find_depth_format();

    create_image(
        swap_chain_extent.width,
        swap_chain_extent.height,
        1,
        depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &depth_image,
        &depth_image_memory
    );
    depth_image_view = create_image_view(depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

auto Hello_Triangle_Application::create_texture_image() -> void
{
    auto texture_width = 0;
    auto texture_height = 0;
    auto texture_channels = 0;
    auto texture_path = asset_dir + "viking_room.png";
    auto pixels = stbi_load(texture_path.c_str(), &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    auto image_size = (VkDeviceSize) texture_width * texture_height * 4;
    texture_mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture_width, texture_height)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    auto staging_buffer = VkBuffer{};
    auto staging_buffer_memory = VkDeviceMemory{};

    create_buffer(
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_buffer_memory
    );

    auto data = (void*) nullptr;
    vkMapMemory(logical_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(logical_device, staging_buffer_memory);

    stbi_image_free(pixels);

    create_image(
        texture_width,
        texture_height,
        texture_mip_levels,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &texture_image,
        &texture_image_memory
    );

    transition_image_layout(texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture_mip_levels);
    copy_buffer_to_image(staging_buffer, texture_image, static_cast<uint32_t>(texture_width), static_cast<uint32_t>(texture_height));
    //transition_image_layout(texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, texture_mip_levels);
    generate_mipmaps(texture_image, VK_FORMAT_R8G8B8A8_SRGB, texture_width, texture_height, texture_mip_levels);

    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

auto Hello_Triangle_Application::create_texture_image_view() -> void
{
    texture_image_view = create_image_view(texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, texture_mip_levels);
}

auto Hello_Triangle_Application::create_texture_sampler() -> void
{
    auto sampler_create_info = VkSamplerCreateInfo{};
    sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter = VK_FILTER_LINEAR;
    sampler_create_info.minFilter = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable = VK_TRUE;
    auto properties = VkPhysicalDeviceProperties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    sampler_create_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable = VK_FALSE;
    sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias = 0.0f;
    sampler_create_info.minLod = 0.0f;
    sampler_create_info.maxLod = static_cast<float>(texture_mip_levels);

    auto result = vkCreateSampler(logical_device, &sampler_create_info, nullptr, &texture_sampler);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

auto Hello_Triangle_Application::create_vertex_buffer() -> void
{
    auto buffer_size = sizeof(model_vertices[0]) * model_vertices.size();

    auto staging_buffer = VkBuffer{};
    auto staging_buffer_memory = VkDeviceMemory{};
    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_buffer_memory
    );

    auto data = (void*) nullptr;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model_vertices.data(), (size_t) buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &vertex_buffer,
        &vertex_buffer_memory
    );

    copy_buffer(staging_buffer, vertex_buffer, buffer_size);
    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

auto Hello_Triangle_Application::create_index_buffer() -> void
{
    auto buffer_size = sizeof(model_indices[0]) * model_indices.size();

    auto staging_buffer = VkBuffer{};
    auto staging_buffer_memory = VkDeviceMemory{};
    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_buffer_memory
    );

    auto data = (void*) nullptr;
    vkMapMemory(logical_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, model_indices.data(), (size_t) buffer_size);
    vkUnmapMemory(logical_device, staging_buffer_memory);

    create_buffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &index_buffer,
        &index_buffer_memory
    );

    copy_buffer(staging_buffer, index_buffer, buffer_size);
    vkDestroyBuffer(logical_device, staging_buffer, nullptr);
    vkFreeMemory(logical_device, staging_buffer_memory, nullptr);
}

auto Hello_Triangle_Application::create_uniform_buffers() -> void
{
    auto buffer_size = sizeof(Uniform_Buffer_Object);

    uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
    uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (auto i = (size_t) 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        create_buffer(
            buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &uniform_buffers[i],
            &uniform_buffers_memory[i]
        );
        vkMapMemory(logical_device, uniform_buffers_memory[i], 0, buffer_size, 0, &uniform_buffers_mapped[i]);
    }
}

auto Hello_Triangle_Application::create_descriptor_pool() -> void
{
    auto pool_sizes = std::array<VkDescriptorPoolSize, 2>{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    auto descriptor_pool_create_info = VkDescriptorPoolCreateInfo{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_create_info.pPoolSizes = pool_sizes.data();
    descriptor_pool_create_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    auto result = vkCreateDescriptorPool(logical_device, &descriptor_pool_create_info, nullptr, &descriptor_pool);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

auto Hello_Triangle_Application::create_descriptor_sets() -> void
{
    auto layouts = std::vector<VkDescriptorSetLayout>{MAX_FRAMES_IN_FLIGHT, descriptor_set_layout};

    auto descriptor_set_allocate_info = VkDescriptorSetAllocateInfo{};
    descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_allocate_info.descriptorPool = descriptor_pool;
    descriptor_set_allocate_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptor_set_allocate_info.pSetLayouts = layouts.data();

    descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
    auto result = vkAllocateDescriptorSets(logical_device, &descriptor_set_allocate_info, descriptor_sets.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor sets!");
    }

    for (auto i = (size_t) 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto buffer_info = VkDescriptorBufferInfo{};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(Uniform_Buffer_Object);

        auto image_info = VkDescriptorImageInfo{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = texture_image_view;
        image_info.sampler = texture_sampler;

        auto write_descriptor_sets = std::array<VkWriteDescriptorSet, 2>{};
        write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[0].dstSet = descriptor_sets[i];
        write_descriptor_sets[0].dstBinding = 0;
        write_descriptor_sets[0].dstArrayElement = 0;
        write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_sets[0].descriptorCount = 1;
        write_descriptor_sets[0].pBufferInfo = &buffer_info;
        write_descriptor_sets[0].pImageInfo = nullptr;
        write_descriptor_sets[0].pTexelBufferView = nullptr;

        write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[1].dstSet = descriptor_sets[i];
        write_descriptor_sets[1].dstBinding = 1;
        write_descriptor_sets[1].dstArrayElement = 0;
        write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_sets[1].descriptorCount = 1;
        write_descriptor_sets[1].pBufferInfo = nullptr;
        write_descriptor_sets[1].pImageInfo = &image_info;
        write_descriptor_sets[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }
}

auto Hello_Triangle_Application::create_command_buffers() -> void
{
    command_buffers.resize(MAX_FRAMES_IN_FLIGHT);
    auto allocate_info = VkCommandBufferAllocateInfo{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = (uint32_t) command_buffers.size();

    auto result = vkAllocateCommandBuffers(logical_device, &allocate_info, command_buffers.data());
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create command buffer!");
    }
}

auto Hello_Triangle_Application::create_sync_objects() -> void
{
    image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
    in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

    auto semaphore_create_info = VkSemaphoreCreateInfo{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    auto fence_create_info = VkFenceCreateInfo{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto i = (size_t) 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        auto semaphore_result1 = vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &image_available_semaphores[i]);
        auto semaphore_result2 = vkCreateSemaphore(logical_device, &semaphore_create_info, nullptr, &render_finished_semaphores[i]);
        auto fence_result = vkCreateFence(logical_device, &fence_create_info, nullptr, &in_flight_fences[i]);

        if (semaphore_result1 != VK_SUCCESS || semaphore_result2 != VK_SUCCESS || fence_result != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

auto Hello_Triangle_Application::draw_frame() -> void
{
    vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

    auto image_index = (uint32_t) 0;
    auto next_image_result = vkAcquireNextImageKHR(logical_device, swap_chain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
    if (next_image_result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swap_chain();
        return;
    }
    else if (next_image_result != VK_SUCCESS && next_image_result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(logical_device, 1, &in_flight_fences[current_frame]);

    vkResetCommandBuffer(command_buffers[current_frame], 0);

    record_command_buffer(command_buffers[current_frame], image_index);

    update_uniform_buffer(current_frame);

    auto submit_info = VkSubmitInfo{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    auto wait_semaphores = std::vector<VkSemaphore>{image_available_semaphores[current_frame]};
    auto wait_stages = std::vector<VkPipelineStageFlags>{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores.data();
    submit_info.pWaitDstStageMask = wait_stages.data();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];

    auto signal_semaphores = std::vector<VkSemaphore>{render_finished_semaphores[current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores.data();

    auto result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit queue!");
    }

    auto present_info = VkPresentInfoKHR{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores.data();

    auto swap_chains = std::vector<VkSwapchainKHR>{swap_chain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains.data();
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    auto queue_present_result = vkQueuePresentKHR(present_queue, &present_info);
    if (queue_present_result == VK_ERROR_OUT_OF_DATE_KHR || queue_present_result == VK_SUBOPTIMAL_KHR || frame_buffer_resized) {
        frame_buffer_resized = false;
        recreate_swap_chain();
    }
    else if (next_image_result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto Hello_Triangle_Application::create_shader_module(std::vector<unsigned char> const& code) -> VkShaderModule
{
    auto create_info = VkShaderModuleCreateInfo {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    auto shader_module = VkShaderModule{};
    auto result = vkCreateShaderModule(logical_device, &create_info, nullptr, &shader_module);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }

    return shader_module;
}

auto Hello_Triangle_Application::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) -> void
{
    auto command_buffer_begin_info = VkCommandBufferBeginInfo{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = 0;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    auto result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer");
    }

    auto render_pass_begin_info = VkRenderPassBeginInfo{};
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.framebuffer = swap_chain_framebuffers[image_index];
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = swap_chain_extent;
    auto clear_color = std::array<VkClearValue, 2>{};
    clear_color[0].color = {{0.7f, 0.2f, 0.5f, 1.0f}};
    clear_color[1].depthStencil = {1.0f, 0};
    render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_color.size());
    render_pass_begin_info.pClearValues = clear_color.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

    auto viewport = VkViewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swap_chain_extent.width);
    viewport.height = static_cast<float>(swap_chain_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    auto scissor = VkRect2D{};
    scissor.offset = {0, 0};
    scissor.extent = swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    auto vertex_buffers = std::vector<VkBuffer>{vertex_buffer};
    auto offsets = std::vector<VkDeviceSize>{0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers.data(), offsets.data());

    vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, nullptr);

    vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(model_indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    auto render_result = vkEndCommandBuffer(command_buffer);
    if (render_result != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }
}

auto Hello_Triangle_Application::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory) -> void
{
    auto buffer_create_info = VkBufferCreateInfo{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    auto result = vkCreateBuffer(logical_device, &buffer_create_info, nullptr, buffer);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create a buffer!");
    }

    auto memory_requirements = VkMemoryRequirements{};
    vkGetBufferMemoryRequirements(logical_device, *buffer, &memory_requirements);

    auto memory_allocate_info = VkMemoryAllocateInfo{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, properties);

    auto memory_allocate_result = vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, buffer_memory);
    if (memory_allocate_result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(logical_device, *buffer, *buffer_memory, 0);
}

auto Hello_Triangle_Application::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) -> void
{
    auto command_buffer = begin_single_time_commands();

    auto copy_region = VkBufferCopy{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    end_single_time_commands(command_buffer);
}

auto Hello_Triangle_Application::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) -> void
{
    auto command_buffer = begin_single_time_commands();

    auto buffer_image_copy = VkBufferImageCopy{};
    buffer_image_copy.bufferOffset = 0;
    buffer_image_copy.bufferRowLength = 0;
    buffer_image_copy.bufferImageHeight = 0;
    buffer_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_image_copy.imageSubresource.mipLevel = 0;
    buffer_image_copy.imageSubresource.baseArrayLayer = 0;
    buffer_image_copy.imageSubresource.layerCount = 1;
    buffer_image_copy.imageOffset = {0, 0, 0};
    buffer_image_copy.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);

    end_single_time_commands(command_buffer);
}

auto Hello_Triangle_Application::update_uniform_buffer(uint32_t current_image) -> void
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

    auto ubo = Uniform_Buffer_Object{};
    ubo.model_matrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view_matrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.projection_matrix = glm::perspective(glm::radians(45.0f), swap_chain_extent.width / (float) swap_chain_extent.height, 0.1f, 10.0f);
    ubo.projection_matrix[1][1] *= -1.0f;

    memcpy(uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

auto Hello_Triangle_Application::create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory) -> void
{
    auto image_info = VkImageCreateInfo{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = static_cast<uint32_t>(width);
    image_info.extent.height = static_cast<uint32_t>(height);
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_levels;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0;

    auto result = vkCreateImage(logical_device, &image_info, nullptr, image);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    auto memory_requirements = VkMemoryRequirements{};
    vkGetImageMemoryRequirements(logical_device, *image, &memory_requirements);

    auto memory_allocate_info = VkMemoryAllocateInfo{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(memory_requirements.memoryTypeBits, properties);

    auto allocate_result = vkAllocateMemory(logical_device, &memory_allocate_info, nullptr, image_memory);
    if (allocate_result != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(logical_device, *image, *image_memory, 0);
}

auto Hello_Triangle_Application::create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) -> VkImageView
{
    auto image_view_create_info = VkImageViewCreateInfo{};
    image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_create_info.image = image;
    image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_create_info.format = format;
    image_view_create_info.subresourceRange.aspectMask = aspect_flags;
    image_view_create_info.subresourceRange.baseMipLevel = 0;
    image_view_create_info.subresourceRange.levelCount = mip_levels;
    image_view_create_info.subresourceRange.baseArrayLayer = 0;
    image_view_create_info.subresourceRange.layerCount = 1;

    auto image_view = VkImageView{};
    auto result = vkCreateImageView(logical_device, &image_view_create_info, nullptr, &image_view);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return image_view;
}

auto Hello_Triangle_Application::begin_single_time_commands() -> VkCommandBuffer
{
    auto command_buffer_allocate_info = VkCommandBufferAllocateInfo{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.commandBufferCount = 1;

    auto command_buffer = VkCommandBuffer{};
    vkAllocateCommandBuffers(logical_device, &command_buffer_allocate_info, &command_buffer);

    auto command_buffer_begin_info = VkCommandBufferBeginInfo{};
    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

    return command_buffer;
}

auto Hello_Triangle_Application::end_single_time_commands(VkCommandBuffer command_buffer) -> void
{
    vkEndCommandBuffer(command_buffer);

    auto submit_info = VkSubmitInfo{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(logical_device, command_pool, 1, &command_buffer);
}

auto Hello_Triangle_Application::transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t mip_levels) -> void
{
    auto command_buffer = begin_single_time_commands();

    auto source_stage = VkPipelineStageFlags{};
    auto destination_stage = VkPipelineStageFlags{};

    auto image_memory_barrier = VkImageMemoryBarrier{};
    image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.oldLayout = old_layout;
    image_memory_barrier.newLayout = new_layout;
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_barrier.subresourceRange.baseMipLevel = 0;
    image_memory_barrier.subresourceRange.levelCount = mip_levels;
    image_memory_barrier.subresourceRange.baseArrayLayer = 0;
    image_memory_barrier.subresourceRange.layerCount = 1;
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        image_memory_barrier.srcAccessMask = 0;
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);

    end_single_time_commands(command_buffer);
}

auto Hello_Triangle_Application::find_depth_format() -> VkFormat
{
    auto formats = std::vector<VkFormat>{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    return find_support_format(
        &formats,
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

auto Hello_Triangle_Application::has_stencil_component(VkFormat format) -> bool
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

auto Hello_Triangle_Application::generate_mipmaps(VkImage image, VkFormat image_format, int32_t tex_width, int32_t tex_height, uint32_t mip_levels) -> void
{
    auto format_properties = VkFormatProperties{};
    vkGetPhysicalDeviceFormatProperties(physical_device, image_format, &format_properties);
    if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support liner blitting!");
    }

    auto command_buffer = begin_single_time_commands();

    auto barrier = VkImageMemoryBarrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    auto mip_width = tex_width;
    auto mip_height = tex_height;

    for (auto i = (uint32_t) 1; i < mip_levels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        auto blit = VkImageBlit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(
            command_buffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR
        );

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        if (mip_width > 1) mip_width /= 2;
        if (mip_height > 1) mip_height /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        command_buffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    end_single_time_commands(command_buffer);
}

auto Hello_Triangle_Application::cleanup_swap_chain() -> void
{
    for (auto framebuffer: swap_chain_framebuffers) {
        vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
    }

    vkDestroyImageView(logical_device, depth_image_view, nullptr);
    vkDestroyImage(logical_device, depth_image, nullptr);
    vkFreeMemory(logical_device, depth_image_memory, nullptr);

    for (auto image_view: swap_chain_image_views) {
        vkDestroyImageView(logical_device, image_view, nullptr);
    }

    vkDestroySwapchainKHR(logical_device, swap_chain, nullptr);
}

auto Hello_Triangle_Application::recreate_swap_chain() -> void
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(logical_device);

    cleanup_swap_chain();

    create_swap_chain();
    create_image_views();
    create_depth_resources();
    create_framebuffers();
}

auto Hello_Triangle_Application::find_queue_families(VkPhysicalDevice device) -> Queue_Family_Indices
{
    auto indices = Queue_Family_Indices{};

    auto queue_family_count = (uint32_t) 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    auto queue_families = std::vector<VkQueueFamilyProperties>{queue_family_count};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    auto i = 0;
    for (auto const& queue_family: queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        auto present_support = (VkBool32) false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

        if (present_support) {
            indices.present_family = i;
        }

        if (indices.complete()) {
            break;
        }

        i++;
    }

    return indices;
}

auto Hello_Triangle_Application::check_device_extension_support(VkPhysicalDevice device) -> bool
{
    auto extension_count = (uint32_t) 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    auto available_extensions = std::vector<VkExtensionProperties>{extension_count};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    auto required_extensions = std::set<std::string>{device_extensions.begin(), device_extensions.end()};

    for (auto const& extension: available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

auto Hello_Triangle_Application::query_swap_chain_support(VkPhysicalDevice device) -> Swap_Chain_Support_Details
{
    auto details = Swap_Chain_Support_Details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    auto format_count = (uint32_t) 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    auto present_mode_count = (uint32_t) 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

auto Hello_Triangle_Application::is_device_suitable(VkPhysicalDevice device) -> bool
{
    auto device_properties = VkPhysicalDeviceProperties{};
    auto device_features = VkPhysicalDeviceFeatures{};
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);
    std::cout << device_properties.deviceName << std::endl;

    auto indices = find_queue_families(device);
    auto extension_supported = check_device_extension_support(device);

    auto swap_chain_adequate = false;
    if (extension_supported) {
        auto swap_chain_support = query_swap_chain_support(device);
        swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
    }

    auto supported_features = VkPhysicalDeviceFeatures{};
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    return indices.complete() && extension_supported && swap_chain_adequate && supported_features.samplerAnisotropy;
}

auto Hello_Triangle_Application::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) -> uint32_t
{
    auto memory_properties = VkPhysicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    for (auto i = (uint32_t) 0; i < memory_properties.memoryTypeCount; i++) {
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

auto Hello_Triangle_Application::find_support_format(std::vector<VkFormat>* candidates, VkImageTiling tiling, VkFormatFeatureFlags features) -> VkFormat
{
    for (auto format: *candidates) {
        auto properties = VkFormatProperties{};
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }

        throw std::runtime_error("failed to find supported format!");
    }
}

auto Hello_Triangle_Application::choose_swap_surface_format(std::vector<VkSurfaceFormatKHR> const& available_formats) -> VkSurfaceFormatKHR
{
    for (auto const& available_format: available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

auto Hello_Triangle_Application::choose_swap_present_mode(std::vector<VkPresentModeKHR> const& available_present_modes) -> VkPresentModeKHR
{
    for (auto const& available_present_mode: available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

auto Hello_Triangle_Application::choose_swap_extent(VkSurfaceCapabilitiesKHR const& capabilities) -> VkExtent2D
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    auto width = 0;
    auto height = 0;

    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual_extent;
}

auto Hello_Triangle_Application::setup_debug_messenger() -> void
{
    if (!enable_validation_layers) return;

    auto create_info = VkDebugUtilsMessengerCreateInfoEXT{};
    populate_debug_messenger_create_info(&create_info);

    if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

auto Hello_Triangle_Application::debug_callback(
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

auto Hello_Triangle_Application::present_all_available_extensions() -> void
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

auto Hello_Triangle_Application::get_required_extensions() -> std::vector<const char*>
{
    auto glfw_extension_count = (uint32_t) 0;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    auto extensions = std::vector<const char*>{glfw_extensions, glfw_extensions + glfw_extension_count};

    if (enable_validation_layers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

auto Hello_Triangle_Application::check_validation_layers_support() -> bool
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

auto Hello_Triangle_Application::populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* create_info) -> void
{
    *create_info = {};
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info->pfnUserCallback = debug_callback;
}

auto Hello_Triangle_Application::frame_buffer_resized_callback(GLFWwindow* window, int width, int height) -> void
{
    auto app = reinterpret_cast<Hello_Triangle_Application*>(glfwGetWindowUserPointer(window));
    app->frame_buffer_resized = true;
}
