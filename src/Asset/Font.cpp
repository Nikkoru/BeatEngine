#include "BeatEngine/Asset/Font.h"

Font::operator bool() const {
	return m_SFMLFont != nullptr;
}

Font& Font::operator=(const Font& other) {
	this->m_SFMLFont = other.m_SFMLFont;
	return *this;
}

Font& Font::operator=(const Font&& other) noexcept {
	this->m_SFMLFont = std::move(other.m_SFMLFont);
	return *this;
}

const std::shared_ptr<sf::Font>& Font::GetSFMLFont() {
	return m_SFMLFont;
}