#include "BeatEngine/Renderers/Vulkan/Swapchain.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"

#include <algorithm>
#include <cassert>
#include <volk.h>

namespace VK {
void Swapchain::Init(VkPhysicalDevice physicalDevice, VkDevice device, const QueueInfo& info, VkSurfaceKHR surface) {
    m_Queue = info;

    m_Surface = surface;

    const VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2{
        .sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
        .pNext = nullptr,
        .surface = m_Surface
    };
    VkSurfaceCapabilities2KHR capabilities2{
        .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR,
        .pNext = nullptr,
        .surfaceCapabilities{}
    };
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilities2KHR(physicalDevice, &surfaceInfo2, &capabilities2));

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2, &formatCount, nullptr);
    std::vector<VkSurfaceFormat2KHR> formats(
        formatCount,
        VkSurfaceFormat2KHR{
            .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
            .pNext = nullptr,
            .surfaceFormat{}
        }
    );
    vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2, &formatCount, formats.data());

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    const VkSurfaceFormat2KHR surfaceFormat2 = _SelectSurfaceFormat(formats);
    const VkPresentModeKHR    presentMode    = _SelectPresentMode(presentModes);

    uint32_t minImageCount       = capabilities2.surfaceCapabilities.minImageCount;
    uint32_t preferredImageCount = std::max(m_ImageCount, minImageCount);

    uint32_t maxImageCount = (capabilities2.surfaceCapabilities.maxImageCount == 0) 
        ? preferredImageCount
        : capabilities2.surfaceCapabilities.maxImageCount;

    m_ImageCount = std::clamp(preferredImageCount, minImageCount, maxImageCount);

    m_Format = surfaceFormat2.surfaceFormat.format;

    const VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext            = nullptr,
        .flags{},
        .surface          = surface,
        .minImageCount    = m_ImageCount,
        .imageFormat      = surfaceFormat2.surfaceFormat.format,
        .imageColorSpace  = surfaceFormat2.surfaceFormat.colorSpace,
        .imageExtent      = capabilities2.surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform     = capabilities2.surfaceCapabilities.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = presentMode,
        .clipped          = VK_TRUE,
    };
    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &m_Swapchain));

    {
      uint32_t imageCount = 0;
      vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
      assert(m_ImageCount <= imageCount && "Wrong swapchain setup");
      m_ImageCount = imageCount;
    }
    std::vector<VkImage> swapImages(m_ImageCount);
    vkGetSwapchainImagesKHR(device, m_Swapchain, &m_ImageCount, swapImages.data());

    // Frames-in-flight: how many CPU frame slots run concurrently with the GPU.
    // Default of 2 is the canonical modern choice (one being recorded on the
    // CPU while the previous one executes on the GPU). Capped at imageCount
    // because we can never have more frames in flight than swapchain images.
    m_FramesInFlight = std::min(m_FramesInFlight, m_ImageCount);

    // Per-image storage: VkImage, VkImageView, and the presentSemaphore
    // (binary semaphore that present waits on; must follow the image).
    m_Images.resize(m_ImageCount);
    VkImageViewCreateInfo imageViewCreateInfo{
        .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format   = m_Format,
        .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY, .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1},
    };
    const VkSemaphoreCreateInfo semaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for(uint32_t i = 0; i < m_ImageCount; i++) {
      m_Images[i].image = swapImages[i];
      // DBG_VK_NAME(m_nextImages[i].image);
      imageViewCreateInfo.image = m_Images[i].image;
      VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &m_Images[i].imageView));
      // DBG_VK_NAME(m_nextImages[i].imageView);
      VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_Images[i].presentSemaphore));
      // DBG_VK_NAME(m_nextImages[i].presentSemaphore);
    }

    m_InFlightSlots.resize(m_FramesInFlight);
    for(size_t i = 0; i < m_FramesInFlight; ++i)
    {
      VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &m_InFlightSlots[i].acquireSemaphore));
      // DBG_VK_NAME(m_inFlightSlots[i].acquireSemaphore);
    }
}

void Swapchain::Uninit(VkDevice device) {
    AddVulkanLog("Destroying Swapchain");

    vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
    for(auto& slot : m_InFlightSlots) {
        vkDestroySemaphore(device, slot.acquireSemaphore, nullptr);
    }
    for(auto& image : m_Images) {
        vkDestroyImageView(device, image.imageView, nullptr);
        vkDestroySemaphore(device, image.presentSemaphore, nullptr);
    }
}

void Swapchain::Reinit(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface) {
    vkQueueWaitIdle(m_Queue.queue);

    m_ResourceIndex = 0;
    m_Outdated = false;
    Uninit(device);
    Init(physicalDevice, device, m_Queue, surface);
}

VkResult Swapchain::AcquireNextImage(VkDevice device) {
    const auto signalSem = GetAcquireSemaphore();
    auto result = vkAcquireNextImageKHR(
        device,
        m_Swapchain,
        (std::numeric_limits<uint64_t>::max)(),
        signalSem,
        VK_NULL_HANDLE,
        &m_ImageIndex
    );
    if (VK_CHECK_SWAPCHAIN(result)) {
        m_Outdated = true;
    }

    return result;
}

void Swapchain::PresentFrame(VkQueue queue) {
    const auto waitSem = GetPresentSemaphore();

    const VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSem,
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &m_ImageIndex,
        .pResults = nullptr
    };

    if (VK_CHECK_SWAPCHAIN(vkQueuePresentKHR(queue, &presentInfo))) {
        m_Outdated = true;
    }

    m_ResourceIndex = (m_ResourceIndex + 1) % m_FramesInFlight;
}

VkSurfaceFormat2KHR Swapchain::_SelectSurfaceFormat(std::vector<VkSurfaceFormat2KHR>& formats) {
    if (formats.size() == 1 && formats[0].surfaceFormat.format == VK_FORMAT_UNDEFINED) {
        VkSurfaceFormat2KHR result{
            .sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR,
            .pNext = nullptr,
            .surfaceFormat = {
                .format = VK_FORMAT_B8G8R8A8_UNORM,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            }
        };
        return result;
    }

    const auto preferredFormats = std::to_array<VkSurfaceFormat2KHR>({
        {.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, .surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}},
        {.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR, .surfaceFormat = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}},
    });

    for(const auto& preferredFormat : preferredFormats) {
        for(const auto& availableFormat : formats) {
            if (availableFormat.surfaceFormat.format == preferredFormat.surfaceFormat.format
            && availableFormat.surfaceFormat.colorSpace == preferredFormat.surfaceFormat.colorSpace) {
                return availableFormat;
            }
        }
    }

    return formats[0];

}

VkPresentModeKHR Swapchain::_SelectPresentMode(std::vector<VkPresentModeKHR>& modes) {
    if (false) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    bool mailboxSupported = false, immediateSupported = false;

    for (VkPresentModeKHR mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            mailboxSupported = true;
        if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            immediateSupported = true;
    }

    if (mailboxSupported) {
        return VK_PRESENT_MODE_MAILBOX_KHR;
    }

    if (immediateSupported) {
        return VK_PRESENT_MODE_IMMEDIATE_KHR;  // Best mode for low latency
    }

    return VK_PRESENT_MODE_FIFO_KHR;  // Fallback to FIFO if neither MAILBOX nor IMMEDIATE is available
}
} // namespace VK
