#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <optional>

class OpenGLRenderer : public Renderer {
private:
public:
    OpenGLRenderer() = default;
    ~OpenGLRenderer() override = default;
public:
    void Init(std::string windowTitle, Vector2u windowSize, bool imgui = false) override;
    void Uninit() override;
    void Render() override;
    void Display() override;
    void Clear() override;
    void Update() override;

    std::optional<Base::Event> PollEvent() const override;
};
