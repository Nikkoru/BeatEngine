#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"

class AppSettingsChangedEvent : public Base::Event {
public:
    AppSettingsChangedEvent() : Base::Event(typeid(AppSettingsChangedEvent)) {}
};

class AppResizedEvent : public Base::Event {
public:
    Vector2u Size; 
public:
    AppResizedEvent(Vector2u newSize) : Base::Event(typeid(AppResizedEvent)), Size(newSize) {} 
};

class AppExitingEvent : public Base::Event {
public:
    AppExitingEvent() : Base::Event(typeid(AppExitingEvent)) {}
};
