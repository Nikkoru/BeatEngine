#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"

struct AppSettingsChangedEvent : Base::Event {
    AppSettingsChangedEvent() : Base::Event(typeid(AppSettingsChangedEvent)) {}
};

struct AppResizedEvent : Base::Event {
    Vector2u Size; 

    AppResizedEvent(Vector2u newSize) : Base::Event(typeid(AppResizedEvent)), Size(newSize) {} 
};

struct AppExitingEvent : Base::Event {
    AppExitingEvent() : Base::Event(typeid(AppExitingEvent)) {}
};
