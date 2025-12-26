#pragma once

#include "BeatEngine/Base/Settings.h"

#include <SFML/System/Vector2.hpp>

class GameSettings : public Base::Settings {
public:
	unsigned int FpsLimit = 60;
    bool VSync = false;

	sf::Vector2i WindowPosition = { -1, -1 };
	sf::Vector2u WindowSize = { static_cast<unsigned int>(800), static_cast<unsigned int>(600) };

    bool WindowFullScreen = false;
public:
	GameSettings() : Base::Settings(typeid(GameSettings), "[Game]") {}
	~GameSettings() override = default;

	void Read(const char* line) override;
	std::string Write() override;

	void SetDefaults() override;
};
