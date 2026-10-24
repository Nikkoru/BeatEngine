#pragma once

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Shader.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/Context.h"
#include "BeatEngine/Renderers/Vulkan/Allocator.hpp"
#include "BeatEngine/Renderers/Vulkan/ImageCache.hpp"
#include "BeatEngine/Renderers/Vulkan/ImmediateExecutor.h"
#include "BeatEngine/Renderers/Vulkan/Swapchain.h"
#include "BeatEngine/Renderers/Vulkan/VulkanRendererData.hpp"

#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <shaderc/shaderc.hpp>

class VulkanRenderer : public Renderer {
public: 
    VulkanRenderer(AppContext* ctx = nullptr, std::shared_ptr<BaseWindow> window = nullptr, uint32_t idx = 0);
    ~VulkanRenderer() override;
private:
    VK::Context m_VKContext;
    VK::Allocator m_Allocator;
    VK::Swapchain m_Swapchain;

    // ImageCache m_Cache;

    VkSurfaceKHR m_Surface{}; 
    VkSemaphore m_FrameTimelineSemaphore{};

    VulkanShader m_DefaultVertShader{};
    VulkanShader m_DefaultFragShader{};

    // (SSBO)
    GPUBuffer m_VertexBuffer{};
    GPUBuffer m_DrawCmdBuffer{};

    using VertexID = uint32_t;
    VertexID m_ActiveItem{};

    struct DrawData {
        VertexArray vertices;
        RenderState state;
        VulkanRendererData data{};
    };

    std::vector<DrawData> m_ItemsToDraw;


    std::unique_ptr<VulkanTexture> m_ErrorTexture{};
    std::unique_ptr<VulkanTexture> m_WhiteTexture{};

    VkClearColorValue m_ClearValue{ { .0f, .0f, .0f, 1.f } };

    uint64_t m_FrameCount{};

    // for cmpute
    VkPipelineLayout m_ComputePipelineLayout{};
    VkPipeline m_ComputePipeline{};

    VK::ImmediateExecutor m_Executor;
    VkCommandBuffer m_ActiveCmd{};

    struct FrameData {
        VkCommandPool cmdPool;
        VkCommandBuffer cmdBuffer;
        uint64_t lastSignalValue;
    };
    std::vector<FrameData> m_FrameData;

    bool m_Rendering{ true };
public:
    void Render() override;
    void RenderImGui() override;
    void Clear() override;
    void Display() override;
    void Update() override;
    void SetGlobalShader(std::shared_ptr<Shader> shader) override;
    void ProcessEvent(Optional<Base::Event> event) override;

    void DrawVertices(VertexArray& vertices, RenderState state = RenderState::Default) override;

    void DrawElement(GraphicalElement& element, RenderState state = RenderState::Default) override;

    unsigned int GetMaxTextureSize() override;
    std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path) override;
    std::shared_ptr<Texture> CreateTexture(const uint8_t* pixelData, Vector2u size) override;
    std::shared_ptr<Texture> CreateEmptyTexture(Vector2u size) override;
    std::shared_ptr<Texture> GetErrorTexture() override { return nullptr; }
    void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) override;
    void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData) override { UpdateTexture(texture, pixelData, texture->GetSize(), { 0, 0 }); };
    void UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) override;
    void DestroyTexture(std::shared_ptr<Texture> texture) override;
    std::shared_ptr<Shader> CreateShader(const std::filesystem::path& path, Shader::Type type) override;

    void ShowImGuiRenderTabContent() override;

    Optional<Base::Event> PollEvent() const override;
    std::shared_ptr<BaseWindow> GetWindow() const;
private:
    void _CreateFrameSubmission(uint32_t frameCount);
    void _CreateImGuiDescriptorPool();
    void _InitImGui();

    void _CreateDefaultShaders();

    bool _PrepareForRendering(); 
    void _RenderFrameData();
    void _BeginCmdRecording();
    void _EndCmdRecording();

    void _PrepareForImGui();
    void _EndFrame();
};
