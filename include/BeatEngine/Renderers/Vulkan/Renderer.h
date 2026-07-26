#pragma once

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/Instance.h"

#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <shaderc/shaderc.hpp>

class VulkanRenderer : public Renderer {
public: 
    VulkanRenderer() : VulkanRenderer(nullptr) {}
    VulkanRenderer(GameContext* context) : Renderer(context) {}
    ~VulkanRenderer() override = default;
private:
    uint32_t m_DeviceIndex{};


    // Vulkan::Core
    VK::Instance m_Instance;
    AllocatedImage m_AllocatedDrawImage{};
    VkCommandBuffer m_ActiveCmd{};
    bool m_Active{ false };

    struct DrawData {
        GPUBuffer DrawCommandBuffer;
        GPUBuffer VertexBuffer;
    };

    struct FrameDrawData {
        std::unordered_map<std::type_index, std::vector<DrawData>> DrawDatas;
    };

    class DefaultPushConstants : public PushConstants {
    public:
        VkDeviceAddress CommandBuffer;
    public:
        DefaultPushConstants(VkDeviceAddress cmd) : CommandBuffer(cmd) {}
    };

    std::shared_ptr<Shader> m_DefaultVertexShader{};
    std::shared_ptr<Shader> m_DefaultFragmentShader{};

    std::array<FrameDrawData, FRAME_OVERLAP> m_RenderFramesData;
    std::unordered_map<std::type_index, std::vector<VkPipelineLayout>> m_Layouts;
    std::unordered_map<std::type_index, std::vector<VertexArray>> m_Highlights;
private:
    bool m_StopRendering{ false };
private:
    VulkanTexture& GetErrorTexture() { return m_Instance.GetErrorTexture(); }
    VulkanTexture& GetWhiteTexture() { return m_Instance.GetWhiteTexture(); }
public:
    void Init(std::string windowTitle, Vector2u windowSize, VSyncMode vSync = VSyncMode::Disable) override;
    void Uninit() override;
public:
    void Render() override;
    void RenderImGui() override;
    void Clear() override;
    void Display() override;
    void Update() override;
    void SetGlobalShader(std::shared_ptr<Shader> shader) override;
    void ProcessEvent(Optional<Base::Event> event) override;

    void DrawVertices(VertexArray& vertices, RenderState state = RenderState::Default) override;
    void InitVertices(VertexArray& vertices, RenderState state) override;
    void UninitVertices(VertexArray& vertices) override;


    unsigned int GetMaxTextureSize() override;
    std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path) override;
    std::shared_ptr<Texture> CreateTexture(const uint8_t* pixelData, Vector2u size) override;
    std::shared_ptr<Texture> CreateEmptyTexture(Vector2u size) override;
    void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) override;
    void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData) override { UpdateTexture(texture, pixelData, texture->GetSize(), { 0, 0 }); };
    void UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) override;
    void DestroyTexture(std::shared_ptr<Texture> texture) override;
    std::shared_ptr<Font> CreateFont(const std::filesystem::path& path) override;
    void DestroyFont(std::shared_ptr<Font> font) override;
    std::shared_ptr<Shader> CreateShader(const std::filesystem::path& path, Shader::Type type) override;

    const AllocatedImage& GetImageFromID(ImageID id) { return m_Instance.GetImageFromID(id); }

    void ShowImGuiRenderTabContent() override;

    Optional<Base::Event> PollEvent() const override;

    uint32_t GetFreeBufferIndex(std::type_index viewID); 
    uint32_t GetFreeHighlightIndex(std::type_index viewID);

    void SetDeviceIndex(uint32_t index) { m_DeviceIndex = index; }
public:
    std::shared_ptr<BaseWindow> GetWindow() const;
};
