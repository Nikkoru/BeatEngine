#pragma once
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <filesystem>
#include <memory>

class GameContext;
class Renderer {
private:
    friend class GraphicsManager;
protected:
    std::shared_ptr<BaseWindow> m_Window{ nullptr };
    GameContext* m_Context;
    bool m_Profile{ false };
public:
    Renderer() : Renderer(nullptr) {};
    Renderer(GameContext* context) : m_Context(context) {}
    virtual ~Renderer() = default;
public:
    virtual void Init(std::string windowTitle, Vector2u windowSize) = 0;
    virtual void Uninit() = 0;
    virtual void Render() = 0;
    virtual void RenderImGui() {};
    virtual void Display() = 0;
    virtual void Clear() = 0;
    virtual void Update() = 0;

    virtual void SetGlobalShader(std::shared_ptr<Shader> shader) = 0;

    virtual std::shared_ptr<Texture> CreateTexture(std::filesystem::path path) = 0;
    virtual std::shared_ptr<Shader> CreateShader(std::filesystem::path path, Shader::Type type) = 0;

    virtual void ShowImGuiRenderTabContent() {}

    inline std::shared_ptr<BaseWindow> GetWindow() { return m_Window; }
    inline void SetWindow(std::shared_ptr<BaseWindow> window) { m_Window = window; }

    virtual std::optional<Base::Event> PollEvent() const = 0;
};
