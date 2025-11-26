#pragma once

#include <SFML/Graphics/Font.hpp>

#include "BeatEngine/Base/Asset.h"

class Font : public Base::Asset {
private:
	std::shared_ptr<sf::Font> m_SFMLFont = nullptr;
public:
	Font() = default;
	Font(const Font& other) : m_SFMLFont(other.m_SFMLFont) {}
	Font(const Font&& other) noexcept : m_SFMLFont(std::move(other.m_SFMLFont)) {}
	Font(sf::Font font) : m_SFMLFont(std::make_shared<sf::Font>(font)) {}

	explicit operator bool() const;
	Font& operator=(const Font& other);
	Font& operator=(const Font&& other) noexcept;
public:
	const std::shared_ptr<sf::Font>& GetSFMLFont();
};