#pragma once

#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "SDL3/SDL_video.h"
#include <string>

class SDLWindow : public BaseWindow {
private:
    SDL_Window* m_WindowImpl{ nullptr };
    SDL_WindowFlags m_Flags{};



    bool m_Fullscreen{ false };
public:
    SDLWindow() = default;
    ~SDLWindow() override = default;
public:
    void Init(GameContext* context = nullptr, std::string windowTitle = "BeatEngine Game", Vector2u windowSize = { 1280, 720 }) override;
    void Uninit() override;

    void UninitImGui() override;

    void SetSize(const Vector2u size) override;
    void SetMinimumSize(const Vector2u size) override;
    void SetMaximumSize(const Vector2u size) override;
    void SetTitle(const std::string title) override;
    void SetPosition(const Vector2i pos) override;
    void SetFullscreen(bool fullscreen) override;
    void SetVSyncMode(VSyncMode vsync) override;
    void SetCursorGrabbed(bool grabbed) override;
    void SetCursorVisible(bool visible) override;

    Vector2u GetSize() const override;
    Vector2u GetMinimumSize() const override;
    Vector2u GetMaximumSize() const override;
    std::string GetTitle() const override;
    Vector2i GetPosition() const override;
    bool IsFullscreen() const override;
    bool IsCursorGrabbed() const override;
    bool IsCursorVisible() const override;

    std::optional<Base::Event> PollEvent() override;

    void OnRender() override;
    void OnDisplay() override;
public:
    void SetFlags(SDL_WindowFlags flags);
    void AddFlags(SDL_WindowFlags flags);
    void RemoveFlags(SDL_WindowFlags flags);

    void ClearFlags();
private:
    void LogActiveFlags();
public:
    SDL_Window* GetWindowImpl();
};
