#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

void VK_CHECK(VkResult result);

namespace vkb {
    VkInstance CreateInstance(std::string appName, uint32_t apiVersion);
    VkPhysicalDevice CreatePhysicalDevice(VkInstance instance, uint32_t deviceIndex = 0);
    VkDevice CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily);
    VkSwapchainKHR CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB);

    uint32_t GetQueueFamily(VkPhysicalDevice device);
}
