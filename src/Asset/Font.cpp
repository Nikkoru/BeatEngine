#include "BeatEngine/Asset/Font.h"

Font::operator bool() const {
// 	return m_SFMLFont != nullptr;
    return false;
}

Font& Font::operator=(const Font& other) {
    (void)other;
	// this->m_SFMLFont = other.m_SFMLFont;
	return *this;
}

Font& Font::operator=(const Font&& other) noexcept {
    (void)other;
	// this->m_SFMLFont = std::move(other.m_SFMLFont);
	return *this;
}

const std::shared_ptr<Font> Font::GetFont() {
	return nullptr;
}
