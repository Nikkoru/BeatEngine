#pragma once

#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include <source_location>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_enum_string_helper.h>

constexpr auto NO_TIMEOUT{ (std::numeric_limits<unsigned long>::max)() };

void VK_CHECK_SOURCE(VkResult result, const std::source_location location);
#define VK_CHECK(err) VK_CHECK_SOURCE(err, std::source_location::current()) 

bool VK_CHECK_SWAPCHAIN_SOURCE(VkResult result, const std::source_location location);
#define VK_CHECK_SWAPCHAIN(res) VK_CHECK_SWAPCHAIN_SOURCE(res, std::source_location::current())

void AddNameToVKObject(VkDevice device, VkObjectType type, uint64_t objectHandle, std::string name);

namespace vkb {
    VkInstance CreateInstance(std::string appName, uint32_t apiVersion, std::vector<const char*> pInstExt = {}, std::vector<const char*> pInstLayers = {});
    VkPhysicalDevice CreatePhysicalDevice(VkInstance instance, std::vector<VkExtensionProperties>& availableExts, uint32_t deviceIndex = 0, VkPhysicalDeviceProperties* props = nullptr);
    VkDevice CreateDevice(VkPhysicalDevice physicalDevice, uint32_t queueFamily, std::vector<const char*>& requestedExts, std::vector<VkExtensionProperties>& deviceExts);
    VkSwapchainKHR CreateSwapchainKHR(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, unsigned int width, unsigned int height, VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE, VkFormat imageFormat = VK_FORMAT_B8G8R8A8_UNORM);

    uint32_t GetQueueFamily(VkPhysicalDevice device);
    std::vector<VkImage> GetSwapchainImages(VkDevice device, VkSwapchainKHR swapchain);
    std::vector<VkImageView> GetSwapchainImageViews(VkDevice device, std::vector<VkImage>& images, VkFormat format);
    
    VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags flags);

    VkPipelineLayout CreatePipelineLayout(VkDevice device, VkDescriptorSetLayout descLayout, VkPushConstantRange range);
}

namespace vku {
    void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout curLayout, VkImageLayout newLayout);
    void CopyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
}

namespace vki {
    VkImageCreateInfo GetImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo GetImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    VkRenderingAttachmentInfo GetRenderingAttachmentInfo(VkImageView target, VkClearValue* clear, VkImageLayout layout);
    VkRenderingInfo GetRenderingInfo(VkExtent2D extent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment);

    VkSubmitInfo2 GetSubmitInfo(const VkCommandBufferSubmitInfo* cmdInfo, const VkSemaphoreSubmitInfo* signalSemaphoreInfo, const VkSemaphoreSubmitInfo* waitSemaphoreInfo);
    VkCommandBufferSubmitInfo GetCommandBufferSubmitInfo(VkCommandBuffer cmd);
    VkCommandPoolCreateInfo GetCommandPoolCreateInfo(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex);
    VkCommandBufferAllocateInfo GetCommandBufferAllocateInfo(VkCommandPool commandPool, uint32_t commandBufferCount);
    VkSemaphoreSubmitInfo GetSemaphoreSubmitInfo(VkPipelineStageFlags2 stageFlags, VkSemaphore semaphore);

    VkPipelineInputAssemblyStateCreateInfo GetPipelineInputAssemblyStateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable);
}
