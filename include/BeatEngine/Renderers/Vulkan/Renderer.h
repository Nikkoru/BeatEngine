#pragma once

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Renderers/Vulkan/FrameData.h"

#include <array>
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include <vector>

class VulkanRenderer : public Renderer {
private:
    VkInstance m_Instance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
    VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
    VkDevice m_Device{ VK_NULL_HANDLE };
    VkSurfaceKHR m_Surface{ VK_NULL_HANDLE };

    VkPipeline m_GraphicsPipeline{ VK_NULL_HANDLE };
    VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };
    VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
    std::vector<VkFramebuffer> m_Framebuffers;

    VkSwapchainKHR m_Swapchain{ VK_NULL_HANDLE };
    VkFormat m_SwapchainFormat{ VK_FORMAT_B8G8R8A8_SRGB };
    
    std::vector<VkImage> m_Images;
    std::vector<VkImageView> m_ImageViews;
    VkExtent2D m_SwapchainExtent;

    VkCommandBuffer m_ActiveCmdBuffer;
    uint32_t m_ActiveImageIndex;
private:
    VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };
    VkDescriptorSetLayout m_DescriptorSetLayoutTex{ VK_NULL_HANDLE };
    VkDescriptorSet m_DescriptorSetTex{ VK_NULL_HANDLE };

    bool m_StopRendering{ false };
    bool m_UpdateSwapchain{ false };
    bool m_CreateDebugMessenger{ true };
    int m_FrameNumber{ 0 };
private:
    VmaAllocator m_Allocator{ VK_NULL_HANDLE };
private:
    FrameData m_Frames[FRAME_OVERLAP];
    inline FrameData& GetCurrentFrame() { return m_Frames[m_FrameNumber % FRAME_OVERLAP]; }
    std::vector<VkSemaphore> m_RenderSemaphores;
    VkCommandPool m_CommandPool;
    std::array<VkCommandBuffer, FRAME_OVERLAP> m_CommandBuffers;

    VkQueue m_GraphicsQueue;
    uint32_t m_GraphicsQueueFamily;
private:
    void pInitVulkan();
    void pUninitVulkan();

    void pInitSwapchain();
    void pUninitSwapchain();
    
    void pInitCommands();
    void pUninitCommands();
    
    void pInitSyncStructures();
    void pUninitSyncStructures();

    void pInitRenderPass();
    void pUninitRenderPass();

    void pInitPipeline();
    void pUninitPipeline();

    void pInitFramebuffers();
    void pUninitFramebuffers();
private:
    void CreateDebugCallback();
public:
    void Init(std::string windowTitle, Vector2u windowSize) override;
    void Uninit() override;
    void Render() override;
    void Clear() override;
    void Display() override;
    void Update() override;

    std::shared_ptr<Texture> CreateTexture(std::filesystem::path path) override;

    std::optional<Base::Event> PollEvent() const override;
public:
    std::shared_ptr<BaseWindow> GetWindow() const;
};
