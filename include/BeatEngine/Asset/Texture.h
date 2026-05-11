#pragma once
#include "BeatEngine/Base/Asset.h"

class Texture : public Base::Asset {
public:
	Texture() = default;
	Texture(const Texture& other) { (void)other; }
	Texture(const Texture&& other) noexcept { (void)other; }

	// Texture(sf::Texture texture) : m_SFMLTexture(std::make_shared<sf::Texture>(texture)) {}
public:
	explicit operator bool() const;

	Texture& operator=(const Texture& other);
	Texture& operator=(const Texture&& other) noexcept;
};
