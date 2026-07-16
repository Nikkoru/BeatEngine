#include "BeatEngine/Renderers/Vulkan/ImmediateExecutor.h"
#include "BeatEngine/Renderers/Vulkan/Boilerplate.h"

#include "backends/imgui_impl_vulkan.h"
#include <functional>
#include <vulkan/vulkan_core.h>


void VK::ImmediateExecutor::Init(VkDevice device, uint32_t graphicsQueueFamily, VkQueue graphicsQueue) {
    assert(!m_Initialized);

    this->m_Device = device;
    this->m_GraphicsQueueFamily = graphicsQueueFamily;
    this->m_Queue = graphicsQueue;

    const auto poolCreateInfo = vki::GetCommandPoolCreateInfo(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, graphicsQueueFamily);
    VK_CHECK(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &m_ImmCommandPool));

    const auto cmdAllocInfo = vki::GetCommandBufferAllocateInfo(m_ImmCommandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &m_ImmCommandBuffer));

    const auto fenceCreateInfo = VkFenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &m_ImmFence));

    m_Initialized = true;
}

void VK::ImmediateExecutor::Uninit() {
    assert(m_Initialized);
    vkDestroyCommandPool(m_Device, m_ImmCommandPool, nullptr);
    vkDestroyFence(m_Device, m_ImmFence, nullptr);
}

void VK::ImmediateExecutor::ImmediateSumbit(std::function<void(VkCommandBuffer)>&& func) const {
    assert(m_Initialized);
    VK_CHECK(vkResetFences(m_Device, 1, &m_ImmFence));
    VK_CHECK(vkResetCommandBuffer(m_ImmCommandBuffer, 0));

    auto cmd = m_ImmCommandBuffer;
    const auto cmdBeginInfo = VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo{}
    };

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
    func(cmd);
    VK_CHECK(vkEndCommandBuffer(cmd));

    const auto cmdInfo = vki::GetCommandBufferSubmitInfo(cmd);
    const auto submit = vki::GetSubmitInfo(&cmdInfo, nullptr, nullptr);

    VK_CHECK(vkQueueSubmit2(m_Queue, 1, &submit, m_ImmFence));

    VK_CHECK(vkWaitForFences(m_Device, 1, &m_ImmFence, true, NO_TIMEOUT));
}
