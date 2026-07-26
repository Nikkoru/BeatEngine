#include "BeatEngine/Manager/GraphicsManager.h"
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Base/Signal.h"
#include "BeatEngine/Enum/GameFlags.h"
#include "BeatEngine/Events/GameEvent.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/RendererData.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/SignalManager.h"
#include "BeatEngine/Signals/GameSignals.h"
#include "imgui.h"

#include "BeatEngine/GameContext.h"

#ifdef BEATENGINE_VULKAN_RENDERER
#include "BeatEngine/Renderers/Vulkan/Renderer.h"
#elif defined(BEATENGINE_OPENGL_RENDERER)
#include "BeatEngine/Renderers/OpenGL/Renderer.h"
#endif

#include <filesystem>
#include <memory>
#include <string>

GraphicsManager::GraphicsManager() {
    SignalManager::GetInstance()->RegisterCallback<GameUninitGraphicsSignal>(typeid(GraphicsManager), [&](std::shared_ptr<Base::Signal> sig) {
        auto uninitSignal = std::static_pointer_cast<GameUninitGraphicsSignal>(sig);

        UninitElement(uninitSignal->Element);
    });
}

GraphicsManager::~GraphicsManager() {
    SignalManager::GetInstance()->RemoveCallbacks(typeid(GraphicsManager));
}

void GraphicsManager::MakeRenderer(std::shared_ptr<Renderer> renderer) {
    if (!renderer->m_Context)
        renderer->m_Context = m_Context;
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
    m_Renderer->Update(); 
}

void GraphicsManager::Close() {
    m_Renderer->Uninit();
}

void GraphicsManager::ShowImGuiDebugWindow() {
    if (!(m_Context->GFlags & GameFlags_ImGui)) return;

    ImGui::Begin("GraphicsManager Debug");
    if (ImGui::BeginTabBar("SelectionTabBar")) {
        if (ImGui::BeginTabItem("Renderer")) {
            ImGui::BeginChild("RendererChild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_AlwaysUseWindowPadding);
            m_Renderer->ShowImGuiRenderTabContent();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Window")) {
            m_Renderer->GetWindow()->ImGuiWindowContent();
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
    m_Renderer->SetFramerateLimit(fps);
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

void GraphicsManager::DrawElement(GraphicalElement& element, RenderState state) {
    if (!element.m_Initialized)
        InitElement(element, state);
    DrawVertices(element.m_Vertices, state);
}

void GraphicsManager::DrawVertices(VertexArray& vertices, RenderState state) {
    m_Renderer->DrawVertices(vertices, state);
}

void GraphicsManager::InitElement(GraphicalElement& element, RenderState state) {
    m_Renderer->InitVertices(element.m_Vertices, state);
}

void GraphicsManager::UninitElement(GraphicalElement& element) {
    m_Renderer->UninitVertices(element.m_Vertices);
}

std::shared_ptr<Renderer> GraphicsManager::GetRenderer() {
    return m_Renderer;
}

std::shared_ptr<Texture> GraphicsManager::CreateTexture(const std::filesystem::path& path) {
    return m_Renderer->CreateTexture(path);
}

std::shared_ptr<Texture> GraphicsManager::CreateTexture(const uint8_t* pixelData, Vector2u size) {
    return m_Renderer->CreateTexture(pixelData, size);
}

std::shared_ptr<Texture> GraphicsManager::CreateEmptyTexture(Vector2u size) {
    return m_Renderer->CreateEmptyTexture(size);
}

void GraphicsManager::UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) {
    m_Renderer->UpdateTexture(texture, pixelData, size, dest);
}

void GraphicsManager::UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData) {
    m_Renderer->UpdateTexture(texture, pixelData);
}

void GraphicsManager::UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) {
    m_Renderer->UpdateTexture(dstTexture, srcTexture);
}

void GraphicsManager::DestroyTexture(std::shared_ptr<Texture> texture) {
    m_Renderer->DestroyTexture(texture);
}


unsigned int GraphicsManager::GetMaxTextureSize() {
    return m_Renderer->GetMaxTextureSize();
}

std::shared_ptr<Font> GraphicsManager::CreateFont(const std::filesystem::path& path) {
    return m_Renderer->CreateFont(path);
}

std::shared_ptr<Shader> GraphicsManager::CreateShader(const std::filesystem::path& path, Shader::Type type) {
    return m_Renderer->CreateShader(path, type);
}

std::shared_ptr<BaseWindow> GraphicsManager::GetWindow() {
    return m_Renderer->GetWindow();
}

Optional<Base::Event> GraphicsManager::PollEvent() {
    auto event = m_Renderer->PollEvent();
    m_Renderer->ProcessEvent(event);
    
    if (event.HasValue())
        if (auto data = event->GetIf<GameResizedEvent>(event))
            m_UICamera.SetSize(Vector2f{ data->Size });

    return event;
}

bool GraphicsManager::IsOpen() {
    return this->m_Open;
}
