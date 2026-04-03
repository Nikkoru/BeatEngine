#pragma once
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <filesystem>
#include <memory>
class Renderer {
protected:
    std::shared_ptr<BaseWindow> m_Window;
public:
    Renderer() = default;
    virtual ~Renderer() = default;
public:
    virtual void Init(std::string windowTitle, Vector2u windowSize, bool imgui = false) = 0;
    virtual void Uninit() = 0;
    virtual void Render() = 0;
    virtual void RenderImGui() = 0;
    virtual void Display() = 0;
    virtual void Clear() = 0;
    virtual void Update() = 0;

    virtual std::shared_ptr<Texture> CreateTexture(std::filesystem::path path) = 0;
    virtual std::shared_ptr<Shader> CreateShader(std::filesystem::path path, Shader::Type type) = 0;

    inline std::shared_ptr<BaseWindow> GetWindow() { return m_Window; }

    virtual std::optional<Base::Event> PollEvent() const = 0;
};
