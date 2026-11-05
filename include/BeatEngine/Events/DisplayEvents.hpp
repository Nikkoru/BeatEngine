#pragma once

#include "BeatEngine/Base/Event.h"

struct DisplayOrientationChangedEvent : Base::Event {
    DisplayOrientationChangedEvent() : Base::Event(typeid(DisplayOrientationChangedEvent)) {}
};

struct DisplayAddedEvent : Base::Event {
    DisplayAddedEvent() : Base::Event(typeid(DisplayAddedEvent)) {}
};

struct DisplayRemovedEvent : Base::Event {
    DisplayRemovedEvent() : Base::Event(typeid(DisplayRemovedEvent)) {}
};

struct DisplayMovedEvent : Base::Event {
    DisplayMovedEvent() : Base::Event(typeid(DisplayMovedEvent)) {}
};
