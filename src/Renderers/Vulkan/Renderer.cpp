#include "BeatEngine/Renderers/Vulkan/Renderer.h"

#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/DescriptorBuilder.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/System/Clock.h"
#include "BeatEngine/System/Time.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/GameContext.h"

#include <SDL3/SDL_video.h>
#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_sdl3.h>
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

template<typename... Args>
void AddVulkanLog(std::string fmt, Args&&... elms) {
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", fmt, elms...);
}

void VulkanRenderer::pInitVulkan() {
    AddVulkanLog("Initializing Vulkan handlers");
    volkInitialize();

    std::vector<const char*> extensions = {};
    std::vector<const char*> layers = {};

    if (m_Context->EFlags & EnvFlags_Debug) {
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    m_Instance = vkb::CreateInstance("BeatEngine Game", VK_API_VERSION_1_3, extensions, layers);
    volkLoadInstance(m_Instance);

    m_PhysicalDevice = vkb::CreatePhysicalDevice(m_Instance);
    m_GraphicsQueueFamily = vkb::GetQueueFamily(m_PhysicalDevice);
    m_Device = vkb::CreateDevice(m_PhysicalDevice, m_GraphicsQueueFamily);
    volkLoadDevice(m_Device);
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_DEVICE, (uint64_t)m_Device, "VkDevice");
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)m_PhysicalDevice, "VkPhysicalDevice");
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_INSTANCE, (uint64_t)m_Instance, "VkInstance");
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);

    VmaVulkanFunctions vkFunctions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        .vkCreateImage = vkCreateImage,
    };
    VmaAllocatorCreateInfo allocatorInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m_PhysicalDevice,
        .device = m_Device,
        .pVulkanFunctions = &vkFunctions,
        .instance = m_Instance
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
    
    if (m_Context->EFlags & EnvFlags_Debug)
        CreateDebugCallback();

    SDL_Vulkan_CreateSurface(std::static_pointer_cast<SDLWindow>(m_Window)->GetWindowImpl(), m_Instance, nullptr, &m_Surface);
    
    m_Uninitializers.AddCallback([this](){ 
        AddVulkanLog("Destroying Vulkan handlers");

        vmaDestroyAllocator(m_Allocator);
        vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        vkDestroyDevice(m_Device, nullptr);
        vkDestroyInstance(m_Instance, nullptr);
    });
}

void VulkanRenderer::pInitSwapchain() {
    AddVulkanLog("Initializing Swapchain");

    auto pair = m_Window->GetSize();
    auto width = pair.X, height = pair.Y;
    
    auto extent = VkExtent3D{
        .width = width,
        .height = height,
        .depth = 1
    };

    m_Swapchain = vkb::CreateSwapchainKHR(m_Device, m_PhysicalDevice, m_Surface, width, height);
    m_SwapchainImages = vkb::GetSwapchainImages(m_Device, m_Swapchain);
    m_SwapchainImageViews = vkb::GetSwapchainImageViews(m_Device, m_SwapchainImages, m_SwapchainFormat);
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "VkSwapchainKHR");
    
    m_DrawImage = {
        .ImageExtent = extent,
        .ImageFormat = m_SwapchainFormat
    };

    VkImageUsageFlags drawImageUsages = 
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
        VK_IMAGE_USAGE_STORAGE_BIT | 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto info = vki::GetImageCreateInfo(m_DrawImage.ImageFormat, drawImageUsages, extent);

    VmaAllocationCreateInfo allocInfo{
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    vmaCreateImage(m_Allocator, &info, &allocInfo, &m_DrawImage.Image, &m_DrawImage.Allocation, nullptr);
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_IMAGE, (uint64_t)m_DrawImage.Image, "AllocatedImage_VkImage");
    
    VkImageViewCreateInfo viewInfo = vki::GetImageViewCreateInfo(m_DrawImage.ImageFormat, m_DrawImage.Image, VK_IMAGE_ASPECT_COLOR_BIT); 
    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_DrawImage.ImageView));
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_DrawImage.ImageView, "AllocatedImage_VkImageView");

    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying Swapchain");

        vkDestroyImageView(m_Device, m_DrawImage.ImageView, nullptr);
        vmaDestroyImage(m_Allocator, m_DrawImage.Image, m_DrawImage.Allocation);
        
        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    });
}

void VulkanRenderer::pInitCommands() {
    AddVulkanLog("Initializing Command interface");
    VkCommandPoolCreateInfo commandPoolInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, 
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, 
        .queueFamilyIndex = m_GraphicsQueueFamily
    };

    for (auto i = 0; i < FRAME_OVERLAP; i++) {
        VK_CHECK(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_Frames[i].CommandPool));
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_COMMAND_POOL, uint64_t(m_Frames[i].CommandPool), std::format("VkCommandPool_Frame{}", i));

        VkCommandBufferAllocateInfo cbAllocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, 
            .commandPool = m_Frames[i].CommandPool, 
            .commandBufferCount = 1
        };
        VK_CHECK(vkAllocateCommandBuffers(m_Device, &cbAllocInfo, &m_Frames[i].ActiveCmdBuffer));
    }



    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying Command interface");
        //
        // vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    });
}

void VulkanRenderer::pInitSyncStructures() {
    AddVulkanLog("Initializing Sync Structures");
    VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    for (auto i = 0; i < FRAME_OVERLAP; i++) {
        auto semaphorePresentName = std::format("VkSemaphore_Present{}", i);
        auto semaphoreRenderName = std::format("VkSemaphore_Render{}", i);
        auto fenceName = std::format("VkFence_Render{}", i);

        VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Frames[i].RenderFence));
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_FENCE, (uint64_t)m_Frames[i].RenderFence, fenceName);
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frames[i].PresentSemaphore));
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_Frames[i].PresentSemaphore, semaphorePresentName);
        VK_CHECK(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frames[i].RenderSemaphore));
        AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_Frames[i].RenderSemaphore, semaphoreRenderName);
    }

    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying Sync Structures");

        for (auto i = 0; i < FRAME_OVERLAP; i++) {
            vkDestroyFence(m_Device, m_Frames[i].RenderFence, nullptr);
            vkDestroySemaphore(m_Device, m_Frames[i].PresentSemaphore, nullptr);
            vkDestroySemaphore(m_Device, m_Frames[i].RenderSemaphore, nullptr);
        }
    });
}

void VulkanRenderer::pInitRenderPass() {
    AddVulkanLog("Initializing Render Pass");

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

    // VkSubpassDependency subpassDependecy{
    //     .srcSubpass = VK_SUBPASS_EXTERNAL,
    //     .dstSubpass = 0,
    //     .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .srcAccessMask = 0,
    //     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    // };

    VkRenderPassCreateInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &attachmentDescriptions,
        .subpassCount = 1,
        .pSubpasses = &subpassDescriptions,
        .dependencyCount = 0
        // .dependencyCount = 1,
        // .pDependencies = &subpassDependecy    
    };

    VK_CHECK(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass));

    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying Render Pass");
        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    });
};

void VulkanRenderer::pInitPipeline() {
    AddVulkanLog("Initializing pipeline");

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

    m_PipelineMgr.Init(m_ImageDescriptorLayout, m_ImageDescriptor);

    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying pipeline");
        m_PipelineMgr.DestroyAll(m_Device);
    });
}

void VulkanRenderer::pInitDescriptors() {
    AddVulkanLog("Initializing Descriptors");

    std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    };

    m_GlobalDescriptorAllocator.InitPool(m_Device, 10, sizes);

    DescriptorLayoutBuilder builder;
    builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    m_ImageDescriptorLayout = builder.Build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);

    m_ImageDescriptor = m_GlobalDescriptorAllocator.Allocate(m_Device, m_ImageDescriptorLayout);

    VkDescriptorImageInfo imgInfo{
        .imageView = m_DrawImage.ImageView,
        .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    };

    VkWriteDescriptorSet drawImageWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .pNext = nullptr,
        .dstSet = m_ImageDescriptor,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .pImageInfo = &imgInfo
    };

    vkUpdateDescriptorSets(m_Device, 1, &drawImageWrite, 0, nullptr);

    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying Descriptors");

        m_GlobalDescriptorAllocator.DestroyPool(m_Device);

        vkDestroyDescriptorSetLayout(m_Device, m_ImageDescriptorLayout, nullptr);
    });
}

void VulkanRenderer::pInitFramebuffers() {
    AddVulkanLog("Initializing framebuffers");
    VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = m_RenderPass,
        .attachmentCount = 1,
        .pAttachments = &m_DrawImage.ImageView,
        .width = m_Window->GetSize().X,
        .height = m_Window->GetSize().Y,
        .layers = 1
    };
    VK_CHECK(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_DrawImage.Framebuffer));

    m_Uninitializers.AddCallback([this]() {
        AddVulkanLog("Destroying Framebuffers");
        vkDestroyFramebuffer(m_Device, m_DrawImage.Framebuffer, nullptr);
    });
}

void VulkanRenderer::pInitImGui() {
    AddVulkanLog("Initializing ImGui Hooks");
    VkDescriptorPoolSize poolSizes[] = { 
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
    };

	VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
        .pPoolSizes = poolSizes

    };

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &imguiPool));

    ImGui::CreateContext();
    
    if (m_Context->GFlags & GameFlags_ImGuiDocking)
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForVulkan(std::static_pointer_cast<SDLWindow>(m_Window)->GetWindowImpl());
    
    ImGui_ImplVulkan_InitInfo info{
        .ApiVersion = VK_API_VERSION_1_3,
        .Instance = m_Instance,
        .PhysicalDevice = m_PhysicalDevice,
        .Device = m_Device,
        .QueueFamily = m_GraphicsQueueFamily,
        .Queue = m_GraphicsQueue,
        .DescriptorPool = imguiPool,
        .MinImageCount = 3,
        .ImageCount = 3,
        .PipelineCache = VK_NULL_HANDLE,
        .PipelineInfoMain = {
            .RenderPass = m_RenderPass,
            .Subpass = 0,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT
        },
        // .UseDynamicRendering = true,
        .Allocator = VK_NULL_HANDLE,
        .CheckVkResultFn = VK_CHECK
    };
    ImGui_ImplVulkan_Init(&info);

    m_Uninitializers.AddCallback([this, imguiPool]() {
        AddVulkanLog("Destroying ImGui Hooks");

        ImGui_ImplVulkan_Shutdown();
        m_Window->UninitImGui();
        vkDestroyDescriptorPool(m_Device, imguiPool, nullptr);
    });
}

void VulkanRenderer::Init(std::string windowTitle, Vector2u windowSize) {
    AddVulkanLog("Initializing VulkanRenderer");

    if (m_Window == nullptr) {
        m_Window = std::make_shared<SDLWindow>();
        std::static_pointer_cast<SDLWindow>(m_Window)->SetFlags(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    }

    m_Window->Init(m_Context, windowTitle, windowSize);

    pInitVulkan();
    pInitSwapchain();
    pInitCommands();
    pInitRenderPass();
    pInitFramebuffers();
    pInitDescriptors();
    pInitPipeline();
    pInitSyncStructures();
    
    if (m_Context->GFlags & GameFlags_ImGui) 
        pInitImGui();
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
    AddVulkanLog("Waiting on GPU to shutdown");
    
    vkDeviceWaitIdle(m_Device);

    AddVulkanLog("Shutting down the renderer");

    m_Uninitializers.Flush();

    m_Window->Uninit();
}

void VulkanRenderer::Render() {
    if (m_Context->GFlags & GameFlags_ImGui)
        ImGui_ImplVulkan_NewFrame();
    m_Window->OnRender();
    
    auto frameIndex = (m_FrameNumber % FRAME_OVERLAP);
    
    VK_CHECK(vkWaitForFences(m_Device, 1, &GetCurrentFrame().RenderFence, true, 1'000'000'000));
    VK_CHECK(vkResetFences(m_Device, 1, &GetCurrentFrame().RenderFence));

    if (VK_CHECK_SWAPCHAIN(vkAcquireNextImageKHR(m_Device, m_Swapchain, 1'000'000'000, GetCurrentFrame().PresentSemaphore, VK_NULL_HANDLE, &m_ActiveImageIndex)))
        UpdateSwapchain();
   
    VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().ActiveCmdBuffer, 0));
    VkCommandBufferBeginInfo cmdInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    VK_CHECK(vkBeginCommandBuffer(GetCurrentFrame().ActiveCmdBuffer, &cmdInfo));
    vku::TransitionImage(m_PipelineMgr, GetCurrentFrame().ActiveCmdBuffer, m_DrawImage.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    
    VkRenderingInfo renderingInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea{ .extent{
                .width = static_cast<uint32_t>(GetWindow()->GetSize().X),
                .height = static_cast<uint32_t>(GetWindow()->GetSize().Y),
        } },
        .layerCount = 1,
        .colorAttachmentCount = 0,
    };

    // static Clock clock{};
    // clock.Start();
    // auto time = clock.Get();
    // auto sec = time.AsSeconds();
    // VkClearValue clearColor{ static_cast<float>(sin(sec)), static_cast<float>(cos(sec)), static_cast<float>(tan(sec)), 1.0f };
    // 
    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = m_RenderPass,
        .framebuffer = m_DrawImage.Framebuffer,
        .renderArea{
            .extent{
                .width = static_cast<uint32_t>(GetWindow()->GetSize().X),
                .height = static_cast<uint32_t>(GetWindow()->GetSize().Y),
            }
        },
        .clearValueCount = 0
    };

    if (m_PipelineMgr.BindIfAny(GetCurrentFrame().ActiveCmdBuffer)) {
        VkExtent2D extent{
            .width = m_Window->GetSize().X,
            .height = m_Window->GetSize().Y
        };
        auto colorAttachment = vki::GetRenderingAttachmentInfo(m_DrawImage.ImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        auto renderInfo = vki::GetRenderingInfo(extent, &colorAttachment, nullptr);
        vkCmdBeginRendering(GetCurrentFrame().ActiveCmdBuffer, &renderInfo);
    }
    else {
        vkCmdBeginRenderPass(GetCurrentFrame().ActiveCmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    // vkCmdDraw(GetCurrentFrame().ActiveCmdBuffer, 3, 1, 0, 0);
}

void VulkanRenderer::RenderImGui() {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), GetCurrentFrame().ActiveCmdBuffer);
}

void VulkanRenderer::Display() {
    RenderImGui();

    auto frameIndex = (m_FrameNumber % FRAME_OVERLAP);
    
    if (m_PipelineMgr.Has())
        vkCmdEndRendering(GetCurrentFrame().ActiveCmdBuffer);
    else
        vkCmdEndRenderPass(GetCurrentFrame().ActiveCmdBuffer);

    vku::TransitionImage(m_PipelineMgr, GetCurrentFrame().ActiveCmdBuffer, m_SwapchainImages[m_ActiveImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    VK_CHECK(vkEndCommandBuffer(GetCurrentFrame().ActiveCmdBuffer));

    auto cmdInfo = vki::GetCommandBufferSubmitInfo(GetCurrentFrame().ActiveCmdBuffer);
    auto waitInfo = vki::GetSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, GetCurrentFrame().PresentSemaphore);
    auto signalInfo = vki::GetSemaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, GetCurrentFrame().RenderSemaphore);
    
    auto submitInfo = vki::GetSubmitInfo(&cmdInfo, &signalInfo, &waitInfo);
    vkQueueSubmit2(m_GraphicsQueue, 1, &submitInfo, GetCurrentFrame().RenderFence);

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &GetCurrentFrame().RenderSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &m_Swapchain,
        .pImageIndices = &m_ActiveImageIndex
    };
    VK_CHECK(vkQueuePresentKHR(m_GraphicsQueue, &presentInfo));

    m_FrameNumber++;
}

void VulkanRenderer::Clear() {
    static Clock clock{};
    clock.Start();
    auto time = clock.Get();
    auto sec = time.AsSeconds();

    VkClearColorValue clearColor{ static_cast<float>(sin(sec)), static_cast<float>(cos(sec)), static_cast<float>(tan(sec)), 1.0f };
    VkImageSubresourceRange imageRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    //
    VkImageMemoryBarrier2 presentToClearBarrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_SwapchainImages[m_ActiveImageIndex],
        .subresourceRange = imageRange
    };

    VkDependencyInfo barrierDependency{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &presentToClearBarrier
    };
    //
    vkCmdPipelineBarrier2(GetCurrentFrame().ActiveCmdBuffer, &barrierDependency);
    //
    vkCmdClearColorImage(GetCurrentFrame().ActiveCmdBuffer, m_SwapchainImages[m_ActiveImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageRange);
}

void VulkanRenderer::Update() {
}

void VulkanRenderer::SetGlobalShader(std::shared_ptr<Shader> shader) {

}

void VulkanRenderer::UpdateSwapchain() {
    auto oldSwapchain = m_Swapchain;

    auto size = m_Window->GetSize();
    Logger::AddLog("\e[0;41mVulkan\033[0m", "", "Updating swapchain to size ({}, {})", size.X, size.Y);
    VK_CHECK(vkDeviceWaitIdle(m_Device));
    
    m_Swapchain = vkb::CreateSwapchainKHR(m_Device, m_PhysicalDevice, m_Surface, size.X, size.Y, oldSwapchain);
    vkDestroyImageView(m_Device, m_DrawImage.ImageView, nullptr);

    uint32_t imageCount{};
    VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, &m_DrawImage.Image));

    VkImageViewCreateInfo viewInfo{ 
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, 
        .image = m_DrawImage.Image, 
        .viewType = VK_IMAGE_VIEW_TYPE_2D, 
        .format = VK_FORMAT_B8G8R8A8_UNORM, 
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, 
            .levelCount = 1, 
            .layerCount = 1
        }
    };
    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_DrawImage.ImageView));

    vkDestroySwapchainKHR(m_Device, oldSwapchain, nullptr);
    AddNameToVKObject(m_Device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, (uint64_t)m_Swapchain, "VkSwapchainKHR");
}

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(std::filesystem::path path) {
    return nullptr;
}

std::shared_ptr<Shader> VulkanRenderer::CreateShader(std::filesystem::path path, Shader::Type type) {
    std::shared_ptr<VulkanShader> shader = std::make_shared<VulkanShader>();
        
    shader->SetType(type);
    shader->GetFileContents(path);

    if (!shader->Compile(m_Device))
        return nullptr;

    return shader;
}

std::optional<Base::Event> VulkanRenderer::PollEvent() const {
    return m_Window->PollEvent();
}

std::shared_ptr<BaseWindow> VulkanRenderer::GetWindow() const {
    return m_Window;
}
