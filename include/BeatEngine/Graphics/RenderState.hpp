#pragma once

#ifdef _MSC_VER
#include <compare>
#endif

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Graphics/Color.h"
#include "BeatEngine/Graphics/DrawCommand.hpp"

class PushConstants {
};

struct RenderState {
    // TODO: give the user more control over how its going to draw things
    const std::shared_ptr<Shader> VertShader{};
    const std::shared_ptr<Shader> CompShader{};
    const std::shared_ptr<Shader> FragShader{};

    std::shared_ptr<DrawCommand> _DrawCommand{};
    size_t DrawCommandSize{};

    bool HighlightVertices{ false };
    LinearColor HighlightColor{ 0.f, 0.f, 1.f, 1.f };

    // this is used when the primitive type is LineList or LineStrip
    float LineWidth{ 1.f };

    std::shared_ptr<PushConstants> _PushConstants{};
    size_t PushConstantsSize{ sizeof(PushConstants) };

    bool DrawInGlobal{ false };

    static const RenderState Default;

#ifndef _MSC_VER
    bool operator<=>(const RenderState& other) const = default;
#else
    std::partial_ordering operator<=>(const RenderState& other) const = default;
#endif
};
