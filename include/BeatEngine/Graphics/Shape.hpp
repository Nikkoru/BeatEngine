#pragma once

#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/PrimitiveType.hpp"
#include "BeatEngine/Graphics/Transformable.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Graphics/VertexArray.hpp"

class Renderer;
class Texture;
class Shape : public GraphicalElement, public Transformable {
protected:
    Base::AssetHandle<Texture> m_Texture{};
    LinearColor m_Color{};
    VertexArray m_Vertices{};
public:
    Shape() = default;
    ~Shape() override = default;
public:
    void UpdateVertex();
    void SetColor(LinearColor color) { m_Color = color; UpdateVertex(); }

    LinearColor GetColor() const { return m_Color; }

    virtual void Draw(Renderer* const mgr, RenderState state = RenderState::Default) override;
public:
    void SetTexture(Base::AssetHandle<Texture>& texture) { m_Texture = texture; }

    virtual size_t GetPointCount() = 0;
    virtual PrimitiveType GetPrimitiveType() { return PrimitiveType::TriangleList; }
    virtual Vector2f GetPointPosition(size_t index) = 0;
    virtual Vector2f GetUV(size_t index) = 0;
    virtual LinearColor GetPointColor(size_t index) const { (void)index; return m_Color; }
};
