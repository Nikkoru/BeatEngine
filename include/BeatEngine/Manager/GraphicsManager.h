#pragma once

#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/GameState.h"
#include "BeatEngine/Graphics/Renderer.h"
#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>

class GraphicsManager {
private:
    std::shared_ptr<GameContext> m_Context{ nullptr };
    std::shared_ptr<GameState> m_State{ nullptr };
    std::shared_ptr<Renderer> m_Renderer{ nullptr };
    bool m_Open = true;

    std::string m_WindowTitle{};
    Vector2u m_WindowSize{};
public:
    GraphicsManager() = default;
    GraphicsManager(std::shared_ptr<GameContext> context, std::shared_ptr<GameState> state) : m_Context(context), m_State(state) {}
    ~GraphicsManager() = default;
public:
    template<typename RendererT>
        requires(std::is_base_of_v<Renderer, RendererT>)
    void MakeRenderer();

    void Init();
    void Update();
    void Close();

    void SetWindowTitle(std::string windowTitle);

    void Render();
    void Display();
    void Clear();

    std::shared_ptr<Texture> CreateTexture(std::filesystem::path path);

    std::shared_ptr<BaseWindow> GetWindow();
    std::shared_ptr<Renderer> GetRenderer();

    std::optional<Base::Event> PollEvent();
public:
    bool IsOpen();
};

#include "BeatEngine/Manager/GraphicsManager.inl"
