#pragma once

#include <cstdint>
#include <limits>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

using ImageID = uint32_t;
static constexpr ImageID NULL_IMAGE_ID = std::numeric_limits<uint32_t>::max();

struct AllocatedImage {
    VkImage Image{ VK_NULL_HANDLE };
    VkImageView ImageView{ VK_NULL_HANDLE };
    VmaAllocation Allocation{ VK_NULL_HANDLE };
    VkFramebuffer Framebuffer{ VK_NULL_HANDLE };
    VkImageUsageFlags Usage{};
    VkExtent3D Extent{};
    VkFormat Format{};

    uint32_t MipLevels{};

    ImageID CachedID{ NULL_IMAGE_ID };
};

inline bool operator==(AllocatedImage lhs, AllocatedImage rhs) {
    return 
        lhs.Format == rhs.Format &&
        lhs.Extent.depth == rhs.Extent.depth &&
        lhs.Extent.width == rhs.Extent.width &&
        lhs.Extent.height == rhs.Extent.height &&
        lhs.Allocation == rhs.Allocation &&
        lhs.Framebuffer == rhs.Framebuffer &&
        lhs.Usage == rhs.Usage &&
        lhs.Image == rhs.Image &&
        lhs.ImageView == rhs.ImageView;
}

inline bool operator!=(AllocatedImage lhs, AllocatedImage rhs) {
    return !(lhs == rhs);
}
