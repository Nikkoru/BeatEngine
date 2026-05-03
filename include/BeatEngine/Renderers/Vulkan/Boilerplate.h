#pragma once

#include "BeatEngine/Renderers/Vulkan/PipelineManager.h"
#include <source_location>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(err) VK_CHECK_SOURCE(err, std::source_location::current()) 

void VK_CHECK_SOURCE(VkResult result, const std::source_location location);

void AddNameToVKObject(VkDevice device, VkObjectType type, uint64_t objectHandle, std::string name);

namespace vkb {
    VkInstance CreateInstance(std::string appName, uint32_t apiVersion, std::vector<const char*> pInstExt = {}, std::vector<const char*> pInstLayers = {});
    VkPhysicalDevice CreatePhysicalDevice(VkInstance instance, uint32_t deviceIndex = 0, VkPhysicalDeviceProperties* props = nullptr);
    VkDevice CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily);
    VkSwapchainKHR CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE, VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM);

    uint32_t GetQueueFamily(VkPhysicalDevice device);
    std::vector<VkImage> GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);
    std::vector<VkImageView> GetSwapchainImageViews(VkDevice device, std::vector<VkImage> images, VkFormat format);
    
    VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags flags);
}

namespace vku {
    void TransitionImage(PipelineManager mgr, VkCommandBuffer cmd, VkImage image, VkImageLayout curLayout, VkImageLayout newLayout);
    void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
}

namespace vki {
    VkImageCreateInfo GetImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo GetImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
    VkRenderingAttachmentInfo GetRenderingAttachmentInfo(VkImageView target, VkClearValue* clear, VkImageLayout layout);
    VkRenderingInfo GetRenderingInfo(VkExtent2D extent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);
    VkSubmitInfo2 GetSubmitInfo(VkCommandBufferSubmitInfo* cmdInfo, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
    VkCommandBufferSubmitInfo GetCommandBufferSubmitInfo(VkCommandBuffer cmd);
    VkSemaphoreSubmitInfo GetSemaphoreSubmitInfo(VkPipelineStageFlags2 stageFlags, VkSemaphore semaphore);
}
