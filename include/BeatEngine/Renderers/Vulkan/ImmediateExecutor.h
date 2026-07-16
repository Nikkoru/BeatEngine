#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>

namespace VK {
class ImmediateExecutor {
private:
    bool m_Initialized{ false };

    VkDevice m_Device{ nullptr };
    uint32_t m_GraphicsQueueFamily{};
    VkQueue m_Queue{ nullptr };

    VkCommandBuffer m_ImmCommandBuffer;
    VkCommandPool m_ImmCommandPool;
    VkFence m_ImmFence;
public:
    void Init(VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue);
    void Uninit();

    void ImmediateSumbit(std::function<void(VkCommandBuffer)>&& func) const;
};
}
