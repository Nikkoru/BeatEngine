#pragma once
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "imgui.h"
#include <limits>
#include <utility>

class GraphicsManager;
class Texture : public Base::Asset {
public:
    inline static uint32_t NULL_ID{ (std::numeric_limits<uint32_t>::max)() };
private:
    friend class TextElement;
protected:
    Vector2u m_Size{};
    uint32_t m_CacheID{ NULL_ID };
public:
	Texture() = default;
    Texture(uint32_t id, Vector2u size = {}) : m_Size(size), m_CacheID(id) {};
	Texture(const Texture& other) : m_Size(other.m_Size), m_CacheID(other.m_CacheID) { }
	Texture(const Texture&& other) noexcept : m_Size(std::move(other.m_Size)), m_CacheID(std::move(other.m_CacheID)) {}
public:
	explicit operator bool() const;

	Texture& operator=(const Texture& other);
	Texture& operator=(const Texture&& other) noexcept;

    virtual bool IsValid() { return false; }
    virtual ImTextureID GetImGuiTexture(GraphicsManager& mgr) { (void)mgr; return ImTextureID_Invalid; };

    Vector2u GetSize() { return m_Size; }
    uint32_t GetID() { return m_CacheID; }
protected:
    virtual void MakeCopy(const Texture& other) { (void)other; }
    virtual void MakeMove(const Texture&& other) noexcept { (void)other; }
};
