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
        .apiVersion = VK_API_VERSION_1_4
    };

    VkPhysicalDeviceUnifiedImageLayoutsFeaturesKHR unifiedImageLayoutsFeature{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFIED_IMAGE_LAYOUTS_FEATURES_KHR};
    // Descriptor heap replaces traditional descriptor sets/pools with GPU buffer-based bindless descriptors.
    // Samplers and images are written into heap buffers and accessed by index in the shaders.
    VkPhysicalDeviceDescriptorHeapFeaturesEXT descriptorHeapFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_FEATURES_EXT};
    // Untyped pointers: required by descriptor heap
    VkPhysicalDeviceShaderUntypedPointersFeaturesKHR untypedPtrFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_UNTYPED_POINTERS_FEATURES_KHR};
    // Shader objects: replace VkPipeline for graphics with linkable, reusable VkShaderEXT
    // objects bound via vkCmdBindShadersEXT. Pairs naturally with the layout=NULL design:
    // there is no graphics pipeline object at all, only shader objects + dynamic state.
    VkPhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT};
    // Extended dynamic state 3: required by shader objects for blend/rasterization
    // state that no longer lives in a pipeline object.
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT dynamicState3Features{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_FEATURES_EXT};
    // Vertex input dynamic state: required by shader objects (vertex bindings/attributes
    // become a vkCmdSetVertexInputEXT call instead of pipeline state).
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT vertexInputDynamicStateFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT};


    config.deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true, nullptr);
    config.deviceExtensions.emplace_back(VK_KHR_UNIFIED_IMAGE_LAYOUTS_EXTENSION_NAME, true, &unifiedImageLayoutsFeature);
    config.deviceExtensions.emplace_back(VK_EXT_DESCRIPTOR_HEAP_EXTENSION_NAME, true, &descriptorHeapFeatures);  // Bindless descriptor heap for textures and samplers
    config.deviceExtensions.emplace_back(VK_KHR_SHADER_UNTYPED_POINTERS_EXTENSION_NAME, true, &untypedPtrFeatures);  // Required by bindless
    config.deviceExtensions.emplace_back(VK_EXT_SHADER_OBJECT_EXTENSION_NAME, true, &shaderObjectFeatures);  // Graphics: shader objects instead of pipelines
    config.deviceExtensions.emplace_back(VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME, true, &dynamicState3Features);  // Required for shader-object blend/rasterization state
    config.deviceExtensions.emplace_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME, true, &vertexInputDynamicStateFeatures);

    config.instanceExtensions.emplace_back("VK_KHR_surface");
    config.instanceExtensions.emplace_back("VK_KHR_get_surface_capabilities2");

    m_VKContext.Init(config);
    SDL_Vulkan_CreateSurface(dynamic_cast<SDLWindow*>(m_Window.get())->GetWindowImpl(), m_VKContext.GetInstance(), nullptr, &m_Surface);

    m_Executor.Init(m_VKContext.GetDevice(), m_VKContext.GetGraphicsQueue().familyIndex, m_VKContext.GetGraphicsQueue().queue);
    m_Swapchain.Init(m_VKContext.GetPhysicalDevice(), m_VKContext.GetDevice(), m_VKContext.GetGraphicsQueue(), m_Surface);
    _CreateFrameSubmission(m_Swapchain.GetFramesInFlight());
    _CreateImGuiDescriptorPool();
    _InitImGui();
}

VulkanRenderer::~VulkanRenderer() {
    AddVulkanLog("Waiting on GPU to shutdown");

    AddVulkanLog("Shutting down the renderer");
}

void VulkanRenderer::Render() {
    ImGui::NewFrame();
    // if (_PrepareForRendering()) {
    //     _BeginCmdRecording();
    // }
}

void VulkanRenderer::RenderImGui() {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ActiveCmd);
    ImGui::EndFrame();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui::UpdatePlatformWindows();
}

void VulkanRenderer::Display() {
    if (!m_Rendering) return;

    _RenderFrameData();
    _EndCmdRecording();
    _EndFrame();
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
    return nullptr;
}

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(const uint8_t* pixelData, Vector2u size) {
    return nullptr;
} 

std::shared_ptr<Texture> VulkanRenderer::CreateEmptyTexture(Vector2u size) {
    return nullptr;
}

void VulkanRenderer::UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) {

}

void VulkanRenderer::UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) {

}

void VulkanRenderer::DestroyTexture(std::shared_ptr<Texture> texture) {

}


std::shared_ptr<Shader> VulkanRenderer::CreateShader(const std::filesystem::path& path, Shader::Type type) {
    return nullptr;
}

void VulkanRenderer::ShowImGuiRenderTabContent() {
    ImGui::Text("Vulkan Renderer");
    // ImGui::Text("Frame N°%u", m_VKContext.GetCurrentFrameNumber());
    if (ImGui::BeginTabBar("vulkanTabBar")) {
        if (ImGui::BeginTabItem("Device")) {
            // auto apiVer = m_VKContext.GetDeviceProperties().apiVersion;
            // auto apiStr = std::format("{}.{}.{}", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_PATCH(apiVer));
            // ImGui::Text("Active Device: %s ", m_VKContext.GetDeviceProperties().deviceName);
            // ImGui::Text("API Version: %s (%u)", apiStr.c_str(), m_VKContext.GetDeviceProperties().apiVersion);
            // ImGui::Text("Driver Version: %u", m_VKContext.GetDeviceProperties().driverVersion);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Renderer")) {
            // m_StopRendering ? ImGui::TextColored({255, 0, 0, 255}, "Not Rendering") : ImGui::TextColored({0, 255, 0, 255}, "Rendering");
            ImGui::SeparatorText("Alloc");
            // ImGui::Text("Image: %u", m_VKContext.GetImageAllocations());
            // ImGui::Text("Buffer: %u", m_VKContext.GetBufferAllocations());
            // ImGui::Text("Total: %u", m_VKContext.GetImageAllocations() + m_VKContext.GetBufferAllocations());
            ImGui::SeparatorText("Swapchain");
            // ImGui::Text("Extent: (X: %u, Y: %u)", m_VKContext.GetSwapchainExtent().X, m_VKContext.GetSwapchainExtent().Y);
            // ImGui::Text("Format: %s", string_VkFormat(m_VKContext.GetSwapchainFormat()));
            // ImGui::Text("Images: %zu", m_VKContext.GetSwapchainImagesSize());
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
        m_Swapchain.RequestRebuild();
    }
}

void VulkanRenderer::DrawVertices(VertexArray& vertices, RenderState state) {
    m_ItemsToDraw.emplace_back(vertices, state);

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
}

void VulkanRenderer::_CreateImGuiDescriptorPool() {

}

void VulkanRenderer::_InitImGui() {
    ImGui::CreateContext();
    m_Window->InitImGui();
}

bool VulkanRenderer::_PrepareForRendering() {
    
}

void VulkanRenderer::_RenderFrameData() {
    
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
