#include "BeatEngine/Renderers/Vulkan.h"
#include "BeatEngine/Enum/LogType.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/VulkanWindow.h"
#include "BeatEngine/Renderers/VulkanFrameData.h"
#include "BeatEngine/Renderers/VulkanBoilerplate.h"

#include <cstdint>
#include <memory>
#include <volk.h>
#include <vk_mem_alloc.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>

void VulkanRenderer::pInitVulkan() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing Vulkan handlers");
    volkInitialize();
    
    m_Instance = vkb::CreateInstance("BeatEngine Game", VK_API_VERSION_1_3);
    volkLoadInstance(m_Instance);
    
    m_PhysicalDevice = vkb::CreatePhysicalDevice(m_Instance);
    m_GraphicsQueueFamily = vkb::GetQueueFamily(m_PhysicalDevice);
    m_Device = vkb::CreateDevice(m_PhysicalDevice, m_GraphicsQueueFamily);
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);

    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage
    };
    VmaAllocatorCreateInfo allocatorInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m_PhysicalDevice,
        .device = m_Device,
        .pVulkanFunctions = &vkFunctions,
        .instance = m_Instance
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));

    SDL_Vulkan_CreateSurface(std::static_pointer_cast<VulkanWindow>(m_Window)->GetWindowImpl(), m_Instance, nullptr, &m_Surface);
}

void VulkanRenderer::pUninitVulkan() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying Vulkan handlers");

    vmaDestroyAllocator(m_Allocator);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanRenderer::pInitSwapchain() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing Swapchain");

    auto pair = m_Window->GetSize();
    auto width = pair.X, height = pair.Y;

    m_Swapchain = vkb::CreateSwapchainKHR(m_Device, m_PhysicalDevice, m_Surface, width, height);
    uint32_t imageCount{ 0 };
	VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr));
	m_Images.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data()));
	m_ImageViews.resize(imageCount);
    for (auto i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
            .image = m_Images[i], 
            .viewType = VK_IMAGE_VIEW_TYPE_2D, 
            .format = VK_FORMAT_B8G8R8A8_SRGB, 
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
                .levelCount = 1, 
                .layerCount = 1 
            } 
        };
		VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageViews[i]));
	}
}

void VulkanRenderer::pUninitSwapchain() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying Swapchain");

    for (auto i = 0; i < m_ImageViews.size(); i++) {
		vkDestroyImageView(m_Device, m_ImageViews[i], nullptr);
	}
    
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
}

void VulkanRenderer::pInitCommands() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing Command interface");
    VkCommandPoolCreateInfo commandPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 
        .queueFamilyIndex = m_GraphicsQueueFamily
    };
	VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_CommandPool));

	VkCommandBufferAllocateInfo cbAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
        .commandPool = m_CommandPool, 
        .commandBufferCount = FRAME_OVERLAP
    };
	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cbAllocInfo, m_CommandBuffers.data()));
}

void VulkanRenderer::pUninitCommands() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying Command interface");

    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
}

void VulkanRenderer::pInitSyncStructures() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing Sync Structures");
    VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (auto i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Frames[i].RenderFence));
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frames[i].PresentSemaphore));
    }
    m_RenderSemaphores.resize(m_Images.size());
    for (auto& semaphore : m_RenderSemaphores) {
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &semaphore));
    }
}

void VulkanRenderer::pUninitSyncStructures() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying Sync Structures");

    for (auto i = 0; i < FRAME_OVERLAP; i++) {
        vkDestroyFence(m_Device, m_Frames[i].RenderFence, nullptr);
        vkDestroySemaphore(m_Device, m_Frames[i].PresentSemaphore, nullptr);
    }

    for (auto i = 0; i < m_RenderSemaphores.size(); i++) {
        vkDestroySemaphore(m_Device, m_RenderSemaphores[i], nullptr);
    }
}

void VulkanRenderer::pInitDescriptor() {
    
}

void VulkanRenderer::pInitDescriptorLayout() {

}

void VulkanRenderer::pInitDescriptorPool() {

}

void VulkanRenderer::pInitPipeline() {
    // VkPipelineLayoutCreateInfo pipelineLayoutInfo{
    //     .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    //     .setLayoutCount = 1,
    //     .pSetLayouts
    // }
}

void VulkanRenderer::pUninitPipeline() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying pipeline");
}

void VulkanRenderer::Init(std::string windowTitle, Vector2u windowSize) {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing VulkanRenderer");

    if (m_Window == nullptr)
        m_Window = std::make_shared<VulkanWindow>();

    m_Window->Init(windowTitle, windowSize);

    pInitVulkan();
    pInitSwapchain();
    pInitCommands();
    pInitSyncStructures();
}

void VulkanRenderer::Uninit() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Waiting on GPU to shutdown");
    
    vkDeviceWaitIdle(m_Device);

    pUninitCommands();
    pUninitSyncStructures();
    pUninitSwapchain();
    pUninitVulkan();

    m_Window->Uninit();
}

void VulkanRenderer::Render() {
    
    auto frameIndex = (m_FrameNumber % FRAME_OVERLAP);
    
    VK_CHECK(vkWaitForFences(m_Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000));
    VK_CHECK(vkResetFences(m_Device, 1, &GetCurrentFrame().RenderFence));

    vkAcquireNextImageKHR(m_Device, m_Swapchain, 1000000000, GetCurrentFrame().PresentSemaphore, VK_NULL_HANDLE, &m_ActiveImageIndex);
   
    m_ActiveCmdBuffer = m_CommandBuffers[frameIndex];
    VK_CHECK(vkResetCommandBuffer(m_ActiveCmdBuffer, 0));
    VkCommandBufferBeginInfo cmdInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK(vkBeginCommandBuffer(m_ActiveCmdBuffer, &cmdInfo));
    
    auto outputBarrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .image = m_Images[m_ActiveImageIndex],
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };

    // std::array<VkImageMemoryBarrier2, 2> outputBarriers{
    //     VkImageMemoryBarrier2{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //         .srcAccessMask = 0,
    //         .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //         .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    //         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //         .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    //         .image = m_Images[m_ActiveImageIndex],
    //         .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    //     },
    //     VkImageMemoryBarrier2{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //         .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //         .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //         .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //         .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //         .newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
    //         .image = depthImage,
    //         .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, .levelCount = 1, .layerCount = 1 }
    //     }
    // };

    VkDependencyInfo barrierDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &outputBarrier

    };
    vkCmdPipelineBarrier2(m_ActiveCmdBuffer, &barrierDependencyInfo);
    VkRenderingAttachmentInfo colorAttachmentInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_ImageViews[m_ActiveImageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{ .color{ 0.0f, 0.0f, 0.0f, 1.0f } }
    };
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{ .extent{
                .width = static_cast<uint32_t>(GetWindow()->GetSize().X),
                .height = static_cast<uint32_t>(GetWindow()->GetSize().Y),
        } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentInfo
    };
    vkCmdBeginRendering(m_ActiveCmdBuffer, &renderingInfo);
    VkViewport vp{
        .width = static_cast<float>(GetWindow()->GetSize().X),
        .height = static_cast<float>(GetWindow()->GetSize().Y)
    };
    vkCmdSetViewport(m_ActiveCmdBuffer, 0, 1, &vp);
    // vkCmdBindPipeline(m_ActiveCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
    vkCmdEndRendering(m_ActiveCmdBuffer);
    VkImageMemoryBarrier2 barrierPresent{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = m_Images[frameIndex],
        .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    };

    VkDependencyInfo barrierPresentDependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrierPresent
    };
    vkCmdPipelineBarrier2(m_ActiveCmdBuffer, &barrierPresentDependencyInfo);
    VK_CHECK(vkEndCommandBuffer(m_ActiveCmdBuffer));
    
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &GetCurrentFrame().PresentSemaphore,
        .pWaitDstStageMask = &waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_ActiveCmdBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_RenderSemaphores[m_ActiveImageIndex],
    };
    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, GetCurrentFrame().RenderFence);
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderSemaphores[m_ActiveImageIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &m_ActiveImageIndex
    };
    VK_CHECK(vkQueuePresentKHR(m_GraphicsQueue, &presentInfo));

    m_FrameNumber++;
}

void VulkanRenderer::Display() {

}

void VulkanRenderer::Clear() {

}

void VulkanRenderer::Update() {
    

    if (false) {
        
    }
}

std::optional<Base::Event> VulkanRenderer::PollEvent() const {
    return m_Window->PollEvent();
}

std::shared_ptr<BaseWindow> VulkanRenderer::GetWindow() const {
    return m_Window;
}
