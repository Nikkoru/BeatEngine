#pragma once

#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

struct AllocatedImage {
    VkImage Image{ VK_NULL_HANDLE };
    VkImageView ImageView{ VK_NULL_HANDLE };
    VmaAllocation Allocation{ VK_NULL_HANDLE };
    VkSemaphore RenderSemaphore{VK_NULL_HANDLE};
    VkFramebuffer Framebuffer{ VK_NULL_HANDLE };
    VkExtent3D ImageExtent{};
    VkFormat ImageFormat{};
};
