#include "BeatEngine/Windows/SDL/Window.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Manager/EventManager.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Logger.h"

#include <backends/imgui_impl_sdl3.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

#include <memory>

void SDLWindow::Init(std::string windowTitle, Vector2u windowSize) {
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Initializing SDL Window");

    SDL_Init(SDL_INIT_VIDEO);

    if (windowTitle == "")
        windowTitle = "BeatEngine Game";
    
    m_WindowImpl = SDL_CreateWindow(windowTitle.c_str(), windowSize.X, windowSize.Y, m_Flags);
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Window created. Size = ({}, {}), Title = {}", windowSize.X, windowSize.Y, windowTitle);
    LogActiveFlags();
}

void SDLWindow::Uninit() {
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Destroying SDL window");

    SDL_DestroyWindow(m_WindowImpl);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
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
    std::string title{};
    SDL_SetWindowTitle(m_WindowImpl, title.data());

    return title;
}

Vector2i SDLWindow::GetPosition() const {
    Vector2i pos;
    SDL_GetWindowPosition(m_WindowImpl, &pos.X, &pos.Y);

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

std::optional<Base::Event> SDLWindow::PollEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                EventManager::GetInstance()->Send(std::make_shared<GameResizedEvent>(Vector2u{ static_cast<unsigned int>(e.window.data1), static_cast<unsigned int>(e.window.data2) }));
                break;
            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_PIPE) {
                    SignalManager::GetInstance()->Send(std::make_shared<GameToggleImGui>());
               }
        }

        ImGui_ImplSDL3_ProcessEvent(&e);
    } 

    return {};
}

void SDLWindow::OnRender() {
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    ImGui::Render();
}

void SDLWindow::OnDisplay() {

}

void SDLWindow::SetFlags(SDL_WindowFlags flags) {
    m_Flags = flags;
}

void SDLWindow::AddFlags(SDL_WindowFlags flags) {
    m_Flags |= flags;
}

void SDLWindow::RemoveFlags(SDL_WindowFlags flags) {
    m_Flags &= ~flags;
}

void SDLWindow::ClearFlags() {
    m_Flags = 0;
}

void SDLWindow::LogActiveFlags() {
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Active flags:");
    
    if (m_Flags & SDL_WINDOW_FULLSCREEN)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_FULLSCREEN");
    if (m_Flags & SDL_WINDOW_OPENGL)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_OPENGL");
    if (m_Flags & SDL_WINDOW_OCCLUDED)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_OCCLUDED");
    if (m_Flags & SDL_WINDOW_HIDDEN)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_HIDDEN");
    if (m_Flags & SDL_WINDOW_BORDERLESS)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_BORDERLESS");
    if (m_Flags & SDL_WINDOW_RESIZABLE)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_RESIZABLE");
    if (m_Flags & SDL_WINDOW_MINIMIZED)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MINIMIZED");
    if (m_Flags & SDL_WINDOW_MAXIMIZED)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MAXIMIZED");
    if (m_Flags & SDL_WINDOW_MOUSE_GRABBED)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MOUSE_GRABBED");
    if (m_Flags & SDL_WINDOW_INPUT_FOCUS)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_INPUT_FOCUS");
    if (m_Flags & SDL_WINDOW_MOUSE_FOCUS)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MOUSE_FOCUS");
    if (m_Flags & SDL_WINDOW_EXTERNAL)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_EXTERNAL");
    if (m_Flags & SDL_WINDOW_MODAL)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MODAL");
    if (m_Flags & SDL_WINDOW_HIGH_PIXEL_DENSITY)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_HIGH_PIXEL_DENSITY");
    if (m_Flags & SDL_WINDOW_MOUSE_CAPTURE)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MOUSE_CAPTURE");
    if (m_Flags & SDL_WINDOW_MOUSE_RELATIVE_MODE)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_MOUSE_RELATIVE_MODE");
    if (m_Flags & SDL_WINDOW_ALWAYS_ON_TOP)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_ALWAYS_ON_TOP");
    if (m_Flags & SDL_WINDOW_UTILITY)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_UTILITY");
    if (m_Flags & SDL_WINDOW_TOOLTIP)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_TOOLTIP");
    if (m_Flags & SDL_WINDOW_POPUP_MENU)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_POPUP_MENU");
    if (m_Flags & SDL_WINDOW_KEYBOARD_GRABBED)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_KEYBOARD_GRABBED");
    if (m_Flags & SDL_WINDOW_FILL_DOCUMENT)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_FILL_DOCUMENT");
    if (m_Flags & SDL_WINDOW_VULKAN)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_VULKAN");
    if (m_Flags & SDL_WINDOW_METAL)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_METAL");
    if (m_Flags & SDL_WINDOW_TRANSPARENT)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_TRANSPARENT");
    if (m_Flags & SDL_WINDOW_NOT_FOCUSABLE)
        Logger::AddLog("\e[30;46mSDL\033[0m", "", "    SDL_WINDOW_NOT_FOCUSABLE");
}

SDL_Window* SDLWindow::GetWindowImpl() {
    return m_WindowImpl;
}
