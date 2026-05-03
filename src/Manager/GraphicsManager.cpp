#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/GameContext.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "imgui.h"
#include <future>

#ifdef BEATENGINE_VULKAN_RENDERER
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#elif defined(BEATENGINE_OPENGL_RENDERER)
#include "BeatEngine/Renderers/OpenGL/Renderer.h"
#endif

#include <filesystem>
#include <memory>
#include <string>

void GraphicsManager::MakeRenderer(std::shared_ptr<Renderer> renderer) {
    m_Renderer = renderer;
}

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
    
    if (m_WindowFullscreen)
        m_Renderer->Init(m_WindowTitle, { static_cast<unsigned int>(-1), static_cast<unsigned int>(-1) });
    else
        m_Renderer->Init(m_WindowTitle, m_WindowSize);
}

void GraphicsManager::Update() {
    
}

void GraphicsManager::Close() {
    m_Renderer->Uninit();
}

void GraphicsManager::ShowImGuiDebugWindow() {
    ImGui::Begin("GraphicsManager Debug");
    if (ImGui::BeginTabBar("SelectionTabBar")) {
        if (ImGui::BeginTabItem("Renderer")) {
            ImGui::BeginChild("RendererChild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding);
            m_Renderer->ShowImGuiRenderTabContent();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Window")) {
            auto window = GetWindow();
            ImGui::Text("Window Title: %s", window->GetTitle().c_str());
            ImGui::Text("Window Size: (X: %u, Y: %u)", window->GetSize().X, window->GetSize().Y);
            ImGui::Text("Window Position: (X: %u, Y: %u)", window->GetPosition().X, window->GetPosition().Y);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void GraphicsManager::SetWindowFullscreen(bool fullscreen) {
    m_WindowFullscreen = fullscreen;

    if (m_Renderer != nullptr && m_Renderer->GetWindow() != nullptr) {
        GetWindow()->SetFullscreen(fullscreen);
    }
}

void GraphicsManager::SetWindowTitle(std::string windowTitle) {
    m_WindowTitle = windowTitle;

    if (m_Renderer != nullptr && m_Renderer->GetWindow() != nullptr)
        m_Renderer->GetWindow()->SetTitle(windowTitle);
}

void GraphicsManager::SetWindowSize(Vector2u size) {
    m_WindowSize = size;

    if (m_Renderer != nullptr && m_Renderer->GetWindow() != nullptr)
        m_Renderer->GetWindow()->SetSize(size);
}

void GraphicsManager::SetFramerateLimit(unsigned int fps) {
    
}

void GraphicsManager::Render() {
    m_Renderer->Render();
}

void GraphicsManager::RenderImGui() {
    m_Renderer->RenderImGui();
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

std::shared_ptr<Shader> GraphicsManager::CreateShader(std::filesystem::path path, Shader::Type type) {
    return m_Renderer->CreateShader(path, type);
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
