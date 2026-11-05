#pragma once

#include "BeatEngine/Base/Event.h"

struct GamepadAxisMotionEvent : Base::Event {
    GamepadAxisMotionEvent() : Base::Event(typeid(GamepadAxisMotionEvent)) {}
};

struct GamepadButtonDownEvent : Base::Event {
    GamepadButtonDownEvent() : Base::Event(typeid(GamepadButtonDownEvent)) {}
};

struct GamepadButtonUpEvent : Base::Event {
    GamepadButtonUpEvent() : Base::Event(typeid(GamepadButtonUpEvent)) {}
};

struct GamepadSensorUpdateEvent : Base::Event {
    GamepadSensorUpdateEvent() : Base::Event(typeid(GamepadSensorUpdateEvent)) {}
};
