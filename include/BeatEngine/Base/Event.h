#pragma once

#include <typeindex>

namespace Base {
	class Event {
	public:
		Event() : ID(typeid(nullptr)) {}
		Event(std::type_index id) : ID(id) {}
		virtual ~Event() = default;
	public:
		std::type_index ID;
	};
}