#pragma once

#include "BeatEngine/Graphics/Vector2.h"
#ifdef _MSC_VER
#include <compare>
#endif

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/DrawCommand.hpp"

class PushConstants {
};

struct RenderState {
    Shader* vertShader{};
    Shader* compShader{};
    Shader* fragShader{};

    // bool highlightVertices{ false };
    LinearColor highlightColor{ 0.f, 0.f, 1.f, 1.f };

    // this is used when the primitive type is LineList or LineStrip
    float lineWidth{ 1.f };

    std::shared_ptr<PushConstants> pushConstants{};
    size_t pushConstantsSize{ sizeof(PushConstants) };

    bool enableDepthBias{ false };
    // TODO: make Vector3<T>
    std::array<float, 3> depthBias{ 0.0f, 0.0f, 0.0f };

    bool enableDepthBounds{ false };
    Vector2f depthBounds{ 0.0f, 0.0f };

    bool discardRasterization{ false };
    bool enableDepthTest{ false };
    bool enableDepthClamp{ false };
    bool enableStencilTest{ false };
    bool enableAlphaToCoverage{ false };
    bool enableBlend{ false };

    // TODO:
    // CullMode cullMode{ CullMode::None };
    // TODO:
    // FrontFace face{ FrontFace::CounterClockWise };
    // TODO:
    // PolygonMode polygonMode{ PolygonMode::Fill };
    // TODO:
    // MSAA rasterization{ MSAA::Bit1 };
    // TODO:
    // BlendFactorEqu factors{ BlendFactor::SrcAlpha, BlendFactor::OneMinusSrcAlpha, BlendOp::Add };

    static const RenderState Default;

#ifndef _MSC_VER
    bool operator<=>(const RenderState& other) const = default;
#else
    std::partial_ordering operator<=>(const RenderState& other) const = default;
#endif
};
