#pragma once

#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>

class GameContext;
class GameState;
class BaseWindow;
class Renderer;
class GraphicsManager {
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
    std::shared_ptr<Renderer> m_Renderer{ nullptr };
    bool m_Open = true;

    std::string m_WindowTitle{};
    Vector2u m_WindowSize{};
public:
    GraphicsManager() = default;
    GraphicsManager(GameContext* context, GameState* state)
        : m_Context(context), m_State(state) {}
    ~GraphicsManager() = default;
public:
    template<typename RendererT>
        requires(std::is_base_of_v<Renderer, RendererT>)
    void MakeRenderer();
    void MakeRenderer(std::shared_ptr<Renderer> renderer);

    void Init();
    void Update();
    void Close();

    void SetWindowTitle(std::string windowTitle);
    void SetWindowSize(Vector2u size);
    void SetFramerateLimit(unsigned int fps);

    void Render();
    void RenderImGui();
    void Display();
    void Clear();

    std::shared_ptr<Texture> CreateTexture(std::filesystem::path path);
    std::shared_ptr<Shader> CreateShader(std::filesystem::path path, Shader::Type type);

    std::shared_ptr<BaseWindow> GetWindow();
    std::shared_ptr<Renderer> GetRenderer();

    std::optional<Base::Event> PollEvent();
public:
    bool IsOpen();
};

#include "BeatEngine/Manager/GraphicsManager.inl"
