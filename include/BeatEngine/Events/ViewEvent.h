#pragma once

#include "BeatEngine/Base/Event.h"

#include <typeindex>

class ViewPopEvent : public Base::Event {
public:
    std::type_index ViewID;
public:
	ViewPopEvent(std::type_index id) : Base::Event(typeid(ViewPopEvent)), ViewID(id) {}
};

class ViewPushEvent : public Base::Event {
public:
	std::type_index ViewID;
public:
	ViewPushEvent(std::type_index id) : Base::Event(typeid(ViewPushEvent)), ViewID(id) {}
};

class ViewSuspendEvent : public Base::Event {
public:
	std::type_index ViewID;
public:
	ViewSuspendEvent(std::type_index id) : Base::Event(typeid(ViewSuspendEvent)), ViewID(id) {}
};

class ViewUnsuspendEvent : public Base::Event {
public:
	std::type_index ViewID;
public:
	ViewUnsuspendEvent(std::type_index id) : Base::Event(typeid(ViewUnsuspendEvent)), ViewID(id) {}
};
