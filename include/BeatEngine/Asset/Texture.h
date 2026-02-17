#pragma once
#include "BeatEngine/Base/Asset.h"

#include <memory>

class Texture : public Base::Asset {
private:
	// std::shared_ptr<sf::Texture> m_SFMLTexture = nullptr;
public:
	inline const std::shared_ptr<Texture> GetTexture() {
		// return m_SFMLTexture;
        return nullptr;
	}

	Texture() = default;
	Texture(const Texture& other) /*: m_SFMLTexture(other.m_SFMLTexture)*/ {}
	Texture(const Texture&& other) noexcept /*: m_SFMLTexture(std::move(other.m_SFMLTexture))*/ {}

	// Texture(sf::Texture texture) : m_SFMLTexture(std::make_shared<sf::Texture>(texture)) {}
public:
	explicit operator bool() const;

	Texture& operator=(const Texture& other);
	Texture& operator=(const Texture&& other) noexcept;
};
