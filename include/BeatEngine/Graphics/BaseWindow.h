#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Util/Optional.hpp"
#include "BeatEngine/Windows/Cursor.hpp"
#include <string>

enum class WindowDriver {
    None = 0,
    X11,
    Wayland,
    Cocoa,
    Windows
};

class AppContext;
class BaseWindow {
protected:
    AppContext* m_Context{ nullptr };
    std::string m_RendererName{};
    WindowDriver m_WindowDriver{ WindowDriver::None };
    bool m_Open{ true };
public:
    BaseWindow() = default;
    virtual ~BaseWindow() = default;
protected:
    void SetCursorImpl(Cursor& cursor, std::shared_ptr<CursorImpl> impl) const { cursor.m_Impl = impl; }
    std::shared_ptr<CursorImpl> GetCursorImpl(Cursor& cursor) const { return cursor.m_Impl; }
public:
    void PrepareInitFor(std::string renderer) { m_RendererName = renderer; }
    virtual void Init(AppContext* context = nullptr) = 0;

    virtual void Uninit() = 0;

    virtual void UninitImGui() = 0;
    virtual void InitImGui() = 0;

    virtual void Close() = 0;

    virtual Optional<Base::Event> PollEvent() = 0;

    virtual void SetSize(const Vector2u size) { (void)size; };
    virtual void SetMinimumSize(const Vector2u size) { (void)size; };
    virtual void SetMaximumSize(const Vector2u size) { (void)size; };
    virtual void SetTitle(const std::string title) { (void)title; };
    virtual void SetPosition(const Vector2i pos) { (void)pos; };
    virtual void SetFullscreen(bool fullscreen) { (void)fullscreen; };
    virtual void SetVSyncMode(VSyncMode vsync) { (void)vsync; };
    virtual void SetCursorGrabbed(bool grabbed) { (void)grabbed; };
    virtual void SetCursorVisible(bool visible) { (void)visible; };
    virtual void SetCursor(Cursor& cursor) { (void)cursor; };

    virtual Vector2u GetSize() const { return {}; }
    virtual Vector2u GetMinimumSize() const { return {}; }
    virtual Vector2u GetMaximumSize() const { return {}; }
    virtual std::string GetTitle() const { return {}; }
    virtual Vector2i GetPosition() const { return {}; }
    virtual Vector2f GetMousePosition() const { return {}; }
    virtual bool IsFullscreen() const { return {}; }
    virtual bool IsCursorGrabbed() const { return {}; }
    virtual bool IsCursorVisible() const { return {}; }

    bool IsOpen() const { return m_Open; }
    virtual Cursor CreateCursor() const { return {}; }

    WindowDriver GetWindowDriver() const { return m_WindowDriver; }

    virtual void OnRender() {};
    virtual void OnDisplay() {};

    virtual void ImGuiWindowContent() {};
};
