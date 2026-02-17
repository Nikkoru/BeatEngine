#include "BeatEngine/Asset/Texture.h"

Texture::operator bool() const {
    return false;
	// return m_SFMLTexture != nullptr;
}

Texture& Texture::operator=(const Texture& other) {
	// this->m_SFMLTexture = other.m_SFMLTexture;
	return *this;
}
Texture& Texture::operator=(const Texture&& other) noexcept {
	// this->m_SFMLTexture = std::move(other.m_SFMLTexture);
	return *this;
}
