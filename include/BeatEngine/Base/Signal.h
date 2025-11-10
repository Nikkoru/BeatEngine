#pragma once

#include <typeindex>

namespace Base {
	class Signal {
	public:
		std::type_index ID = typeid(Signal);
	public:
		Signal() = default;
		Signal(std::type_index sigID) : ID(sigID) {}
		virtual ~Signal() = default;
	};
}