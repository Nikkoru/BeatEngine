#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Renderer.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <optional>

class OpenGLRenderer : public Renderer {
private:
public:
    OpenGLRenderer();
    ~OpenGLRenderer() override = default;
public:
    void Render() override;
    void Display() override;
    void Clear() override;
    void Update() override;

    void SetGlobalShader(std::shared_ptr<Shader> shader) override;

    std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path) override;
    std::shared_ptr<Shader> CreateShader(const std::vector<std::filesystem::path>& paths) override;

    Optional<Base::Event> PollEvent() const override;
};
