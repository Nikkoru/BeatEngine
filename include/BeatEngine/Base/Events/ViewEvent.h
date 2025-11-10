#pragma once

#include "Base/Event.h"

#include <typeindex>

class ViewPopEvent : public Base::Event {
public:
	ViewPopEvent() : Base::Event(typeid(ViewPopEvent)) {}
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