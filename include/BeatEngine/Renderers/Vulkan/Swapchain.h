#pragma once

#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"

#include <array>
#include <cstdint>
#include <source_location>
#include <vulkan/vulkan_core.h>


namespace VK {
class Swapchain {
private:
    VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
    std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;

    VkFormat m_Format{ VK_FORMAT_B8G8R8A8_UNORM };
    Vector2u m_Extent{};
    AllocatedImage m_DrawImage;
    std::array<FrameData, FRAME_OVERLAP> m_Frames;

    uint32_t m_ActiveImageIndex{};

    bool m_Outdated{ false };

    UninitQueue m_Uninitializers;
public:
    void Create(const Core& core, unsigned int width, unsigned int height, VSyncMode vSync = Disable, VkFormat format = VK_FORMAT_B8G8R8A8_UNORM);
    void Uninit();
    Vector2u GetExtent() { return m_Extent; }

    const std::vector<VkImage>& GetImages() { return m_Images; }
    VkImage& GetImage(uint32_t imageIndex) { return m_Images[imageIndex]; }
    VkImageView& GetImageView(uint32_t imageIndex) { return m_ImageViews[imageIndex]; }
    size_t GetImagesSize() { return m_Images.size(); }

    VkFormat GetFormat() { return m_Format; }

    void BeginFrame(VkDevice device, size_t frameIndex) const;
    void ResetFence(VkDevice device, size_t frameIndex) const;

    std::pair<VkImage, uint32_t> AcquireImage(VkDevice device, size_t frameIndex);
    void SubmitAndPresent(const VkCommandBuffer cmd, VkQueue graphicsQueue, size_t frameIndex, uint32_t imageIndex);

    FrameData& GetFrame(uint32_t index) { return m_Frames[index]; }

    bool IsOutdated() { return m_Outdated; } 
private:
    void CreateCommandBuffers(VkDevice device, uint32_t graphicsQueueFamily);
    void CreateSync(VkDevice device);
};
}
