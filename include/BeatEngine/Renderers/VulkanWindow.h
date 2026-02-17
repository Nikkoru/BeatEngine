#pragma once

#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "SDL3/SDL_video.h"
#include <string>
class VulkanWindow : public BaseWindow {
private:
    SDL_Window* m_WindowImpl{ nullptr };
    bool m_Fullscreen{ false };
public:
    VulkanWindow() = default;
    ~VulkanWindow() override = default;
public:
    void Init(std::string windowTitle = "", Vector2u windowSize = { 0, 0 }) override;
    void Uninit() override;

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
public:
    SDL_Window* GetWindowImpl();
};
