#pragma once

#include "BeatEngine/Graphics/Shape.hpp"
class RectShape : public Shape {
private:
    Vector2f m_Size{};
public:
    RectShape() { UpdateVertex(); }
    size_t GetPointCount() override { return 4; }
    Vector2f GetPointPosition(size_t index) override;
public:
    void SetSize(Vector2f size) { m_Size = size; UpdateVertex(); } 

    Vector2f GetSize() { return m_Size; }
};
