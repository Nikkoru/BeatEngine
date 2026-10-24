#pragma once

#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/QueueInfo.hpp"
#include <vector>

#include <vulkan/vulkan_core.h>

namespace VK {
class Swapchain {
private:
    VkPhysicalDevice m_PhysicalDevice{};
    VkDevice m_Device{};

    VkSwapchainKHR m_Swapchain{};
    VkFormat m_Format{};
    VkSurfaceKHR m_Surface{};
    VkCommandPool m_CmdPool{};

    uint32_t m_ImageIndex{};
    uint32_t m_ResourceIndex{};

    struct SwapchainImage {
        VkImage image{};
VkImageView imageView{};
        VkSemaphore presentSemaphore{}; // signal when rendering done, waited by present 
    };
    struct InFlightSlot {
        VkSemaphore acquireSemaphore{}; // signal when acquire image from swapchai
    };

    std::vector<SwapchainImage> m_Images;
    std::vector<InFlightSlot> m_InFlightSlots;

    uint32_t m_ImageCount = 3;
    uint32_t m_FramesInFlight = FRAME_OVERLAP;

    bool m_Outdated{ false };
public:
    Swapchain() = default;
    ~Swapchain() = default;

    void Init(VkPhysicalDevice physicalDevice, VkDevice device, const QueueInfo& info, VkSurfaceKHR surface);

    void RequestRebuild() { m_Outdated = false; }
    bool IsOutdated() { return m_Outdated; }
    VkImage GetImage() const { return m_Images[m_ImageIndex].image; }
    VkImageView GetImageView() const { return m_Images[m_ImageIndex].imageView; }
    VkFormat GetFormat() const { return m_Format; }

    uint32_t GetImageCount() const { return m_ImageCount; }
    uint32_t GetFramesInFlight() const { return m_FramesInFlight; }
    uint32_t GetImageIndex() const { return m_ImageIndex; }
    uint32_t GetResourceIndex() const { return m_ResourceIndex; }

    VkSemaphore GetAcquireSemaphore() const { return m_InFlightSlots[m_ResourceIndex].acquireSemaphore; }
    VkSemaphore GetPresentSemaphore() const { return m_Images[m_ImageIndex].presentSemaphore; }

    VkResult AcquireNextImage(VkDevice device);
    void PresentFrame(VkQueue queue);
private:
    VkSurfaceFormat2KHR _SelectSurfaceFormat(std::vector<VkSurfaceFormat2KHR>& formats);
    VkPresentModeKHR _SelectPresentMode(std::vector<VkPresentModeKHR>& modes);
};
}
