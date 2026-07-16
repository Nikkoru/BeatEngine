#include "BeatEngine/Graphics/RectShape.hpp"

Vector2f RectShape::GetPointPosition(size_t index) {
    switch (index) {
        default:
        case 0:
            return m_Position;
        case 1:
            return { m_Position.X, m_Position.Y + m_Size.Y };
        case 2:
            return { m_Position.X + m_Size.X, m_Position.Y };
        case 3:
            return { m_Position.X + m_Size.X , m_Position.Y + m_Size.Y };
    }
}
