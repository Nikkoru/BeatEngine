#include "BeatEngine/Renderers/VulkanBoilerplate.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Exception.h"

#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <print>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <volk.h>
#include <format>

void VK_CHECK(VkResult result) {
    if (result) {
        auto msg = std::format("Vulkan error: {}", string_VkResult(result));
        Logger::AddCritical("", msg);
        THROW_RUNTIME_ERROR(msg);
    }
}


VkInstance vkb::CreateInstance(std::string appName, uint32_t apiVersion, std::vector<const char*> pInstExt, std::vector<const char*> pInstLayers) {
    VkInstance instance{ VK_NULL_HANDLE };

    uint32_t instanceExtensionCount{};

    auto instanceExtension = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
    
    for (auto i = 0; i < instanceExtensionCount; i++)
        pInstExt.emplace_back(instanceExtension[i]);

    VkApplicationInfo appInfo{ 
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = appName.c_str(),
        .apiVersion = apiVersion
    };

    VkInstanceCreateInfo instanceInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(pInstLayers.size()),
        .ppEnabledLayerNames = pInstLayers.data(), 
        .enabledExtensionCount = static_cast<uint32_t>(pInstExt.size()),
        .ppEnabledExtensionNames = pInstExt.data()
    };
    
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));

    return instance;
}

VkPhysicalDevice vkb::CreatePhysicalDevice(VkInstance instance, uint32_t deviceIndex) {
    VkPhysicalDevice device{ VK_NULL_HANDLE };

    uint32_t devCount{};

    VK_CHECK(vkEnumeratePhysicalDevices(instance, &devCount, nullptr));
    std::vector<VkPhysicalDevice> devices(devCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &devCount, devices.data()));
    
    VkPhysicalDeviceProperties2 devProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    if (deviceIndex >= devices.size()) {
        std::println("Not a valid index. autoselecting index 0");
        deviceIndex = 0;
    }

    vkGetPhysicalDeviceProperties2(devices[deviceIndex], &devProperties);
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Selected device: {}", devProperties.properties.deviceName);

    device = devices[deviceIndex];

    return device;
}

uint32_t vkb::GetQueueFamily(VkPhysicalDevice device) {
    uint32_t queueFamily{};
    uint32_t count{};

    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());

    for (size_t i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamily = i;
			break;
		}
	}

    return queueFamily;
}

VkDevice vkb::CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily) {
    VkDevice device { VK_NULL_HANDLE };
    const float qfpriorities{ 1.0f };

    VkDeviceQueueCreateInfo queueInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &qfpriorities
    };

    VkPhysicalDeviceVulkan12Features features12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .descriptorBindingVariableDescriptorCount = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true
    };
    VkPhysicalDeviceVulkan13Features features13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &features12,
        .synchronization2 = true,        
        .dynamicRendering = true
    };
	const VkPhysicalDeviceFeatures features10{ .samplerAnisotropy = VK_TRUE };

    const std::vector<const char*> deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features13,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &features10
    };

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device));
    return device;
}

VkSwapchainKHR vkb::CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkSwapchainKHR oldSwapchain, VkFormat imageFormat) {
    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCaps));

    VkSwapchainCreateInfoKHR swapchainInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = surfaceCaps.minImageCount,
        .imageFormat = imageFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent{
            .width = width,
            .height = height
        },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    if (oldSwapchain)
        swapchainInfo.oldSwapchain = oldSwapchain;

    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain));

    return swapchain;
}
