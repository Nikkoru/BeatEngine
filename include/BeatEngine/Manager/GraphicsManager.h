#pragma once

#include "BeatEngine/GameContext.h"
#include "BeatEngine/Graphics/Renderer.h"
#include <string>
#include <type_traits>

class GraphicsManager {
private:
    GameContext* m_Context;
    std::shared_ptr<Renderer> m_Renderer{ nullptr };
    bool m_Open = true;

    std::string m_WindowTitle{};
    Vector2u m_WindowSize{};
public:
    GraphicsManager() = default;
    GraphicsManager(GameContext* context) : m_Context(context) {}
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

    std::shared_ptr<BaseWindow> GetWindow();
    std::shared_ptr<Renderer> GetRenderer();

    std::optional<Base::Event> PollEvent();
public:
    bool IsOpen();
};

#include "BeatEngine/Manager/GraphicsManager.inl"
