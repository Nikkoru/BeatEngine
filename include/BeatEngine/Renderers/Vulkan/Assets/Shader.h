#pragma once

#include "BeatEngine/Asset/Shader.h"

class VulkanShader : public Shader {
public:
    ~VulkanShader() override = default;
public:
    bool GetFileContents(const std::filesystem::path path, std::vector<char>& buffer) override;
    bool Compile(std::string_view vertexData, std::string_view fragmentData) override;
};

class VulkanShaderData : public ShaderData {
private:
    std::vector<uint32_t> m_Buffer;
    size_t m_Size{};
};
