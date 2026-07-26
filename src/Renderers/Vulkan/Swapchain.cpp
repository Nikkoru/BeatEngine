#include "BeatEngine/Renderers/Vulkan/Swapchain.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"

#include <cstdint>
#include <limits>
#include <volk.h>
#include <vulkan/vulkan_core.h>


void VK::Swapchain::Create(const Core& core, std::shared_ptr<BaseWindow> window, unsigned int width, unsigned int height, VSyncMode vSync, VkFormat format) {
    if (m_VSync == VSyncMode::None)
        m_VSync = vSync;

    VkPresentModeKHR presentMode{};
    switch (m_VSync) {
    default:
    case None:
        m_VSync = VSyncMode::Disable;
    case Disable:
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        break;
    case SyncWithFirstRefresh:
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
    case SyncWithSecondRefresh:
        presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        break;
    case Adaptative:
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
        break;
    }
    
    m_Format = format;
    {
        auto capabilities = VkSurfaceCapabilitiesKHR{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core.PhysicalDevice, core.Surface, &capabilities);
        if (capabilities.currentExtent.width != UINT32_MAX)
            width = capabilities.currentExtent.width;
        else
            width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);

        if (capabilities.currentExtent.height != UINT32_MAX)
            height = capabilities.currentExtent.height;
        else
            height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    }

    VkSwapchainKHR oldSwapchain = nullptr;
    if (m_Swapchain)
        oldSwapchain = m_Swapchain;

    m_Swapchain = vkb::CreateSwapchainKHR(
        core.Device, 
        core.PhysicalDevice, 
        core.Surface, 
        width, 
        height,
        presentMode,
        oldSwapchain
    );

    if (oldSwapchain) {
        for (const auto& frame : m_Frames) {
            vkDestroyFence(core.Device, frame.RenderFence, nullptr);
            vkDestroySemaphore(core.Device, frame.PresentSemaphore, nullptr);
            vkDestroySemaphore(core.Device, frame.RenderSemaphore, nullptr);
        }
        for (const auto& imageView : m_ImageViews) {
            vkDestroyImageView(core.Device, imageView, nullptr);
        }
        m_ImageViews.clear();
        
        vkDestroySwapchainKHR(core.Device, oldSwapchain, nullptr);
    }
    else
        m_Uninitializers.AddCallback([&]() {
            for (const auto& frame : m_Frames) {
                vkDestroyFence(core.Device, frame.RenderFence, nullptr);
                vkDestroySemaphore(core.Device, frame.PresentSemaphore, nullptr);
                vkDestroySemaphore(core.Device, frame.RenderSemaphore, nullptr);
            }
            for (const auto& imageView : m_ImageViews) {
                vkDestroyImageView(core.Device, imageView, nullptr);
            }
            m_ImageViews.clear();
            
            vkDestroySwapchainKHR(core.Device, m_Swapchain, nullptr);
        });

    m_Extent = { width, height };
    m_Images = vkb::GetSwapchainImages(core.Device, m_Swapchain);
    m_ImageViews = vkb::GetSwapchainImageViews(core.Device, m_Images, m_Format);

    m_Outdated = false;

    CreateCommandBuffers(core.Device, core.GraphicsQueueFamily);
    CreateSync(core.Device);

    AddNameToVKObject(core.Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "VkSwapchainKHR");
}

void VK::Swapchain::Uninit() {
    AddVulkanLog("Destroying Swapchain");
    m_Uninitializers.Flush();
}

void VK::Swapchain::BeginFrame(VkDevice device, size_t frameIndex) const {
    const auto& frame = m_Frames[frameIndex];
    VK_CHECK(vkWaitForFences(device, 1, &frame.RenderFence, true, NO_TIMEOUT));
    VK_CHECK(vkResetFences(device, 1, &frame.RenderFence));

}

void VK::Swapchain::ResetFence(VkDevice device, size_t frameIndex) const {
    const auto& frame = m_Frames[frameIndex];
    VK_CHECK(vkResetFences(device, 1, &frame.RenderFence));
}

std::pair<VkImage, uint32_t> VK::Swapchain::AcquireImage(VkDevice device, size_t frameIndex) {
    uint32_t imgIndex{};
    m_Outdated = VK_CHECK_SWAPCHAIN(vkAcquireNextImageKHR(
        device, 
        m_Swapchain, 
        NO_TIMEOUT, 
        m_Frames[frameIndex].PresentSemaphore, 
        VK_NULL_HANDLE, 
        &imgIndex
    ));

    if (m_Outdated)
        return {};

    m_ActiveImageIndex = imgIndex;

    return { m_Images[imgIndex], imgIndex };
}

void VK::Swapchain::SubmitAndPresent(const VkCommandBuffer cmd, VkQueue graphicsQueue, size_t frameIndex, uint32_t imageIndex) {
    const auto& frame = m_Frames[frameIndex];
    {
        auto cmdInfo = vki::GetCommandBufferSubmitInfo(cmd);
        auto waitInfo = vki::GetSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.PresentSemaphore);
        auto signalInfo = vki::GetSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, frame.RenderSemaphore);
        
        auto submitInfo = vki::GetSubmitInfo(&cmdInfo, &signalInfo, &waitInfo);
        vkQueueSubmit2(graphicsQueue, 1, &submitInfo, frame.RenderFence);
    }
    {
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &frame.RenderSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &m_Swapchain,
            .pImageIndices = &imageIndex,
            .pResults{}
        };

        m_Outdated = VK_CHECK_SWAPCHAIN(vkQueuePresentKHR(graphicsQueue, &presentInfo));
    }
}

void VK::Swapchain::CreateCommandBuffers(VkDevice device, uint32_t graphicsQueueFamily) {
    AddVulkanLog("Initializing Command interface");
    VkCommandPoolCreateInfo commandPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 
        .queueFamilyIndex = graphicsQueueFamily
    };

    for (unsigned int i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &m_Frames[i].CommandPool));
        AddNameToVKObject(device, VK_OBJECT_TYPE_COMMAND_POOL, uint64_t(m_Frames[i].CommandPool), std::format("VkCommandPool_Frame{}", i));

        VkCommandBufferAllocateInfo cbAllocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
            .pNext = nullptr,
            .commandPool = m_Frames[i].CommandPool, 
            .level{},
            .commandBufferCount = 1,
        };
        VK_CHECK(vkAllocateCommandBuffers(device, &cbAllocInfo, &m_Frames[i].ActiveCmdBuffer));
    }
}

void VK::Swapchain::CreateSync(VkDevice device) {
    VkSemaphoreCreateInfo semaphoreInfo{ 
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags{}
    };
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,

        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (unsigned int i = 0; i < FRAME_OVERLAP; i++) {
        auto semaphorePresentName = std::format("VkSemaphore_Present{}", i);
        auto semaphoreRenderName = std::format("VkSemaphore_Render{}", i);
        auto fenceName = std::format("VkFence_Render{}", i);

        VK_CHECK(vkCreateFence(device, &fenceInfo, nullptr, &m_Frames[i].RenderFence));
        AddNameToVKObject(device, VK_OBJECT_TYPE_FENCE, (uint64_t)m_Frames[i].RenderFence, fenceName);
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_Frames[i].PresentSemaphore));
        AddNameToVKObject(device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_Frames[i].PresentSemaphore, semaphorePresentName);
        VK_CHECK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_Frames[i].RenderSemaphore));
        AddNameToVKObject(device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_Frames[i].RenderSemaphore, semaphoreRenderName);
    }

    m_Uninitializers.AddCallback([&, device]() {
        for (unsigned int i = 0; i < FRAME_OVERLAP; i++) {
            vkDestroyFence(device, m_Frames[i].RenderFence, nullptr);
            vkDestroySemaphore(device, m_Frames[i].PresentSemaphore, nullptr);
            vkDestroySemaphore(device, m_Frames[i].RenderSemaphore, nullptr);
        }
    });
}
