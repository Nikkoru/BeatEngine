#include "BeatEngine/Renderers/Vulkan/Swapchain.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"

#include <cstdint>
#include <limits>
#include <volk.h>
#include <vulkan/vulkan_core.h>


void VK::Swapchain::Create(const Core& core, unsigned int width, unsigned int height, VSyncMode vSync, VkFormat format) {
    VkPresentModeKHR presentMode{};
    switch (vSync) {
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
    VkSwapchainKHR swapchain{};
    if (!m_Swapchain) {
        swapchain = vkb::CreateSwapchainKHR(
            core.Device, 
            core.PhysicalDevice, 
            core.Surface, 
            width, 
            height,
            presentMode
        );
    }
    else {
        swapchain = vkb::CreateSwapchainKHR(
            core.Device, 
            core.PhysicalDevice, 
            core.Surface, 
            width, 
            height,
            presentMode,
            m_Swapchain
        );
    }

    m_Format = format;
    
    if (!m_Swapchain) {
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
    }
    else {
        vkDestroySwapchainKHR(core.Device, m_Swapchain, nullptr);

        for (const auto& imageView : m_ImageViews) {
            vkDestroyImageView(core.Device, imageView, nullptr);
        }
        m_ImageViews.clear();
    }

    m_Extent = { width, height };
    m_Images = vkb::GetSwapchainImages(core.Device, swapchain);
    m_ImageViews = vkb::GetSwapchainImageViews(core.Device, m_Images, m_Format);


    m_Outdated = false;

    if (!m_Swapchain) {
        CreateCommandBuffers(core.Device, core.GraphicsQueueFamily);
        CreateSync(core.Device);
    }
    m_Swapchain = swapchain;
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
            .pImageIndices = &imageIndex
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
    VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
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
