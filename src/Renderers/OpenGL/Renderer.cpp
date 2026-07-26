#include "BeatEngine/Renderers/OpenGL/Renderer.h"
// #include "BeatEngine/Graphics/VSyncMode.h"
#include "BeatEngine/Windows/SDL/Window.h"
#include "SDL3/SDL_video.h"
#include <memory>

void OpenGLRenderer::Init(std::string windowTitle, Vector2u windowSize, VSyncMode mode) {
    if (m_Window == nullptr) {
        m_Window = std::make_shared<SDLWindow>();
        std::static_pointer_cast<SDLWindow>(m_Window)->SetWindowFlags(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    m_Window->Init(m_Context, windowTitle, windowSize);

    (void)mode;
    // auto a = SDL_GL_CreateContext(std::static_pointer_cast<SDLWindow>(m_Window)->GetWindowImpl());
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
void OpenGLRenderer::SetGlobalShader(std::shared_ptr<Shader> shader) {
    (void)shader;
}

std::shared_ptr<Texture> OpenGLRenderer::CreateTexture(const std::filesystem::path& path) {
    (void)path;
    return {};
}

std::shared_ptr<Font> OpenGLRenderer::CreateFont(const std::filesystem::path& path) {
    (void)path;
    return {};
}

std::shared_ptr<Shader> OpenGLRenderer::CreateShader(const std::filesystem::path& path, Shader::Type type) {
    (void)path;
    (void)type;
    return {};
}


Optional<Base::Event> OpenGLRenderer::PollEvent() const {
    return m_Window->PollEvent();
}
