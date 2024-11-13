#include "engine.hpp"

#include <set>
#include <limits>
#include <algorithm>

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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WINTH, HEIGHT, "Vulkan", nullptr, nullptr);

}

auto Hello_Triangle_Application::init_vulkan() -> void
{
    create_instance();

    create_surface();

    setup_debug_messenger();

    pick_physical_device();

    create_logical_device();

    create_swap_chain();
}

auto Hello_Triangle_Application::main_loop() -> void
{
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

auto Hello_Triangle_Application::clean_up() -> void
{
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

auto Hello_Triangle_Application::create_image_view() -> void
{
    swap_chain_image_views.resize(swap_chain_images.size());

    for (auto i = (size_t) 0; i < swap_chain_images.size(); i++) {
        auto create_info = VkImageViewCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = swap_chain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(logical_device, &create_info, nullptr, &swap_chain_image_views[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
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

    return indices.complete() && extension_supported && swap_chain_adequate;
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

auto Hello_Triangle_Application::setup_debug_messenger() -> void
{
    if (!enable_validation_layers) return;

    auto create_info = VkDebugUtilsMessengerCreateInfoEXT{};
    populate_debug_messenger_create_info(&create_info);

    if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}
