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

#include <thread>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <volk.h>

#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/ImageData.h"
#include "BeatEngine/Util/Profiler.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/System/Clock.h"
#include "BeatEngine/System/Time.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Util/Graphics.hpp"

void VulkanRenderer::Init(std::string windowTitle, Vector2u windowSize, VSyncMode vSync) {
    AddVulkanLog("Initializing VulkanRenderer");

    if (m_vSyncMode == VSyncMode::Default)
        m_vSyncMode = vSync;

    if (m_Window == nullptr) {
        m_Window = std::make_shared<SDLWindow>();
        std::static_pointer_cast<SDLWindow>(m_Window)->SetWindowFlags(SDL_WINDOW_RESIZABLE);
    }
    m_Window->PrepareInitFor("Vulkan");

    m_Window->Init(m_Context, windowTitle, windowSize);

    m_Instance.Init(m_Context, windowTitle, m_DeviceIndex, m_Window, vSync);
    m_AllocatedDrawImage = m_Instance.CreateDrawImage(m_Window->GetSize());

    {
        AddVulkanLog("Creating default shaders");

        const auto pushConstantRange = VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = sizeof(DefaultPushConstants)
        };

        auto vertexShader = std::make_shared<VulkanShader>();
        vertexShader->SetType(Shader::Type::Vertex);
        vertexShader->SetStage(VK_SHADER_STAGE_VERTEX_BIT);
        vertexShader->SetNextStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        vertexShader->AddPushConstants(pushConstantRange);
        vertexShader->AddDescLayout(m_Instance.GetBindlessDescSetLayout());
        if (!vertexShader->LoadFromCompiledFile(m_Instance.GetDevice(), "shaders/base.vert.spv"))
            THROW_RUNTIME_ERROR("Failed to create shader!");

        const auto fragShader = std::make_shared<VulkanShader>();
        fragShader->SetType(Shader::Type::Fragment);
        fragShader->SetStage(VK_SHADER_STAGE_FRAGMENT_BIT);
        fragShader->AddPushConstants(pushConstantRange);
        fragShader->AddDescLayout(m_Instance.GetBindlessDescSetLayout());
        if (!fragShader->LoadFromCompiledFile(m_Instance.GetDevice(), "shaders/base.frag.spv"))
            THROW_RUNTIME_ERROR("Failed to create shader");

        m_DefaultVertexShader = vertexShader;
        m_DefaultFragmentShader = fragShader;
    }

}

void VulkanRenderer::Uninit() {
    AddVulkanLog("Waiting on GPU to shutdown");

    m_Instance.WaitIdle();

    AddVulkanLog("Shutting down the renderer");

    for (const auto& frameData : m_RenderFramesData) {
        for (const auto& [viewID, drawDatas] : frameData.DrawDatas) {
            for (const auto& drawData : drawDatas) {
                m_Instance.DestroyBuffer(drawData.VertexBuffer);
                m_Instance.DestroyBuffer(drawData.DrawCommandBuffer);
            }
        }
    }

    for (const auto& [viewID, layouts] : m_Layouts) {
        for (const auto& layout : layouts) {
            vkDestroyPipelineLayout(m_Instance.GetDevice(), layout, nullptr);
        }
    }

    m_Instance.DestroyImage(m_AllocatedDrawImage);
    m_Instance.Uninit();
    m_Window->Uninit();
}

void VulkanRenderer::Render() {
    // if (m_TargetFps != 0 && m_vSyncMode == VSyncMode::Disable ) {
    //     std::this_thread::sleep_for(std::chrono::seconds(1 / m_TargetFps));
    // }

    Profiler::StartProfile({ typeid(VulkanRenderer), "Render" }, IM_COL32(255, 0, 35, 255));
    if (m_Context->GFlags & GameFlags_ImGui)
        ImGui_ImplVulkan_NewFrame();
    m_Window->OnRender();

    m_ActiveCmd = m_Instance.BeginFrame();
    m_Active = true;
}

void VulkanRenderer::RenderImGui() {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_ActiveCmd);
    ImGui::EndFrame();
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        ImGui::UpdatePlatformWindows();
}

void VulkanRenderer::Display() {
    m_Instance.EndFrame(m_ActiveCmd, m_AllocatedDrawImage);
    m_Active = false;

    Profiler::EndProfile({ typeid(VulkanRenderer), "Render" });
}

void VulkanRenderer::Clear() {
    static Clock clock{};
    clock.Start();
    auto time = clock.Get();
    auto sec = time.AsSeconds();



    VkClearColorValue clearColor{ { tan(sec), cos(sec), tan(sec), 1.0f} };
    VkImageSubresourceRange imageRange{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    vku::TransitionImage(m_ActiveCmd, m_AllocatedDrawImage.Image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    m_Instance.ClearImage(m_ActiveCmd, clearColor, imageRange);
}

void VulkanRenderer::Update() {
    if (m_Instance.NeedsRecreateSwapchain() && m_Window->GetWindowDriver() != WindowDriver::Wayland) {
        if (m_Active)
            THROW_RUNTIME_ERROR("CommandBuffer is still active when recreating the swapchain.");
        m_Instance.DestroyImage(m_AllocatedDrawImage);
        m_Instance.RecreateSwapchain(m_Window, m_Window->GetSize());
        m_AllocatedDrawImage = m_Instance.CreateDrawImage(m_Window->GetSize());
    }
}

void VulkanRenderer::SetGlobalShader(std::shared_ptr<Shader> shader) {
    (void)shader;
    // TODO: implement the load of shaders using pipelines
}

unsigned int VulkanRenderer::GetMaxTextureSize() {
    return m_Instance.GetDeviceProperties().limits.maxImageDimension2D;
};

std::shared_ptr<Texture> VulkanRenderer::CreateTexture(const std::filesystem::path& path) {
    auto data = ImageData::LoadImage(path);
    if (!data.PixelData && !data.HDRPixelData) {
        Logger::AddError(typeid(VulkanRenderer), "Failed to create texture from {}", path.string());
        return m_Instance.GetErrorTexture();
    }

    auto extent = VkExtent3D{
        .width = static_cast<uint32_t>(data.Size.X),
        .height = static_cast<uint32_t>(data.Size.Y),
        .depth = 1
    };

    auto info = vki::GetImageCreateInfo(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, extent);
    VulkanTexture texture{};
    if (data.HDRPixelData) 
        texture = m_Instance.CreateImage(info, data.HDRPixelData);
    else
        texture = m_Instance.CreateImage(info, data.PixelData);

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
    texture = m_Instance.CreateImage(info, static_cast<const void*>(pixelData));
    texture.m_Size = size;

    return std::make_shared<VulkanTexture>(texture);
} 

std::shared_ptr<Texture> VulkanRenderer::CreateEmptyTexture(Vector2u size) {
    auto info = vki::GetImageCreateInfo(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, { size.X, size.Y, 1 });

    VulkanTexture texture{};
    texture = m_Instance.CreateImage(info, nullptr);
    texture.m_Size = size;

    return std::make_shared<VulkanTexture>(texture);
}

void VulkanRenderer::UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) {
    auto vulkanTexture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    m_Instance.AttachImageData(vulkanTexture->m_CacheID, pixelData, dest, size);
}
void VulkanRenderer::UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) {
    auto dstVulkan = std::dynamic_pointer_cast<VulkanTexture>(dstTexture);
    auto srcVulkan = std::dynamic_pointer_cast<VulkanTexture>(srcTexture);
    m_Instance.CopyImageToImage(srcVulkan->m_CacheID, dstVulkan->m_CacheID);
}

void VulkanRenderer::DestroyTexture(std::shared_ptr<Texture> texture) {
    auto vulkanTexture = std::dynamic_pointer_cast<VulkanTexture>(texture);
    m_Instance.DestroyImage(vulkanTexture->m_CacheID);
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
    if (auto resizedEvent = event->GetIf<GameResizedEvent>(event); resizedEvent && m_Window->GetWindowDriver() == WindowDriver::Wayland) {
        m_Instance.RecreateSwapchain(m_Window, resizedEvent->Size);
        m_Instance.DestroyImage(m_AllocatedDrawImage);
        m_AllocatedDrawImage = m_Instance.CreateDrawImage(resizedEvent->Size);
    }
}

void VulkanRenderer::DrawVertices(VertexArray& vertices, RenderState state) {
    if (vertices.GetSize() <= 0) return;

	auto& drawDatas = m_RenderFramesData.at(m_Instance.GetCurrentFrameIndex()).DrawDatas;

	auto viewID = state.DrawInGlobal ? typeid(nullptr) : m_Context->ActiveView;

	if (!drawDatas.contains(viewID))
		drawDatas[viewID] = {};
	if (!m_Layouts.contains(viewID))
		m_Layouts[viewID] = {};

    if (!IsVertexArrayInitialized(vertices)) {
        InitVertices(vertices, state);
    }
    uint32_t vertexID{};

    vertexID = GetVertexArrayID(vertices);

    auto& cmdBuffer = drawDatas.at(viewID).at(vertexID).DrawCommandBuffer;
    auto& vertexBuffer = drawDatas.at(viewID).at(vertexID).VertexBuffer;
    
    auto pipelineLayout = m_Layouts.at(viewID).at(vertexID);

    if (vertices.GetSize() * sizeof(Vertex) != vertexBuffer.BufferSize && vertices.GetSize() > 0) {
        for (unsigned int i = 0; i < FRAME_OVERLAP; ++i) {
            auto& buffer = m_RenderFramesData[i].DrawDatas.at(viewID).at(vertexID).VertexBuffer;
            
            m_Instance.DestroyBuffer(buffer);

            buffer = m_Instance.CreateBuffer(
                vertices.GetSize() * sizeof(Vertex),
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT
            );
        }
    }

    memcpy(
        cmdBuffer.AllocationInfo.pMappedData,
        state._DrawCommand.get(),
        state.DrawCommandSize
    );

    memcpy(
        vertexBuffer.AllocationInfo.pMappedData,
        vertices.GetData(),
        vertices.GetSize() * sizeof(Vertex)
    );

    VkRenderingAttachmentInfo colorInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = m_AllocatedDrawImage.ImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode{},
        .resolveImageView{},
        .resolveImageLayout{},
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue{
            .color = {
                .float32 = { 0.f, 0.f, 0.f, 1.f },
                // .int32 = { 0, 0, 0, 1 },
                // .uint32 = { 0, 0, 0, 1 }
            }
        }
    };

    const auto renderInfo = vki::GetRenderingInfo(
        { m_AllocatedDrawImage.Extent.width, m_AllocatedDrawImage.Extent.height }, 
        &colorInfo, 
        nullptr
    );

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

    {
        auto imageBarrier = VkImageMemoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask{} ,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = m_AllocatedDrawImage.Image,
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

    vkCmdBeginRendering(m_ActiveCmd, &renderInfo);

    {
        std::shared_ptr<VulkanShader> vertShader = std::dynamic_pointer_cast<VulkanShader>(m_DefaultVertexShader);
        std::shared_ptr<VulkanShader> fragShader = std::dynamic_pointer_cast<VulkanShader>(m_DefaultFragmentShader);
        if (state.VertShader) {
            vertShader = std::dynamic_pointer_cast<VulkanShader>(state.VertShader);
        }
        if (state.FragShader) {
            fragShader = std::dynamic_pointer_cast<VulkanShader>(state.FragShader);
        }

        vkCmdBindShadersEXT(m_ActiveCmd, 1, &vertShader->GetStage(), vertShader->GetShaderImpl());
        vkCmdBindShadersEXT(m_ActiveCmd, 1, &fragShader->GetStage(), fragShader->GetShaderImpl());

        auto stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        vkCmdBindShadersEXT(m_ActiveCmd, 1, &stage, nullptr);
        stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        vkCmdBindShadersEXT(m_ActiveCmd, 1, &stage, nullptr);
        stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        vkCmdBindShadersEXT(m_ActiveCmd, 1, &stage, nullptr);
    }

    {
        const auto viewport = VkViewport{
            .x = 0,
            .y = 0,
            .width = (float)m_AllocatedDrawImage.Extent.width,
            .height = (float)m_AllocatedDrawImage.Extent.height,
            .minDepth = .0f,
            .maxDepth = 1.f
        };
        vkCmdSetViewportWithCount(m_ActiveCmd, 1, &viewport);

        const auto scissor = VkRect2D{
            .offset{},
            .extent = { m_AllocatedDrawImage.Extent.width, m_AllocatedDrawImage.Extent.height }
        };
        vkCmdSetScissorWithCount(m_ActiveCmd, 1, &scissor);
    }

    vkCmdSetPolygonModeEXT(m_ActiveCmd, VK_POLYGON_MODE_FILL);
    switch (vertices.GetType()) {
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

    vkCmdSetLineWidth(m_ActiveCmd, state.LineWidth);
    vkCmdSetPrimitiveRestartEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetDepthBiasEnable(m_ActiveCmd, VK_TRUE);
    vkCmdSetDepthBias(m_ActiveCmd, 0.0f, 0.0f, 0.0f);
    vkCmdSetDepthBoundsTestEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetRasterizerDiscardEnable(m_ActiveCmd, VK_FALSE); 
    vkCmdSetDepthTestEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetDepthClampEnableEXT(m_ActiveCmd, VK_FALSE);
    vkCmdSetDepthWriteEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetStencilTestEnable(m_ActiveCmd, VK_FALSE);
    vkCmdSetCullMode(m_ActiveCmd, VK_CULL_MODE_NONE);
    vkCmdSetFrontFace(m_ActiveCmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    vkCmdSetRasterizationSamplesEXT(m_ActiveCmd, VK_SAMPLE_COUNT_1_BIT);
    vkCmdSetAlphaToCoverageEnableEXT(m_ActiveCmd, VK_FALSE);

    VkColorBlendEquationEXT blendEquation{};
    blendEquation.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendEquation.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendEquation.colorBlendOp        = VK_BLEND_OP_ADD;
    blendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendEquation.alphaBlendOp       = VK_BLEND_OP_ADD;

    vkCmdSetColorBlendEquationEXT(m_ActiveCmd, 0, 1, &blendEquation);

    m_Instance.BindBindlessDescSet(m_ActiveCmd, pipelineLayout);

    vkCmdBindVertexBuffers2(
        m_ActiveCmd,
        0,
        1,
        &vertexBuffer.Buffer,
        &vertexOffset,
        &vertexBuffer.BufferSize,
        &vertexStride
    );

    vkCmdSetVertexInputEXT(
        m_ActiveCmd,
        1,
        &vertexBinding,
        vertexAttributes.size(),
        vertexAttributes.data()
    );

    VkSampleMask sampleMask = 0xFFFFFFFF;
    vkCmdSetSampleMaskEXT(m_ActiveCmd, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

    VkBool32 blendEnable = VK_FALSE;
    vkCmdSetColorBlendEnableEXT(m_ActiveCmd, 0, 1, &blendEnable);

    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                       VK_COLOR_COMPONENT_G_BIT | 
                                       VK_COLOR_COMPONENT_B_BIT | 
                                       VK_COLOR_COMPONENT_A_BIT;
    vkCmdSetColorWriteMaskEXT(m_ActiveCmd, 0, 1, &colorWriteMask);
    

    std::shared_ptr<PushConstants> pushConstants{};
    uint32_t pushSize;
    if (state._PushConstants) {
        pushConstants = state._PushConstants;
        pushSize = state.PushConstantsSize;
    }
    else {
        pushConstants = std::make_shared<DefaultPushConstants>(cmdBuffer.Address);
        pushSize = sizeof(DefaultPushConstants);
    }

    vkCmdPushConstants(
        m_ActiveCmd,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        pushSize,
        pushConstants.get()
    );

    vkCmdDraw(m_ActiveCmd, vertices.GetSize(), 1, 0, 0);

    vkCmdEndRendering(m_ActiveCmd);

    auto presentBarrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_2_NONE_KHR, 
        .dstAccessMask{},
        .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex{},
        .dstQueueFamilyIndex{},
        .image         = m_AllocatedDrawImage.Image,
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

    if ((m_Context->GFlags & GameFlags_ImGui) && (m_Context->EFlags & EnvFlags_Debug)) {
        ImGui::Begin("Rendered elements so far");
        if (ImGui::TreeNode(IsVertexArrayHighlight(vertices) ? std::format("VertexArrayHighlight_Of{}_ArrayID{}_ID{}_In_{}", GetVertexArrayHighlightSourceID(vertices), GetVertexArrayHighlightID(vertices), GetVertexArrayID(vertices), (state.DrawInGlobal ? "Global" : viewID.name())).c_str() : std::format("VertexArray_ID{}_In_{}", GetVertexArrayID(vertices), (state.DrawInGlobal ? "Global" : viewID.name())).c_str())) {
            ImGui::Text("PrimitiveType : %s", PrimitiveTypeUtils::ToString(vertices.GetType()).c_str()); 
            ImGui::Text("View drawing to: %s", state.DrawInGlobal ? "Global" : viewID.name());
            ImGui::Text("Size: %zu", vertices.GetSize());
            ImGui::SeparatorText("Vertex");
            for (size_t i = 0; i < vertices.GetSize(); i++) {
                auto vertex = vertices[i];
                if (ImGui::TreeNode(std::format("Vertex{}", i).c_str())) {
                    ImGui::Text("Position: (X: %f, Y: %f)", vertex.Position.X, vertex.Position.Y);
                    ImGui::Text("UV: (X: %f, Y: %f)", vertex.UV.X, vertex.UV.Y);
                    ImGui::ColorButton("VertexColor", { vertex.Color.R, vertex.Color.G, vertex.Color.B, vertex.Color.A });
                    ImGui::TreePop();
                }

            }
            ImGui::TreePop();
        }
        ImGui::End();
    }

    if (state.HighlightVertices) {
        if (!m_Highlights.contains(m_Context->ActiveView))
            m_Highlights[m_Context->ActiveView] = {};

        VertexArray* highlightVertices = nullptr;
        
        if (
            GetVertexArrayHighlightID(vertices) != (std::numeric_limits<uint32_t>::max)() &&
            GetVertexArrayHighlightID(vertices) < m_Highlights.at(m_Context->ActiveView).size()
        ) {
            highlightVertices = &m_Highlights.at(m_Context->ActiveView).at(GetVertexArrayHighlightID(vertices));
        }
        else {
            auto highlightVertex = Util::GetOutlineVertices(vertices);

            for (auto& vertex : highlightVertex) {
                vertex.Color = state.HighlightColor;
            }

            highlightVertex.SetType(PrimitiveType::LineList);
            SetVertexArrayInitializedStatus(highlightVertex, false);

            SetVertexArrayID(highlightVertex, 0);
            SetVertexHighlightID(highlightVertex, GetVertexArrayID(vertices), GetFreeHighlightIndex(m_Context->ActiveView));
            SetVertexArrayHighlightStatus(highlightVertex, true);

            SetVertexHighlightID(vertices, GetVertexArrayHighlightID(highlightVertex));

            m_Highlights.at(m_Context->ActiveView).emplace_back(highlightVertex);
            highlightVertices = &m_Highlights.at(m_Context->ActiveView).back();
        }

        if (highlightVertices == nullptr) return;

        auto highlightState = state;
        highlightState.HighlightVertices = false;
        highlightState._DrawCommand->textureID = NULL_IMAGE_ID;
        highlightState.LineWidth = 1.f;
        DrawVertices(*highlightVertices, highlightState);
    }

}

void VulkanRenderer::InitVertices(VertexArray& vertices, RenderState state) {
    uint32_t vertexID{};

    auto viewID = state.DrawInGlobal ? typeid(nullptr) : m_Context->ActiveView;

    if (IsVertexArrayHighlight(vertices))
        vertexID = GetFreeHighlightIndex(viewID);
    else
        vertexID = GetFreeBufferIndex(viewID);

    SetVertexArrayID(vertices, vertexID);
    SetVertexArrayInitializedStatus(vertices, true);

    {
        VkPipelineLayout pipelineLayout{};
        const auto layout = m_Instance.GetBindlessDescSetLayout();
        uint32_t size{};
        if (state.PushConstantsSize == sizeof(PushConstants))
            size = sizeof(DefaultPushConstants);
        else
            size = state.PushConstantsSize;

        const auto pushConstantRange = VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = size
        };
        pipelineLayout = vkb::CreatePipelineLayout(m_Instance.GetDevice(), layout, pushConstantRange);

        m_Layouts.at(viewID).emplace(m_Layouts.at(viewID).begin() + vertexID, pipelineLayout);
    }
    
    for (unsigned int i = 0; i < FRAME_OVERLAP; ++i) {
        if (!m_RenderFramesData.at(i).DrawDatas.contains(viewID))
            m_RenderFramesData.at(i).DrawDatas[viewID] = {};

        DrawData drawData{};
         
        drawData.DrawCommandBuffer = m_Instance.CreateBuffer(
            state.DrawCommandSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        );

        if (!vertices.IsEmpty())
             drawData.VertexBuffer = m_Instance.CreateBuffer(
                vertices.GetSize() * sizeof(Vertex), 
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT 
            );
        else
            drawData.VertexBuffer = m_Instance.CreateBuffer(
                sizeof(Vertex), 
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT
            );

        m_RenderFramesData.at(i).DrawDatas.at(viewID).emplace(m_RenderFramesData.at(i).DrawDatas.at(viewID).begin() + vertexID, drawData);
    }
}

void VulkanRenderer::UninitVertices(VertexArray& vertices) {
    if (m_Layouts.at(m_Context->ActiveView).size() <= GetVertexArrayID(vertices)) return;

    auto layout = m_Layouts.at(m_Context->ActiveView).at(GetVertexArrayID(vertices));

    vkDestroyPipelineLayout(m_Instance.GetDevice(), layout, nullptr);

    for (unsigned int i = 0; i < FRAME_OVERLAP; i++) {
        if (!m_RenderFramesData.at(i).DrawDatas.contains(m_Context->ActiveView)) return;
        if (m_RenderFramesData.at(i).DrawDatas.at(m_Context->ActiveView).size() <= GetVertexArrayID(vertices)) return;
        if (!m_Layouts.contains(m_Context->ActiveView)) return;

        auto& cmdBuffer = m_RenderFramesData.at(m_Instance.GetCurrentFrameIndex()).DrawDatas.at(m_Context->ActiveView).at(GetVertexArrayID(vertices)).DrawCommandBuffer;
        auto& vertexBuffer = m_RenderFramesData.at(m_Instance.GetCurrentFrameIndex()).DrawDatas.at(m_Context->ActiveView).at(GetVertexArrayID(vertices)).VertexBuffer;

        m_Instance.DestroyBuffer(cmdBuffer);
        m_Instance.DestroyBuffer(vertexBuffer);
    }
}

uint32_t VulkanRenderer::GetFreeBufferIndex(std::type_index viewID) {
    if (!m_RenderFramesData.at(m_Instance.GetCurrentFrameIndex()).DrawDatas.contains(viewID))
        m_RenderFramesData.at(m_Instance.GetCurrentFrameIndex()).DrawDatas[viewID] = {};

    return m_RenderFramesData.at(m_Instance.GetCurrentFrameIndex()).DrawDatas.at(viewID).size();
}

uint32_t VulkanRenderer::GetFreeHighlightIndex(std::type_index viewID) {
    if (!m_Highlights.contains(viewID))
        m_Highlights[viewID] = {};

    return m_Highlights.at(viewID).size();
}

std::shared_ptr<BaseWindow> VulkanRenderer::GetWindow() const {
    return m_Window;
}
