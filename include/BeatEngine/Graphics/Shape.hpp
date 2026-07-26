#pragma once

#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/PrimitiveType.hpp"
#include "BeatEngine/Graphics/Vector2.h"

class Shape : public GraphicalElement {
protected:
    Vector2f m_Position{};
    LinearColor m_Color{};
public:
    Shape() = default;
    ~Shape() override = default;
public:
    void Draw(GraphicsManager& mgr, RenderState state = RenderState::Default) override;
public:
    void UpdateVertex();
    void SetColor(LinearColor color) { m_Color = color; UpdateVertex(); }
    void SetPosition(Vector2f pos) { m_Position = pos; UpdateVertex(); }

    LinearColor GetColor() const { return m_Color; }
    Vector2f GetPosition() const { return m_Position; }
public:
    virtual size_t GetPointCount() = 0;
    virtual PrimitiveType GetPrimitiveType() { return PrimitiveType::TriangleList; }
    virtual Vector2f GetPointPosition(size_t index) = 0;
    virtual Vector2f GetUV(size_t index) = 0;
    virtual LinearColor GetPointColor(size_t index) const { (void)index; return m_Color; }
};
