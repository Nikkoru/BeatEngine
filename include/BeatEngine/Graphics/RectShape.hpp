#pragma once

#include "BeatEngine/Graphics/PrimitiveType.hpp"
#include "BeatEngine/Graphics/Shape.hpp"
class RectShape : public Shape {
private:
    Vector2f m_Size{};
public:
    RectShape() { UpdateVertex(); }
    size_t GetPointCount() override { return 4; }
    Vector2f GetPointPosition(size_t index) override;
    Vector2f GetUV(size_t index) override;
    PrimitiveType GetPrimitiveType() override { return PrimitiveType::TriangleStrip; };
public:
    void SetSize(Vector2f size); 

    Vector2f GetSize() { return m_Size; }
};
