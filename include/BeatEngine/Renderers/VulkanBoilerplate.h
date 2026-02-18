#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

void VK_CHECK(VkResult result);

namespace vkb {
    VkInstance CreateInstance(std::string appName, uint32_t apiVersion, std::vector<const char*> pInstExt = {}, std::vector<const char*> pInstLayers = {});
    VkPhysicalDevice CreatePhysicalDevice(VkInstance instance, uint32_t deviceIndex = 0);
    VkDevice CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily);
    VkSwapchainKHR CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE, VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB);

    uint32_t GetQueueFamily(VkPhysicalDevice device);
}
