#include "BeatEngine/Renderers/Vulkan/Renderer.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <memory>

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_sdl3.h>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>

#include <optional>
#include <source_location>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#include "BeatEngine/Enum/EnvFlags.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/DescriptorBuilder.h"
#include "BeatEngine/Renderers/Vulkan/ImageData.h"
#include "BeatEngine/Renderers/Vulkan/RendererData.hpp"
#include "BeatEngine/Util/Exception.h"
#include "BeatEngine/Util/Profiler.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/System/Clock.h"
#include "BeatEngine/System/Time.h"
#include "BeatEngine/GameContext.h"



void VulkanRenderer::pInitRenderPass() {
    AddVulkanLog("Initializing Render Pass");

    // VkAttachmentReference colorAttachmentReferences{
    //     .attachment = 0,
    //     .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    // };
    //
    // VkSubpassDescription subpassDescriptions{
    //     .flags{},
    //     .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    //     .inputAttachmentCount = 0,
    //     .pInputAttachments = nullptr,
    //     .colorAttachmentCount = 1,
    //     .pColorAttachments = &colorAttachmentReferences,
    //     .pResolveAttachments = nullptr,
    //     .pDepthStencilAttachment = nullptr,
    //     .preserveAttachmentCount = 0,
    //     .pPreserveAttachments = nullptr,
    // };
    //
    // VkAttachmentDescription attachmentDescriptions{
    //     .flags{},
    //     .format = m_SwapchainFormat,
    //     .samples = VK_SAMPLE_COUNT_1_BIT,
    //     .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    //     .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    //     .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    //     .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    //     .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    // };
    //
    // VkSubpassDependency subpassDependecy{
    //     .srcSubpass = VK_SUBPASS_EXTERNAL,
    //     .dstSubpass = 0,
    //     .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     .srcAccessMask = 0,
    //     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    // };
    //
    // VkRenderPassCreateInfo renderPassInfo{
    //     .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    //     .attachmentCount = 1,
    //     .pAttachments = &attachmentDescriptions,
    //     .subpassCount = 1,
    //     .pSubpasses = &subpassDescriptions,
    //     .dependencyCount = 0
        // .dependencyCount = 1,
        // .pDependencies = &subpassDependecy    
    // };
    //
    // VK_CHECK(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass));
    // AddNameToVKObject(m_Device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_RenderPass, "VkRenderPass");

    // m_Uninitializers.AddCallback([this]() {
    //     AddVulkanLog("Destroying Render Pass");
    //     vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    // });
};

void VulkanRenderer::pInitPipeline() {
    AddVulkanLog("Initializing Pipeline");

    m_PipelineMgr.Init(m_ImageDescriptorLayout, m_ImageDescriptor);

    // m_Uninitializers.AddCallback([this]() {
    //     AddVulkanLog("Destroying pipeline");
    //     m_PipelineMgr.DestroyAll(m_Device);
    // });
}

void VulkanRenderer::pInitDescriptors() {
    AddVulkanLog("Initializing Descriptors");

    // std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
    //     { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
    // };
    //
    // m_GlobalDescriptorAllocator.InitPool(m_Device, 10, sizes);
    //
    // DescriptorLayoutBuilder builder;
    // builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    // m_ImageDescriptorLayout = builder.Build(m_Device, VK_SHADER_STAGE_COMPUTE_BIT);
    //
    // m_ImageDescriptor = m_GlobalDescriptorAllocator.Allocate(m_Device, m_ImageDescriptorLayout);
    //
    // VkDescriptorImageInfo imgInfo{
    //     .imageView = m_DrawImage.ImageView,
    //     .imageLayout = VK_IMAGE_LAYOUT_GENERAL
    // };
    //
    // VkWriteDescriptorSet drawImageWrite = {
    //     .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
    //     .pNext = nullptr,
    //     .dstSet = m_ImageDescriptor,
    //     .dstBinding = 0,
    //     .descriptorCount = 1,
    //     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    //     .pImageInfo = &imgInfo
    // };
    //
    // vkUpdateDescriptorSets(m_Device, 1, &drawImageWrite, 0, nullptr);
    //
    // m_Uninitializers.AddCallback([this]() {
    //     AddVulkanLog("Destroying Descriptors");
    //
    //     m_GlobalDescriptorAllocator.DestroyPool(m_Device);
    //
    //     vkDestroyDescriptorSetLayout(m_Device, m_ImageDescriptorLayout, nullptr);
    // });
}

void VulkanRenderer::pInitFramebuffers() {
    // AddVulkanLog("Initializing framebuffers");
    // VkFramebufferCreateInfo framebufferInfo{
    //     .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    //     .pNext = nullptr,
    //     .renderPass = m_RenderPass,
    //     .attachmentCount = 1,
    //     .pAttachments = &m_DrawImage.ImageView,
    //     .width = m_Window->GetSize().X,
    //     .height = m_Window->GetSize().Y,
    //     .layers = 1
    // };
    // VK_CHECK(vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_DrawImage.Framebuffer));
    //
    // m_Uninitializers.AddCallback([this]() {
    //     AddVulkanLog("Destroying Framebuffers");
    //     vkDestroyFramebuffer(m_Device, m_DrawImage.Framebuffer, nullptr);
    // });
}

void VulkanRenderer::Init(std::string windowTitle, Vector2u windowSize) {
    AddVulkanLog("Initializing VulkanRenderer");

    if (m_Window == nullptr) {
        m_Window = std::make_shared<SDLWindow>();
        std::static_pointer_cast<SDLWindow>(m_Window)->SetWindowFlags(SDL_WINDOW_RESIZABLE);
    }
    m_Window->PrepareInitFor("Vulkan");

    m_Window->Init(m_Context, windowTitle, windowSize);

    m_Instance.Init(m_Context, windowTitle, m_Window, VSyncMode::Disable);
    m_AllocatedDrawImage = m_Instance.CreateDrawImage(m_Window->GetSize());
}


void VulkanRenderer::Uninit() {
    AddVulkanLog("Waiting on GPU to shutdown");

    m_Instance.WaitIdle();

    AddVulkanLog("Shutting down the renderer");

    m_Instance.DestroyImage(m_AllocatedDrawImage);

    // m_Uninitializers.Flush();
    m_Instance.Uninit();

    m_Window->Uninit();
}

void VulkanRenderer::Render() {
    Profiler::StartProfile({ typeid(VulkanRenderer), "Render" }, IM_COL32(255, 0, 35, 255));
    if (m_Context->GFlags & GameFlags_ImGui)
        ImGui_ImplVulkan_NewFrame();
    m_Window->OnRender();

    m_ActiveCmd = m_Instance.BeginFrame();
}

void VulkanRenderer::RenderImGui() {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ActiveCmd);
    ImGui::EndFrame();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui::UpdatePlatformWindows();
}

void VulkanRenderer::Display() {
    Profiler::EndProfile({ typeid(VulkanRenderer), "Render" });

    // if (m_Context->GFlags & GameFlags_ImGui) 
    //     RenderImGui();
    m_Instance.EndFrame(m_ActiveCmd, m_AllocatedDrawImage);
}

void VulkanRenderer::Clear() {
    // static Clock clock{};
    // clock.Start();
    // auto time = clock.Get();
    // auto sec = time.AsSeconds();
    //

    VkClearColorValue clearColor{ { .0f, .0f, .0f, 1.0f} };
    VkImageSubresourceRange imageRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    vku::TransitionImage(m_PipelineMgr, m_ActiveCmd, m_AllocatedDrawImage.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    m_Instance.ClearImage(m_ActiveCmd, clearColor, imageRange);
}

void VulkanRenderer::Update() {
    if (m_Instance.NeedsRecreateSwapchain()) {
        m_Instance.RecreateSwapchain(m_Window->GetSize());
        m_Instance.DestroyImage(m_AllocatedDrawImage);
        m_AllocatedDrawImage = m_Instance.CreateDrawImage(m_Window->GetSize());
    }
}

void VulkanRenderer::SetGlobalShader(std::shared_ptr<Shader> shader) {
    (void)shader;
    // TODO: implement the load of shaders using pipelines
}

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(const std::filesystem::path& path) {
    // TODO: implement the creation of texture and save data in higher class Texture as Shader
    auto data = ImageData::LoadImage(path);
    if (!data.PixelData || !data.HDRPixelData) {
        Logger::AddError(typeid(VulkanRenderer), "Failed to create texture from {}", path.string());
        return std::make_shared<VulkanTexture>(m_Instance.GetErrorTexture());
    }
    auto extent = VkExtent3D{
        .width = m_Window->GetSize().X,
        .height = m_Window->GetSize().Y,
        .depth = 1
    };


    auto info = vki::GetImageCreateInfo(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, extent);
    VulkanTexture texture{};
    if (data.HDRPixelData) 
        texture = m_Instance.CreateImage(info, data.PixelData);
    else
        texture = m_Instance.CreateImage(info, data.HDRPixelData); 
    // m_Instance.AttachTextureData(texture, data.PixelData);

    return std::make_shared<VulkanTexture>(texture);
}

std::shared_ptr<Font> VulkanRenderer::CreateFont(const std::filesystem::path& path) {
    (void)path;

    // TODO: use freetype for font loading
    
   
    return nullptr;
}

std::shared_ptr<Shader> VulkanRenderer::CreateShader(const std::filesystem::path& path, Shader::Type type) {
    std::shared_ptr<VulkanShader> shader = std::make_shared<VulkanShader>();
        
    shader->SetType(type);

    if (!shader->Compile(m_Instance.GetDevice(), path)) {
        Logger::AddError(typeid(VulkanRenderer), "Failed to compile shader \"{}\"!", path.filename().string());
        return nullptr;
    }

    return shader;
}

void VulkanRenderer::ShowImGuiRenderTabContent() {
    ImGui::Text("Vulkan Renderer");
    ImGui::Text("Frame N°%u", m_Instance.GetCurrentFrameNumber());
    if (ImGui::BeginTabBar("vulkanTabBar")) {
        if (ImGui::BeginTabItem("Device")) {
            auto apiVer = m_Instance.GetDeviceProperties().apiVersion;
            auto apiStr = std::format("{}.{}.{}", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_PATCH(apiVer));
            ImGui::Text("Active Device: %s ", m_Instance.GetDeviceProperties().deviceName);
            ImGui::Text("API Version: %s (%u)", apiStr.c_str(), m_Instance.GetDeviceProperties().apiVersion);
            ImGui::Text("Driver Version: %u", m_Instance.GetDeviceProperties().driverVersion);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Renderer")) {
            m_StopRendering ? ImGui::TextColored({255, 0, 0, 255}, "Not Rendering") : ImGui::TextColored({0, 255, 0, 255}, "Rendering");
            ImGui::SeparatorText("Alloc");
            ImGui::Text("Image: %u", m_Instance.GetImageAllocations());
            ImGui::Text("Buffer: %u", m_Instance.GetBufferAllocations());
            ImGui::Text("Total: %u", m_Instance.GetImageAllocations() + m_Instance.GetBufferAllocations());
            ImGui::SeparatorText("Swapchain");
            ImGui::Text("Extent: (X: %u, Y: %u)", m_Instance.GetSwapchainExtent().X, m_Instance.GetSwapchainExtent().Y);
            ImGui::Text("Format: %s", string_VkFormat(m_Instance.GetSwapchainFormat()));
            ImGui::Text("Images: %zu", m_Instance.GetSwapchainImagesSize());
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
    if (auto resizedEvent = event->GetIf<GameResizedEvent>(event)) {
        m_Instance.RecreateSwapchain(m_Window->GetSize());
        m_Instance.DestroyImage(m_AllocatedDrawImage);
        m_AllocatedDrawImage = m_Instance.CreateDrawImage(m_Window->GetSize());
    }
}

void VulkanRenderer::DrawElement(GraphicalElement& element) {
    auto vulkanData = std::dynamic_pointer_cast<VulkanRendererData>(element.Data);
    vulkanData->MakeDraw(m_Instance, m_AllocatedDrawImage, m_ActiveCmd);
}

void VulkanRenderer::InitElement(GraphicalElement& element) {
    if (!element.Data) {
        element.Data = std::make_shared<VulkanRendererData>();
    }
    auto vulkanData = std::dynamic_pointer_cast<VulkanRendererData>(element.Data);
    if (!element.Data->IsInitialized()) {
        vulkanData->Init(m_Instance, m_AllocatedDrawImage.Format);
    }
}

void VulkanRenderer::UninitElement(GraphicalElement& element) {
    if (!element.Data) return;
    auto vulkanData = std::dynamic_pointer_cast<VulkanRendererData>(element.Data);
    vulkanData->Uninit(m_Instance);
}

std::shared_ptr<BaseWindow> VulkanRenderer::GetWindow() const {
    return m_Window;
}
