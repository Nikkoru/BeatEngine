#pragma once

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/PrimitiveType.hpp"
#include "BeatEngine/Graphics/VertexArray.hpp"
#include <filesystem>
#include <memory>

class GraphicsManager;
class RendererData {
private:
    friend class Renderer;
protected:
    bool m_Initialized{ false };
    std::shared_ptr<DrawCommand> m_Command;
    std::shared_ptr<Shader> m_VertShader{ nullptr };
    std::shared_ptr<Shader> m_FragShader{ nullptr };
    std::shared_ptr<Texture> m_Texture{ nullptr };
public:
    VertexArray Vertices;
    PrimitiveType Type;
    size_t DrawCommandSize{ sizeof(DrawCommand) };
public:
    RendererData() = default;
    virtual ~RendererData() = default;

    bool IsInitialized() { return m_Initialized; }

    virtual void DrawImGuiDrawData() {};

    void SetVertices(VertexArray& vertices) { Vertices = vertices; }

    virtual void SetShader(std::filesystem::path& path, Shader::Type type) = 0;
    virtual void SetShader(std::shared_ptr<Shader> shader) = 0;
    void SetTexture(std::shared_ptr<Texture> texture) { m_Texture = texture; };
    virtual void PrepareDrawing() {};
    virtual void SetCommand(std::shared_ptr<DrawCommand> cmd) = 0;
};
