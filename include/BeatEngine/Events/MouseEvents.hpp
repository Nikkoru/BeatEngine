#pragma once

#include "BeatEngine/Base/Event.h"
#include "BeatEngine/Graphics/Vector2.h"
#include "BeatEngine/Windows/Mouse.hpp"

struct MouseMovedEvent : Base::Event {
    Vector2i Position;

    MouseMovedEvent(Vector2i pos) : Base::Event(typeid(MouseMovedEvent)), Position(pos) {}
};

struct MouseButtonDownEvent : Base::Event {
    Mouse::Button Button;
    Vector2i Position;

    MouseButtonDownEvent(Mouse::Button button, Vector2i pos) : Base::Event(typeid(MouseButtonDownEvent)), Button(button), Position(pos) {}
};

struct MouseButtonUpEvent : Base::Event {
    Mouse::Button Button;
    Vector2i Position;

    MouseButtonUpEvent(Mouse::Button button, Vector2i pos) : Base::Event(typeid(MouseButtonUpEvent)),Button(button), Position(pos) {}
};
