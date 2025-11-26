#pragma once

#include <BeatEngine/Base/System.h>

class SettingsSystemTest : public Base::System {
public:
	void Start() override;
	void Stop() override;

	void Update(float dt) override;
};