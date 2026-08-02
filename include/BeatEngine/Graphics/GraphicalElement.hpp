#pragma once

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/DrawCommand.hpp"
#include "BeatEngine/Graphics/PrimitiveType.hpp"
#include "BeatEngine/Graphics/RenderState.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Graphics/VertexArray.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>

class RendererData;
class GraphicsManager;
class GraphicalElement {
private:
    friend class GraphicsManager;
private:
    // this is meant to be used by the renderer, in the case of vulkan it needs to know what
    // element is this, so it can use the correct buffers binded to the id
    uint32_t m_ElementID;
    // also we need to know if it is already initialized so we can know if the requested id
    // needs to be freed or not, this overrides whatever it was in that id without questions
    bool m_Initialized{ false };
protected:
    Vector2f m_Padding{};
    VertexArray m_Vertices{};
    PrimitiveType m_PrimitiveType{};
    Base::AssetHandle<Texture> m_Texture{};
public:
    GraphicalElement() = default;
    virtual ~GraphicalElement() = default;

    virtual void Draw(GraphicsManager& mgr, RenderState state = RenderState::Default) { BaseDraw(mgr, state); };
    void BaseDraw(GraphicsManager& mgr, RenderState state);
    virtual void UninitGraphics(GraphicsManager& mgr);

    void SetTexture(Base::AssetHandle<Texture> texture);

    virtual void DrawImGuiDrawData() { BaseDrawImGuiDrawData(); };
    void DrawWindowImGuiDrawData();
    void BaseDrawImGuiDrawData();
protected:
    virtual void SetCommand(std::shared_ptr<DrawCommand> cmd);
    virtual void CastCommands() {};
};
