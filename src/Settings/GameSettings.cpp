#include "BeatEngine/Settings/GameSettings.h"
#include "BeatEngine/Util/Helper.h"

#include <cstdio>
#include <format>

void GameSettings::Read(const char* line) {
	int x, y;
    char buf[6];

	if (sscanf(line, "fpsLimit=%i", &x) == 1) {
		FpsLimit = x;
	}
    else if (sscanf(line, "vsync=%s", &buf) == 1) {
        buf[5] = '\0';

        VSync = Helper::ParseBoolFromString(buf);
    }
    else if (sscanf(line, "windowPos=%i,%i", &x, &y) == 2) {
		WindowPosition = sf::Vector2i(x, y);
	}
    else if (sscanf(line, "windowSize=%i,%i", &x, &y) == 2) {
		WindowSize = sf::Vector2u(x, y);
	}
    else if (sscanf(line, "fullscreen=%s", &buf) == 1) {
        buf[5] = '\0';

        WindowFullScreen = Helper::ParseBoolFromString(buf);
    }
}

std::string GameSettings::Write() {
	std::string config;

	config += m_SettingsTag + "\n";
	config += std::format("fpsLimit={}", FpsLimit) + "\n";
    config += std::format("vsync={}", (VSync ? "true" :"false")) + "\n";
	config += std::format("windowPos={},{}", WindowPosition.x, WindowPosition.y) + "\n";
	config += std::format("windowSize={},{}", WindowSize.x, WindowSize.y) + "\n";
    config += std::format("fullscreen={}", (WindowFullScreen ? "true" : "false"));

	return config;
}

void GameSettings::SetDefaults() {
	FpsLimit = 60;
    VSync = false;
    WindowFullScreen = false;
	WindowPosition = { -1, -1 };
	WindowSize = { 800, 600 };
}
