#include "BeatEngine/Renderers/Vulkan/Renderer.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_sdl3.h>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include "BeatEngine/Enum/AppFlags.hpp"
#include "BeatEngine/Events/AppEvent.hpp"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/Core.h"
#include "BeatEngine/Renderers/Vulkan/ImageData.h"
#include "BeatEngine/Util/Profiler.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/AppContext.hpp"
#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Util/Graphics.hpp"
#include "BeatEngine/Renderers/Vulkan/VulkanRendererData.hpp"

// Lots of implementation for initialization and fragmentation of dedicated
// objects are based in the NVIDIA sample using Vulkan 1.4, shader_object 
// and Dynamic Rendering.
// 
// github link: https://github.com/nvpro-samples/vk_minimal_latest

VulkanRenderer::VulkanRenderer(AppContext* ctx, std::shared_ptr<BaseWindow> window, uint32_t) :
    Renderer(ctx) {
    m_Window = window;

    if (m_Window) {
        m_Window->PrepareInitFor("Vulkan");
        m_Window->Init(ctx);
    }

    AddVulkanLog("Initializing VulkanRenderer");

    VK::ContextCreateConfig config{
        .instanceExtensions{},
        .instanceLayers{},
        .deviceExtensions{},
        .apiVersion = VK_API_VERSION_1_3
    };

    VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeature{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR,
        .pNext = nullptr,
        .unifiedImageLayouts = VK_TRUE,
        .unifiedImageLayoutsVideo = VK_TRUE
    }; 
    VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT,
        .pNext = nullptr,
        .descriptorHeap = VK_TRUE,
        .descriptorHeapCaptureReplay{}
    }; // Descriptor heap replaces traditional descriptor sets/pools with GPU buffer-based bindless descriptors.
       // Samplers and images are written into heap buffers and accessed by index in the shaders.
    VkPhysicalDeviceShaderUntypedPointersFeaturesKHR untypedPtrFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR,
        .pNext = nullptr,
        .shaderUntypedPointers = VK_TRUE
    }; // Untyped pointers: required by descriptor heap
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT,
        .pNext = nullptr,
        .shaderObject = VK_TRUE
    }; // Shader objects: replace VkPipeline for graphics with linkable, reusable VkShaderEXT
       // objects bound via vkCmdBindShadersEXT. Pairs naturally with the layout=NULL design:
       // there is no graphics pipeline object at all, only shader objects + dynamic state.
    
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{}; // to avoid warnings about uninitialized members
        // Extended dynamic state 3: required by shader objects for blend/rasterization
        // state that no longer lives in a pipeline object.
    dynamicState3Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT;
    dynamicState3Features.pNext = nullptr;
    
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicStateFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = nullptr,
        .vertexInputDynamicState = VK_TRUE
    }; // Vertex input dynamic state: required by shader objects (vertex bindings/attributes
       // become a vkCmdSetVertexInputEXT call instead of pipeline state).

    auto shaderRelaxedFeatures = VkPhysicalDeviceShaderRelaxedExtendedInstructionFeaturesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_RELAXED_EXTENDED_INSTRUCTION_FEATURES_KHR,
        .pNext = nullptr,
        .shaderRelaxedExtendedInstruction = VK_TRUE
    };

    VkPhysicalDeviceColorWriteEnableFeaturesEXT colorFeatures {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COLOR_WRITE_ENABLE_FEATURES_EXT,
        .pNext = nullptr,
        .colorWriteEnable = VK_TRUE
    };

    config.deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true, nullptr);
    config.deviceExtensions.emplace_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME, true, &unifiedImageLayoutsFeature);
    config.deviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME, false, &descriptorHeapFeatures);  // Bindless descriptor heap for textures and samplers
    config.deviceExtensions.emplace_back(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME, false, &untypedPtrFeatures);  // Required by bindless
    config.deviceExtensions.emplace_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME, true, &shaderObjectFeatures);  // Graphics: shader objects instead of pipelines
    config.deviceExtensions.emplace_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, true, &dynamicState3Features);  // Required for shader-object blend/rasterization state
    config.deviceExtensions.emplace_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME, true, &vertexInputDynamicStateFeatures);
    config.deviceExtensions.emplace_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, true, nullptr);
    config.deviceExtensions.emplace_back(VK_KHR_MAINTENANCE_5_EXTENSION_NAME, true, nullptr);

    config.deviceExtensions.emplace_back(VK_KHR_SHADER_RELAXED_EXTENDED_INSTRUCTION_EXTENSION_NAME, false, &shaderRelaxedFeatures);
    config.deviceExtensions.emplace_back(VK_EXT_COLOR_WRITE_ENABLE_EXTENSION_NAME, false, &colorFeatures);

    config.instanceExtensions.emplace_back("VK_KHR_surface");
    config.instanceExtensions.emplace_back("VK_KHR_get_surface_capabilities2");

    m_VKContext.Init(config);
    SDL_Vulkan_CreateSurface(dynamic_cast<SDLWindow*>(m_Window.get())->GetWindowImpl(), m_VKContext.GetInstance(), nullptr, &m_Surface);

    m_Executor.Init(m_VKContext.GetDevice(), m_VKContext.GetGraphicsQueue().familyIndex, m_VKContext.GetGraphicsQueue().queue);
    m_Swapchain.Init(m_VKContext.GetPhysicalDevice(), m_VKContext.GetDevice(), m_VKContext.GetGraphicsQueue(), m_Surface);
    m_Allocator.Init(m_VKContext, &m_Executor);

    _CreateFrameSubmission(m_Swapchain.GetFramesInFlight());
    _InitImGui();

    _CreateDefaultShaders();
    _CreateFallbackTextures();

    CreateMainCamera();
}

VulkanRenderer::~VulkanRenderer() {
    AddVulkanLog("Waiting on GPU to shutdown");
    vkDeviceWaitIdle(m_VKContext.GetDevice());

    AddVulkanLog("Shutting down the renderer");
    
    m_Uninitializers.Flush();

    m_Allocator.Destroy(m_VKContext);
    m_Swapchain.Uninit(m_VKContext.GetDevice());
    m_Executor.Uninit();
    vkDestroySurfaceKHR(m_VKContext.GetInstance(), m_Surface, nullptr);
    m_Window->Uninit();
    m_VKContext.Uninit();
}

void VulkanRenderer::Render() {
    if (m_TargetFps > 0 && !m_vSync)
        m_Pacer.PaceFrame(m_TargetFps);

    Profiler::StartProfile({ typeid(VulkanRenderer), "Render" }, {1.f, 1.f, 1.f, 1.f});
    ImGui_ImplVulkan_NewFrame();
    m_Window->ImGuiNewFrame();
    ImGui::NewFrame();
    if (m_Rendering = _PrepareForRendering(); m_Rendering) {
        _BeginCmdRecording();
    }

    Profiler::EndProfile({ typeid(VulkanRenderer), "Render" });
}

void VulkanRenderer::_RenderImGui() {
    const std::array<VkRenderingAttachmentInfo, 1> colorAttachment{{{
        .sType          = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext          = nullptr,
        .imageView      = m_Swapchain.GetImageView(),
        .imageLayout    = VK_IMAGE_LAYOUT_GENERAL,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_LOAD,   // Load the image from previous rendering 
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,  // Store the image (keep the image)
        .clearValue     = {{{0.0f, 0.0f, 0.0f, 1.0f}}},
    }}};

    // Details of the dynamic rendering
    const VkRenderingInfo renderingInfo{
        .sType                  = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .pNext                  = nullptr,
        .flags{},
        .renderArea             = { { 0, 0 }, { m_Window->GetSize().X, m_Window->GetSize().Y }},
        .layerCount             = 1,
        .colorAttachmentCount   = uint32_t(colorAttachment.size()),
        .pColorAttachments      = colorAttachment.data(),
    };

    vkCmdBeginRendering(m_ActiveCmd, &renderingInfo);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ActiveCmd);
    vkCmdEndRendering(m_ActiveCmd);
}

void VulkanRenderer::Display() {
    Profiler::StartProfile({ typeid(VulkanRenderer), "Display" }, {1.f, 1.f, 1.f, 1.f});

    if (m_Rendering)
        _RenderFrameData();

    ImGui::Render();
    ImGui::EndFrame();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui::UpdatePlatformWindows();

    if (m_Rendering) {
        _RenderImGui();
        _PreparePipelineBarrierForPresenting();
        _EndCmdRecording();
        _EndFrame();
    }

    Profiler::EndProfile({ typeid(VulkanRenderer), "Display" });
}

void VulkanRenderer::Clear() {
}

void VulkanRenderer::Update() {
}

void VulkanRenderer::SetGlobalShader(std::shared_ptr<Shader> shader) {
}

unsigned int VulkanRenderer::GetMaxTextureSize() {

};

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(const std::filesystem::path& path) {
    auto data = ImageData::LoadImage(path);
    if (!data.PixelData && !data.HDRPixelData) {
        Logger::AddError(typeid(VulkanRenderer), "Failed to create texture from {}", path.string());
        return nullptr;
    }
    auto extent = VkExtent3D{
        .width = static_cast<uint32_t>(data.Size.X),
        .height = static_cast<uint32_t>(data.Size.Y),
        .depth = 1
    };

    auto info = vki::GetImageCreateInfo(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, extent);
    VulkanTexture texture{};
    if (data.HDRPixelData) 
        texture = m_Allocator.CreateImage(info, data.HDRPixelData);
    else
        texture = m_Allocator.CreateImage(info, data.PixelData);

    texture.m_Size = { static_cast<unsigned int>(data.Size.X), static_cast<unsigned int>(data.Size.Y) };

    return std::make_shared<VulkanTexture>(texture);
}

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(const uint8_t* pixelData, Vector2u size) {
    auto extent = VkExtent3D{
        .width = static_cast<uint32_t>(size.X),
        .height = static_cast<uint32_t>(size.Y),
        .depth = 1
    };

    VulkanTexture texture{};

    auto info = vki::GetImageCreateInfo(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, extent);
    texture = m_Allocator.CreateImage(info, static_cast<const void*>(pixelData));
    texture.m_Size = size;

    return std::make_shared<VulkanTexture>(texture);
} 

std::shared_ptr<Texture> VulkanRenderer::CreateEmptyTexture(Vector2u size) {
    auto info = vki::GetImageCreateInfo(
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        { size.X, size.Y, 1 }
    );

    VulkanTexture texture{};
    texture = m_Allocator.CreateImage(info, nullptr);
    texture.m_Size = size;

    return std::make_shared<VulkanTexture>(texture);
}

void VulkanRenderer::UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) {
    auto vulkanTexture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    m_Allocator.AttachImageData(vulkanTexture->m_CacheID, pixelData, dest, size);
}

void VulkanRenderer::UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) {
    auto dstVulkan = std::dynamic_pointer_cast<VulkanTexture>(dstTexture);
    auto srcVulkan = std::dynamic_pointer_cast<VulkanTexture>(srcTexture);
    m_Allocator.CopyImageToImage(srcVulkan->m_CacheID, dstVulkan->m_CacheID);
}

void VulkanRenderer::DestroyTexture(std::shared_ptr<Texture> texture) {
    auto vulkanTexture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    m_Allocator.DestroyImage(vulkanTexture->m_CacheID);   
}


std::shared_ptr<Shader> VulkanRenderer::CreateShader(const std::vector<std::filesystem::path>& paths) {
    return nullptr;
}

void VulkanRenderer::BindShaderToObject(Shader* shader) {
    auto vkShader = dynamic_cast<VulkanShader*>(shader);
    m_ItemsToDraw.at(m_ActiveItem).shader = vkShader;
}

void VulkanRenderer::BindTextureToObject(Texture* texture) {
    m_ItemsToDraw.at(m_ActiveItem).texture = dynamic_cast<VulkanTexture*>(texture);
}

void VulkanRenderer::ShowImGuiRenderTabContent() {
    ImGui::Text("Vulkan Renderer");
    if (ImGui::BeginTabBar("vulkanTabBar")) {
        if (ImGui::BeginTabItem("Device")) {
            auto apiVer = m_VKContext.GetDeviceProperties().apiVersion;
            auto driverVer = m_VKContext.GetDeviceProperties().driverVersion;
            auto apiStr = std::format("{}.{}.{}", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_PATCH(apiVer));
            auto driverStr = std::format("{}.{}.{}", VK_VERSION_MAJOR(driverVer), VK_VERSION_MINOR(driverVer), VK_VERSION_PATCH(driverVer));
            ImGui::Text("Active Device: %s ", m_VKContext.GetDeviceProperties().deviceName);
            ImGui::Text("API Version: %s (%u)", apiStr.c_str(), apiVer);
            ImGui::Text("Driver Version: %s (%u)", driverStr.c_str(), driverVer);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Renderer")) {
            ImGui::Text("Frame count: %lu", m_FrameCount);
            ImGui::SeparatorText("Base");
            if (m_vSync)
                ImGui::TextColored({ 0.f, 1.f, 0.f, 1.f }, "VSync");
            else
                ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, "VSync");
            ImGui::Text("Target FPS: %u", m_TargetFps);
            ImGui::SeparatorText("Alloc");
            ImGui::Text("Image: %zu", m_Allocator.GetImageAllocations());
            ImGui::Text("Buffer: %zu", m_Allocator.GetBufferAllocations());
            ImGui::Text("Total: %zu", m_Allocator.GetAllAllocations());
            ImGui::SeparatorText("Swapchain");
            ImGui::Text("Format: %s", string_VkFormat(m_Swapchain.GetFormat()));
            ImGui::Text("Images: %u", m_Swapchain.GetImageCount());
            ImGui::Text("Frames in flight: %u", m_Swapchain.GetFramesInFlight());
            ImGui::Text("Image index: %u", m_Swapchain.GetImageIndex());
            ImGui::Text("Resource index: %u", m_Swapchain.GetResourceIndex());
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

}

Optional<Base::Event> VulkanRenderer::PollEvent() const {
    return m_Window->PollEvent();
}

void VulkanRenderer::ProcessEvent(Optional<Base::Event> event) {
    if (!event.HasValue()) return;
    if (auto resizedEvent = event->GetIf<AppResizedEvent>(event); resizedEvent) {
        Logger::AddDebug("", "Resized");
        m_Swapchain.RequestRebuild();
    }
}

void VulkanRenderer::DrawVertices(VertexArray& vertices, RenderState state) {
    if (!IsVertexArrayInitialized(vertices)) {
        SetVertexArrayRendererData(vertices, std::make_shared<VulkanRendererData>());
        SetVertexArrayDestroyFunc(vertices, [&](RendererData* data) {
            auto vkData = dynamic_cast<VulkanRendererData*>(data);
            if (vkData->m_VertexBuffer.Buffer != VK_NULL_HANDLE) {
                m_Allocator.DestroyBuffer(vkData->m_VertexBuffer);
                vkData->m_VertexBuffer.Buffer = VK_NULL_HANDLE;
            }

            if (vkData->m_DrawBuffer.Buffer != VK_NULL_HANDLE) {
                m_Allocator.DestroyBuffer(vkData->m_DrawBuffer);
                vkData->m_DrawBuffer.Buffer = VK_NULL_HANDLE;
            }
        });
        SetVertexArrayInitializedStatus(vertices, true);
    }

    auto* vkData = static_cast<VulkanRendererData*>(GetVertexArrayRendererData(vertices));
    m_ItemsToDraw.emplace_back(&vertices, vkData, state);
    m_ActiveItem = m_ItemsToDraw.size() - 1;
}

void VulkanRenderer::DrawElement(GraphicalElement& element, RenderState state) {
    element.Draw(this, state);
}

std::shared_ptr<BaseWindow> VulkanRenderer::GetWindow() const {
    return m_Window;
}

void VulkanRenderer::_CreateFrameSubmission(uint32_t frameCount) {
    auto dev = m_VKContext.GetDevice(); 

    m_FrameData.resize(frameCount);

    const uint64_t initialValue = 0;

    VkSemaphoreTypeCreateInfo timelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .pNext = nullptr,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = initialValue 
    };

    const VkSemaphoreCreateInfo semaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &timelineCreateInfo,
        .flags{}
    };

    VK_CHECK(vkCreateSemaphore(dev, &semaphoreCreateInfo, nullptr, &m_FrameTimelineSemaphore));
    AddNameToVKObject(dev, VK_OBJECT_TYPE_SEMAPHORE, (uint64_t)m_FrameTimelineSemaphore, "m_FrameTimelineSemaphore");


    const uint32_t queueFamily = m_VKContext.GetGraphicsQueue().familyIndex;

    for (uint32_t i = 0; i < frameCount; i++) {
        m_FrameData[i].lastSignalValue = initialValue;
        m_FrameData[i].cmdPool = vkb::CreateCommandPool(dev, queueFamily);

        const VkCommandBufferAllocateInfo cmdAllocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_FrameData[i].cmdPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VK_CHECK(vkAllocateCommandBuffers(dev, &cmdAllocInfo, &m_FrameData[i].cmdBuffer));
    }

    m_Uninitializers.AddCallback([&]() {
        AddVulkanLog("Destroying frame submission objects");

        vkDestroySemaphore(m_VKContext.GetDevice(), m_FrameTimelineSemaphore, nullptr);
        for (auto& frameData : m_FrameData)
            vkDestroyCommandPool(m_VKContext.GetDevice(), frameData.cmdPool, nullptr);
    });
}

void VulkanRenderer::_CreateImGuiDescriptorPool() {
    VkPhysicalDeviceProperties2 deviceProperties2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = nullptr,
        .properties{}
    };
    vkGetPhysicalDeviceProperties2(m_VKContext.GetPhysicalDevice(), &deviceProperties2);
    const auto& deviceProperties = deviceProperties2.properties;

    constexpr uint32_t kMaxTextures = 128;
    const uint32_t samplerSize = std::min<uint32_t>(
        IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
        deviceProperties.limits.maxDescriptorSetSamplers
    );
    const uint32_t sampledImageSize = std::min<uint32_t>(
        std::max<uint32_t>(
            IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE, 
            kMaxTextures
        ),
        deviceProperties.limits.maxDescriptorSetSampledImages
    );
    const uint32_t maxDescriptorSets = samplerSize + sampledImageSize;

    const std::array<VkDescriptorPoolSize, 2> poolSizes = {{
        {VK_DESCRIPTOR_TYPE_SAMPLER, samplerSize},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampledImageSize},
    }};
    const VkDescriptorPoolCreateInfo          poolInfo  = {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext          = nullptr,
        .flags          = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets        = maxDescriptorSets,
        .poolSizeCount  = uint32_t(poolSizes.size()),
        .pPoolSizes     = poolSizes.data(),
    };

    VK_CHECK(vkCreateDescriptorPool(m_VKContext.GetDevice(), &poolInfo, nullptr, &m_UIDescriptorPool));

    m_Uninitializers.AddCallback([&]() {
        AddVulkanLog("Destroying ImGui descriptor pool");
        vkDestroyDescriptorPool(m_VKContext.GetDevice(), m_UIDescriptorPool, nullptr);
    });
}

void VulkanRenderer::_InitImGui() {
    AddVulkanLog("Initializing ImGui backend");
    _CreateImGuiDescriptorPool();

    ImGui::CreateContext();
    m_Window->InitImGui();

    auto format = m_Swapchain.GetFormat();

    ImGui_ImplVulkan_InitInfo info{
        .Instance = m_VKContext.GetInstance(),
        .PhysicalDevice = m_VKContext.GetPhysicalDevice(),
        .Device = m_VKContext.GetDevice(),
        .QueueFamily = m_VKContext.GetGraphicsQueue().familyIndex,
        .Queue = m_VKContext.GetGraphicsQueue().queue,
        .DescriptorPool = m_UIDescriptorPool,
        .MinImageCount = 2,
        .ImageCount = m_Swapchain.GetImageCount(),
        .PipelineInfoMain = {
            .PipelineRenderingCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
                .pNext = nullptr,
                .colorAttachmentCount = 1,
                .pColorAttachmentFormats = &format,
            },
        },
        .PipelineInfoForViewports = {},
        .UseDynamicRendering = true
    };
    ImGui_ImplVulkan_Init(&info);
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;

    m_Uninitializers.AddCallback([&]() {
        AddVulkanLog("Destroying ImGui Hooks");

        ImGui_ImplVulkan_Shutdown();
        m_Window->UninitImGui();
    });
}

bool VulkanRenderer::_PrepareForRendering() {
    if (m_Swapchain.IsOutdated()) {
        m_Swapchain.Reinit(m_VKContext.GetPhysicalDevice(), m_VKContext.GetDevice(), m_Surface);
    }
    auto& frame = m_FrameData[m_Swapchain.GetResourceIndex()];
    auto lastSignal = frame.lastSignalValue;

    const VkSemaphoreWaitInfo waitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .pNext = nullptr,
        .flags{},
        .semaphoreCount = 1,
        .pSemaphores = &m_FrameTimelineSemaphore,
        .pValues = &lastSignal
    };
    VK_CHECK(vkWaitSemaphores(m_VKContext.GetDevice(), &waitInfo, NO_TIMEOUT));

    auto result = m_Swapchain.AcquireNextImage(m_VKContext.GetDevice());
    return !VK_CHECK_SWAPCHAIN(result);
}

void VulkanRenderer::_CreateDefaultShaders() {
    AddVulkanLog("Creating default shaders");

    const auto pushConstantRange = VkPushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(DefaultPushConstants)
    };

    m_DefaultShader.m_CreateBufFunc = [this](size_t size, VkBufferUsageFlags flags) {
        return _ShaderCreateBuffer(size, flags);
    };
    m_DefaultShader.m_UpdateDescFunc = [this](size_t count, const VkWriteDescriptorSet& write) {
        _ShaderUpdateDescriptorSet(count, write);
    };
    m_DefaultShader.m_CreateDescFunc = [this](const VkDescriptorSetLayoutCreateInfo& info, VkDescriptorSetLayout* layout) {
        _ShaderCreateDescriptorLayout(info, layout);
    };
    m_DefaultShader.m_CreatePoolFunc = [this](const VkDescriptorPoolCreateInfo& info, VkDescriptorPool* pool) {
        _ShaderCreateDescriptorPool(info, pool);
    };
    m_DefaultShader.m_AllocSetFunc = [this](const VkDescriptorSetAllocateInfo& info, VkDescriptorSet* set) {
        _ShaderAllocDescriptorSet(info, set);
    };

    m_DefaultShader.AddPushConstantRange(pushConstantRange);
    m_DefaultShader.AddDescLayout(m_Allocator.GetBindlessDescSetLayout());
    if (!m_DefaultShader.LoadFromCompiledFile(m_VKContext.GetDevice(), { "shaders/base.vert.spv", "shaders/base.frag.spv" }))
        THROW_RUNTIME_ERROR("Failed to create shader!");

    m_Uninitializers.AddCallback([&]() {
        AddVulkanLog("Destroying default shaders");

        m_DefaultShader.Destroy(m_VKContext.GetDevice(), m_Allocator);
    });
}

void VulkanRenderer::_CreateFallbackTextures() {
    AddVulkanLog("Creating fallback textures");

    const auto black = 0xFF000000;
    const auto magenta = 0xFFFF00FF;

    std::array<uint32_t, 4> pixels{ black, magenta, magenta, black };
    m_ErrorTexture = std::make_unique<VulkanTexture>(
        m_Allocator.CreateImage({
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags{},
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = {
                .width = 2,
                .height = 2,
                .depth = 1
            },
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
            &pixels
        )
    );
    m_Allocator.SetMissingImageID(m_ErrorTexture->m_CacheID);

    uint32_t pixel = 0xFFFFFFFF;
    m_WhiteTexture = std::make_unique<VulkanTexture>(
        m_Allocator.CreateImage({
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags{},
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .extent = {
                .width = 1,
                .height = 1,
                .depth = 1
            },
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
        )
    );

    m_Uninitializers.AddCallback([&]() {
        AddVulkanLog("Destroying fallback textures");

        m_Allocator.DestroyImage(m_ErrorTexture->GetID());
        m_Allocator.DestroyImage(m_WhiteTexture->GetID());
    });
}

void VulkanRenderer::_RenderFrameData() {
    // TODO: Use Batch Rendering strategy

    VulkanShader* prevShader = nullptr;
    RenderState prevState = RenderState::Default;
    PrimitiveType prevType = PrimitiveType::None;

    VkRenderingAttachmentInfo colorInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = m_Swapchain.GetImageView(),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode{},
        .resolveImageView{},
        .resolveImageLayout{},
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{
            .color = {
                .float32 = { 0.f, 0.f, 0.f, 1.f }
            }
        }
    };

    const auto renderInfo = vki::GetRenderingInfo(
        { m_Window->GetSize().X, m_Window->GetSize().Y },
        &colorInfo,
        nullptr
    );

    _PreparePipelineBarrierForRendering();

    vkCmdBeginRendering(m_ActiveCmd, &renderInfo);
    {
        const auto viewport = VkViewport{
            .x = 0,
            .y = 0,
            .width = static_cast<float>(m_Window->GetSize().X),
            .height = static_cast<float>(m_Window->GetSize().Y),
            .minDepth = .0f,
            .maxDepth = 1.f
        };
        vkCmdSetViewportWithCount(m_ActiveCmd, 1, &viewport);

        const auto scissor = VkRect2D{
            .offset{},
            .extent = { m_Window->GetSize().X, m_Window->GetSize().Y }
        };
        vkCmdSetScissorWithCount(m_ActiveCmd, 1, &scissor);
    }

    bool first = true;
    for (auto& item : m_ItemsToDraw) {
        // m_DefaultShader.m_PushConstants = nullptr;

        auto* curShader = item.shader;

        if (!curShader)
            curShader = &m_DefaultShader;

        if (item.state != prevState || first) {
            prevState = item.state;
            _SetRenderState(item.state);
        }

        if (curShader != prevShader)
            prevShader = curShader;

        if (curShader == &m_DefaultShader) {
            if (item.data->m_DrawBuffer.Buffer == VK_NULL_HANDLE) {
                item.data->m_DrawBuffer = m_Allocator.CreateBuffer(
                    sizeof(DrawCommand),
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                );
            }

            DrawCommand drawCmd{
                .projection = m_MainCamera.GetProjection(),
                .transform = item.state.transform.GetMatrix(),
                .padding{},
                .textureID = item.texture ? item.texture->GetID() : NULL_IMAGE_ID,
                .shaderID{}
            };
            memcpy(
               item.data->m_DrawBuffer.AllocationInfo.pMappedData,
               &drawCmd,
               sizeof(drawCmd)
            );
        }

        if (curShader == &m_DefaultShader) {
            curShader->PushConstant<DefaultPushConstants>(item.data->m_DrawBuffer.Address);
            m_Allocator.BindBindlessDescSet(m_ActiveCmd, curShader->GetLayout());
        }

        curShader->Bind(m_ActiveCmd);

        if (item.vertices->GetType() != prevType || first) {
            first = false;
            prevType = item.vertices->GetType();
            _SetPrimitiveType(item.vertices->GetType());
        }

        auto bufSize = item.vertices->GetSize() * sizeof(Vertex);

        if (bufSize != item.data->m_VertexBuffer.BufferSize && !item.vertices->IsEmpty()) {
            if (item.data->m_VertexBuffer.Buffer != VK_NULL_HANDLE)
                m_Allocator.DestroyBuffer(item.data->m_VertexBuffer);
            item.data->m_VertexBuffer = m_Allocator.CreateBuffer(
                bufSize,
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT 
            );
        }
        if (item.data->m_VertexBuffer.Buffer == VK_NULL_HANDLE) continue;
        memcpy(
            item.data->m_VertexBuffer.AllocationInfo.pMappedData,
            item.vertices->GetData(),
            bufSize 
        );

        _SetVertexBindings(item);

        vkCmdDraw(m_ActiveCmd, item.vertices->GetSize(), item.state.drawCount, 0, 0);
    }
    vkCmdEndRendering(m_ActiveCmd);

    _ImGuiCreateItemDrawWindow();

    _PreparePipelineBarrierForImGui();

    m_ItemsToDraw.clear();
    m_ActiveItem = 0;
}

void VulkanRenderer::_SetVertexBindings(const DrawData& item) {
    const auto vertexBinding = VkVertexInputBindingDescription2EXT{
        .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
        .pNext = nullptr,
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        .divisor = 1
    };

    const auto vertexAttributes = std::array{
        VkVertexInputAttributeDescription2EXT{
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .pNext = nullptr,
            .location = 0,
            .binding = vertexBinding.binding,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = 16 * 0
        },
        VkVertexInputAttributeDescription2EXT{
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .pNext = nullptr,
            .location = 5,
            .binding = vertexBinding.binding,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = 16 * 1
        },
        VkVertexInputAttributeDescription2EXT{
            .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
            .pNext = nullptr,
            .location = 6,
            .binding = vertexBinding.binding,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = 16 * 2
        }
    };

    VkDeviceSize vertexOffset = 0;
    VkDeviceSize vertexStride = sizeof(Vertex);

    vkCmdBindVertexBuffers2(
        m_ActiveCmd,
        0,
        1,
        &item.data->m_VertexBuffer.Buffer,
        &vertexOffset,
        &item.data->m_VertexBuffer.BufferSize,
        &vertexStride
    );

    vkCmdSetVertexInputEXT(
        m_ActiveCmd,
        1,
        &vertexBinding,
        vertexAttributes.size(),
        vertexAttributes.data()
    );
}

void VulkanRenderer::_PreparePipelineBarrierForRendering() {
    auto imageBarrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask{},
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_Swapchain.GetImage(),
        .subresourceRange = {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    VkDependencyInfo dependencyInfo{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags{},
        .memoryBarrierCount{},
        .pMemoryBarriers{},
        .bufferMemoryBarrierCount{},
        .pBufferMemoryBarriers{},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &imageBarrier,
    };

    vkCmdPipelineBarrier2(m_ActiveCmd, &dependencyInfo);
}

void VulkanRenderer::_PreparePipelineBarrierForImGui() {
    auto presentBarrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_2_NONE_KHR, 
        .dstAccessMask{},
        .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout     = VK_IMAGE_LAYOUT_GENERAL,
        .srcQueueFamilyIndex{},
        .dstQueueFamilyIndex{},
        .image         = m_Swapchain.GetImage(),
        .subresourceRange {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        },
    };

    VkDependencyInfo presentDependencyInfo{};
    presentDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    presentDependencyInfo.imageMemoryBarrierCount = 1;
    presentDependencyInfo.pImageMemoryBarriers = &presentBarrier;

    vkCmdPipelineBarrier2(m_ActiveCmd, &presentDependencyInfo);

}

void VulkanRenderer::_PreparePipelineBarrierForPresenting() {
    auto presentBarrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_2_NONE_KHR, 
        .dstAccessMask{},
        .oldLayout     = VK_IMAGE_LAYOUT_GENERAL,
        .newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex{},
        .dstQueueFamilyIndex{},
        .image         = m_Swapchain.GetImage(),
        .subresourceRange {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        },
    };

    VkDependencyInfo presentDependencyInfo{};
    presentDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    presentDependencyInfo.imageMemoryBarrierCount = 1;
    presentDependencyInfo.pImageMemoryBarriers = &presentBarrier;

    vkCmdPipelineBarrier2(m_ActiveCmd, &presentDependencyInfo);
}

void VulkanRenderer::_SetRenderState(const RenderState& state) {
    vkCmdSetPolygonModeEXT(m_ActiveCmd, VK_POLYGON_MODE_FILL);

    vkCmdSetLineWidth(m_ActiveCmd, state.lineWidth);
    vkCmdSetPrimitiveRestartEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetDepthBiasEnable(m_ActiveCmd, state.enableDepthBias ? VK_TRUE : VK_FALSE);
    vkCmdSetDepthBias(m_ActiveCmd, state.depthBias[0], state.depthBias[1], state.depthBias[2]);
    vkCmdSetDepthBoundsTestEnable(m_ActiveCmd, state.enableDepthBounds ? VK_TRUE : VK_FALSE);
    vkCmdSetDepthBounds(m_ActiveCmd, state.depthBounds.X, state.depthBounds.Y);
    vkCmdSetRasterizerDiscardEnable(m_ActiveCmd, state.discardRasterization ? VK_TRUE : VK_FALSE);
    vkCmdSetDepthTestEnable(m_ActiveCmd, state.enableDepthTest ? VK_TRUE : VK_FALSE);
    vkCmdSetDepthClampEnableEXT(m_ActiveCmd, state.enableDepthClamp ? VK_TRUE : VK_FALSE);
    vkCmdSetDepthWriteEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetStencilTestEnable(m_ActiveCmd, state.enableStencilTest ? VK_TRUE : VK_FALSE);
    vkCmdSetCullMode(m_ActiveCmd, VK_CULL_MODE_NONE);
    vkCmdSetFrontFace(m_ActiveCmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    vkCmdSetAlphaToOneEnableEXT(m_ActiveCmd, VK_FALSE);
    vkCmdSetLogicOpEnableEXT(m_ActiveCmd, VK_FALSE);

    vkCmdSetRasterizationSamplesEXT(m_ActiveCmd, VK_SAMPLE_COUNT_1_BIT);
    vkCmdSetAlphaToCoverageEnableEXT(m_ActiveCmd, state.enableAlphaToCoverage ? VK_TRUE : VK_FALSE);

    VkColorBlendEquationEXT blendEquation{};
    blendEquation.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendEquation.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendEquation.colorBlendOp        = VK_BLEND_OP_ADD;
    blendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendEquation.alphaBlendOp       = VK_BLEND_OP_ADD;

    vkCmdSetColorBlendEquationEXT(m_ActiveCmd, 0, 1, &blendEquation);

    VkSampleMask sampleMask = 0xFFFFFFFF;
    vkCmdSetSampleMaskEXT(m_ActiveCmd, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

    VkBool32 blendEnable = state.enableBlend ? VK_TRUE : VK_FALSE;
    vkCmdSetColorBlendEnableEXT(m_ActiveCmd, 0, 1, &blendEnable);

    VkBool32 writeEnable = VK_TRUE;
    vkCmdSetColorWriteEnableEXT(m_ActiveCmd, 1, &writeEnable);

    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                       VK_COLOR_COMPONENT_G_BIT | 
                                       VK_COLOR_COMPONENT_B_BIT | 
                                       VK_COLOR_COMPONENT_A_BIT;
    vkCmdSetColorWriteMaskEXT(m_ActiveCmd, 0, 1, &colorWriteMask);
}

void VulkanRenderer::_SetPrimitiveType(PrimitiveType type) {
    switch (type) {
    default:
    case PrimitiveType::None:
    case PrimitiveType::PointList:
        vkCmdSetPrimitiveTopology(m_ActiveCmd, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
        break;
    case PrimitiveType::LineList:
        vkCmdSetPrimitiveTopology(m_ActiveCmd, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        break;
    case PrimitiveType::LineStrip:
        vkCmdSetPrimitiveTopology(m_ActiveCmd, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
        break;
    case PrimitiveType::TriangleList:
        vkCmdSetPrimitiveTopology(m_ActiveCmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        break;
    case PrimitiveType::TriangleStrip:
        vkCmdSetPrimitiveTopology(m_ActiveCmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        break;
    case PrimitiveType::TriangleFan:
        vkCmdSetPrimitiveTopology(m_ActiveCmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
        break;
    }
}

void VulkanRenderer::_BeginCmdRecording() {
    auto device = m_VKContext.GetDevice();

    auto& frame = m_FrameData[m_Swapchain.GetResourceIndex()];

    VK_CHECK(vkResetCommandPool(device, frame.cmdPool, 0));
    m_ActiveCmd = frame.cmdBuffer;

    const VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    VK_CHECK(vkBeginCommandBuffer(m_ActiveCmd, &beginInfo));
}

void VulkanRenderer::_EndCmdRecording() {
    VK_CHECK(vkEndCommandBuffer(m_ActiveCmd));
}

GPUBuffer VulkanRenderer::_ShaderCreateBuffer(size_t size, VkBufferUsageFlags flags) {
    return m_Allocator.CreateBuffer(size, flags);
}

void VulkanRenderer::_ShaderCreateDescriptorLayout(const VkDescriptorSetLayoutCreateInfo& info, VkDescriptorSetLayout* layout) {
    vkCreateDescriptorSetLayout(m_VKContext.GetDevice(), &info, nullptr, layout);
}

void VulkanRenderer::_ShaderCreateDescriptorPool(const VkDescriptorPoolCreateInfo& info, VkDescriptorPool* pool) {
    vkCreateDescriptorPool(m_VKContext.GetDevice(), &info, nullptr, pool);
}

void VulkanRenderer::_ShaderAllocDescriptorSet(const VkDescriptorSetAllocateInfo& info, VkDescriptorSet* set) {
    vkAllocateDescriptorSets(m_VKContext.GetDevice(), &info, set);
}

void VulkanRenderer::_ShaderUpdateDescriptorSet(size_t count, const VkWriteDescriptorSet& write) {
    vkUpdateDescriptorSets(m_VKContext.GetDevice(), count, &write, 0, nullptr);
}

void VulkanRenderer::_ImGuiCreateItemDrawWindow() {
    ImGui::Begin("Rendered items");
    auto i = 0;
    auto j = m_ItemsToDraw.size();
    for (auto& item : m_ItemsToDraw) {
        ImGui::PushID(i);
        if (ImGui::TreeNode("Item")) {
            if (ImGui::TreeNode("VertexArray")) {
                auto& vertices = *item.vertices;
                ImGui::Text("Type: %s", PrimitiveTypeUtils::ToString(vertices.GetType()).c_str());
                ImGui::Text("Count: %zu", vertices.GetSize());
                if (ImGui::TreeNode("Vertices")) {
                    for (const auto& vertex : vertices) {
                        ImGui::PushID(j);
                        if (ImGui::TreeNode("Vertex")) {
                            ImGui::Text("Position: %fx%f", vertex.Position.X, vertex.Position.Y);
                            ImGui::Text("UV: %fx%f", vertex.UV.X, vertex.UV.Y);
                            ImGui::ColorButton("Color", { vertex.Color.R, vertex.Color.G, vertex.Color.B, vertex.Color.A });
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                        j++;
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            } 
            if (item.data && ImGui::TreeNode("Data")) {
                ImGui::Text("Vertex buffer size: %zu", item.data->m_VertexBuffer.BufferSize);
                ImGui::TreePop();
            }

            if (item.shader) {
                ImGui::TextColored({ 0.f, 1.f, 0.f, 1.f }, "Binded shader");
            }
            else
                ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, "No binded shader");

            if (item.texture) {
                if (ImGui::TreeNode("Texture")) {
                    ImGui::Text("TextureID: %u", item.texture->GetID());
                    ImGui::Text("Size: %ux%u", item.texture->GetSize().X, item.texture->GetSize().Y);
                    ImGui::TreePop();
                }
            }
            else
                ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, "No binded texture");
            ImGui::TreePop();
        }
        ImGui::PopID();
        i++;
    }
    ImGui::End();
}

void VulkanRenderer::_PrepareForImGui() {
    
}

void VulkanRenderer::_EndFrame() {
    std::vector<VkSemaphoreSubmitInfo> waitSemaphores;
    std::vector<VkSemaphoreSubmitInfo> signalSemaphores;

    waitSemaphores.emplace_back(
        vki::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            m_Swapchain.GetAcquireSemaphore()
        )
    );

    signalSemaphores.emplace_back(
        vki::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            m_Swapchain.GetPresentSemaphore()
        )
    );

    const auto frameSlot = m_Swapchain.GetResourceIndex();
    auto& frame = m_FrameData[frameSlot];

    const auto signalFrameValue = m_FrameCount++;
    frame.lastSignalValue = signalFrameValue;

    signalSemaphores.emplace_back(
        vki::GetSemaphoreSubmitInfo(
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            m_FrameTimelineSemaphore,
            signalFrameValue
        )
    );

    const std::array<VkCommandBufferSubmitInfo, 1> cmdBufferInfo{
        vki::GetCommandBufferSubmitInfo(m_ActiveCmd)
    };

    const std::array<VkSubmitInfo2, 1> submitInfo{
        vki::GetSubmitInfo(
            cmdBufferInfo.data(),
            signalSemaphores.data(),
            signalSemaphores.size(),
            waitSemaphores.data(),
            waitSemaphores.size()
        )
    };

    VK_CHECK(
        vkQueueSubmit2(
            m_VKContext.GetGraphicsQueue().queue,
            submitInfo.size(),
            submitInfo.data(),
            nullptr
        )
    );

    m_Swapchain.PresentFrame(m_VKContext.GetGraphicsQueue().queue);
}
