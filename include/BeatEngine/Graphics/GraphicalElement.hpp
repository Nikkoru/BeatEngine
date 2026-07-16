#pragma once

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Graphics/VertexArray.hpp"
#include "imgui.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>

class RendererData;
class GraphicsManager;
class GraphicalElement {
protected:
    Vector2f m_Padding{};
    VertexArray m_Vertices{};
    const std::shared_ptr<Texture> m_Texture;
public:
    std::shared_ptr<RendererData> Data;
public:
    virtual ~GraphicalElement() = default;

    virtual void Draw(GraphicsManager& mgr) { BaseDraw(mgr); };
    void BaseDraw(GraphicsManager& mgr);
    virtual void UninitGraphics(GraphicsManager& mgr);

    virtual void DrawImGuiDrawData() { BaseDrawImGuiDrawData(); };
    void DrawWindowImGuiDrawData();
    void BaseDrawImGuiDrawData();
protected:
    virtual void SetCommand(std::shared_ptr<DrawCommand> cmd);
    virtual void CastCommands() {};
};
