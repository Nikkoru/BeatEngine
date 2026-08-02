#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Events/MouseEvents.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Logger.h"
#include "BeatEngine/Util/Profiler.h"
#include "BeatEngine/Windows/Mouse.hpp"
#include "SDL3/SDL_keycode.h"
#include "imgui.h"

#include <SDL3/SDL_keyboard.h>
#include <backends/imgui_impl_sdl3.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <optional>
#include <vulkan/vulkan_core.h>

#include <memory>

template<typename ...Args>
void AddSDLLog(std::string_view fmt, Args&&... elms) {
#ifndef _WIN32
    Logger::AddLog("\e[30;46mSDL\033[0m", "", fmt, elms...);
#else
    Logger::AddLog("\x1b[30;46mSDL\033[0m", "", fmt, elms...);
#endif
}

void SDLWindow::Init(GameContext* context, std::string windowTitle, Vector2u windowSize) {
    if (m_InitFlags == 0)
        m_InitFlags = SDL_INIT_VIDEO | SDL_INIT_GAMEPAD;
    m_Context = context;

    AddSDLLog("Initializing SDL Window");

   SDL_Init(m_InitFlags);

    if (windowTitle == "")
        windowTitle = m_Context->ProgramName;

    if (windowSize == Vector2u{ static_cast<unsigned int>(-1), static_cast<unsigned int>(-1) })
        m_WindowFlags |= SDL_WINDOW_FULLSCREEN; 

    PrepareForRenderer();

    m_WindowImpl = SDL_CreateWindow(windowTitle.c_str(), windowSize.X, windowSize.Y, m_WindowFlags);
    if (windowSize == Vector2u{}) {
        windowSize = GetSize();
    }
    
    auto driverName = SDL_GetCurrentVideoDriver();

    if (driverName) {
        if (strcmp(driverName, "x11") == 0) m_WindowDriver = WindowDriver::X11;
        else if (strcmp(driverName, "wayland") == 0) m_WindowDriver = WindowDriver::Wayland;
        else if (strcmp(driverName, "cocoa") == 0) m_WindowDriver = WindowDriver::Cocoa;
        else if (strcmp(driverName, "windows") == 0) m_WindowDriver = WindowDriver::Windows;
    }
    else {
        m_WindowDriver = WindowDriver::None;
        driverName = "No video driver";
    }


    AddSDLLog("Window created. Size = ({}, {}), Driver = {}, Title = {}", windowSize.X, windowSize.Y, driverName, windowTitle);
    LogActiveFlags();
}

void SDLWindow::Uninit() {
    AddSDLLog("Destroying SDL window");

    SDL_DestroyWindow(m_WindowImpl);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}

void SDLWindow::InitImGui() {
    if (m_RendererName == "Vulkan") {
        ImGui_ImplSDL3_InitForVulkan(m_WindowImpl);
    }
}

void SDLWindow::UninitImGui() {
    ImGui_ImplSDL3_Shutdown();
}

void SDLWindow::SetSize(const Vector2u size) {
    SDL_SetWindowSize(m_WindowImpl, size.X, size.Y);
}

void SDLWindow::SetMinimumSize(const Vector2u size) {
    SDL_SetWindowMinimumSize(m_WindowImpl, size.X, size.Y);
}

void SDLWindow::SetMaximumSize(const Vector2u size) {
    SDL_SetWindowMaximumSize(m_WindowImpl, size.X, size.Y);
}

void SDLWindow::SetTitle(const std::string title) {
    SDL_SetWindowTitle(m_WindowImpl, title.c_str());
}

void SDLWindow::SetPosition(const Vector2i position) {
    SDL_SetWindowPosition(m_WindowImpl, position.X, position.Y);
}

void SDLWindow::SetFullscreen(bool fullscreen) {
    SDL_SetWindowFullscreen(m_WindowImpl, fullscreen);
    m_Fullscreen = fullscreen;
}

void SDLWindow::SetVSyncMode(VSyncMode vsync) {
    switch (vsync) {
    case VSyncMode::SyncWithFirstRefresh:
        SDL_SetWindowSurfaceVSync(m_WindowImpl, 1);
        break;
    case VSyncMode::SyncWithSecondRefresh:
        SDL_SetWindowSurfaceVSync(m_WindowImpl, 2);
        break;
    case VSyncMode::Adaptative:
        SDL_SetWindowSurfaceVSync(m_WindowImpl, SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE);
        break;
    case VSyncMode::Disable:
    default:
        SDL_SetWindowSurfaceVSync(m_WindowImpl, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
        break;
    }
}

void SDLWindow::SetCursorGrabbed(bool grabbed) {
    SDL_SetWindowMouseGrab(m_WindowImpl, grabbed);
}

void SDLWindow::SetCursorVisible(bool visible) {
    if (visible)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
}

Vector2u SDLWindow::GetSize() const {
    int x{}, y{};
    SDL_GetWindowSize(m_WindowImpl, &x, &y);
    
    return { static_cast<unsigned int>(x), static_cast<unsigned int>(y) };
}

Vector2u SDLWindow::GetMinimumSize() const {
    int x{}, y{};
    SDL_GetWindowMinimumSize(m_WindowImpl, &x, &y);
    
    return { static_cast<unsigned int>(x), static_cast<unsigned int>(y) };
}

Vector2u SDLWindow::GetMaximumSize() const {
    int x{}, y{};
    SDL_GetWindowMaximumSize(m_WindowImpl, &x, &y);
    
    return { static_cast<unsigned int>(x), static_cast<unsigned int>(y) };
}

std::string SDLWindow::GetTitle() const {
    return SDL_GetWindowTitle(m_WindowImpl);
}

Vector2i SDLWindow::GetPosition() const {
    Vector2i pos;
    SDL_GetWindowPosition(m_WindowImpl, &pos.X, &pos.Y);

    return pos;
}

Vector2f SDLWindow::GetMousePosition() const {
    Vector2f pos;
    SDL_GetMouseState(&pos.X, &pos.Y);

    return pos;
}

bool SDLWindow::IsFullscreen() const {
    return m_Fullscreen;
}

bool SDLWindow::IsCursorGrabbed() const {
    return SDL_GetWindowMouseGrab(m_WindowImpl);
}

bool SDLWindow::IsCursorVisible() const {
    return SDL_CursorVisible();
}

Optional<Base::Event> SDLWindow::PollEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
        switch (e.type) {
            case SDL_EVENT_QUIT:
                return GameExitingEvent();
            case SDL_EVENT_WINDOW_RESIZED:
                EventManager::GetInstance()->Send(std::make_shared<GameResizedEvent>(Vector2u{ static_cast<unsigned int>(e.window.data1), static_cast<unsigned int>(e.window.data2) }));
                return GameResizedEvent{ Vector2u{ static_cast<unsigned int>(e.window.data1), static_cast<unsigned int>(e.window.data2) } };
            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_PIPE) {
                        SignalManager::GetInstance()->Send(std::make_shared<GameToggleDrawingDebugInfo>());
               }

                else if (e.key.key == SDLK_G)
                    if (e.key.mod & SDL_KMOD_CTRL || e.key.mod & SDL_KMOD_LCTRL || e.key.mod & SDL_KMOD_RCTRL) {
                        SignalManager::GetInstance()->Send(std::make_shared<GameToggleImGui>());
                    }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                return MouseMovedEvent{{ static_cast<int>(e.motion.x), static_cast<int>(e.motion.y) }};
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                {
                    Mouse::Button btn{};
                    switch (e.button.button) {
                        case SDL_BUTTON_LEFT:
                            btn = Mouse::Button::Left;
                            break;
                        case SDL_BUTTON_MIDDLE:
                            btn = Mouse::Button::Middle;
                            break;
                        case SDL_BUTTON_RIGHT:
                            btn = Mouse::Button::Right;
                            break;
                        case SDL_BUTTON_X1:
                            btn = Mouse::Button::Extra1;
                            break;
                        case SDL_BUTTON_X2:
                            btn = Mouse::Button::Extra1;
                            break;
                    }
                    return MouseButtonDownEvent{btn, { static_cast<int>(e.button.x), static_cast<int>(e.button.y) }};
                }
            case SDL_EVENT_MOUSE_BUTTON_UP:
                Mouse::Button btn{};
                switch (e.button.button) {
                    case SDL_BUTTON_LEFT:
                        btn = Mouse::Button::Left;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        btn = Mouse::Button::Middle;
                        break;
                    case SDL_BUTTON_RIGHT:
                        btn = Mouse::Button::Right;
                        break;
                    case SDL_BUTTON_X1:
                        btn = Mouse::Button::Extra1;
                        break;
                    case SDL_BUTTON_X2:
                        btn = Mouse::Button::Extra1;
                        break;
                }
                return MouseButtonUpEvent{btn, { static_cast<int>(e.button.x), static_cast<int>(e.button.y) }};
        }

    } 

    return std::nullopt;
}

void SDLWindow::OnRender() {
    Profiler::StartProfile({ typeid(SDLWindow), "OnRender" }, IM_COL32(0, 50, 255, 255));
    if (m_Context->GFlags & GameFlags_ImGui) {
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
    }
    Profiler::EndProfile({ typeid(SDLWindow), "OnRender" });
}

void SDLWindow::OnDisplay() {

}

void SDLWindow::ImGuiWindowContent() {
    ImGui::Text("SDL Window");
    ImGui::Text("Window Title: %s", GetTitle().c_str());
    ImGui::Text("Window Size: (X: %u, Y: %u)", GetSize().X, GetSize().Y);
    ImGui::Text("Window Position: (X: %u, Y: %u)", GetPosition().X, GetPosition().Y);
    ImGui::Text("Mouse Position: (X: %f, Y: %f)", GetMousePosition().X, GetMousePosition().Y);
}

SDL_WindowFlags SDLWindow::GetWindowFlags() {
    return m_WindowFlags;
}

void SDLWindow::SetWindowFlags(SDL_WindowFlags flags) {
    m_WindowFlags = flags;
}

void SDLWindow::AddWindowFlags(SDL_WindowFlags flags) {
    m_WindowFlags |= flags;
}

void SDLWindow::RemoveWindowFlags(SDL_WindowFlags flags) {
    m_WindowFlags &= ~flags;
}

void SDLWindow::ClearWindowFlags() {
    m_WindowFlags = 0;
}

void SDLWindow::SetInitFlags(SDL_InitFlags flags) {
    m_InitFlags = flags;
}

void SDLWindow::AddInitFlags(SDL_InitFlags flags) {
    m_InitFlags |= flags;
}

void SDLWindow::RemoveInitFlags(SDL_InitFlags flags) {
    m_InitFlags &= ~flags;
}

void SDLWindow::ClearInitFlags() {
    m_InitFlags = 0;
}

void SDLWindow::PrepareForRenderer() {
    if (m_RendererName == "Vulkan") {
        AddWindowFlags(SDL_WINDOW_VULKAN);
    }
    else if (m_RendererName == "OpenGL") {
        AddWindowFlags(SDL_WINDOW_OPENGL);
    }
    else if (m_RendererName == "Metal") {
        AddWindowFlags(SDL_WINDOW_METAL);
    }

}

void SDLWindow::LogActiveFlags() {
    AddSDLLog("Active Window flags:");
    
    if (m_WindowFlags & SDL_WINDOW_FULLSCREEN)
        AddSDLLog("    SDL_WINDOW_FULLSCREEN");
    if (m_WindowFlags & SDL_WINDOW_OPENGL)
        AddSDLLog("    SDL_WINDOW_OPENGL");
    if (m_WindowFlags & SDL_WINDOW_OCCLUDED)
        AddSDLLog("    SDL_WINDOW_OCCLUDED");
    if (m_WindowFlags & SDL_WINDOW_HIDDEN)
        AddSDLLog("    SDL_WINDOW_HIDDEN");
    if (m_WindowFlags & SDL_WINDOW_BORDERLESS)
        AddSDLLog("    SDL_WINDOW_BORDERLESS");
    if (m_WindowFlags & SDL_WINDOW_RESIZABLE)
        AddSDLLog("    SDL_WINDOW_RESIZABLE");
    if (m_WindowFlags & SDL_WINDOW_MINIMIZED)
        AddSDLLog("    SDL_WINDOW_MINIMIZED");
    if (m_WindowFlags & SDL_WINDOW_MAXIMIZED)
        AddSDLLog("    SDL_WINDOW_MAXIMIZED");
    if (m_WindowFlags & SDL_WINDOW_MOUSE_GRABBED)
        AddSDLLog("    SDL_WINDOW_MOUSE_GRABBED");
    if (m_WindowFlags & SDL_WINDOW_INPUT_FOCUS)
        AddSDLLog("    SDL_WINDOW_INPUT_FOCUS");
    if (m_WindowFlags & SDL_WINDOW_MOUSE_FOCUS)
        AddSDLLog("    SDL_WINDOW_MOUSE_FOCUS");
    if (m_WindowFlags & SDL_WINDOW_EXTERNAL)
        AddSDLLog("    SDL_WINDOW_EXTERNAL");
    if (m_WindowFlags & SDL_WINDOW_MODAL)
        AddSDLLog("    SDL_WINDOW_MODAL");
    if (m_WindowFlags & SDL_WINDOW_HIGH_PIXEL_DENSITY)
        AddSDLLog("    SDL_WINDOW_HIGH_PIXEL_DENSITY");
    if (m_WindowFlags & SDL_WINDOW_MOUSE_CAPTURE)
        AddSDLLog("    SDL_WINDOW_MOUSE_CAPTURE");
    if (m_WindowFlags & SDL_WINDOW_MOUSE_RELATIVE_MODE)
        AddSDLLog("    SDL_WINDOW_MOUSE_RELATIVE_MODE");
    if (m_WindowFlags & SDL_WINDOW_ALWAYS_ON_TOP)
        AddSDLLog("    SDL_WINDOW_ALWAYS_ON_TOP");
    if (m_WindowFlags & SDL_WINDOW_UTILITY)
        AddSDLLog("    SDL_WINDOW_UTILITY");
    if (m_WindowFlags & SDL_WINDOW_TOOLTIP)
        AddSDLLog("    SDL_WINDOW_TOOLTIP");
    if (m_WindowFlags & SDL_WINDOW_POPUP_MENU)
        AddSDLLog("    SDL_WINDOW_POPUP_MENU");
    if (m_WindowFlags & SDL_WINDOW_KEYBOARD_GRABBED)
        AddSDLLog("    SDL_WINDOW_KEYBOARD_GRABBED");
    if (m_WindowFlags & SDL_WINDOW_FILL_DOCUMENT)
        AddSDLLog("    SDL_WINDOW_FILL_DOCUMENT");
    if (m_WindowFlags & SDL_WINDOW_VULKAN)
        AddSDLLog("    SDL_WINDOW_VULKAN");
    if (m_WindowFlags & SDL_WINDOW_METAL)
        AddSDLLog("    SDL_WINDOW_METAL");
    if (m_WindowFlags & SDL_WINDOW_TRANSPARENT)
        AddSDLLog("    SDL_WINDOW_TRANSPARENT");
    if (m_WindowFlags & SDL_WINDOW_NOT_FOCUSABLE)
        AddSDLLog("    SDL_WINDOW_NOT_FOCUSABLE");

    AddSDLLog("Active Init flags:");
    if (m_InitFlags & SDL_INIT_AUDIO)
        AddSDLLog("    SDL_INIT_AUDIO");
    if (m_InitFlags & SDL_INIT_VIDEO)
        AddSDLLog("    SDL_INIT_VIDEO");
    if (m_InitFlags & SDL_INIT_JOYSTICK)
        AddSDLLog("    SDL_INIT_JOYSTICK");
    if (m_InitFlags & SDL_INIT_HAPTIC)
        AddSDLLog("    SDL_INIT_HAPTIC");
    if (m_InitFlags & SDL_INIT_GAMEPAD)
        AddSDLLog("    SDL_INIT_GAMEPAD");
    if (m_InitFlags & SDL_INIT_EVENTS)
        AddSDLLog("    SDL_INIT_EVENTS");
    if (m_InitFlags & SDL_INIT_SENSOR)
        AddSDLLog("    SDL_INIT_SENSOR");
    if (m_InitFlags & SDL_INIT_CAMERA)
        AddSDLLog("    SDL_INIT_CAMERA");
}

SDL_Window* SDLWindow::GetWindowImpl() {
    return m_WindowImpl;
}
