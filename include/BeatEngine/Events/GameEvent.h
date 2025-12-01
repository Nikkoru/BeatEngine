#pragma once

#include "BeatEngine/Base/Event.h"

class GameSettingsChanged : public Base::Event {
	GameSettingsChanged() : Base::Event(typeid(GameSettingsChanged)) {}
};