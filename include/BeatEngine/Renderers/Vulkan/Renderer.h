#pragma once

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/Assets/Texture.h"
#include "BeatEngine/Renderers/Vulkan/DescriptorAllocator.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/PipelineManager.h"
#include "BeatEngine/Renderers/Vulkan/UninitQueue.h"
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
    // UninitQueue m_Uninitializers;

    // Vulkan::Core
    VK::Instance m_Instance;
    AllocatedImage m_AllocatedDrawImage{};
    VkCommandBuffer m_ActiveCmd{};

    // Vulkan::Pipeline
    PipelineManager m_PipelineMgr;
    VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
    std::vector<VkFramebuffer> m_Framebuffers;

    DescriptorAllocator m_GlobalDescriptorAllocator;
    VkDescriptorSet m_ImageDescriptor{ VK_NULL_HANDLE };
    VkDescriptorSetLayout m_ImageDescriptorLayout{ VK_NULL_HANDLE };
private:
    bool m_StopRendering{ false };
private:
    void pInitRenderPass();
    void pInitPipeline();
    void pInitDescriptors();
    void pInitFramebuffers();
    void pInitImGui();
private:
    VulkanTexture& GetErrorTexture() { return m_Instance.GetErrorTexture(); }
    VulkanTexture& GetWhiteTexture() { return m_Instance.GetWhiteTexture(); }
public:
    void Init(std::string windowTitle, Vector2u windowSize) override;
    void Uninit() override;
public:
    void Render() override;
    void RenderImGui() override;
    void Clear() override;
    void Display() override;
    void Update() override;
    void SetGlobalShader(std::shared_ptr<Shader> shader) override;
    void ProcessEvent(Optional<Base::Event> event) override;

    void DrawElement(GraphicalElement& element) override;
    void InitElement(GraphicalElement& element) override;
    void UninitElement(GraphicalElement& element) override;

    std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path) override;
    std::shared_ptr<Font> CreateFont(const std::filesystem::path& path) override;
    std::shared_ptr<Shader> CreateShader(const std::filesystem::path& path, Shader::Type type) override;

    void ShowImGuiRenderTabContent() override;

    Optional<Base::Event> PollEvent() const override;
public:

    std::shared_ptr<BaseWindow> GetWindow() const;
};
