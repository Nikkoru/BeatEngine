#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"

class GameSettingsChangedEvent : public Base::Event {
public:
    GameSettingsChangedEvent() : Base::Event(typeid(GameSettingsChangedEvent)) {}
};

class GameResizedEvent : public Base::Event {
public:
    Vector2u Size; 
public:
    GameResizedEvent(Vector2u newSize) : Base::Event(typeid(GameResizedEvent)), Size(newSize) {} 
};

class GameExitingEvent : public Base::Event {
public:
    GameExitingEvent() : Base::Event(typeid(GameExitingEvent)) {}
};
