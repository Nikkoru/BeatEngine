#pragma once

#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/Vector2.h"

class Shape : public GraphicalElement {
protected:
    Vector2f m_Position{};
    LinearColor m_Color{};
public:
    Shape() = default;
    ~Shape() override = default;
public:
    void Draw(GraphicsManager& mgr) override;
public:
    void UpdateVertex();
    void SetColor(LinearColor color) { m_Color = color; }
    void SetPosition(Vector2f pos) { m_Position = pos; }

    LinearColor GetColor() const { return m_Color; }
    Vector2f GetPosition() const { return m_Position; }
public:
    virtual size_t GetPointCount() = 0;
    virtual Vector2f GetPointPosition(size_t index) = 0;
    virtual LinearColor GetPointColor() const { return m_Color; }
};
