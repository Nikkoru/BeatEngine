#pragma once
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <filesystem>
#include <memory>

class GameContext;
class Renderer {
protected:
    std::shared_ptr<BaseWindow> m_Window;
    GameContext* m_Context;
public:
    Renderer() : Renderer(nullptr) {};
    Renderer(GameContext* context) : m_Context(context) {}
    virtual ~Renderer() = default;
public:
    virtual void Init(std::string windowTitle, Vector2u windowSize) = 0;
    virtual void Uninit() = 0;
    virtual void Render() = 0;
    virtual void RenderImGui() = 0;
    virtual void Display() = 0;
    virtual void Clear() = 0;
    virtual void Update() = 0;
    virtual void SetGlobalShader(std::shared_ptr<Shader> shader) = 0;

    virtual std::shared_ptr<Texture> CreateTexture(std::filesystem::path path) = 0;
    virtual std::shared_ptr<Shader> CreateShader(std::filesystem::path path, Shader::Type type) = 0;

    virtual void ShowImGuiRenderTabContent() {}

    inline std::shared_ptr<BaseWindow> GetWindow() { return m_Window; }

    virtual std::optional<Base::Event> PollEvent() const = 0;
};
