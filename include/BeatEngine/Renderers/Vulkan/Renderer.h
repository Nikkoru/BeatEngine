#pragma once

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/AllocatedImage.h"
#include "BeatEngine/Renderers/Vulkan/DescriptorAllocator.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"
#include "BeatEngine/Renderers/Vulkan/PipelineManager.h"
#include "BeatEngine/Renderers/Vulkan/UninitQueue.h"

#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

class VulkanRenderer : public Renderer {
public: 
    VulkanRenderer() : VulkanRenderer(nullptr) {}
    VulkanRenderer(GameContext* context) : Renderer(context) {}
    ~VulkanRenderer() override = default;
private:
    UninitQueue m_Uninitializers;

    VkInstance m_Instance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
    VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
    VkPhysicalDeviceProperties m_DeviceProperties{};
    VkDevice m_Device{ VK_NULL_HANDLE };
    VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };
    VkQueue m_GraphicsQueue{ VK_NULL_HANDLE };
    uint32_t m_GraphicsQueueFamily{};

    PipelineManager m_PipelineMgr;
    VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
    std::vector<VkFramebuffer> m_Framebuffers;

    VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
    VkFormat m_SwapchainFormat{ VK_FORMAT_B8G8R8A8_UNORM };
    
    AllocatedImage m_DrawImage;
    FrameData m_Frames[FRAME_OVERLAP];

    uint32_t m_ActiveImageIndex;

    std::vector<VkImage> m_SwapchainImages;
	std::vector<VkImageView> m_SwapchainImageViews;
    DescriptorAllocator m_GlobalDescriptorAllocator;
    VkDescriptorSet m_ImageDescriptor{ VK_NULL_HANDLE };
    VkDescriptorSetLayout m_ImageDescriptorLayout{ VK_NULL_HANDLE };
private:
    bool m_StopRendering{ false };
    bool m_UpdateSwapchain{ false };
    bool m_CreateDebugMessenger{ true };
    int m_FrameNumber{};
    VmaAllocator m_Allocator{ VK_NULL_HANDLE };
private:
    inline FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; }
private:
    void pInitVulkan();
    void pInitSwapchain();
    void pInitCommands();
    void pInitSyncStructures();
    void pInitRenderPass();
    void pInitPipeline();
    void pInitDescriptors();
    void pInitFramebuffers();
    void pInitImGui();
private:
    void CreateDebugCallback();
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

    void UpdateSwapchain();

    std::shared_ptr<Texture> CreateTexture(std::filesystem::path path) override;
    std::shared_ptr<Shader> CreateShader(std::filesystem::path path, Shader::Type type) override;

    void ShowImGuiRenderTabContent() override;

    std::optional<Base::Event> PollEvent() const override;
public:
    std::shared_ptr<BaseWindow> GetWindow() const;
};
