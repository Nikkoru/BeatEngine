#include "BeatEngine/Renderers/VulkanWindow.h"
#include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "BeatEngine/Logger.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>

#include <memory>

void VulkanWindow::Init(std::string windowTitle, Vector2u windowSize) {
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Initializing SDL Window");

    auto title = windowTitle;
    if (title == "")
        title = "BeatEngine Game";

    auto size = windowSize;
    if (size == Vector2u{0, 0})
        size = { 1280, 720 };

    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_WindowFlags winFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    m_WindowImpl = SDL_CreateWindow(windowTitle.c_str(), size.X, size.Y, winFlags);
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Window created. Size = ({}, {}), Title = {}", size.X, size.Y, title);

}

void VulkanWindow::Uninit() {
    Logger::AddLog("\e[30;46mSDL\033[0m", "", "Destroying SDL window");

    SDL_DestroyWindow(m_WindowImpl);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}

// void VulkanWindow::UpdateWindow() {
//     SDL_SetWindowTitle(m_WindowImpl, m_WindowTitle.c_str());
//     SDL_SetWindowPosition(m_WindowImpl, m_WindowPosition.X, m_WindowPosition.Y);
//     SDL_SetWindowSize(m_WindowImpl, m_WindowSize.X, m_WindowSize.Y);
//     SDL_SetWindowFullscreen(m_WindowImpl, m_Fullscreen);
//     if (m_VSync)
//         SDL_SetWindowSurfaceVSync(m_WindowImpl, 1);
//     else
//         SDL_SetWindowSurfaceVSync(m_WindowImpl, SDL_WINDOW_SURFACE_VSYNC_DISABLED);
// }
//

void VulkanWindow::SetSize(const Vector2u size) {
    SDL_SetWindowSize(m_WindowImpl, size.X, size.Y);
}

void VulkanWindow::SetMinimumSize(const Vector2u size) {
    SDL_SetWindowMinimumSize(m_WindowImpl, size.X, size.Y);
}

void VulkanWindow::SetMaximumSize(const Vector2u size) {
    SDL_SetWindowMaximumSize(m_WindowImpl, size.X, size.Y);
}

void VulkanWindow::SetTitle(const std::string title) {
    SDL_SetWindowTitle(m_WindowImpl, title.c_str());
}

void VulkanWindow::SetPosition(const Vector2i position) {
    SDL_SetWindowPosition(m_WindowImpl, position.X, position.Y);
}

void VulkanWindow::SetFullscreen(bool fullscreen) {
    SDL_SetWindowFullscreen(m_WindowImpl, fullscreen);
    m_Fullscreen = fullscreen;
}

void VulkanWindow::SetVSyncMode(VSyncMode vsync) {
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

void VulkanWindow::SetCursorGrabbed(bool grabbed) {
    SDL_SetWindowMouseGrab(m_WindowImpl, grabbed);
}

void VulkanWindow::SetCursorVisible(bool visible) {
    if (visible)
        SDL_ShowCursor();
    else
        SDL_HideCursor();
}

Vector2u VulkanWindow::GetSize() const {
    int x{}, y{};
    SDL_GetWindowSize(m_WindowImpl, &x, &y);
    
    return { static_cast<unsigned int>(x), static_cast<unsigned int>(y) };
}

Vector2u VulkanWindow::GetMinimumSize() const {
    int x{}, y{};
    SDL_GetWindowMinimumSize(m_WindowImpl, &x, &y);
    
    return { static_cast<unsigned int>(x), static_cast<unsigned int>(y) };
}

Vector2u VulkanWindow::GetMaximumSize() const {
    int x{}, y{};
    SDL_GetWindowMaximumSize(m_WindowImpl, &x, &y);
    
    return { static_cast<unsigned int>(x), static_cast<unsigned int>(y) };
}

std::string VulkanWindow::GetTitle() const {
    std::string title{};
    SDL_SetWindowTitle(m_WindowImpl, title.data());

    return title;
}

Vector2i VulkanWindow::GetPosition() const {
    Vector2i pos;
    SDL_GetWindowPosition(m_WindowImpl, &pos.X, &pos.Y);

    return pos;
}

bool VulkanWindow::IsFullscreen() const {
    return m_Fullscreen;
}

bool VulkanWindow::IsCursorGrabbed() const {
    return SDL_GetWindowMouseGrab(m_WindowImpl);
}

bool VulkanWindow::IsCursorVisible() const {
    return SDL_CursorVisible();
}

std::optional<Base::Event> VulkanWindow::PollEvent() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            SignalManager::GetInstance()->Send(std::make_shared<GameExitSignal>());
        }
    } 
    return {};
}

SDL_Window* VulkanWindow::GetWindowImpl() {
    return m_WindowImpl;
}
