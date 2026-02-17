#pragma once

#include "BeatEngine/Base/Event.h"

#include <typeindex>

class EventViewPop : public Base::Event {
public:
    std::type_index ViewID;
public:
	EventViewPop(std::type_index id) : Base::Event(typeid(EventViewPop)), ViewID(id) {}
};

class EventViewPush : public Base::Event {
public:
	std::type_index ViewID;
public:
	EventViewPush(std::type_index id) : Base::Event(typeid(EventViewPush)), ViewID(id) {}
};

class EventViewSuspend : public Base::Event {
public:
	std::type_index ViewID;
public:
	EventViewSuspend(std::type_index id) : Base::Event(typeid(EventViewSuspend)), ViewID(id) {}
};

class EventViewUnsuspend : public Base::Event {
public:
	std::type_index ViewID;
public:
	EventViewUnsuspend(std::type_index id) : Base::Event(typeid(EventViewUnsuspend)), ViewID(id) {}
};
