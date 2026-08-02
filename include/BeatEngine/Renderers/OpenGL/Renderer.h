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
    void Init(std::string windowTitle, Vector2u windowSize, VSyncMode vSync) override;
    void Uninit() override;
    void Render() override;
    void RenderImGui() override;
    void Display() override;
    void Clear() override;
    void Update() override;

    void SetGlobalShader(std::shared_ptr<Shader> shader) override;

    std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path) override;
    std::shared_ptr<Shader> CreateShader(const std::filesystem::path& path, Shader::Type type) override;

    Optional<Base::Event> PollEvent() const override;
};
