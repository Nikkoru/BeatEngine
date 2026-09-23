#pragma once

#include "BeatEngine/Base/Settings.h"
#include "BeatEngine/Graphics/Vector2.h"


class AppSettings : public Base::Settings {
public:
	unsigned int FpsLimit = 60;
    bool VSync = false;

	Vector2i WindowPosition = { -1, -1 };
	Vector2u WindowSize = { static_cast<unsigned int>(800), static_cast<unsigned int>(600) };

    bool WindowFullScreen = false;
public:
	AppSettings() : Base::Settings(typeid(AppSettings), "[App]") {}
	~AppSettings() override = default;

	void Read(const char* line) override;
	std::string Write() override;

	void SetDefaults() override;
};
