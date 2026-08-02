#pragma once

#include "BeatEngine/Asset/Font.h"
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Camera/Camera.h"
#include "BeatEngine/Graphics/GraphicalElement.hpp"
#include "BeatEngine/Graphics/RenderState.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Util/Optional.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>

class GameContext;
class GameState;
class BaseWindow;
class Renderer;
class GraphicsManager {
private:
    GameContext* m_Context{ nullptr };
    GameState* m_State{ nullptr };
    std::shared_ptr<Renderer> m_Renderer{ nullptr };
    bool m_Open = true;

    std::string m_WindowTitle{};
    Vector2u m_WindowSize{};
    bool m_WindowFullscreen{ false };

    Camera* m_MainCamera{ nullptr };
    Camera m_UICamera{};
public:
    GraphicsManager();
    GraphicsManager(GameContext* context, GameState* state)
        : m_Context(context), m_State(state) {}
    ~GraphicsManager();
public:
    template<typename RendererT>
        requires(std::is_base_of_v<Renderer, RendererT>)
    void MakeRenderer();
    void MakeRenderer(std::shared_ptr<Renderer> renderer);

    void Init();
    void Update();
    void Close();

    void SetMainCamera(Camera& camera) { m_MainCamera = &camera; }
    Camera* GetMainCamera() { return m_MainCamera; }
    Camera& GetUICamera() { return m_UICamera; }

    void ShowImGuiDebugWindow();

    void SetWindowFullscreen(bool fullscreen);
    void SetWindowTitle(std::string windowTitle);
    void SetWindowSize(Vector2u size);
    void SetFramerateLimit(unsigned int fps);

    void Render();
    void RenderImGui();
    void Display();
    void Clear();

    void DrawElement(GraphicalElement& element, RenderState state = RenderState::Default);
    void DrawVertices(VertexArray& vertices, RenderState state = RenderState::Default);
    void InitElement(GraphicalElement& element, RenderState state);
    void UninitElement(GraphicalElement& element);

    std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path);
    std::shared_ptr<Texture> CreateTexture(const uint8_t* pixelData, Vector2u size);
    std::shared_ptr<Texture> CreateEmptyTexture(Vector2u size);
    std::shared_ptr<Texture> GetErrorTexture();
    void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest);
    void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData);
    void UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture);
    void DestroyTexture(std::shared_ptr<Texture> texture);
    unsigned int GetMaxTextureSize();
    std::shared_ptr<Shader> CreateShader(const std::filesystem::path& path, Shader::Type type);

    std::shared_ptr<BaseWindow> GetWindow();
    std::shared_ptr<Renderer> GetRenderer();

    Optional<Base::Event> PollEvent();
public:
    bool IsOpen();
};

#include "BeatEngine/Manager/GraphicsManager.inl"
