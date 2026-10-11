#include "BeatEngine/Graphics/RectShape.hpp"
#include "BeatEngine/Graphics/Vector2.h"

Vector2f RectShape::GetPointPosition(size_t index) {
    switch (index) {
        default:
        case 0:
            return { 0, 0 };
        case 1:
            return { 0, m_Size.Y };
        case 2:
            return { m_Size.X, 0 };
        case 3:
            return { m_Size.X , m_Size.Y };
    }
}

Vector2f RectShape::GetUV(size_t index) {
    switch (index) {
        default:
        case 0:
            return { 0.f, 0.f };
        case 1:
            return { 0.f, 1.f };
        case 2:
            return { 1.f, 0.f };
        case 3:
            return { 1.f, 1.f };
    }
}
void RectShape::SetSize(Vector2f size) {
    if (m_Size == size) return;

    m_Size = size; 
    UpdateVertex(); 
}
