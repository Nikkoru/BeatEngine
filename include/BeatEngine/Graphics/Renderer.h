#pragma once
#include "BeatEngine/Asset/Shader.h"
#include "BeatEngine/Asset/Texture.h"
#include "BeatEngine/Graphics/BaseWindow.h"
#include "BeatEngine/Graphics/RenderState.hpp"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Manager/GraphicsManager.h"
#include <filesystem>
#include <memory>

class GameContext;
class GraphicalElement;
class Renderer {
private:
    friend class GraphicsManager;
protected:
    std::shared_ptr<BaseWindow> m_Window{ nullptr };
    GameContext* m_Context;
    bool m_Profile{ false };

    unsigned int m_TargetFps{ 0 };
    VSyncMode m_vSyncMode{ Default };
public:
    Renderer() : Renderer(nullptr) {};
    Renderer(GameContext* context) : m_Context(context) {}
    virtual ~Renderer() = default;
public:
    virtual void Init(std::string windowTitle, Vector2u windowSize, VSyncMode vSync = VSyncMode::Disable) = 0;
    virtual void Uninit() = 0;
    virtual void Render() = 0;
    virtual void RenderImGui() {};
    virtual void Display() = 0;
    virtual void Clear() = 0;
    virtual void Update() = 0;

    virtual void SetGlobalShader(std::shared_ptr<Shader> shader) = 0;

    // virtual void DrawElement(GraphicalElement& element, RenderState state = RenderState::Default) = 0;
    virtual void DrawVertices(VertexArray& vertices, RenderState state = RenderState::Default) = 0;
    virtual void InitVertices(VertexArray& vertices, RenderState state) = 0;
    virtual void UninitVertices(VertexArray& vertices) = 0;
    // virtual void InitElement(GraphicalElement& element) = 0;
    // virtual void UninitElement(GraphicalElement& element) = 0;

    virtual void ProcessEvent(Optional<Base::Event> event) = 0;

    virtual unsigned int GetMaxTextureSize() = 0;
    virtual std::shared_ptr<Texture> CreateTexture(const std::filesystem::path& path) = 0;
    virtual std::shared_ptr<Texture> CreateTexture(const uint8_t* pixelData, Vector2u size) = 0;
    virtual std::shared_ptr<Texture> CreateEmptyTexture(Vector2u size) = 0;
    virtual void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData, Vector2u size, Vector2u dest) = 0;
    virtual void UpdateTexture(std::shared_ptr<Texture> texture, const void* pixelData) = 0;
    virtual void UpdateTexture(std::shared_ptr<Texture> dstTexture, std::shared_ptr<Texture> srcTexture) = 0;
    virtual void DestroyTexture(std::shared_ptr<Texture> texture) = 0;
    virtual std::shared_ptr<Font> CreateFont(const std::filesystem::path& path) = 0;
    virtual void DestroyFont(std::shared_ptr<Font> font) = 0;
    virtual std::shared_ptr<Shader> CreateShader(const std::filesystem::path& path, Shader::Type type) = 0;

    virtual void ShowImGuiRenderTabContent() {}

    std::shared_ptr<BaseWindow> GetWindow() { return m_Window; }
    void SetWindow(std::shared_ptr<BaseWindow> window) { m_Window = window; }

    void SetFramerateLimit(unsigned int fps) { m_TargetFps = fps; }
    unsigned int GetFramerateLimit() { return m_TargetFps; }

    virtual Optional<Base::Event> PollEvent() const = 0;
protected:
    uint32_t GetVertexArrayHighlightSourceID(VertexArray& vertices) { return vertices.GetSourceID(); }
    uint32_t GetVertexArrayHighlightID(VertexArray& vertices) { return vertices.m_IsHighlight ? vertices.m_HighlightID : vertices.GetArrayID() ; }
    void SetVertexHighlightID(VertexArray& vertices, uint32_t sourceID, uint32_t arrayID = 0) { vertices.SetHighlightID(sourceID, arrayID); }
    uint32_t GetVertexArrayID(VertexArray& vertices) { return vertices.m_ID; }
    void SetVertexArrayID(VertexArray& vertices, uint32_t id) { vertices.m_ID = id; }
    bool IsVertexArrayInitialized(VertexArray& vertices) { return vertices.m_Initialized; }
    void SetVertexArrayInitializedStatus(VertexArray& vertices, bool initialized) { vertices.m_Initialized = initialized; }
    bool IsVertexArrayHighlight(VertexArray& vertices) { return vertices.m_IsHighlight; }
    void SetVertexArrayHighlightStatus(VertexArray& vertices, bool highlight) { vertices.m_IsHighlight = highlight; }
};
