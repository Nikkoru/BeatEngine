#pragma once

#include <map>
#include <string>
enum class PrimitiveType {
    None = 0,
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan
};

class PrimitiveTypeUtils {
public:
    static std::string ToString(const PrimitiveType type) { return m_TypeString.at(type); }
private:
    static inline const std::map<PrimitiveType, std::string> m_TypeString = {
        { PrimitiveType::None, "None" },
        { PrimitiveType::PointList, "PointList" },
        { PrimitiveType::LineList, "LineList" },
        { PrimitiveType::LineStrip, "LineStrip" },
        { PrimitiveType::TriangleList, "TriangleList" },
        { PrimitiveType::TriangleStrip, "TriangleStrip" },
        { PrimitiveType::TriangleFan, "TriangleFan" },
    };
};
