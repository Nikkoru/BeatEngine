#include "BeatEngine/Renderers/Vulkan/Renderer.h"

#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/System/Clock.h"
#include "BeatEngine/System/Time.h"
#include "BeatEngine/Util/Exception.h"
#include "SDL3/SDL_video.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <volk.h>
#include <vk_mem_alloc.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>

void AddNameToVKObject(VkDevice device, VkObjectType type, uint64_t objectHandle, std::string name) {
    VkDebugUtilsObjectNameInfoEXT objNameInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .objectType = type,
        .objectHandle = objectHandle,
        .pObjectName = name.c_str()
    };

    VK_CHECK(vkSetDebugUtilsObjectNameEXT(device, &objNameInfo));
}

bool VK_CHECK_SWAPCHAIN(VkResult result) {
    if (result < VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Swapchain out of date. Requesting update");
			return true;
		}
		auto msg = std::format("Vulkan error: {}", string_VkResult(result));
        Logger::AddCritical("", msg);
        THROW_RUNTIME_ERROR(msg);
	}

    return false;
}


void VulkanRenderer::pInitVulkan() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing Vulkan handlers");
    volkInitialize();

    m_Instance = vkb::CreateInstance("BeatEngine Game", VK_API_VERSION_1_3, { VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, { "VK_LAYER_KHRONOS_validation" });
    volkLoadInstance(m_Instance);

    m_PhysicalDevice = vkb::CreatePhysicalDevice(m_Instance);
    m_GraphicsQueueFamily = vkb::GetQueueFamily(m_PhysicalDevice);
    m_Device = vkb::CreateDevice(m_PhysicalDevice, m_GraphicsQueueFamily);
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_DEVICE, (uint64_t)m_Device, "VkDevice");
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)m_PhysicalDevice, "VkPhysicalDevice");
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_INSTANCE, (uint64_t)m_Instance, "VkInstance");
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

    CreateDebugCallback();

    SDL_Vulkan_CreateSurface(std::static_pointer_cast<SDLWindow>(m_Window)->GetWindowImpl(), m_Instance, nullptr, &m_Surface);
}

void VulkanRenderer::pUninitVulkan() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying Vulkan handlers");

    vmaDestroyAllocator(m_Allocator);
    vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanRenderer::pInitSwapchain() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing Swapchain");

    auto pair = m_Window->GetSize();
    auto width = pair.X, height = pair.Y;

    m_Swapchain = vkb::CreateSwapchainKHR(m_Device, m_PhysicalDevice, m_Surface, width, height);
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "VkSwapchainKHR");

    uint32_t imageCount{ 0 };
	VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr));
	m_Images.resize(imageCount);
	m_ImageViews.resize(imageCount);
	VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data()));
    for (auto i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo viewInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
            .image = m_Images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .components{
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1, 
                .baseArrayLayer = 0,
                .layerCount = 1
            } 
        };
		VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageViews[i]));
        auto imageName = std::format("VkImage{}", i);
        auto imageViewName = std::format("VkImageView{}", i);
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_ImageViews[i], imageViewName);
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_IMAGE, (uint64_t)m_Images[i], imageName);
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
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, uint64_t(m_CommandPool), "VkCommandPool");

	VkCommandBufferAllocateInfo cbAllocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
        .commandPool = m_CommandPool, 
        .commandBufferCount = FRAME_OVERLAP
    };
	VK_CHECK(vkAllocateCommandBuffers(m_Device, &cbAllocInfo, m_CommandBuffers.data()));

    for (int i = 0; i < FRAME_OVERLAP; i++) {
        auto cb = m_CommandBuffers[i];
        auto name = std::format("VkCommandBuffer_Frame{}", i);
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_COMMAND_BUFFER, (uint64_t)cb, name);
    }
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
        auto semaphoreName = std::format("VkSemaphore_Present{}", i);
        auto fenceName = std::format("VkFence_Render{}", i);

        VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Frames[i].RenderFence));
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_FENCE, (uint64_t)m_Frames[i].RenderFence, fenceName);
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frames[i].PresentSemaphore));
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_Frames[i].PresentSemaphore, semaphoreName);
    }
    m_RenderSemaphores.resize(m_Images.size());
    int i = 0;
    for (auto& semaphore : m_RenderSemaphores) {
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &semaphore));
        auto name = std::format("VkSemaphore_Render{}", i);

        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)semaphore, name);
        i++;
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

void VulkanRenderer::pInitRenderPass() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing pipeline");

    VkAttachmentReference colorAttachmentReferences{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpassDescriptions{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReferences,
    };

    VkAttachmentDescription attachmentDescriptions{
        .format = m_SwapchainFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkSubpassDependency subpassDependecy{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachmentDescriptions,
        .subpassCount = 1,
        .pSubpasses = &subpassDescriptions,
        .dependencyCount = 1,
        .pDependencies = &subpassDependecy    
    };

    VK_CHECK(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass));
};

void VulkanRenderer::pUninitRenderPass() {
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
}

void VulkanRenderer::pInitPipeline() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing pipeline");

    VkViewport vp{
        .width = static_cast<float>(GetWindow()->GetSize().X),
        .height = static_cast<float>(GetWindow()->GetSize().Y)
    };
    
    VkRect2D scissor{
        .extent{
            .width = GetWindow()->GetSize().X,
            .height = GetWindow()->GetSize().Y
        }
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentStates{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                          VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT |
                          VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };

    VK_CHECK(vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

    VkPipelineVertexInputStateCreateInfo pipelineVertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    VkPipelineViewportStateCreateInfo pipelineViewportInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &vp,
        .scissorCount = 1, 
        .pScissors = &scissor,
    };
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .lineWidth = 1.0,
    };

    VkPipelineMultisampleStateCreateInfo pipelineMultisampleInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentStates,
    };
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderInfos{
        VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main"
        },

        VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main"
        }
    };

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = shaderInfos.size(),
        .pStages = shaderInfos.data(),
        .pVertexInputState = &pipelineVertexInputInfo,
        .pInputAssemblyState = &pipelineInputAssemblyInfo,
        .pViewportState = &pipelineViewportInfo,
        .pRasterizationState = &pipelineRasterizationInfo,
        .pMultisampleState = &pipelineMultisampleInfo,
        .pColorBlendState = &pipelineColorBlendInfo,
        .layout = m_PipelineLayout,
        .renderPass = m_RenderPass
    };
    VK_CHECK(vkCreateGraphicsPipelines(m_Device, nullptr, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline));
}

void VulkanRenderer::pUninitPipeline() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying pipeline");
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
}

void VulkanRenderer::pInitFramebuffers() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing framebuffers");
    uint32_t framebufferCount = m_ImageViews.size();
    m_Framebuffers.resize(framebufferCount);

    for (int i = 0; i < framebufferCount; ++i) {
        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = m_RenderPass,
            .attachmentCount = 1,
            .pAttachments = &m_ImageViews[i],
            .width = m_Window->GetSize().X,
            .height = m_Window->GetSize().Y,
            .layers = 1
        };

        VK_CHECK(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[i]));
    }
}
void VulkanRenderer::pUninitFramebuffers() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Destroying Framebuffers");
    for (auto i = 0; i < m_Framebuffers.size(); i++)
       vkDestroyFramebuffer(m_Device, m_Framebuffers[i], nullptr);
}

void VulkanRenderer::Init(std::string windowTitle, Vector2u windowSize) {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Initializing VulkanRenderer");

    if (m_Window == nullptr) {
        m_Window = std::make_shared<SDLWindow>();
        std::static_pointer_cast<SDLWindow>(m_Window)->SetFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    }

    m_Window->Init(windowTitle, windowSize);

    pInitVulkan();
    pInitSwapchain();
    pInitCommands();
    pInitRenderPass();
    pInitFramebuffers();
    pInitPipeline();
    pInitSyncStructures();
}

std::string GetDebugSeverityStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return "Verbose";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        return "Info";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        return "Warning";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        return "Error";
    default:
        return "Invalid";
    }
}

std::string GetDebugSeverityColorStr(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        return "\e[30;47m";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        return "\e[30;106m";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        return "\e[0;103m";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        return "\e[0;101m";
    default:
        return "\033[0m";
    }
}

std::string GetDebugTypeStr(VkDebugUtilsMessageTypeFlagsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        return "General";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        return "Validation";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        return "Performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
        return "Device Address Binding";
    default:
        return "Invalid";
    }
}


static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    Logger::AddLog("\e[0;41mVulkan Debug\033[0m", "", "{}", pCallbackData->pMessage);
    Logger::AddLog("\e[0;41mVulkan Debug\033[0m", "", "    Type = {}", GetDebugTypeStr(type));
    Logger::AddLog("\e[0;41mVulkan Debug\033[0m", "", "    Severity = {}{}\033[0m", GetDebugSeverityColorStr(severity), GetDebugSeverityStr(severity));
    Logger::AddLog("\e[0;41mVulkan Debug\033[0m", "", "    Objects: {}", pCallbackData->objectCount);
    for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
        auto name = "No name available";
        if (pCallbackData->pObjects[i].pObjectName)
            name = pCallbackData->pObjects[i].pObjectName;

        Logger::AddLog("\e[0;41mVulkan Debug\033[0m", "", "        {:#x} ({})", pCallbackData->pObjects[i].objectHandle, name);

    }

    return VK_FALSE;
}

void VulkanRenderer::CreateDebugCallback() {
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &DebugCallback,
        .pUserData = nullptr
    };
    
    assert(vkCreateDebugUtilsMessengerEXT && "null");

    VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_Instance, &messengerInfo, nullptr, &m_DebugMessenger));
    // AddNameToVKObject(m_Device, VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT, (uint64_t)m_DebugMessenger, "VkDebugUtilsMessengerEXT");
}

void VulkanRenderer::Uninit() {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Waiting on GPU to shutdown");
    
    vkDeviceWaitIdle(m_Device);

    pUninitCommands();
    pUninitPipeline();
    pUninitFramebuffers();
    pUninitRenderPass();
    pUninitSyncStructures();
    pUninitSwapchain();
    pUninitVulkan();

    m_Window->Uninit();
}

void VulkanRenderer::Render() {
    // static Clock clock;
    // clock.Start();
    
    auto frameIndex = (m_FrameNumber % FRAME_OVERLAP);
    
    VK_CHECK(vkWaitForFences(m_Device, 1, &GetCurrentFrame().RenderFence, true, 1000000000));
    VK_CHECK(vkResetFences(m_Device, 1, &GetCurrentFrame().RenderFence));

    m_UpdateSwapchain = VK_CHECK_SWAPCHAIN(vkAcquireNextImageKHR(m_Device, m_Swapchain, 1000000000, GetCurrentFrame().PresentSemaphore, VK_NULL_HANDLE, &m_ActiveImageIndex));
   
    m_ActiveCmdBuffer = m_CommandBuffers[frameIndex];
    VK_CHECK(vkResetCommandBuffer(m_ActiveCmdBuffer, 0));
    VkCommandBufferBeginInfo cmdInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK(vkBeginCommandBuffer(m_ActiveCmdBuffer, &cmdInfo));
    
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{ .extent{
                .width = static_cast<uint32_t>(GetWindow()->GetSize().X),
                .height = static_cast<uint32_t>(GetWindow()->GetSize().Y),
        } },
        .layerCount = 1,
        .colorAttachmentCount = 0,
    };

    static Clock clock{};
    clock.Start();
    auto time = clock.Get();
    auto sec = time.AsSeconds();
    VkClearValue clearColor{ static_cast<float>(sin(sec)), static_cast<float>(cos(sec)), static_cast<float>(tan(sec)), 1.0f };
    
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_RenderPass,
        .framebuffer = m_Framebuffers[m_ActiveImageIndex],
        .renderArea{
            .extent{
                .width = static_cast<uint32_t>(GetWindow()->GetSize().X),
                .height = static_cast<uint32_t>(GetWindow()->GetSize().Y),
            }
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    vkCmdBeginRenderPass(m_ActiveCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    // vkCmdBeginRendering(m_ActiveCmdBuffer, &renderingInfo);
    vkCmdBindPipeline(m_ActiveCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
    vkCmdDraw(m_ActiveCmdBuffer, 3, 1, 0, 0);
}

void VulkanRenderer::Display() {
    auto frameIndex = (m_FrameNumber % FRAME_OVERLAP);

    vkCmdEndRenderPass(m_ActiveCmdBuffer);
    // vkCmdEndRendering(m_ActiveCmdBuffer);
    // VkImageMemoryBarrier2 barrierClearToPresent{
    //     .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //     .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
    //     .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
    //     .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    //     .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
    //     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    //     .image = m_Images[frameIndex],
    //     .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 }
    // };
    //
    // VkDependencyInfo barrierPresentDependencyInfo{
    //     .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //     .imageMemoryBarrierCount = 1,
    //     .pImageMemoryBarriers = &barrierClearToPresent
    // };
    // vkCmdPipelineBarrier2(m_ActiveCmdBuffer, &barrierPresentDependencyInfo);
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

void VulkanRenderer::Clear() {
    // static Clock clock{};
    // clock.Start();
    // auto time = clock.Get();
    // auto sec = time.AsSeconds();
    //
    // VkClearColorValue clearColor{ static_cast<float>(sin(sec)), static_cast<float>(cos(sec)), static_cast<float>(tan(sec)), 1.0f };
    // VkImageSubresourceRange imageRange{
    //     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //     .baseMipLevel = 0,
    //     .levelCount = 1,
    //     .baseArrayLayer = 0,
    //     .layerCount = 1
    // };
    //
    // VkImageMemoryBarrier2 presentToClearBarrier{
    //     .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //     .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
    //     .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
    //     .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
    //     .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    //     .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //     .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .image = m_Images[m_ActiveImageIndex],
    //     .subresourceRange = imageRange
    // };
    //
    // VkDependencyInfo barrierDependency{
    //     .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //     .imageMemoryBarrierCount = 1,
    //     .pImageMemoryBarriers = &presentToClearBarrier
    // };
    //
    // vkCmdPipelineBarrier2(m_ActiveCmdBuffer, &barrierDependency);
    //
    // vkCmdClearColorImage(m_ActiveCmdBuffer, m_Images[m_ActiveImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageRange);
}

void VulkanRenderer::Update() {
    if (m_UpdateSwapchain) {

        m_UpdateSwapchain = false;

        auto oldSwapchain = m_Swapchain;

        auto size = m_Window->GetSize();
        Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Updating swapchain to size ({}, {})", size.X, size.Y);
        VK_CHECK(vkDeviceWaitIdle(m_Device));
        
        m_Swapchain = vkb::CreateSwapchainKHR(m_Device, m_PhysicalDevice, m_Surface, size.X, size.Y, oldSwapchain);

        for (auto i = 0; i < m_ImageViews.size(); i++) {
            vkDestroyImageView(m_Device, m_ImageViews[i], nullptr);
		}
        uint32_t imageCount{};
        VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr));
        m_Images.resize(imageCount);
        VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data()));

        for (auto i = 0; i < imageCount; i++) {
            VkImageViewCreateInfo viewInfo{ 
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
                .image = m_Images[i], 
                .viewType = VK_IMAGE_VIEW_TYPE_2D, 
                .format = VK_FORMAT_B8G8R8A8_SRGB, 
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
                    .levelCount = 1, 
                    .layerCount = 1
                } 
            };
            VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageViews[i]));
        }

        vkDestroySwapchainKHR(m_Device, oldSwapchain, nullptr);
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "VkSwapchainKHR");
    }
}

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(std::filesystem::path path) {
    return nullptr;
}

std::optional<Base::Event> VulkanRenderer::PollEvent() const {
    return m_Window->PollEvent();
}

std::shared_ptr<BaseWindow> VulkanRenderer::GetWindow() const {
    return m_Window;
}
