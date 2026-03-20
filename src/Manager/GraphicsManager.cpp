#include "BeatEngine/Manager/GraphicsManager.h"

#ifdef BEATENGINE_VULKAN_RENDERER
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#elif defined(BEATENGINE_OPENGL_RENDERER)
#include "BeatEngine/Renderers/OpenGL/Renderer.h"
#endif

#include <filesystem>
#include <memory>
#include <string>

void GraphicsManager::Init() {
#ifdef BEATENGINE_VULKAN_RENDERER
    if (m_Renderer == nullptr)
        MakeRenderer<VulkanRenderer>();
#elif defined(BEATENGINE_OPENGL_RENDERER)
    if (m_Renderer == nullptr)
        MakeRenderer<OpenGLRenderer>();
#else
    assert(m_Renderer && "No renderer defined, define one using GraphicsManager::MakeRenderer<T>() or Game::SetRenderer<T>()")
#endif

    m_Renderer->Init(m_WindowTitle, m_WindowSize);
}

void GraphicsManager::Update() {
    
}

void GraphicsManager::Close() {
    m_Renderer->Uninit();
}

void GraphicsManager::SetWindowTitle(std::string windowTitle) {
    m_WindowTitle = windowTitle;

    if (m_Renderer != nullptr && m_Renderer->GetWindow() != nullptr)
        m_Renderer->GetWindow()->SetTitle(windowTitle);
}

void GraphicsManager::Render() {
    m_Renderer->Render();
}

void GraphicsManager::Display() {
    m_Renderer->Display();
}

void GraphicsManager::Clear() {
    m_Renderer->Clear();
}

std::shared_ptr<Renderer> GraphicsManager::GetRenderer() {
    return m_Renderer;
}

std::shared_ptr<Texture> GraphicsManager::CreateTexture(std::filesystem::path path) {
    return m_Renderer->CreateTexture(path);
}

std::shared_ptr<BaseWindow> GraphicsManager::GetWindow() {
    return m_Renderer->GetWindow();
}

std::optional<Base::Event> GraphicsManager::PollEvent() {
    return m_Renderer->PollEvent();
}

bool GraphicsManager::IsOpen() {
    return this->m_Open;
}
