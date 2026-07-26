#pragma once

#include <vulkan/vulkan_core.h>
struct AllocatedImage;
class BindlessSetManager {
private:
    VkDescriptorPool m_DescPool{};
    VkDescriptorSetLayout m_DescLayout{};
    VkDescriptorSet m_DescSet{};

    VkSampler m_NearestSampler{};
    VkSampler m_LinearSampler{};
    VkSampler m_ShadowMapSampler{};
public:
    void Init(VkDevice device, float MaxAnisotropy);
    void Uninit(VkDevice device);

    VkDescriptorSetLayout GetSetLayout() const { return m_DescLayout; }
    const VkDescriptorSet& GetSet() const { return m_DescSet; }

    VkSampler& GetLinearSampler() { return m_NearestSampler; }
    VkSampler& GetNearestSampler() { return m_LinearSampler; }
    VkSampler& GetShadowMapSampler() { return m_ShadowMapSampler; }

    void AddImage(VkDevice device, uint32_t id, const VkImageView imageView);
    void AddSampler(VkDevice device, uint32_t id, VkSampler sampler);
private:
    void InitDefaultSamplers(VkDevice device, float maxAnisotropy);
};
