#pragma once
#include "BeatEngine/Base/Asset.h"
#include "BeatEngine/Graphics/Vector2.h"
#include <utility>

class Texture : public Base::Asset {
private:
    Vector2u m_Size{};
public:
	Texture() = default;
	Texture(const Texture& other) { *this = other; }
	Texture(const Texture&& other) noexcept { *this = std::move(other); }
public:
	explicit operator bool() const;

	Texture& operator=(const Texture& other);
	Texture& operator=(const Texture&& other) noexcept;

    virtual bool IsValid() { return false; }

    Vector2u GetSize() { return m_Size; }
protected:
    virtual void MakeCopy(const Texture& other) { (void)other; }
    virtual void MakeMove(const Texture&& other) noexcept { (void)other; }
};
