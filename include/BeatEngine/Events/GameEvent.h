#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"

class EventGameSettingsChanged : public Base::Event {
public:
    EventGameSettingsChanged() : Base::Event(typeid(EventGameSettingsChanged)) {}
};

class EventGameResized : public Base::Event {
public:
    Vector2u Size; 
public:
    EventGameResized(Vector2u newSize) : Base::Event(typeid(EventGameResized)), Size(newSize) {} 
};

class EventGameExiting : public Base::Event {
public:
    EventGameExiting() : Base::Event(typeid(EventGameExiting)) {}
};

