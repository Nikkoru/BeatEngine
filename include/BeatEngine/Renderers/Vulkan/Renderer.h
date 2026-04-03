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

#include <array>
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>

class VulkanRenderer : public Renderer {
private:
    UninitQueue m_Uninitializers;

    VkInstance m_Instance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
    VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
    VkDevice m_Device{ VK_NULL_HANDLE };
    VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

    // VkPipeline m_GraphicsPipeline{ VK_NULL_HANDLE };
    // VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
    PipelineManager m_PipelineMgr;
    VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

    VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
    VkFormat m_SwapchainFormat{ VK_FORMAT_R16G16B16A16_SFLOAT };
    
    AllocatedImage m_DrawImage;
    VkExtent2D m_SwapchainEx;

    VkCommandBuffer m_ActiveCmdBuffer;
    uint32_t m_ActiveImageIndex;
private:
    DescriptorAllocator m_GlobalDescriptorAllocator;
    VkDescriptorSet m_ImageDescriptor{ VK_NULL_HANDLE };
    VkDescriptorSetLayout m_ImageDescriptorLayout{ VK_NULL_HANDLE };

    bool m_StopRendering{ false };
    bool m_UpdateSwapchain{ false };
    bool m_CreateDebugMessenger{ true };
    int m_FrameNumber{ 0 };
private:
    VmaAllocator m_Allocator{ VK_NULL_HANDLE };
private:
    FrameData m_Frames[FRAME_OVERLAP];
    inline FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; }
    VkCommandPool m_CommandPool;
    std::array<VkCommandBuffer, FRAME_OVERLAP> m_CommandBuffers;

    VkQueue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamily;
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
    void Init(std::string windowTitle, Vector2u windowSize, bool imgui = false) override;
    void Uninit() override;
    void Render() override;
    void RenderImGui() override;
    void Clear() override;
    void Display() override;
    void Update() override;

    std::shared_ptr<Texture> CreateTexture(std::filesystem::path path) override;
    std::shared_ptr<Shader> CreateShader(std::filesystem::path path, Shader::Type type) override;

    std::optional<Base::Event> PollEvent() const override;
public:
    std::shared_ptr<BaseWindow> GetWindow() const;
};
