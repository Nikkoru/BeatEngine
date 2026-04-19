#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include <optional>
#include <string>

class GameContext;
class BaseWindow {
protected:
    GameContext* m_Context{ nullptr };
public:
    BaseWindow() = default;
    virtual ~BaseWindow() = default;
public:

    virtual void Init(GameContext* context = nullptr, std::string windowTitle = "BeatEngine Game", Vector2u windowSize = { 1280, 720 }) = 0;
    virtual void Uninit() = 0;

    virtual void UninitImGui() = 0;
    // virtual void UpdateWindow();

    virtual std::optional<Base::Event> PollEvent() = 0;

    virtual void SetSize(const Vector2u size) {};
    virtual void SetMinimumSize(const Vector2u size) {};
    virtual void SetMaximumSize(const Vector2u size) {};
    virtual void SetTitle(const std::string title) {};
    virtual void SetPosition(const Vector2i pos) {};
    virtual void SetFullscreen(bool fullscreen) {};
    virtual void SetVSyncMode(VSyncMode vsync) {};
    virtual void SetCursorGrabbed(bool grabbed) {};
    virtual void SetCursorVisible(bool visible) {};

    virtual Vector2u GetSize() const { return {}; }
    virtual Vector2u GetMinimumSize() const { return {}; }
    virtual Vector2u GetMaximumSize() const { return {}; }
    virtual std::string GetTitle() const { return {}; }
    virtual Vector2i GetPosition() const { return {}; }
    virtual bool IsFullscreen() const { return {}; }
    virtual bool IsCursorGrabbed() const { return {}; }
    virtual bool IsCursorVisible() const { return {}; }

    virtual void OnRender() {};
    virtual void OnDisplay() {};
};
