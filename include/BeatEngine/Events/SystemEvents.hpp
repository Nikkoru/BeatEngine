#pragma once

#include "BeatEngine/Base/Event.h"

struct DropFileEvent : Base::Event {
    DropFileEvent() : Base::Event(typeid(DropFileEvent)) {}
};

struct DropTextEvent : Base::Event {
    DropTextEvent() : Base::Event(typeid(DropTextEvent)) {}
};

struct DropBeginEvent : Base::Event {
    DropBeginEvent() : Base::Event(typeid(DropBeginEvent)) {}
};

struct DropEndEvent : Base::Event {
    DropEndEvent() : Base::Event(typeid(DropEndEvent)) {}
};

struct NotificationInvokedEvent : Base::Event {
    NotificationInvokedEvent() : Base::Event(typeid(NotificationInvokedEvent)) {}
};
