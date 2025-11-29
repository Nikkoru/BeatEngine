#include "BeatEngine/Settings/GameSettings.h"

#include <format>

void GameSettings::Read(const char* line) {
	int x, y;

	if (sscanf(line, "fpsLimit=%i", &x) == 1) {
		FpsLimit = x;
	}
	if (sscanf(line, "windowPos=%i,%i", &x, &y) == 1) {
		WindowPosition = sf::Vector2u(x, y);
	}
	if (sscanf(line, "windowSize=%i,%i", &x, &y) == 1) {
		WindowSize = sf::Vector2u(x, y);
	}
}

std::string GameSettings::Write() {
	std::string config;

	config += m_SettingsTag + "\n";
	config += std::format("fpsLimit={}", FpsLimit) + "\n";
	config += std::format("windowPos={},{}", WindowPosition.x, WindowPosition.y) + "\n";
	config += std::format("windowSize={},{}", WindowSize.x, WindowSize.y);

	return config;
}

void GameSettings::SetDefaults() {
	FpsLimit = 60;
	WindowPosition = { static_cast<unsigned int>(-1), static_cast<unsigned int>(-1) };
	WindowSize = { static_cast<unsigned int>(800), static_cast<unsigned int>(600) };
}
