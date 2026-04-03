#include "BeatEngine/Renderers/OpenGL/Renderer.h"
// #include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "SDL3/SDL_video.h"
#include <memory>

void OpenGLRenderer::Init(std::string windowTitle, Vector2u windowSize, bool imgui) {
    if (m_Window == nullptr) {
        m_Window = std::make_shared<SDLWindow>();
        std::static_pointer_cast<SDLWindow>(m_Window)->SetFlags(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    }
    
    m_Window->Init(windowTitle, windowSize);
    auto a = SDL_GL_CreateContext(std::static_pointer_cast<SDLWindow>(m_Window)->GetWindowImpl());
}

void OpenGLRenderer::Uninit() {
    
}

void OpenGLRenderer::Render() {

}

void OpenGLRenderer::Display() {

}

void OpenGLRenderer::Clear() {

}

void OpenGLRenderer::Update() {

}

std::optional<Base::Event> OpenGLRenderer::PollEvent() const {
    return m_Window->PollEvent();
}
