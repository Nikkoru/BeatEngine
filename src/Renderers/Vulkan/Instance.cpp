#include "BeatEngine/Renderers/Vulkan/Instance.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Renderers/Vulkan/GPUBuffer.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "SDL3/SDL_vulkan.h"
#include "backends/imgui_impl_vulkan.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>
#include <volk.h>


void VK::Instance::Init(GameContext* context, std::string appName, uint32_t deviceIndex, std::shared_ptr<BaseWindow> window, VSyncMode vSync) {
    m_Context = context;
    InitVulkan(window, appName.c_str(), deviceIndex);
    m_Executor.Init(m_Core.Device, m_Core.GraphicsQueueFamily, m_Core.GraphicsQueue);

    auto size = window->GetSize();
    m_Swapchain.Create(m_Core, window, size.X, size.Y, vSync);

    CheckDeviceCapabilities();
    m_ImageCache.BindlessSetMgr.Init(m_Core.Device, m_MaxSamplerAnisotropy);

    {
        const auto black = 0xFF000000;
        const auto magenta = 0xFFFF00FF;

        std::array<uint32_t, 4> pixels{ black, magenta, magenta, black };
        m_ErrorTexture = CreateImage(
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags{},
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = {2, 2, 1},
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling{},
                .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                .sharingMode{},
                .queueFamilyIndexCount{},
                .pQueueFamilyIndices{},
                .initialLayout{}
            },
            pixels.data()
        );
        m_ImageCache.SetMissingImageID(m_ErrorTexture.m_CacheID);
    }

    {
        uint32_t pixel = 0xFFFFFFFF;
        m_WhiteTexture = CreateImage(
            {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags{},
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .extent = {1, 1, 1},
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling{},
                .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                .sharingMode{},
                .queueFamilyIndexCount{},
                .pQueueFamilyIndices{},
                .initialLayout{}
            },
            &pixel
        );
    }

    if (m_Context->GFlags & GameFlags_ImGui)
        InitImGui(window);

    m_Uninitializers.AddCallback([&]() {
        DestroyImage(m_ErrorTexture.m_CacheID);
        DestroyImage(m_WhiteTexture.m_CacheID);

        if (m_ImageAllocs > 0)
            THROW_RUNTIME_ERROR("Images are still being allocated. Did you forget to destroy a image?");
        if (m_BufferAllocs > 0)
            THROW_RUNTIME_ERROR("Buffers are still being allocated. Did you forget to destroy a buffer?");
    });
}

void VK::Instance::Uninit() {
    m_Uninitializers.Flush();
}

void VK::Instance::AttachImageData(ImageID textureID, const void* pixelData, Vector2u offset, Vector2u extent, uint32_t layer) {
    auto image = m_ImageCache.GetImage(textureID);
    AttachImageData(image, pixelData, offset, extent, layer);
}

void VK::Instance::AttachImageData(AllocatedImage& image, const void* pixelData, Vector2u offset, Vector2u extent, uint32_t layer) {
    int numChannels = 4;
    uint32_t dataSize{};

    if (extent == Vector2u{})
        dataSize = image.Extent.depth * image.Extent.height * image.Extent.width * numChannels;
    else 
        dataSize = extent.X * extent.Y * numChannels;

    if (extent + offset >= Vector2u{ image.Extent.width, image.Extent.height })
        THROW_RUNTIME_ERROR("Dest position is outside of the image extent");
        

    const auto uploadBuffer = CreateBuffer(dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    memcpy(uploadBuffer.AllocationInfo.pMappedData, pixelData, dataSize);

    VkExtent3D vkExtent;
    vkExtent.depth = image.Extent.depth;
    
    if (extent == Vector2u{}) {
        vkExtent.height = image.Extent.height;
        vkExtent.width = image.Extent.width;
    }
    else {
        vkExtent.height = extent.Y;
        vkExtent.width = extent.X;
    }

    m_Executor.ImmediateSumbit([&](VkCommandBuffer cmd) {
        vku::TransitionImage(cmd, image.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        const auto copyRegion = VkBufferImageCopy{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = layer,
                .layerCount = 1
            },
            .imageOffset = {
                .x = static_cast<int32_t>(offset.X),
                .y = static_cast<int32_t>(offset.Y),
                .z{}
            },
            .imageExtent = vkExtent
        };

        vkCmdCopyBufferToImage(cmd, uploadBuffer.Buffer, image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        if (image.MipLevels > 1) {
            Logger::AddWarning(typeid(VK::Instance), "MipMaps are not implemented yet");
        } else {
            vku::TransitionImage(cmd, image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    });
    DestroyBuffer(uploadBuffer);
}

void VK::Instance::InitVulkan(std::shared_ptr<BaseWindow> window, const char* appName, uint32_t deviceIndex) {
    AddVulkanLog("Initializing Vulkan handlers");
    VK_CHECK(volkInitialize());

    std::vector<const char*> extensions = {};
    std::vector<const char*> layers = {};

    if (m_Context->EFlags & EnvFlags_Debug) {
        extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        layers.emplace_back("VK_LAYER_KHRONOS_validation");
    }

    std::string name = appName;
    if (name.empty())
        name = "BeatEngine";

    m_Core.Instance = vkb::CreateInstance(name, VK_API_VERSION_1_3, extensions, layers);
    volkLoadInstance(m_Core.Instance);

    extensions.clear();
    extensions.emplace_back(VK_KHR_SHADER_RELAXED_EXTENDED_INSTRUCTION_EXTENSION_NAME);

    m_Core.PhysicalDevice = vkb::CreatePhysicalDevice(m_Core.Instance, m_Core.DeviceExtensions, deviceIndex);
    m_Core.GraphicsQueueFamily = vkb::GetQueueFamily(m_Core.PhysicalDevice);
    m_Core.Device = vkb::CreateDevice(m_Core.PhysicalDevice, m_Core.GraphicsQueueFamily, extensions, m_Core.DeviceExtensions);
    volkLoadDevice(m_Core.Device);

    AddNameToVKObject(m_Core.Device, VK_OBJECT_TYPE_DEVICE, (uint64_t)m_Core.Device, "VkDevice");
    AddNameToVKObject(m_Core.Device, VK_OBJECT_TYPE_PHYSICAL_DEVICE, (uint64_t)m_Core.PhysicalDevice, "VkPhysicalDevice");
    AddNameToVKObject(m_Core.Device, VK_OBJECT_TYPE_INSTANCE, (uint64_t)m_Core.Instance, "VkInstance");
    vkGetDeviceQueue(m_Core.Device, m_Core.GraphicsQueueFamily, 0, &m_Core.GraphicsQueue);

    VmaVulkanFunctions vkFunctions{}; // for stoping the -Wmissing-field-initializers we're going to set the variables from the obj and not a from initializer list
    vkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vkFunctions.vkCreateImage = vkCreateImage;

    VmaAllocatorCreateInfo allocatorInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = m_Core.PhysicalDevice,
        .device = m_Core.Device,
        .preferredLargeHeapBlockSize{},
        .pAllocationCallbacks{},
        .pDeviceMemoryCallbacks{},
        .pHeapSizeLimit{},
        .pVulkanFunctions = &vkFunctions,
        .instance = m_Core.Instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
        .pTypeExternalMemoryHandleTypes{}
    };
    VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_Core.Allocator));

    if (m_Context->EFlags & EnvFlags_Debug) {
        VkDebugUtilsMessengerCreateInfoEXT messengerInfo{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags{},
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


        VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_Core.Instance, &messengerInfo, nullptr, &m_Core.DebugMessenger));
    }

    if (auto sdlWindow = std::static_pointer_cast<SDLWindow>(window); !SDL_Vulkan_CreateSurface(sdlWindow->GetWindowImpl(), m_Core.Instance, nullptr, &m_Core.Surface)) {
        std::string msg = "Failed to create surface for SDL Window";
        Logger::AddCritical("", msg);
        THROW_RUNTIME_ERROR(msg);
    }

    m_Uninitializers.AddCallback([&]() {
        m_Swapchain.Uninit();

        AddVulkanLog("Destroying Vulkan handlers");

        vmaDestroyAllocator(m_Core.Allocator);
        vkDestroyDebugUtilsMessengerEXT(m_Core.Instance, m_Core.DebugMessenger, nullptr);
        vkDestroyDevice(m_Core.Device, nullptr);
        vkDestroyInstance(m_Core.Instance, nullptr);
    });
}

void VK::Instance::InitImGui(std::shared_ptr<BaseWindow> window) {
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
        .pNext = nullptr,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = static_cast<uint32_t>(std::size(poolSizes)),
        .pPoolSizes = poolSizes

    };

	VkDescriptorPool imguiPool;
	VK_CHECK(vkCreateDescriptorPool(m_Core.Device, &poolInfo, nullptr, &imguiPool));

    ImGui::CreateContext();
    
    if (m_Context->GFlags & GameFlags_ImGuiDocking)
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    GImGui->ItemUnclipByLog = true; 

    window->InitImGui();

    auto swapchainFormat = m_Swapchain.GetFormat();
     
    ImGui_ImplVulkan_InitInfo info{
        .ApiVersion = VK_API_VERSION_1_3,
        .Instance = m_Core.Instance,
        .PhysicalDevice = m_Core.PhysicalDevice,
        .Device = m_Core.Device,
        .QueueFamily = m_Core.GraphicsQueueFamily,
        .Queue = m_Core.GraphicsQueue,
        .DescriptorPool = imguiPool,
        .DescriptorPoolSize{},
        // .DescriptorPoolSize{},
        .MinImageCount = static_cast<uint32_t>(m_Swapchain.GetImagesSize()),
        .ImageCount = static_cast<uint32_t>(m_Swapchain.GetImagesSize()),
        .PipelineCache = VK_NULL_HANDLE,
        // .PipelineInfoMain{},
        .PipelineInfoMain = {
            // .RenderPass = m_RenderPass,
            .RenderPass = VK_NULL_HANDLE,
            .Subpass = 0,
            .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
            .ExtraDynamicStates{},
            .PipelineRenderingCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                .pNext = nullptr,
                .viewMask{},
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &swapchainFormat,
                .depthAttachmentFormat{},
                .stencilAttachmentFormat{}
            },
            .SwapChainImageUsage{},
        },
        .PipelineInfoForViewports{},
        .UseDynamicRendering = true,
        .Allocator = nullptr,
        .CheckVkResultFn = nullptr,
        .MinAllocationSize = 0,
        .CustomShaderVertCreateInfo{},
        .CustomShaderFragCreateInfo{},
    };

    ImGui_ImplVulkan_Init(&info);

    m_Uninitializers.AddCallback([&, window]() {
        AddVulkanLog("Destroying ImGui Hooks");

        ImGui_ImplVulkan_Shutdown();
        window->UninitImGui();
        vkDestroyDescriptorPool(m_Core.Device, imguiPool, nullptr);
    });
}

ImageID VK::Instance::AddImageToCache(AllocatedImage& image) {
    auto id = m_ImageCache.AddImage(image);
    image.CachedID = id;
    return id;
}

GPUBuffer VK::Instance::CreateBuffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
    const auto bufferInfo = VkBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags{},
        .size = size,
        .usage = usage,
        .sharingMode{},
        .queueFamilyIndexCount{},
        .pQueueFamilyIndices{}
    };

    const auto allocInfo = VmaAllocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = memoryUsage,
        .requiredFlags{},
        .preferredFlags{},
        .memoryTypeBits{},
        .pool{},
        .pUserData{},
        .priority{}
    };

    GPUBuffer buf{};
    VK_CHECK(vmaCreateBuffer(m_Core.Allocator, &bufferInfo, &allocInfo, &buf.Buffer, &buf.Allocation, &buf.AllocationInfo));
    m_BufferAllocs++;

    if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0) {
        const auto deviceAddressInfo = VkBufferDeviceAddressInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .pNext = nullptr,
            .buffer = buf.Buffer
        };

        buf.Address = vkGetBufferDeviceAddress(m_Core.Device, &deviceAddressInfo);
    }

    buf.BufferSize = size;

    return buf;
}

void VK::Instance::DestroyBuffer(const GPUBuffer& buffer) {
    vmaDestroyBuffer(m_Core.Allocator, buffer.Buffer, buffer.Allocation);
    m_BufferAllocs--;
    Logger::AddDebug(typeid(VulkanRenderer), "Current buffer allocs: {}", m_BufferAllocs);
}

AllocatedImage VK::Instance::CreateImageRaw(
    VkImageCreateInfo info,
    std::optional<VmaAllocationCreateInfo> customAllocInfo
) {
    if (info.mipLevels <= 0) {
        info.mipLevels = 1;
    }

    if (info.arrayLayers <= 0) {
        info.arrayLayers = 1;
    }

    if (info.samples <= 0) {
        info.samples = VK_SAMPLE_COUNT_1_BIT;
    }

    if (info.imageType <= 0) {
        info.imageType = VK_IMAGE_TYPE_2D;
    }

    if (auto status = vkGetPhysicalDeviceImageFormatProperties(m_Core.PhysicalDevice, info.format, info.imageType, info.tiling, info.usage, info.flags, &m_Core.ImageProperties); status != VK_SUCCESS) {
        Logger::AddError(typeid(VulkanRenderer), "Unable to create image; {}", string_VkResult(status));
        return {};
    }

    if (m_Core.ImageProperties.maxExtent.width < info.extent.width ||
        m_Core.ImageProperties.maxExtent.depth < info.extent.depth ||
        m_Core.ImageProperties.maxExtent.height < info.extent.height) {
        Logger::AddWarning(
            typeid(VulkanRenderer), 
            "Size mismatch; device can only create images from less or equal this size: (W: {}, H: {}, D: {}), but the required size was: (W: {}, H: {}, D: {})",
            m_Core.ImageProperties.maxExtent.width,
            m_Core.ImageProperties.maxExtent.height,
            m_Core.ImageProperties.maxExtent.depth,
            info.extent.width,
            info.extent.height,
            info.extent.depth
        );
    }

    if (info.mipLevels < 1) {
        Logger::AddWarning("Invalid mip levels");
    }

    static const auto defaultAllocInfo = VmaAllocationCreateInfo{
        .flags{},
        .usage = VMA_MEMORY_USAGE_AUTO,
        .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        .preferredFlags{},
        .memoryTypeBits{},
        .pool{},
        .pUserData{},
        .priority{}
    };

    const auto allocInfo = customAllocInfo 
        ? customAllocInfo.value()
        : defaultAllocInfo;

    AllocatedImage image{};
    image.Format = info.format;
    image.Usage = info.usage;
    image.Extent = info.extent;
    image.MipLevels = info.mipLevels;

    
    VK_CHECK(vmaCreateImage(m_Core.Allocator, &info, &allocInfo, &image.Image, &image.Allocation, nullptr));
    m_ImageAllocs++;

    bool shouldCreateView = ((info.usage & VK_IMAGE_USAGE_SAMPLED_BIT) != 0) ||
                            ((info.usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) ||
                            ((info.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) != 0) ||
                            ((info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0);

    if (shouldCreateView) {
        VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
        if (info.format == VK_FORMAT_D32_SFLOAT) { // TODO: support other depth formats
            aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
        }

        auto viewType = info.arrayLayers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        if (info.arrayLayers == 6) {
            viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        }

        const auto viewCreateInfo = VkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags{},
            .image = image.Image,
            .viewType = viewType,
            .format = info.format,
            .components{},
            .subresourceRange =
                VkImageSubresourceRange{
                    .aspectMask = aspectFlag,
                    .baseMipLevel = 0,
                    .levelCount = info.mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = info.arrayLayers,
                },
        };

        VK_CHECK(vkCreateImageView(m_Core.Device, &viewCreateInfo, nullptr, &image.ImageView));
    }

    return image;
}

void VK::Instance::BindBindlessDescSet(VkCommandBuffer cmd, VkPipelineLayout layout) {
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        layout,
        0,
        1,
        &m_ImageCache.BindlessSetMgr.GetSet(),
        0,
        nullptr
    );
}

VkCommandBuffer VK::Instance::BeginFrame() {
    m_Swapchain.BeginFrame(m_Core.Device, GetCurrentFrameIndex());

    const auto& frame = m_Swapchain.GetFrame(GetCurrentFrameIndex());
    const auto& cmd = frame.ActiveCmdBuffer;
    const auto cmdInfo = VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo{}
    };
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdInfo));

    return cmd;
}

void VK::Instance::EndFrame(VkCommandBuffer cmd, AllocatedImage& drawImage) {
    const auto [swapchainImage, swapchainImageIndex] = m_Swapchain.AcquireImage(m_Core.Device, GetCurrentFrameIndex());

    if (!swapchainImage || NeedsRecreateSwapchain()) {
        if (m_Context->GFlags & GameFlags_ImGui) {
            ImGui::Render();
            ImGui::EndFrame();
        }
        return;
    }

    m_Swapchain.ResetFence(m_Core.Device, GetCurrentFrameIndex());
    
    auto swapchainLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vku::TransitionImage(
        cmd,
        drawImage.Image,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
    );
    vku::TransitionImage(
        cmd, 
        swapchainImage, 
        swapchainLayout, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );
    swapchainLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    // const auto filter = VK_FILTER_LINEAR;
    if (true) {
        vku::CopyImageToImage(
            cmd,
            drawImage.Image,
            swapchainImage,
            {drawImage.Extent.width, drawImage.Extent.height },
            {drawImage.Extent.width, drawImage.Extent.height }
            // props.drawImageBlitRect.x,
            // props.drawImageBlitRect.y,
            // props.drawImageBlitRect.z,
            // props.drawImageBlitRect.w,
            /* filter */);
    } else {
        // will stretch image to swapchain
        vku::CopyImageToImage(
            cmd,
            drawImage.Image,
            swapchainImage,
            { drawImage.Extent.width, drawImage.Extent.height },
            { m_Swapchain.GetExtent().X, m_Swapchain.GetExtent().Y }  
            /* filter */);
    }

    vku::TransitionImage(cmd, swapchainImage, swapchainLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    swapchainLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // vku::TransitionImage({}, cmd, drawImage.Image, swapchainLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    // swapchainLayout = V_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    if (m_Context->GFlags & GameFlags_ImGui) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        ImGui::EndFrame();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui::UpdatePlatformWindows();
    }
    
    VK_CHECK(vkEndCommandBuffer(cmd));

    m_Swapchain.SubmitAndPresent(cmd, m_Core.GraphicsQueue, GetCurrentFrameIndex(), swapchainImageIndex);

    m_FrameNumber++;
}

void VK::Instance::ClearImage(VkCommandBuffer cmd, VkClearColorValue clearColor, VkImageSubresourceRange imageRange) {
    vku::TransitionImage(cmd, m_Swapchain.GetImage(GetCurrentFrameIndex()), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
    vkCmdClearColorImage(cmd, m_Swapchain.GetImage(GetCurrentFrameIndex()), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);
}

AllocatedImage VK::Instance::CreateImage(VkImageCreateInfo info, const void* pixelData, bool cache) {
    auto image = CreateImageRaw(info);
    if (pixelData) {
        AttachImageData(image, pixelData);
    }

    image.Linear = m_ImageCache.BindlessSetMgr.GetLinearSampler();

    if (cache) {
        auto id = m_ImageCache.AddImage(image);
        image.CachedID = id;
    }

    return image;
}

void VK::Instance::CopyImageToImage(ImageID srcID, ImageID dstID) {
    auto src = m_ImageCache.GetImage(srcID);
    auto dst = m_ImageCache.GetImage(dstID);

    m_Executor.ImmediateSumbit([&](VkCommandBuffer cmd) {
        vku::CopyImageToImage(cmd, src.Image, dst.Image, { src.Extent.width, src.Extent.height },{ dst.Extent.width, dst.Extent.height });
    });
}

void VK::Instance::DestroyImage(const ImageID imageID) {
    const auto& allocImage = m_ImageCache.GetImage(imageID);
    DestroyImage(allocImage);
}

void VK::Instance::DestroyImage(const AllocatedImage& image) {
    vkDestroyImageView(m_Core.Device, image.ImageView, nullptr);
    vmaDestroyImage(m_Core.Allocator, image.Image, image.Allocation);
    m_ImageAllocs--;
    Logger::AddDebug(typeid(VulkanRenderer), "Current image allocs: {}", m_ImageAllocs);
}

AllocatedImage VK::Instance::CreateDrawImage(Vector2u size) {
    assert(size.X > 0 && size.Y > 0);
    const auto extent = VkExtent3D{
        .width = size.X,
        .height = size.Y,
        .depth = 1
    };

    VkImageUsageFlags usage{};
    usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
             VK_IMAGE_USAGE_TRANSFER_DST_BIT |
             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
             VK_IMAGE_USAGE_SAMPLED_BIT;

    const auto info = vki::GetImageCreateInfo(m_Swapchain.GetFormat(), usage, extent);
    return CreateImage(info, nullptr, false);
}

void VK::Instance::CheckDeviceCapabilities() {
    AddVulkanLog("Checking device capabilities...");

    vkGetPhysicalDeviceProperties(m_Core.PhysicalDevice, &m_Core.DeviceProperties);

    const auto& props = m_Core.DeviceProperties;

    m_MaxSamplerAnisotropy = m_Core.DeviceProperties.limits.maxSamplerAnisotropy;

    { // store which sampling counts HW supports
        const auto counts = std::array{
            VK_SAMPLE_COUNT_1_BIT,
            VK_SAMPLE_COUNT_2_BIT,
            VK_SAMPLE_COUNT_4_BIT,
            VK_SAMPLE_COUNT_8_BIT,
            VK_SAMPLE_COUNT_16_BIT,
            VK_SAMPLE_COUNT_32_BIT,
            VK_SAMPLE_COUNT_64_BIT,
        };

        const auto supportedByDepthAndColor =
            props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
        m_SupportedSampleCounts = {};
        for (const auto& count : counts) {
            if (supportedByDepthAndColor & count) {
                m_SupportedSampleCounts = (VkSampleCountFlagBits)(m_SupportedSampleCounts | count);
                m_HighestSupportedSample = count;
            }
            else {
                Logger::AddWarning(typeid(VulkanRenderer), "Unsuported sample count: {}", string_VkSampleCountFlagBits(count));
            }
        }
    }
}

void VK::Instance::WaitIdle() {
    vkDeviceWaitIdle(m_Core.Device);
}

void VK::Instance::RecreateSwapchain(std::shared_ptr<BaseWindow> window, Vector2u size) {
    WaitIdle();
    m_Swapchain.Uninit();
    m_Swapchain.Create(m_Core, window, size.X, size.Y);
}

namespace {
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
}

VKAPI_ATTR VkBool32 VKAPI_CALL VK::Instance::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    (void)pUserData;

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
