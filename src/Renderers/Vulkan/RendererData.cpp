#include "BeatEngine/Renderers/Vulkan/RendererData.hpp"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/Vertex.hpp"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/Instance.h"
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Util/Exception.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include <cstring>
#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void VulkanRendererData::Init(VK::Instance& instance, VkFormat drawImageFormat) {
    const auto& device = instance.GetDevice();    

    const auto layout = instance.GetBindlessDescSetLayout();
    const auto pushConstantRange = VkPushConstantRange{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstants)
    };

    m_PipelineLayout = vkb::CreatePipelineLayout(device, layout, pushConstantRange);
    const auto vertexShader = std::make_shared<VulkanShader>();
    vertexShader->SetType(Shader::Type::Vertex);
    vertexShader->SetStage(VK_SHADER_STAGE_VERTEX_BIT);
    vertexShader->SetNextStage(VK_SHADER_STAGE_FRAGMENT_BIT);
    vertexShader->AddPushConstants(pushConstantRange);
    vertexShader->AddDescLayout(layout);
    if (!vertexShader->LoadFromCompiledFile(device, "shaders/base.vert.spv")) {
        THROW_RUNTIME_ERROR("Failed to create shader");
    }

    const auto fragShader = std::make_shared<VulkanShader>();
    fragShader->SetType(Shader::Type::Fragment);
    fragShader->SetStage(VK_SHADER_STAGE_FRAGMENT_BIT);
    fragShader->AddPushConstants(pushConstantRange);
    fragShader->AddDescLayout(layout);
    if (!fragShader->LoadFromCompiledFile(device, "shaders/base.frag.spv")) {
        THROW_RUNTIME_ERROR("Failed to create shader");
    }

    m_VertShader = vertexShader;
    m_FragShader = fragShader;

    for (unsigned int i = 0; i < FRAME_OVERLAP; ++i) {
        auto& cmdBuffer = m_FramesData[i].SpriteDrawCommandBuffer;
        auto& vertexBuffer = m_FramesData[i].VertexBuffer;

        cmdBuffer = instance.CreateBuffer(
            DrawCommandSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        );
        if (Vertices.size() != 0) 
            vertexBuffer = instance.CreateBuffer(
                Vertices.size() * sizeof(Vertex),
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT
            );
        else 
            vertexBuffer = instance.CreateBuffer(
                sizeof(Vertex),
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT 
            );
    }

    m_Initialized = true; 
}

void VulkanRendererData::Uninit(VK::Instance& instance) {
    auto device = instance.GetDevice();

    for (const auto& frame : m_FramesData) {
        instance.DestroyBuffer(frame.SpriteDrawCommandBuffer);
        instance.DestroyBuffer(frame.VertexBuffer);
    }

    vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
}

void VulkanRendererData::MakeDraw(VK::Instance& instance, AllocatedImage& drawImage, VkCommandBuffer cmd) {
    const auto& commandBuffer = GetFrameData(instance.GetCurrentFrameIndex()).SpriteDrawCommandBuffer;
    const auto& vertexBuffer = GetFrameData(instance.GetCurrentFrameIndex()).VertexBuffer;

    if (Vertices.size() * sizeof(Vertex) != vertexBuffer.BufferSize) {
        for (unsigned int i = 0; i < FRAME_OVERLAP; ++i) {
            auto& buffer = m_FramesData[i].VertexBuffer;

            instance.DestroyBuffer(buffer);

            buffer = instance.CreateBuffer(
                Vertices.size() * sizeof(Vertex),
                VK_BUFFER_USAGE_2_VERTEX_BUFFER_BIT
            );
        }
    }

    memcpy(
        commandBuffer.AllocationInfo.pMappedData, 
        m_Command.get(), 
        DrawCommandSize
    );
    memcpy(
        vertexBuffer.AllocationInfo.pMappedData,
        Vertices.data(),
        Vertices.size() * sizeof(Vertex)
    );

    VkRenderingAttachmentInfo colorInfo{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .pNext = nullptr,
        .imageView = drawImage.ImageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    };
    const auto renderInfo = vki::GetRenderingInfo({ drawImage.Extent.width, drawImage.Extent.height }, &colorInfo, nullptr);

    const auto vertexBinding = VkVertexInputBindingDescription2EXT{
        .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
        .pNext = nullptr,
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        .divisor = 1
    }; 

    const auto vertexAtttributes = std::array{

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
            .image = drawImage.Image,
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
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &imageBarrier
        };

        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }


    vkCmdBeginRendering(cmd, &renderInfo);

    {
        auto vertexShader = std::static_pointer_cast<VulkanShader>(m_VertShader);
        vkCmdBindShadersEXT(cmd, 1, &vertexShader->GetStage(), vertexShader->GetShaderImpl());

        auto fragShader = std::static_pointer_cast<VulkanShader>(m_FragShader);
        vkCmdBindShadersEXT(cmd, 1, &fragShader->GetStage(), fragShader->GetShaderImpl());

        auto stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        vkCmdBindShadersEXT(cmd, 1, &stage, nullptr);
        stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        vkCmdBindShadersEXT(cmd, 1, &stage, nullptr);
        stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        vkCmdBindShadersEXT(cmd, 1, &stage, nullptr);
    }

    const auto viewport = VkViewport{
        .x = 0,
        .y = 0,
        .width = (float)drawImage.Extent.width,
        .height = (float)drawImage.Extent.height,
        .minDepth = .0f,
        .maxDepth = 1.f
    };
    vkCmdSetViewportWithCount(cmd, 1, &viewport);

    const auto scissor = VkRect2D{
        .offset{},
        .extent = { drawImage.Extent.width, drawImage.Extent.height }
    };
    vkCmdSetScissorWithCount(cmd, 1, &scissor);

    vkCmdSetPolygonModeEXT(cmd, VK_POLYGON_MODE_FILL);
    vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    vkCmdSetPrimitiveRestartEnable(cmd, VK_FALSE);
    vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
    vkCmdSetDepthBoundsTestEnable(cmd, VK_FALSE);
    vkCmdSetRasterizerDiscardEnable(cmd, VK_FALSE); 
    vkCmdSetDepthTestEnable(cmd, VK_FALSE);
    vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
    vkCmdSetStencilTestEnable(cmd, VK_FALSE);
    vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
    vkCmdSetFrontFace(cmd, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    vkCmdSetRasterizationSamplesEXT(cmd, VK_SAMPLE_COUNT_1_BIT);
    vkCmdSetAlphaToCoverageEnableEXT(cmd, VK_FALSE);

    VkColorBlendEquationEXT blendEquation{};
    blendEquation.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendEquation.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendEquation.colorBlendOp        = VK_BLEND_OP_ADD;
    blendEquation.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blendEquation.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blendEquation.alphaBlendOp       = VK_BLEND_OP_ADD;

    vkCmdSetColorBlendEquationEXT(cmd, 0, 1, &blendEquation);

    // VkDependencyInfo dependencyInfo{
    //     .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //     .pNext = nullptr,
    //     .imageMemoryBarrierCount = 1,
    // };
    // vkCmdPipelineBarrier2();
    

    instance.BindBindlessDescSet(cmd, m_PipelineLayout);

    vkCmdBindVertexBuffers2(
        cmd,
        0,
        1,
        &vertexBuffer.Buffer,
        &vertexOffset,
        &vertexBuffer.BufferSize,
        &vertexStride
    );

    vkCmdSetVertexInputEXT(
        cmd,
        1,
        &vertexBinding,
        vertexAtttributes.size(),
        vertexAtttributes.data()
    );

    VkSampleMask sampleMask = 0xFFFFFFFF;
    vkCmdSetSampleMaskEXT(cmd, VK_SAMPLE_COUNT_1_BIT, &sampleMask);

    VkBool32 blendEnable = VK_FALSE;
    vkCmdSetColorBlendEnableEXT(cmd, 0, 1, &blendEnable);

    VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                       VK_COLOR_COMPONENT_G_BIT | 
                                       VK_COLOR_COMPONENT_B_BIT | 
                                       VK_COLOR_COMPONENT_A_BIT;
    vkCmdSetColorWriteMaskEXT(cmd, 0, 1, &colorWriteMask);

    const auto pushConstants = PushConstants{ commandBuffer.Address };
    vkCmdPushConstants(
        cmd,
        m_PipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(PushConstants),
        &pushConstants
    );

    vkCmdDraw(cmd, Vertices.size(), 1, 0, 0);
    // vkCmdDraw(cmd, 3, 1, 0, 0);
    vkCmdEndRendering(cmd);

    auto presentBarrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask  = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_2_NONE_KHR, 
        .dstAccessMask{},
        .oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image         = drawImage.Image,
        .subresourceRange {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        }
    };

    VkDependencyInfo presentDependencyInfo{};
    presentDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    presentDependencyInfo.imageMemoryBarrierCount = 1;
    presentDependencyInfo.pImageMemoryBarriers = &presentBarrier;

    // Execute the barrier before submitting the command buffer
    vkCmdPipelineBarrier2(cmd, &presentDependencyInfo);
}

void VulkanRendererData::SetCommand(std::shared_ptr<DrawCommand> cmd) {
    m_Command = cmd;
}

void VulkanRendererData::DrawImGuiDrawData() {
    if (!m_Initialized) return;

    ImGui::SeparatorText("Command");
    ImGui::Text("projection: %s", glm::to_string(m_Command->transform).c_str());
    ImGui::Text("padding: X: %f, Y: %f", this->m_Command->padding.x, this->m_Command->padding.y);
    ImGui::Text("shaderID: %u", this->m_Command->shaderID);
    ImGui::Text("textureID: %u", this->m_Command->textureID);
    ImGui::SeparatorText("Vertices");

    for (size_t i = 0; i < Vertices.size(); i++) {
        const auto& vertex = Vertices[i];
        ImGui::Text("Vertex %zu", i);
        ImGui::Text("Position: (X: %f, Y: %f)", vertex.Position.X, vertex.Position.Y);
        ImGui::Text("Color:");
        ImGui::SameLine();
        ImGui::ColorButton(std::format("ColorVertex{}", i).c_str(), { vertex.Color.R, vertex.Color.G, vertex.Color.B, vertex.Color.A });
        ImGui::Text("Texture Coords: (X: %f, Y %f)", vertex.TexCoords.X, vertex.TexCoords.Y);
    }
}

void VulkanRendererData::SetShader(std::filesystem::path& path, Shader::Type type) {

}

