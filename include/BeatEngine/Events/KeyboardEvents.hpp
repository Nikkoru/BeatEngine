#pragma once

#include "BeatEngine/Base/Event.h"

struct KeyDownEvent : Base::Event {
    KeyDownEvent() : Base::Event(typeid(KeyDownEvent)) {}
};

struct KeyUpEvent : Base::Event {
    KeyUpEvent() : Base::Event(typeid(KeyUpEvent)) {}
};

struct KeymapChangedEvent : Base::Event {
    KeymapChangedEvent() : Base::Event(typeid(KeymapChangedEvent)) {}
};
