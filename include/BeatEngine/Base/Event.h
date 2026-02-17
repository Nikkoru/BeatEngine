#pragma once

#include <memory>
#include <type_traits>
#include <typeindex>

namespace Base {
	class Event {
	public:
		Event() : ID(typeid(nullptr)) {}
		Event(std::type_index id) : ID(id) {}
		virtual ~Event() = default;
    public:
        template<typename TEvent>
            requires(std::is_base_of_v<Base::Event, TEvent>)
        bool Is();

        template<typename TEvent>
            requires(std::is_base_of_v<Base::Event, TEvent>)
        std::shared_ptr<TEvent> GetIf();
	public:
		std::type_index ID;
	};
}
