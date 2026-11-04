#pragma once

#include "BeatEngine/Base/Event.h"

#include <typeindex>

struct EventViewPop : Base::Event {
    std::type_index ViewID;

	EventViewPop(std::type_index id) : Base::Event(typeid(EventViewPop)), ViewID(id) {}
};

struct EventViewPush : Base::Event {
	std::type_index ViewID;

	EventViewPush(std::type_index id) : Base::Event(typeid(EventViewPush)), ViewID(id) {}
};

struct EventViewSuspend : Base::Event {
	std::type_index ViewID;

	EventViewSuspend(std::type_index id) : Base::Event(typeid(EventViewSuspend)), ViewID(id) {}
};

struct EventViewUnsuspend : Base::Event {
	std::type_index ViewID;

	EventViewUnsuspend(std::type_index id) : Base::Event(typeid(EventViewUnsuspend)), ViewID(id) {}
};
